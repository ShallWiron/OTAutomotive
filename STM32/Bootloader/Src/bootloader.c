#include "bootloader.h"
#include "flash.h"
#include "uart.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_can.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_bkp.h"

/* ==================== Typedef nội bộ ==================== */

typedef void (*pFunction)(void);

/* ==================== GPIO Init ==================== */

void Bootloader_GPIO_Init(void)
{
    GPIO_InitTypeDef gpioInit;

    /* Bật clock cho GPIOC (LED) và GPIOB (BOOT1 switch) */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOB, ENABLE);
    gpioInit.GPIO_Speed = GPIO_Speed_50MHz;

    /* PC13: LED trạng thái (Active LOW trên Blue Pill) */
    gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioInit.GPIO_Pin = GPIO_Pin_13;
    GPIO_Init(GPIOC, &gpioInit);
    GPIO_ResetBits(GPIOC, GPIO_Pin_13); /* Bật LED (kéo xuống 0) */

    /* PB2: Đọc trạng thái BOOT1 jumper (Pull-Up nội bộ) */
    gpioInit.GPIO_Mode = GPIO_Mode_IPU;
    gpioInit.GPIO_Pin = GPIO_Pin_2;
    GPIO_Init(GPIOB, &gpioInit);
}

/* ==================== Boot Decision ==================== */

uint8_t Bootloader_IsOtaModeRequested(void)
{
    /* 1. Kiểm tra cờ OTA trong Backup Register do App gửi trước khi Soft-Reset */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    if (BKP_ReadBackupRegister(BKP_DR1) == BKP_OTA_FLAG_MAGIC)
    {
        /* Xóa cờ ngay để lần khởi động sau không bị kẹt trong Bootloader */
        PWR_BackupAccessCmd(ENABLE);
        BKP_WriteBackupRegister(BKP_DR1, 0x0000);
        UART_SendString("[BOOT] Phat hien co Magic Key trong BKP_DR1 -> Vao che do OTA!\r\n");
        return 1;
    }

    /*
     * 2. Kênh cứu hộ bàn Lab qua jumper PB2 (BOOT1):
     * - Jumper ở vị trí 0 (GND) -> PB2 = LOW  -> Chạy App bình thường
     * - Jumper ở vị trí 1 (3V3) -> PB2 = HIGH -> Vào Bootloader chờ OTA
     */
    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_2) != 0)
    {
        UART_SendString("[BOOT] Jumper BOOT1 (PB2) = 1 -> Vao che do OTA cuu ho!\r\n");
        return 1;
    }

    return 0;
}

/* ==================== Kiểm tra App hợp lệ ==================== */

uint8_t Bootloader_CheckAppValid(uint32_t app_address)
{
    uint32_t msp_val   = *(__IO uint32_t *)app_address;
    uint32_t reset_val = *(__IO uint32_t *)(app_address + 4U);
    uint32_t max_addr  = app_address + (FLASH_APP1_SIZE_KB * 1024U);

    /* 1. Kiểm tra MSP nằm trong vùng SRAM hợp lệ (0x20000000 - 0x20005000) */
    if ((msp_val & 0x2FFE0000UL) != 0x20000000UL)
    {
        return 0;
    }

    /* 2. Kiểm tra địa chỉ Reset_Handler nằm trong phân vùng Flash của App và có bit Thumb (bit 0 = 1) */
    if ((reset_val & 0x01UL) == 0UL)
    {
        return 0;
    }

    uint32_t reset_addr = reset_val & ~0x01UL;
    if (reset_addr < app_address || reset_addr >= max_addr)
    {
        return 0;
    }

    return 1;
}

/* ==================== Jump To App ==================== */

void Bootloader_JumpToApp(uint32_t app_address)
{
    if (!Bootloader_CheckAppValid(app_address))
    {
        UART_SendString("[BOOT] Vector Table khong hop le tai 0x");
        UART_SendHex32(app_address);
        UART_SendString("\r\n");
        return;
    }

    /* Đợi truyền xong toàn bộ chuỗi log UART trước khi tắt ngoại vi */
    UART_WaitTxComplete();

    /* Vô hiệu hóa ngắt toàn cục */
    __disable_irq();

    /* De-init các ngoại vi đã bật trong Bootloader để trả về trạng thái sạch cho App */
    CAN_DeInit(CAN1);
    TIM_DeInit(TIM2);
    USART_DeInit(USART1);

    /* Reset và tắt SysTick */
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    /* Vô hiệu hóa và xóa toàn bộ ngắt trong NVIC */
    for (uint8_t i = 0; i < 8; i++)
    {
        NVIC->ICER[i] = 0xFFFFFFFFUL;
        NVIC->ICPR[i] = 0xFFFFFFFFUL;
    }

    /* Dời bảng Vector Table về đầu phân vùng của App */
    SCB->VTOR = app_address;

    /* Thiết lập lại MSP từ Vector Table của App */
    __set_MSP(*(__IO uint32_t *)app_address);

    /* Bật lại ngắt để App tự cấu hình trong SystemInit / main */
    __enable_irq();

    /* Lấy địa chỉ Reset_Handler và nhảy sang App */
    uint32_t JumpAddress = *(__IO uint32_t *)(app_address + 4U);
    pFunction Jump = (pFunction)JumpAddress;
    Jump();
}

/* ==================== Dual-Bank Boot & Rollback ==================== */

void Bootloader_BootActiveOrRollback(void)
{
    uint8_t  active_id   = Flash_GetActivePartition();
    uint32_t active_addr = Flash_GetPartitionAddress(active_id);

    UART_SendString("[BOOT] Active Partition trong Option Bytes: App ");
    UART_SendDec(active_id);
    UART_SendString(" (0x");
    UART_SendHex32(active_addr);
    UART_SendString(")\r\n");

    if (Bootloader_CheckAppValid(active_addr))
    {
        UART_SendString("[BOOT] Firmware App ");
        UART_SendDec(active_id);
        UART_SendString(" hop le -> Jumping...\r\n");
        Bootloader_JumpToApp(active_addr);
    }

    /* Nếu phân vùng Active bị lỗi/trống (ví dụ mất nguồn khi đang OTA), thực hiện Rollback */
    uint8_t  fallback_id   = (active_id == PARTITION_APP1) ? PARTITION_APP2 : PARTITION_APP1;
    uint32_t fallback_addr = Flash_GetPartitionAddress(fallback_id);

    UART_SendString("[ROLLBACK] Canh bao: App ");
    UART_SendDec(active_id);
    UART_SendString(" bi loi hoac trong! Thu Rollback sang App ");
    UART_SendDec(fallback_id);
    UART_SendString(" (0x");
    UART_SendHex32(fallback_addr);
    UART_SendString(")...\r\n");

    if (Bootloader_CheckAppValid(fallback_addr))
    {
        /* Cập nhật lại Option Bytes về bản Firmware dự phòng đang hoạt động tốt */
        Flash_SetActivePartition(fallback_id);
        UART_SendString("[ROLLBACK] Khoi phuc thanh cong! Jumping to App ");
        UART_SendDec(fallback_id);
        UART_SendString("...\r\n");
        Bootloader_JumpToApp(fallback_addr);
    }

    UART_SendString("[BOOT ERROR] Ca 2 phan vung App 1 va App 2 deu trong/loi!\r\n");
    UART_SendString("[BOOT] O lai che do Bootloader cho nhan ban cap nhat OTA qua CAN...\r\n");
}

/* ==================== LED ==================== */

void Bootloader_ToggleLED(void)
{
    GPIO_WriteBit(GPIOC, GPIO_Pin_13,
                  (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_13)));
}
