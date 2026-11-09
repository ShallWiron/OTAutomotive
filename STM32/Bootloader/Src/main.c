#include "stm32f10x.h"
#include "delay_tim2.h"
#include "uart.h"
#include "can.h"
#include "flash.h"
#include "bootloader.h"
#include "ota_protocol.h"

int main(void)
{
    Delay_Init();
    UART_Setup();
    CAN_Setup();
    Bootloader_GPIO_Init(); 
    OTA_Init();

    UART_SendString("\r\n========================================\r\n");
    UART_SendString("   STM32 DUAL-BANK CAN OTA BOOTLOADER   \r\n");
    UART_SendString("========================================\r\n");

    if (!Bootloader_IsOtaModeRequested())
    {
        UART_SendString("[BOOT] Khong co co OTA & BOOT1 = 0 -> Khoi dong App...\r\n");
        Bootloader_BootActiveOrRollback();
    }

    UART_SendString("[OTA] STM32 Bootloader san sang nhan Firmware qua CAN (FIFO1)...\r\n");

    while (1)
    {
        OTA_Run();
    }
}
