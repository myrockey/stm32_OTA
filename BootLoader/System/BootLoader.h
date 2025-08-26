#ifndef __BOOT_H
#define __BOOT_H

//OTA升级标志位
#define UPDATE_FLAG 0x55555555

// stm FLASH初始地址
#define ADDR_FLASH_BASE 0x08000000

#define APP1  0x2000
#define APP2  0x8C00
#define UPFLAG1 0xF800

#define APP1ADDR (ADDR_FLASH_BASE + APP1)    // 0x08002000
#define APP2ADDR (ADDR_FLASH_BASE + APP2)   // 0x08008C00
#define UPFLAGADDR   (ADDR_FLASH_BASE + UPFLAG1)   // 0x0800F800

void update_firmware(uint32_t SourceAddress, uint32_t TargetAddress);
void Jump2APP(uint32_t app_address);
void CheckUpdateFlag(void);

#endif
