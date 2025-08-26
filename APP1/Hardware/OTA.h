#ifndef __OTA_H
#define __OTA_H

#include <stdint.h>

// OTA版本固件获取地址
#define SERVER_FIRMWARE_URL "http://yourserver.com/firmware.php"
#define SERVER_VERSION_URL "http://localhost/version.txt"

//OTA升级标志位
#define UPDATE_FLAG 0x55555555

// stm FLASH初始地址
#define ADDR_FLASH_BASE 0x08000000
#define APP2  0x8C00
#define UPFLAG1 0xF800

#define APP2ADDR (ADDR_FLASH_BASE + APP2)   // 0x08008C00
#define UPFLAGADDR   (ADDR_FLASH_BASE + UPFLAG1)   // 0x0800F800
//版本号
#define FIRMWARE_VERSION "1.0.0"

void OTA_SetUpdateFlag(void);
void OTA_Init(const char *version_url,const char *firmware_url,uint32_t target_address);
void OTA_Task(void);

#endif
