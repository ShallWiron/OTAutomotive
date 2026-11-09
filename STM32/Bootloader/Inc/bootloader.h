#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include <stdint.h>

#define BKP_OTA_FLAG_MAGIC      0x5A5AU

void Bootloader_GPIO_Init(void);
uint8_t Bootloader_IsOtaModeRequested(void);
uint8_t Bootloader_CheckAppValid(uint32_t app_address);
void Bootloader_JumpToApp(uint32_t app_address);
void Bootloader_BootActiveOrRollback(void);
void Bootloader_ToggleLED(void);

#endif
