#include "stm32f10x.h"                  // Device header
#include "BootLoader.h"
#include "Flash.h"

void update_firmware(uint32_t SourceAddress, uint32_t TargetAddress) {
    uint16_t i;
    volatile uint32_t nK;
    nK = (TargetAddress - SourceAddress) / 1024;
    for (i = 0; i < nK; i++) {        
				uint8_t cBuffer[1024];;
        Flash_Read(TargetAddress + i * 0x0400, cBuffer, 1024);
        Flash_ErasePage(SourceAddress + i * 0x0400);
        Flash_Write(SourceAddress + i * 0x0400, cBuffer, 1024);
    }
}

void Jump2APP(uint32_t app_address) {
    typedef void (*APP_FUNC)(void);
    volatile uint32_t JumpAPPaddr = *(volatile uint32_t *)(app_address + 4);
    APP_FUNC jump2app = (APP_FUNC)JumpAPPaddr;
		
		// 设置中断向量表的起始地址
    SCB->VTOR = app_address; //显示调用，虽然不调用，其实也默认会在__set_MSP重新设置

    __set_MSP(*(volatile uint32_t *)app_address);
    jump2app();
}

void CheckUpdateFlag(void) {
	  uint32_t flag_value = Flash_ReadWord(UPFLAGADDR);
    if (flag_value == UPDATE_FLAG) {
        update_firmware(APP1ADDR, APP2ADDR);
        Flash_ProgramWord(UPFLAGADDR, 0xFFFFFFFF); // 清除标志位
        NVIC_SystemReset();
    } else {
        Jump2APP(APP1ADDR);
    }
}


