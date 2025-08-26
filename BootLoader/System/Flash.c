#include "Flash.h"
#include "stm32f10x.h"                  // Device header

// 写入Flash
void Flash_Write(uint32_t Address, uint8_t *data, uint32_t size) {
    FLASH_Unlock();
    for (uint32_t i = 0; i < size; i += 4) {
        uint32_t word = (data[i] << 24) | (data[i + 1] << 16) | (data[i + 2] << 8) | data[i + 3];
        FLASH_ProgramWord(Address + i, word);
    }
    FLASH_Lock();
}

// 读取Flash
void Flash_Read(uint32_t Address, uint8_t *data, uint32_t size) {
    for (uint32_t i = 0; i < size; i++) {
        data[i] = *(__IO uint8_t *)(Address + i);
    }
}

/**
  * 函    数：FLASH页擦除
  * 参    数：PageAddress 要擦除页的页地址
  * 返 回 值：无
  */
void Flash_ErasePage(uint32_t PageAddress)
{
	FLASH_Unlock();					//解锁
	FLASH_ErasePage(PageAddress);	//页擦除
	FLASH_Lock();					//加锁
}

/**
  * 函    数：FLASH读取一个32位的字
  * 参    数：Address 要读取数据的字地址
  * 返 回 值：指定地址下的数据
  */
uint32_t Flash_ReadWord(uint32_t Address)
{
	return *((__IO uint32_t *)(Address));	//使用指针访问指定地址下的数据并返回
}

/**
  * 函    数：FLASH编程字
  * 参    数：Address 要写入数据的字地址
  * 参    数：Data 要写入的32位数据
  * 返 回 值：无
  */
void Flash_ProgramWord(uint32_t Address, uint32_t Data)
{
	FLASH_Unlock();							//解锁
	FLASH_ProgramWord(Address, Data);		//编程字
	FLASH_Lock();							//加锁
}
