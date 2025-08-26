/**
 * @file   ota.c
 * @brief  简单、健壮的 OTA 升级模块（ESP8266 + 片内 Flash）
 */

#include "OTA.h"
#include "ESP8266.h"
#include "Flash.h"
#include "Serial.h"
#include "Delay.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

/*-------------------- 配置宏 --------------------*/
#define FIRMWARE_BUFFER_SIZE   1024     /* 单次下载块大小 */
#define MAX_RETRIES            3        /* 单块重试次数 */
#define LOG_ENABLE             1
#define LOG_TAG                "[OTA] "

/*-------------------- 调试打印 --------------------*/
#if LOG_ENABLE
  #define LOG_INFO(fmt, ...)  printf(LOG_TAG fmt "\r\n", ##__VA_ARGS__)
#else
  #define LOG_INFO(...)
#endif

/*-------------------- 内部类型 --------------------*/
typedef enum {
    OTA_IDLE = 0,
    OTA_CHECKING_VERSION,
    OTA_DOWNLOADING_FIRMWARE,
    OTA_UPDATE_SUCCESS,
    OTA_UPDATE_FAILED
} OTA_State_TypeDef;

/*-------------------- 内部变量 --------------------*/
static OTA_State_TypeDef ota_state = OTA_IDLE;

static const char *version_url_g;
static const char *firmware_url_g;
static uint32_t    flash_addr_g;
static uint8_t firmware_buffer[FIRMWARE_BUFFER_SIZE];

/* 下载上下文 */
typedef struct {
    uint32_t total;
    uint32_t offset;
    uint16_t crc_remote;
    uint16_t crc_calc;
    uint8_t  retry_cnt;
} DownloadCtx_t;

static DownloadCtx_t dl;

/*-------------------- CRC16 --------------------*/
static uint16_t crc16_update(uint16_t crc, const uint8_t *data, uint32_t len)
{
    while (len--) {
        crc ^= *data++;
        for (uint8_t i = 0; i < 8; i++)
            crc = (crc & 1) ? (crc >> 1) ^ 0xA001 : (crc >> 1);
    }
    return crc;
}

/*-------------------- 状态迁移 --------------------*/
static inline void ota_change_state(OTA_State_TypeDef new_state)
{
    ota_state = new_state;
}

/*-------------------- 1. 检查版本 --------------------*/
static bool ota_check_new_version(void)
{
    char buf[64];
    int  len = ESP8266_HTTP_GET_SYNC(version_url_g,
                                     (uint8_t *)buf,
                                     sizeof(buf) - 1,
                                     NULL);
    if (len <= 0) return false;

    buf[len] = '\0';
    /* 去掉可能的回车换行 */
    char *p = strpbrk(buf, "\r\n");
    if (p) *p = '\0';

    if (strcmp(buf, FIRMWARE_VERSION) <= 0) {
        LOG_INFO("Already latest (%s).", FIRMWARE_VERSION);
        return false;
    }

    LOG_INFO("New version found: %s", buf);
    return true;
}

/*-------------------- 2. 获取固件头信息 --------------------*/
static bool ota_fetch_header(void)
{
    char head[256];
    int  len = ESP8266_HTTP_GET_SYNC(firmware_url_g,
                                     (uint8_t *)head,
                                     sizeof(head) - 1,
                                     "Range: bytes=0-0");
    if (len <= 0) return false;

    head[len] = '\0';

    /* 解析 Content-Length 和 X-Firmware-CRC */
    if (sscanf(head, "%*[^C]Content-Length: %lu", &dl.total) != 1 ||
        sscanf(head, "%*[^X]X-Firmware-CRC: %hu", &dl.crc_remote) != 1) {
        return false;
    }

    LOG_INFO("Firmware size=%lu, CRC=0x%04X", dl.total, dl.crc_remote);
    return true;
}

/*-------------------- 3. 下载单块 --------------------*/
static bool ota_download_chunk(void)
{
    uint32_t chunk = (dl.total - dl.offset) > FIRMWARE_BUFFER_SIZE ?
                     FIRMWARE_BUFFER_SIZE : (dl.total - dl.offset);

    if (chunk == 0) return false;  /* 已下完 */

    char range[48];
    snprintf(range, sizeof(range), "Range: bytes=%lu-%lu",
             dl.offset, dl.offset + chunk - 1);

    int len = ESP8266_HTTP_GET_SYNC(firmware_url_g,
                                    firmware_buffer,
                                    chunk,
                                    range);
    if (len != (int)chunk) {
        if (++dl.retry_cnt >= MAX_RETRIES) return false;
        LOG_INFO("Chunk retry %u/%u", dl.retry_cnt, MAX_RETRIES);
        return true;   /* 下一循环继续 */
    }

    /* 写入 Flash */
    Flash_Write(flash_addr_g + dl.offset, firmware_buffer, chunk);

    /* 更新 CRC/偏移/重试计数 */
    dl.crc_calc = crc16_update(dl.crc_calc, firmware_buffer, chunk);
    dl.offset  += chunk;
    dl.retry_cnt = 0;

    LOG_INFO("Progress: %lu/%lu", dl.offset, dl.total);
    return true;
}

/*-------------------- 4. 下载完成校验 --------------------*/
static bool ota_verify_crc(void)
{
    if (dl.crc_calc != dl.crc_remote) {
        LOG_INFO("CRC mismatch: calc=0x%04X, remote=0x%04X",
                 dl.crc_calc, dl.crc_remote);
        return false;
    }
    LOG_INFO("CRC OK");
    return true;
}

/*-------------------- 5. 失败处理 --------------------*/
static void ota_fail(void)
{
    LOG_INFO("Update failed!");
    ota_change_state(OTA_IDLE);
}

/*-------------------- 6. 成功处理 --------------------*/
static void ota_success(void)
{
    LOG_INFO("Update success, rebooting...");
    OTA_SetUpdateFlag();
    NVIC_SystemReset();
}

/*-------------------- 对外 API --------------------*/
void OTA_SetUpdateFlag(void) { 	
     Flash_ProgramWord(UPFLAGADDR, UPDATE_FLAG);
}

void OTA_Init(const char *version_url,
              const char *firmware_url,
              uint32_t   target_address)
{
    version_url_g  = version_url;
    firmware_url_g = firmware_url;
    flash_addr_g   = target_address;
    ota_state      = OTA_IDLE;
}

void OTA_Task(void)
{
    switch (ota_state) {
    case OTA_IDLE:
        ota_change_state(OTA_CHECKING_VERSION);
        break;

    case OTA_CHECKING_VERSION:
        if (!ota_check_new_version()) {
            ota_change_state(OTA_IDLE);
            break;
        }
        if (!ota_fetch_header()) {
            ota_fail();
            break;
        }

        /* 初始化下载上下文 */
        memset(&dl, 0, sizeof(dl));
        Flash_ErasePage(flash_addr_g);   /* 擦除目标区域 */
        ota_change_state(OTA_DOWNLOADING_FIRMWARE);
        break;

    case OTA_DOWNLOADING_FIRMWARE:
        if (dl.offset >= dl.total) {
            if (ota_verify_crc())
                ota_change_state(OTA_UPDATE_SUCCESS);
            else
                ota_fail();
            break;
        }

        if (!ota_download_chunk())
            ota_fail();
        break;

    case OTA_UPDATE_SUCCESS:
        ota_success();
        break;

    case OTA_UPDATE_FAILED:
    default:
        ota_fail();
        break;
    }
}
