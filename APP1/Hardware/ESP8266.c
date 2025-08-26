#include "ESP8266.h"
#include "stm32f10x.h"                  // Device header
#include "Timer.h"
#include "RingBuff.h"
#include "Delay.h"
#include "Serial.h"
#include "globals.h"

volatile char WIFI_CONNECT = 0;//服务器连接模式，1-表示已连接，0表示未连接
volatile char PING_MODE = 0;//ping心跳包发送模式，1表示开启30s发送模式，0表示未开启发送或开启2s快速发送模式。
volatile char pingFlag = 0;       //ping报文状态       0：正常状态，等待计时时间到，发送Ping报文
                         //ping报文状态       1：Ping报文已发送，当收到 服务器回复报文的后 将1置为0
uint8_t WIFI_Check_Flag = 0;// WIFI是否断连检测标志
volatile uint32_t g_rx_esp8266_cnt = 0;// 当前接收的字节数

uint8_t g_rx_dma_buf[USART2_DMA_RX_BUFFER_SIZE] = {0};//DMA接收数据缓冲区
volatile uint32_t g_rx_dma_cnt = 0;// 当前接收的字节数

/*函数名：初始化WiFi的复位IO                       */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void WIFI_Reset_IO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;                    //定义一个设置IO端口参数的结构体
	ESP8266_RESET_GPIO_APBX(ESP8266_RESET_GPIO_CLK , ENABLE); //使能PA端口时钟
	
	GPIO_InitStructure.GPIO_Pin = ESP8266_RESET_GPIO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;       //速率50Mhz
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   	    //推免输出方式
	GPIO_Init(ESP8266_RESET_GPIO_PORT, &GPIO_InitStructure);  	    
	RESET_IO(1);                                            //复位IO拉高电平
}

void WIFI_Init(void)
{
	//Serial_Init_ESP8266(g_rx_dma_buf);
	Serial_DMA_Init_ESP8266(g_rx_dma_buf);
	WIFI_Reset_IO_Init();
	RingBuff_Init(&encoeanBuff);//环形缓冲区初始化
}

//发送数据
void WIFI_SendString(char *String)
{
	//Serial_SendString(ESP8266_USARTX, String);
	USART2_DMA_SendData((uint8_t*)String, strlen(String));//使用DMA方式发送
}

//清空接收缓存区
void ESP8266_Buf_Clear(void)
{
	//WiFi接收数据量变量清零                        
	//清空WiFi接收缓冲区 	
	g_rx_esp8266_cnt = 0;
	RingBuff_Init(&encoeanBuff);
}

/**
  * @brief  发送命令
  * @param  cmd 命令字符串
  * @param  res 响应关键词字符串
  * @param  timeOut 超时时间（100ms的倍数）
  * @retval 0-表示响应成功，1-表示响应失败
  */
char ESP8266_SendCommand(char *cmd, char *res, uint8_t timeout)
{
	ESP8266_Buf_Clear();
	WIFI_SendString(cmd);

	int len;
	while(timeout--)
	{
		Delay_ms(100);
		len = RingBuff_GetLen(&encoeanBuff);
		if (len == 0)
        {
            continue;
        }
		uint8_t received_str[len+1];
		RingBuff_ReadNByte(&encoeanBuff,received_str,len);
		received_str[len] = '\0';
		if(strstr((const char *)received_str, res) != NULL)		//如果检索到关键词
		{
			return 0;
		}
	}
	return 1;
}

/*-------------------------------------------------*/
/*函数名：WiFi复位                                 */
/*参  数：timeout：超时时间（100ms的倍数）         */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
char ESP8266_WiFi_Reset(int timeout)
{
	RESET_IO(0);                                    //复位IO拉低电平
	Delay_ms(500);                                  //延时500ms
	RESET_IO(1);                                   	//复位IO拉高电平	
	int len;
	while(timeout--)								//等待超时时间到0 
	{                              		  
		Delay_ms(100);                              //延时100ms
		len = RingBuff_GetLen(&encoeanBuff);
		if (len == 0)
        {
            continue;
        }
		uint8_t received_str[len+1];
		RingBuff_ReadNByte(&encoeanBuff,received_str,len);
		received_str[len] = '\0';
		if(strstr((const char*)received_str, "ready") != NULL)            //如果接收到ready表示复位成功
		{
			return 0;		         				   	//反之，表示正确，说明收到ready，通过break主动跳出while
		}
		LOG_INFO("reset timeout:%d", timeout);                     //串口输出现在的超时时间
	}
	return 1;                      //如果timeout<=0，说明超时时间到了，也没能收到ready，返回1
}

/*-------------------------------------------------*/
/*函数名：WiFi加入路由器指令                       */
/*参  数：timeout：超时时间（1s的倍数）            */
/*返回值：0：正确   其他：错误                     */
/*-------------------------------------------------*/
char ESP8266_WiFi_JoinAP(int timeout)
{		
	ESP8266_Buf_Clear();
	
	char cmd_buffer[CMD_BUFFER_SIZE];
	// 连接到 WiFi
	snprintf(cmd_buffer, sizeof(cmd_buffer), "AT+CWJAP=\"%s\",\"%s\"\r\n", WIFI_SSID, WIFI_PASSWORD);
	WIFI_SendString(cmd_buffer);
	int len;
	while(timeout--)									   //等待超时时间到0
	{                                   
		Delay_ms(1000);                             	   //延时1s
		len = RingBuff_GetLen(&encoeanBuff);
		if (len == 0)
        {
            continue;
        }
		uint8_t received_str[len+1];
		RingBuff_ReadNByte(&encoeanBuff,received_str,len);
		received_str[len] = '\0';
		if(strstr((const char*)received_str, "WIFI GOT IP\r\n\r\nOK") != NULL)   //如果接收到WIFI GOT IP表示成功
		{
			return 0;		
		}
		LOG_INFO("joinAp timeout:%d \r\n", timeout);                            //串口输出现在的超时时间
	}
	return 1;                              //如果timeout<=0，说明超时时间到了，也没能收到WIFI GOT IP，返回1                                              //正确，返回0
}

//连接到OTA服务器
char ESP8266_WIFI_Connect(void) {	
	LOG_INFO("准备设置STA模式\r\n");                
	if(ESP8266_SendCommand("AT+CWMODE=1\r\n","OK",50))			  //设置STA模式，100ms超时单位，总计5s超时时间
	{             
		LOG_INFO("设置STA模式失败，准备重启\r\n");  //返回非0值，进入if
		return 1;                                 //返回2
	}
	LOG_INFO("设置STA模式成功\r\n");     

	LOG_INFO("准备复位模块\r\n");//设置模式后，需重启才能生效                   
	if(ESP8266_WiFi_Reset(50))							  //复位，100ms超时单位，总计5s超时时间
	//if(ESP8266_SendCommand("AT+RST\r\n","OK",100))							  //复位，100ms超时单位，总计5s超时时间
	{                             
		LOG_INFO("复位失败，准备重启\r\n");	      //返回非0值，进入if
		return 2;                                 //返回1
	} 
	LOG_INFO("复位成功\r\n"); 
	
	LOG_INFO("准备取消自动连接\r\n");            	  
	if(ESP8266_SendCommand("AT+CWAUTOCONN=0\r\n","OK",50))		  //取消自动连接，100ms超时单位，总计5s超时时间
	{       
		LOG_INFO("取消自动连接失败，准备重启\r\n"); //返回非0值，进入if
		return 3;                                 //返回3
	}
	LOG_INFO("取消自动连接成功\r\n");         
			
	LOG_INFO("准备连接路由器\r\n");                 	
	if(ESP8266_WiFi_JoinAP(30))							  //连接路由器,1s超时单位，总计30s超时时间
	{                          
		LOG_INFO("连接路由器失败，准备重启\r\n");   //返回非0值，进入if
		return 4;                                 //返回4	
	}
	LOG_INFO("连接路由器成功\r\n");   

	return 0;
 }

// 查询当前WIFI连接状态 返回： +CWJAP: 且 OK
void ESP8266_CheckWiFiStatus(void)
{
	WIFI_Check_Flag = 1;
	WIFI_SendString("AT+CWJAP?\r\n");
	//WIFI_SendString("AT+CIFSR\r\n");//连接成功，才能查到ip，所以也可以判断是否连接状态，返回数据：
	/*
	+CIFSR:STAIP,"192.168.14.220"
	+CIFSR:STAMAC,"ec:fa:bc:97:02:a1"

	OK
	*/
	//Delay_ms(2000);
}

/* USART2中断函数 */
void ESP8266_IRQHandler(void)
{
    // 处理空闲中断
     if(USART_GetITStatus(ESP8266_USARTX, USART_IT_IDLE) != RESET)
    {
        // 读取SR和DR寄存器以清除IDLE标志
        ESP8266_USARTX->SR;
        ESP8266_USARTX->DR;
        
        // 停止DMA传输
        DMA_Cmd(USART2_RX_DMA_CHANNEL, DISABLE);
        
        // 获取接收到的数据长度
        g_rx_dma_cnt = USART2_DMA_RX_BUFFER_SIZE - DMA_GetCurrDataCounter(USART2_RX_DMA_CHANNEL);
        
        if(g_rx_dma_cnt > 0)
        {
			// 已连接服务器时的数据处理
			RingBuff_WriteNByte(&encoeanBuff, g_rx_dma_buf, g_rx_dma_cnt);
			
			// 重置定时器3计数器（ping包计时器）
			//TIM_SetCounter(WIFI_TIM, 0);
        }
        
        // 重新设置DMA传输数量并启动DMA
        USART2_RX_DMA_CHANNEL->CNDTR = USART2_DMA_RX_BUFFER_SIZE;
        DMA_Cmd(USART2_RX_DMA_CHANNEL, ENABLE);
    }
}

/*---------------------------------------------------------------*/
/*函数名：void TIM3_IRQHandler(void)				      			 */
/*功  能：定时器3中断处理函数									 */
/*		  1.控制ping心跳包的发送									 */
/*参  数：无                                       				 */
/*返回值：无                                     				 */
/*其  他：多次快速发送（2s，5次）没有反应，wifi任务由挂起态->就绪态*/
/*---------------------------------------------------------------*/
void WIFI_TIM_IRQHandler(void)
{
	if(TIM_GetITStatus(WIFI_TIM, TIM_IT_Update) != RESET)//如果TIM_IT_Update置位，表示TIM3溢出中断，进入if	
	{  
		LOG_INFO("pingFlag=%d\r\n",pingFlag);
		switch(pingFlag) 					//判断pingFlag的状态
		{                               
			case 0:							//如果pingFlag等于0，表示正常状态，发送Ping报文  
					ESP8266_CheckWiFiStatus(); 		//添加Ping报文到发送缓冲区  
					break;
			case 1:							//如果pingFlag等于1，说明上一次发送到的ping报文，没有收到服务器回复，所以1没有被清除为0，可能是连接异常，我们要启动快速ping模式
					TIM_WIFI_ENABLE_2S(); 	//我们将定时器6设置为2s定时,快速发送Ping报文
					PING_MODE = 0;//关闭发送PING包的定时器3，设置事件标志位
					ESP8266_CheckWiFiStatus();			//添加Ping报文到发送缓冲区  
					break;
			case 2:							//如果pingFlag等于2，说明还没有收到服务器回复
			case 3:				            //如果pingFlag等于3，说明还没有收到服务器回复
			case 4:				            //如果pingFlag等于4，说明还没有收到服务器回复	
					ESP8266_CheckWiFiStatus();  		//添加Ping报文到发送缓冲区 
					break;
			case 5:							//如果pingFlag等于5，说明我们发送了多次ping，均无回复，应该是连接有问题，我们重启连接
					WIFI_CONNECT = 0;       //连接状态置0，表示断开，没连上服务器
					TIM_Cmd(WIFI_TIM, DISABLE); //关TIM3 				
					PING_MODE = 0;			//关闭发送PING包的定时器3，清除事件标志位
					break;			
		}
		pingFlag++;           		   		//pingFlag自增1，表示又发送了一次ping，期待服务器的回复
		TIM_ClearITPendingBit(WIFI_TIM, TIM_IT_Update); //清除TIM3溢出中断标志 	
	}
}

// 断开连接
void ESP8266_Disconnect(void) {
	WIFI_SendString("AT+CIPCLOSE\r\n");
}

// 发送 HTTP GET 请求并接收响应
uint8_t ESP8266_HTTP_GET_SYNC(const char *url, uint8_t *response, uint32_t response_size, const char *extra_headers) 
{
    char cmd[200];
    char host[50];
    char path[100];

    // 提取主机名和路径
    char *host_start = strstr(url, "://") + 3;
    char *path_start = strchr(host_start, '/');
    if (path_start == NULL) {
        return 0; // URL 格式错误
    }
    *path_start = '\0';
    path_start++;
    strcpy(host, host_start);
    strcpy(path, path_start);

    // 发送连接命令
    sprintf(cmd, "AT+CIPSTART=\"TCP\",\"%s\",80", host);
    if (ESP8266_SendCommand(cmd, "OK", 30) != 0) {
        LOG_INFO("Failed to start connection\n");
        return 1; // 连接失败
    }

    // 构造 HTTP GET 请求
    char http_request[512];
    if (extra_headers == NULL || strlen(extra_headers) == 0) {
        sprintf(http_request, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", path, host);
    } else {
        sprintf(http_request, "GET %s HTTP/1.1\r\nHost: %s\r\n%s\r\n", path, host, extra_headers);
    }

    uint32_t request_length = strlen(http_request);
    sprintf(cmd, "AT+CIPSEND=%d", request_length);
    if (ESP8266_SendCommand(cmd, ">", 30) != 0) {
        LOG_INFO("Failed to send data: %s\n", http_request);
        ESP8266_Disconnect();
        return 2; // 发送失败
    }

    // 发送 HTTP 请求
	WIFI_SendString(http_request);

	// 等待返回
	uint32_t tick = GetTick();
	int len;
 	while (GetTick() - tick < 8000)        // 8 s 整体超时
	{
		Delay_ms(100);
		len = RingBuff_GetLen(&encoeanBuff);
		if (len == 0)
        {
            continue;
        }
		if(len > response_size)
		{
			LOG_INFO("transfer limit %d",response_size);
		}

		RingBuff_ReadNByte(&encoeanBuff, response, response_size);

		break;
	}

	return 0;
}

void WIFI_Receive_Task(void)
{
	//服务器连接事件发生执行此任务，否则挂起
	if(WIFI_CONNECT != 1)
	{
		return;
	}

	//等待接收数据通知
	if(WIFI_Check_Flag != 1)
	{
		return;
	}
	WIFI_Check_Flag = 0;

	int len;
	//LOG_INFO("KEY_Task Running\r\n");
	len = RingBuff_GetLen(&encoeanBuff);
	if (len) 
	{
		uint8_t received_str[len+1];
		RingBuff_ReadNByte(&encoeanBuff,received_str,len);
		received_str[len] = '\0';
		// 输出接收到的字符串
		LOG_INFO("Received: %s\n", received_str);

		// ping状态，wifi连接成功
		//+CWJAP 且 OK
		if (strstr((const char*)received_str, "+CWJAP") != NULL && strstr((const char*)received_str, "OK") != NULL) 
		{
			LOG_INFO("PING success\r\n");                       
			if(pingFlag == 1)
			{                   						     //如果pingFlag=1，表示第一次发送
				pingFlag = 0;    				       		 //要清除pingFlag标志
			}
			else if(pingFlag > 1)	
			{ 				 								 //如果pingFlag>1，表示是多次发送了，而且是2s间隔的快速发送
				pingFlag = 0;     				      		 //要清除pingFlag标志
				TIM_WIFI_ENABLE_30S(); 				      		 //PING定时器重回30s的时间
				PING_MODE = 1; //30s的PING定时器，设置事件标志位
			}
		}
	}
}

// WIFI
uint8_t WIFI_Task(void)
{
    //服务器或者wifi已断开，清除事件标志，继续执行本任务，重新连接
	if(WIFI_CONNECT != 1)
	{
        uint8_t wifiState;
		LOG_INFO("wifi connecting...\r\n");                 
		TIM_Cmd(WIFI_TIM, DISABLE);                       //关闭TIM3
		PING_MODE = 0;//关闭发送PING包的定时器3，清除事件标志位
		ESP8266_Buf_Clear();//清空接收缓存区
		wifiState = ESP8266_WIFI_Connect();
		if(wifiState == 0)			  //如果WiFi连接云服务器函数返回0，表示正确，进入if
		{   			     
			LOG_INFO("wifi connect success\r\n"); 
			ESP8266_Buf_Clear();//清空接收缓存区

			WIFI_CONNECT = 1;  //服务器已连接，抛出事件标志 

			//启动定时器30s模式
			TIM_WIFI_ENABLE_30S();
			pingFlag = 0;
			PING_MODE = 1; //30s的PING定时器，设置事件标志位
		}
	}
	
	WIFI_Receive_Task();//WIFI接收数据，并ping连接状态
    
	//服务器连接以及ping心跳包30S发送模式事件发生时执行此任务，否则挂起任务
	if(PING_MODE == 0)
	{
		LOG_INFO("WIFI connect error\r\n");
		return 1;
	}

	return 0;
}
