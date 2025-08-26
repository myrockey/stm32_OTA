#ifndef _RINGBUFF_H_INCLUDED
#define _RINGBUFF_H_INCLUDED

#include "stm32f10x.h"
#include "stdio.h"
#include <string.h>

#define BUFFER_SIZE 1024        /* 环形缓冲区的大小 */

typedef enum {
  CQ_STATUS_OK = 0,
  CQ_STATUS_IS_FULL,
  CQ_STATUS_IS_EMPTY,
  CQ_STATUS_ERR    // 出错
} te_cicrleQueueStatus_t;


typedef struct
{
	uint32_t Head;
	uint32_t Tail;
	uint32_t Length;
	uint8_t Ring_Buff[BUFFER_SIZE];
} RingBuff_t;

extern RingBuff_t encoeanBuff;

te_cicrleQueueStatus_t RingBuff_IsEmpty(RingBuff_t *rb);

te_cicrleQueueStatus_t RingBuff_IsFull(RingBuff_t *rb);

te_cicrleQueueStatus_t RingBuff_ReadNByte(RingBuff_t *pRingBuff, uint8_t *pData, int size);

te_cicrleQueueStatus_t RingBuff_WriteNByte(RingBuff_t *pRingBuff, uint8_t *pData, int size);

int RingBuff_GetLen(RingBuff_t *pRingBuff);

unsigned char RingBuff_GetIndexItem(RingBuff_t *pRingBuff, int index);

uint8_t Write_RingBuff(RingBuff_t *ringBuff , volatile uint8_t data);
uint8_t Read_RingBuff_Byte(RingBuff_t *ringBuff , uint8_t *rData);
void RingBuff_Init(RingBuff_t *rb);

uint16_t RQBuff_GetBuffLenth(RingBuff_t* RQ_Buff);

#endif //_MOD_BUTTON_H_INCLUDED

