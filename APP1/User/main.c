#include "stm32f10x.h"                  // Device header
#include "globals.h"
#include "OTA.h"
#include "system_stm32f10x.h"
#include "ESP8266.h"
#include "Serial.h"
#include "OLED.h"
#include "LED.h"

/**
*
以下是一个基于 STM32 和 ESP8266 的 Wi-Fi 远程 OTA 升级示例，使用标准库代码实现。这个示例包括 Bootloader 和应用程序的实现，支持远程固件下载和更新。
项目结构
Bootloader：负责检查升级标志位，执行固件更新，并跳转到应用程序。
应用程序：负责连接到 Wi-Fi 网络，检查服务器上的固件版本，下载新固件，并设置升级标志位。
硬件需求
STM32F103 系列
ESP8266 Wi-Fi 模块
软件需求
STM32 标准库
ESP8266 AT 指令集
项目配置
核心思想非常简单，我们把整个FLASH分成4部分，bootloader，APP1，APP2，FLAG。
Flash 分区：总大小64K
Bootloader：0x08000000 - 0x08002000（8KB）
APP1：0x08002000 - 0x08008C00（27KB）
APP2：0x08008C00 - 0x0800F800（27KB）
FLAG：0x0800F800 - 0x08010000（2KB）


STM32F103C8T6一共64K，FLASH一共64页，每页1K，bootloader分8K，FLAG分2K，APP1与APP2各27K，也就是我们的应用程序，编译出来不能超过27K的大小。

bootloader是一个独立的固件，在启动后负责检查FLAG区域的升级标志位以确定是否有新的固件需要升级，如果有就跳到升级部分，将APP2的部分copy到APP1，
copy完之后再清空FLAG区域的升级标志，然后重启，就会运行新的APP1也就是升级后的程序了。

这里如果升级到一半断电了，下次重启还是会重新copy一次APP2到APP1，因为升级标志位还没被清空，所以这里不用担心升级的时候断电导致无法重启。

bootloader在启动的时候，如果FLAG区域的升级标志位没有置位，那就直接启动APP1就可以了。
*/

#define USE_OTA 1

// 应用程序实现
int main(void) {
    SystemInit();
#if LOG_ENABLE
  Debug_Serial_Init();
#endif
    WIFI_Init();
    OLED_Init();
    LED_Init();

    OTA_Init(SERVER_VERSION_URL, SERVER_FIRMWARE_URL, APP2ADDR);
    while (1) {
        if(WIFI_Task() != 0)
        {
            continue;
        }
        OTA_Task();
        
        // 应用程序逻辑
        LED1_ON();
        OLED_ShowString(1,1,FIRMWARE_VERSION);
    }
}

