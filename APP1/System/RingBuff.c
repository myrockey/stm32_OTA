#include "stm32f10x.h"
#include "stdio.h"
#include <string.h>
#include "RingBuff.h"
#include "globals.h"

RingBuff_t encoeanBuff;

void RingBuff_Init(RingBuff_t *rb) //初始化函数
{
  rb->Head = 0; //头指针置于起始位
  rb->Tail = 0; //尾指针置于起始位
  rb->Length = 0; //计录当前数据长度 判断是否存有数据
  //memset(rb->Ring_Buff,0,sizeof(rb->Ring_Buff));
}

/**
 * @brief  判断队列是否为空
 * @note   
 * @param  *rb: 结构体指针
 * @retval 返回0和1,1代表空，0代表非空
 */
te_cicrleQueueStatus_t RingBuff_IsEmpty(RingBuff_t *rb)
{
  return (rb->Head == rb->Tail) ? CQ_STATUS_IS_EMPTY : CQ_STATUS_OK;
}
/**
 * @brief  判断队列是否为满
 * @note   
 * @param  *rb: 结构体指针
 * @retval 返回0和1,1代表满，0代表非满
 */
te_cicrleQueueStatus_t RingBuff_IsFull(RingBuff_t *rb)
{
  return ((rb->Tail + 1) % BUFFER_SIZE == rb->Head) ? CQ_STATUS_IS_FULL : CQ_STATUS_OK;
}

/**
  *功能：数据写入环形缓冲区
  *入参1：要写入的数据
  *入参2：buffer指针
  *返回值：buffer是否已满
  */
uint8_t Write_RingBuff(RingBuff_t *ringBuff , uint8_t data)
{
    //将单字节数据存入到环形buffer的tail尾部
	ringBuff->Ring_Buff[ringBuff->Tail]=data;    
    //重新指定环形buffer的尾部地址，防止越界非法访问
	ringBuff->Tail = ( ringBuff->Tail + 1 ) % BUFFER_SIZE;
    //存入一个字节数据成功，len加1 
	ringBuff->Length++;    
	if(ringBuff->Length >= BUFFER_SIZE) //判断缓冲区是否已满
	{
		//如果buffer爆掉了，清空buffer，进行重新初始化   不初始化，会复位死机
		// memset(ringBuff, 0, BUFFER_SIZE);
		// RingBuff_Init(&ringBuff);
		LOG_INFO("缓冲区已满\r\n");
		ringBuff->Length = BUFFER_SIZE;
		ringBuff->Head = ( ringBuff->Tail + 1 ) % BUFFER_SIZE;
		//return 1;
	}
	return 0;
}

/**
  *功能：读取缓存区整帧数据-单字节读取
  *入参1：存放提取数据的指针
  *入参2：环形区buffer指针
  *返回值：是否成功提取数据
  */
uint8_t Read_RingBuff_Byte(RingBuff_t *ringBuff , uint8_t *rData)
{
	if(ringBuff->Length == 0)//判断非空
	{
		return 1;
	}
		
    //先进先出FIFO，从缓冲区头出，将头位置数据取出
	*rData = ringBuff->Ring_Buff[ringBuff->Head];
    //将取出数据的位置，数据清零
	ringBuff->Ring_Buff[ringBuff->Head] = 0;
				
	//重新指定buffer头的位置，防止越界非法访问
	ringBuff->Head = (ringBuff->Head + 1) % BUFFER_SIZE;
    //取出一个字节数据后，将数据长度减1
	ringBuff->Length--;
	
	return 0;
}


/*
从环形缓冲区读多个字节
*/
te_cicrleQueueStatus_t RingBuff_ReadNByte(RingBuff_t *pRingBuff, uint8_t *pData, int size)
{
	int i = 0;
	if(NULL == pRingBuff || NULL == pData)
		return CQ_STATUS_ERR;

	for( i = 0; i < size; i++)
	{
		Read_RingBuff_Byte(pRingBuff, pData+i);
	}
	return CQ_STATUS_OK;
}

//向环形缓冲区写多个字节
te_cicrleQueueStatus_t RingBuff_WriteNByte(RingBuff_t *pRingBuff, uint8_t *pData, int size)
{
	int i = 0;
	if(NULL == pRingBuff || NULL == pData)
		return CQ_STATUS_ERR;

	for(i = 0; i < size; i++)
	{
		Write_RingBuff(pRingBuff, *(pData+i));
	}
	return CQ_STATUS_OK;
}


//获取当前环形缓冲区中数据长度
int RingBuff_GetLen(RingBuff_t *pRingBuff)
{
	if(NULL == pRingBuff)
		return 0;

	if(pRingBuff->Tail >= pRingBuff->Head)
	{
		return pRingBuff->Tail - pRingBuff->Head;
	}
	
	return pRingBuff->Tail + BUFFER_SIZE - pRingBuff->Head;
}

uint16_t RQBuff_GetBuffLenth(RingBuff_t* RQ_Buff) {
	return RQ_Buff->Length;
}

//获取当前头部数据
unsigned char RingBuff_GetHeadItem(RingBuff_t *pRingBuff)
{
	if(NULL == pRingBuff)
		return CQ_STATUS_ERR;
	
	return pRingBuff->Ring_Buff[pRingBuff->Head];
}

//获取指定下标数据
unsigned char RingBuff_GetIndexItem(RingBuff_t *pRingBuff, int index)
{
	if(NULL == pRingBuff || index > BUFFER_SIZE-1)
		return CQ_STATUS_ERR;

	return pRingBuff->Ring_Buff[index%BUFFER_SIZE];
}

