#ifndef __ESP8266_H
#define __ESP8266_H

#include "Serial.h"

#define ESP8266_RESET_GPIO_APBX RCC_APB2PeriphClockCmd
#define ESP8266_RESET_GPIO_CLK  RCC_APB2Periph_GPIOA
#define ESP8266_RESET_GPIO_PORT GPIOA
#define ESP8266_RESET_GPIO_PIN	GPIO_Pin_12 //硬件复位引脚
#define RESET_IO(x)    GPIO_WriteBit(GPIOA, ESP8266_RESET_GPIO_PIN, (BitAction)x)  //PA4控制WiFi的复位

#define WIFI_SSID "test" //wifi名称
#define WIFI_PASSWORD "12345678" //wifi密码

#define WIFI_RX_BUFFER_SIZE 1024 //wifi接收数据缓存
#define CMD_BUFFER_SIZE 256

extern volatile char WIFI_CONNECT;//服务器连接模式，1-表示已连接，0表示未连接
extern volatile char PING_MODE;//ping心跳包发送模式，1表示开启30s发送模式，0表示未开启发送或开启2s快速发送模式。
extern volatile char pingFlag;       //ping报文状态       0：正常状态，等待计时时间到，发送Ping报文
                         //ping报文状态       1：Ping报文已发送，当收到 服务器回复报文的后 将1置为0

void WIFI_Init(void);
void WIFI_SendString(char *String);
//void WIFI_Run(void);

// ESP8266相关函数
// 查询当前WIFI连接状态 返回： +CWJAP_DEF: 且 OK
void ESP8266_CheckWiFiStatus(void);
uint8_t ESP8266_HTTP_GET_SYNC(const char *url, uint8_t *response, uint32_t response_size, const char *extra_headers);
uint8_t WIFI_Task(void);
#endif
