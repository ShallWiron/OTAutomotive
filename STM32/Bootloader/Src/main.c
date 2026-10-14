#include "stm32f10x.h"
#include "delay_tim2.h"
#include "uart.h"
#include "can.h"
#include "flash.h"
#include "bootloader.h"
#include "ota_protocol.h"

int main(void)
{
    /* 1. Khởi tạo các Driver và Middleware */
    Delay_Init();
    UART_Setup();
    CAN_Setup();
    Bootloader_GPIO_Init(); /* Khởi tạo GPIO cho Bootloader (PB2, PC13) */
    OTA_Init();

    UART_SendString("\r\n========================================\r\n");
    UART_SendString("   STM32 DUAL-BANK CAN OTA BOOTLOADER   \r\n");
    UART_SendString("========================================\r\n");

    /* 2. Kiểm tra chế độ khởi động (Cờ BKP hoặc Jumper PB2) */
    if (!Bootloader_IsOtaModeRequested())
    {
        UART_SendString("[BOOT] Khong co co OTA & BOOT1 = 0 -> Khoi dong App...\r\n");
        /*
         * Đọc Active Partition từ Option Bytes (App 1 hoặc App 2).
         * Nếu phân vùng Active bị lỗi (do mất nguồn giữa chừng khi OTA),
         * tự động Rollback sang phân vùng còn lại.
         */
        Bootloader_BootActiveOrRollback();
    }

    UART_SendString("[OTA] STM32 Bootloader san sang nhan Firmware qua CAN (FIFO1)...\r\n");

    /* 3. Vòng lặp chính xử lý luồng cập nhật CAN OTA */
    while (1)
    {
        OTA_Run();
    }
}
