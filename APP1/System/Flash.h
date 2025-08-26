#ifndef __FLASH_H
#define __FLASH_H

#include <stdint.h>

void Flash_Write(uint32_t Address, uint8_t *data, uint32_t size);
void Flash_Read(uint32_t Address, uint8_t *data, uint32_t size);
void Flash_ErasePage(uint32_t PageAddress);
uint32_t Flash_ReadWord(uint32_t Address);
void Flash_ProgramWord(uint32_t Address, uint32_t Data);

#endif
