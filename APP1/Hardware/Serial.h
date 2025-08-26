#ifndef __SERIAL_H
#define __SERIAL_H

#include "stm32f10x.h"                  // Device header

#include <stdio.h>

// 配置蓝牙模块HC_05
//使用宏定义容易修改串口
#define HC_05_GPIO_PORT    	GPIOA		                /* GPIO端口 */
#define HC_05_GPIO_APBX     RCC_APB2PeriphClockCmd
#define HC_05_GPIO_CLK 	    RCC_APB2Periph_GPIOA		/* GPIO端口时钟 */

#define HC_05_TX_GPIO_PIN	GPIO_Pin_9
#define HC_05_RX_GPIO_PIN	GPIO_Pin_10	

#define HC_05_USARTX        USART1
#define HC_05_APBX          RCC_APB2PeriphClockCmd
#define HC_05_CLK           RCC_APB2Periph_USART1
#define HC_05_IRQn          USART1_IRQn
#define HC_05_IRQHandler    USART1_IRQHandler

// 配置WIFI模块ESP8266
#define ESP8266_USART_BAUDRATE 115200
#define ESP8266_GPIO_PORT   GPIOA		                /* GPIO端口 */
#define ESP8266_GPIO_APBX   RCC_APB2PeriphClockCmd
#define ESP8266_GPIO_CLK 	RCC_APB2Periph_GPIOA		/* GPIO端口时钟 */

#define ESP8266_TX_GPIO_PIN	GPIO_Pin_2
#define ESP8266_RX_GPIO_PIN	GPIO_Pin_3
//#define ESP8266_RESET_GPIO_PIN	GPIO_Pin_12

#define ESP8266_USARTX      USART2
#define ESP8266_APBX        RCC_APB1PeriphClockCmd
#define ESP8266_CLK         RCC_APB1Periph_USART2
#define ESP8266_IRQn        USART2_IRQn
#define ESP8266_IRQHandler  USART2_IRQHandler

// USART2 DMA配置
#define ESP8266_DMA_APBX                RCC_AHBPeriphClockCmd
#define ESP8266_DMA_CLK                 RCC_AHBPeriph_DMA1
#define USART2_TX_DMA_CHANNEL           DMA1_Channel7
#define USART2_RX_DMA_CHANNEL           DMA1_Channel6
#define USART2_TX_DMA_FLAG_TC           DMA1_FLAG_TC7
#define USART2_RX_DMA_FLAG_TC           DMA1_FLAG_TC6
#define USART2_TX_DMA_FLAG_GL           DMA1_FLAG_GL7
#define USART2_RX_DMA_FLAG_GL           DMA1_FLAG_GL6
// DMA缓冲区大小
#define USART2_DMA_RX_BUFFER_SIZE       1024
#define USART2_DMA_TX_BUFFER_SIZE       1024


// 配置天问语音识别模块ASRPRO
#define ASRPRO_GPIO_PORT    GPIOB		                /* GPIO端口 */
#define ASRPRO_GPIO_APBX    RCC_APB2PeriphClockCmd
#define ASRPRO_GPIO_CLK 	RCC_APB2Periph_GPIOB		/* GPIO端口时钟 */

#define ASRPRO_TX_GPIO_PIN	GPIO_Pin_10
#define ASRPRO_RX_GPIO_PIN	GPIO_Pin_11	

#define ASRPRO_USARTX       USART3
#define ASRPRO_APBX         RCC_APB1PeriphClockCmd
#define ASRPRO_CLK          RCC_APB1Periph_USART3
#define ASRPRO_IRQn         USART3_IRQn
#define ASRPRO_IRQHandler   USART3_IRQHandler

// 调试（fputc重写 使用 printf输出日志），切换使用的USART 
#define USE_USARTX HC_05_USARTX

void Serial_DMA_Init_ESP8266(uint8_t * rxBuffer);
void USART2_DMA_SendData(uint8_t *pData, uint16_t Size);

void Serial_SendByte(USART_TypeDef * pUSARTx, uint8_t Byte);
void Serial_SendArray(USART_TypeDef * pUSARTx, uint8_t *Array, uint16_t Length);//uint8_t = char 
void Serial_SendString(USART_TypeDef * pUSARTx, char *String);
void Serial_SendNumber(USART_TypeDef * pUSARTx, uint32_t Number, uint8_t Length);
void Serial_Printf(USART_TypeDef * pUSARTx, char *format, ...);

#endif
