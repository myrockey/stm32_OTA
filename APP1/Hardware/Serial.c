#include "Serial.h"
#include <stdio.h>
#include <stdarg.h>

// //配置调试
void Debug_Serial_Init(void) {
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;

  // 使能GPIOA和USART1时钟
  DEBUG_GPIO_APBX(DEBUG_GPIO_CLK, ENABLE);
  DEBUG_APBX(DEBUG_CLK, ENABLE);

  // 配置USART2的TX和RX引脚
  GPIO_InitStructure.GPIO_Pin = DEBUG_TX_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(DEBUG_GPIO_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = DEBUG_RX_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(DEBUG_GPIO_PORT, &GPIO_InitStructure);

  // 配置USART2
  USART_InitStructure.USART_BaudRate = DEBUG_USART_BAUDRATE;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(DEBUG_USARTX, &USART_InitStructure);

  USART_Cmd(DEBUG_USARTX, ENABLE);
}


// DMA配置函数
void DMA_Init_ESP8266(uint8_t * rxBuffer)
{
  DMA_InitTypeDef DMA_InitStructure;

  // 开启DMA1时钟
  ESP8266_DMA_APBX(ESP8266_DMA_CLK, ENABLE);

  // 配置DMA发送通道
  DMA_DeInit(USART2_TX_DMA_CHANNEL);
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ESP8266_USARTX->DR;
  DMA_InitStructure.DMA_MemoryBaseAddr = 0;  // 后续发送时设置
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_InitStructure.DMA_BufferSize = 0;      // 后续发送时设置
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(USART2_TX_DMA_CHANNEL, &DMA_InitStructure);

  // 配置DMA接收通道
  DMA_DeInit(USART2_RX_DMA_CHANNEL);
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)rxBuffer; // 设置DMA接收内存地址
  DMA_InitStructure.DMA_BufferSize = USART2_DMA_RX_BUFFER_SIZE; // 设置DMA接收缓冲区大小
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  // 普通模式
  DMA_Init(USART2_RX_DMA_CHANNEL, &DMA_InitStructure);

  // 使能USART2的DMA发送和接收请求
  USART_DMACmd(ESP8266_USARTX, USART_DMAReq_Tx, ENABLE);
  USART_DMACmd(ESP8266_USARTX, USART_DMAReq_Rx, ENABLE);

  DMA_Cmd(USART2_RX_DMA_CHANNEL, ENABLE); // 使能DMA接收通道

  //DMA_Cmd(USART2_TX_DMA_CHANNEL, ENABLE);
}

// 使用DMA发送数据
void USART2_DMA_SendData(uint8_t *pData, uint16_t Size)
{   
  // 清除标志
  DMA_ClearFlag(USART2_TX_DMA_FLAG_GL);
  DMA_Cmd(USART2_TX_DMA_CHANNEL, DISABLE);
  USART2_TX_DMA_CHANNEL->CMAR = (uint32_t)pData;
  USART2_TX_DMA_CHANNEL->CNDTR = Size;//重新写入需要传输数据的数量
  //DMA_SetCurrDataCounter(USART2_TX_DMA_CHANNEL,Size);//重新写入需要传输数据的数量
  // 启动传输
  DMA_Cmd(USART2_TX_DMA_CHANNEL, ENABLE);

  while(DMA_GetFlagStatus(USART2_TX_DMA_FLAG_TC) == RESET);
}

// 使用DMA接收数据
void USART2_DMA_ReceiveData(uint8_t *pData, uint16_t Size)
{
  // 清除标志
  DMA_ClearFlag(USART2_RX_DMA_FLAG_GL);

  // 设置数据地址和长度
  USART2_RX_DMA_CHANNEL->CMAR = (uint32_t)pData;
  USART2_RX_DMA_CHANNEL->CNDTR = Size;

  // 启动接收
  DMA_Cmd(USART2_RX_DMA_CHANNEL, ENABLE);
}

// //配置串口+DMA+中断
void Serial_DMA_Init_ESP8266(uint8_t * rxBuffer) {
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  // 使能GPIOA和USART2时钟
  ESP8266_GPIO_APBX(ESP8266_GPIO_CLK, ENABLE);
  ESP8266_APBX(ESP8266_CLK, ENABLE);

  // 配置USART2的TX和RX引脚
  GPIO_InitStructure.GPIO_Pin = ESP8266_TX_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(ESP8266_GPIO_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = ESP8266_RX_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(ESP8266_GPIO_PORT, &GPIO_InitStructure);

  // 配置USART2
  USART_InitStructure.USART_BaudRate = ESP8266_USART_BAUDRATE;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(ESP8266_USARTX, &USART_InitStructure);

  USART_Cmd(ESP8266_USARTX, ENABLE);

  // 初始化DMA
  DMA_Init_ESP8266(rxBuffer);

  // 配置NVIC
  NVIC_InitStructure.NVIC_IRQChannel = ESP8266_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  // 使能空闲中断，用于DMA接收
  USART_ITConfig(ESP8266_USARTX, USART_IT_IDLE, ENABLE); 
  //USART_ITConfig(ESP8266_USARTX, USART_IT_TC, ENABLE);//发送数据完成触发	
}

/**
  * 函    数：串口发送一个字节
  * 参    数：Byte 要发送的一个字节
  * 返 回 值：无
  */
void Serial_SendByte(USART_TypeDef * pUSARTx, uint8_t Byte)
{
	USART_SendData(pUSARTx, Byte);		//将字节数据写入数据寄存器，写入后USART自动生成时序波形
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);	//等待发送完成
	/*下次写入数据寄存器会自动清除发送完成标志位，故此循环后，无需清除标志位*/
}

/**
  * 函    数：串口发送一个数组
  * 参    数：Array 要发送数组的首地址
  * 参    数：Length 要发送数组的长度
  * 返 回 值：无
  */
void Serial_SendArray(USART_TypeDef * pUSARTx, uint8_t *Array, uint16_t Length)
{
	uint16_t i;
	for (i = 0; i < Length; i ++)		//遍历数组
	{
		Serial_SendByte(pUSARTx, Array[i]);		//依次调用Serial_SendByte发送每个字节数据
	}
}

/**
  * 函    数：串口发送一个字符串
  * 参    数：String 要发送字符串的首地址
  * 返 回 值：无
  */
void Serial_SendString(USART_TypeDef * pUSARTx, char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i ++)//遍历字符数组（字符串），遇到字符串结束标志位后停止
	{
		Serial_SendByte(pUSARTx, String[i]);		//依次调用Serial_SendByte发送每个字节数据
	}
}

/**
  * 函    数：次方函数（内部使用）
  * 返 回 值：返回值等于X的Y次方
  */
uint32_t Serial_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;	//设置结果初值为1
	while (Y --)			//执行Y次
	{
		Result *= X;		//将X累乘到结果
	}
	return Result;
}

/**
  * 函    数：串口发送数字
  * 参    数：Number 要发送的数字，范围：0~4294967295
  * 参    数：Length 要发送数字的长度，范围：0~10
  * 返 回 值：无
  */
void Serial_SendNumber(USART_TypeDef * pUSARTx, uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i ++)		//根据数字长度遍历数字的每一位
	{
		Serial_SendByte(pUSARTx, Number / Serial_Pow(10, Length - i - 1) % 10 + '0');	//依次调用Serial_SendByte发送每位数字
	}
}

/**
  * 函    数：使用printf需要重定向的底层函数
  * 参    数：保持原始格式即可，无需变动
  * 返 回 值：保持原始格式即可，无需变动
  */
int fputc(int ch, FILE *f)
{
	Serial_SendByte(DEBUG_USARTX, ch);			//将printf的底层重定向到自己的发送字节函数
	return ch;
}

//重定向C库函数scanf到串口,重写向后可使用scanf、getchar等函数
int fgetc(FILE *f)
{
    /* 等待串口输入数据 */
    while(USART_GetFlagStatus(DEBUG_USARTX,USART_FLAG_TXE)==RESET);
    return (int)USART_ReceiveData(DEBUG_USARTX);
}

/**
  * 函    数：自己封装的prinf函数
  * 参    数：format 格式化字符串
  * 参    数：... 可变的参数列表
  * 返 回 值：无
  */
void Serial_Printf(USART_TypeDef * pUSARTx, char *format, ...)
{
	char String[100];				//定义字符数组
	va_list arg;					//定义可变参数列表数据类型的变量arg
	va_start(arg, format);			//从format开始，接收参数列表到arg变量
	vsprintf(String, format, arg);	//使用vsprintf打印格式化字符串和参数列表到字符数组中
	va_end(arg);					//结束变量arg
	Serial_SendString(pUSARTx, String);		//串口发送字符数组（字符串）
}
