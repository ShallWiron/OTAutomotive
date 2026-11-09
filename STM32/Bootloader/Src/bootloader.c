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

typedef void (*pFunction)(void);

void Bootloader_GPIO_Init(void)
{
    GPIO_InitTypeDef gpioInit;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOB, ENABLE);
    gpioInit.GPIO_Speed = GPIO_Speed_50MHz;

    gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioInit.GPIO_Pin = GPIO_Pin_13;
    GPIO_Init(GPIOC, &gpioInit);
    GPIO_ResetBits(GPIOC, GPIO_Pin_13);

    gpioInit.GPIO_Mode = GPIO_Mode_IPU;
    gpioInit.GPIO_Pin = GPIO_Pin_2;
    GPIO_Init(GPIOB, &gpioInit);
}

uint8_t Bootloader_IsOtaModeRequested(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    if (BKP_ReadBackupRegister(BKP_DR1) == BKP_OTA_FLAG_MAGIC)
    {
        PWR_BackupAccessCmd(ENABLE);
        BKP_WriteBackupRegister(BKP_DR1, 0x0000);
        UART_SendString("[BOOT] Phat hien co Magic Key trong BKP_DR1 -> Vao che do OTA!\r\n");
        return 1;
    }

    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_2) != 0)
    {
        UART_SendString("[BOOT] Jumper BOOT1 (PB2) = 1 -> Vao che do OTA cuu ho!\r\n");
        return 1;
    }

    return 0;
}

uint8_t Bootloader_CheckAppValid(uint32_t app_address)
{
    uint32_t msp_val   = *(__IO uint32_t *)app_address;
    uint32_t reset_val = *(__IO uint32_t *)(app_address + 4U);
    uint32_t size_kb  = (app_address == FLASH_APP1_ADDRESS) ? FLASH_APP1_SIZE_KB : FLASH_APP2_SIZE_KB;
    uint32_t max_addr = app_address + (size_kb * 1024U);

    if ((msp_val & 0x2FFE0000UL) != 0x20000000UL)
    {
        return 0;
    }

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

void Bootloader_JumpToApp(uint32_t app_address)
{
    if (!Bootloader_CheckAppValid(app_address))
    {
        UART_SendString("[BOOT] Vector Table khong hop le tai 0x");
        UART_SendHex32(app_address);
        UART_SendString("\r\n");
        return;
    }

    UART_WaitTxComplete();

    __disable_irq();

    CAN_DeInit(CAN1);
    TIM_DeInit(TIM2);
    USART_DeInit(USART1);

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    for (uint8_t i = 0; i < 8; i++)
    {
        NVIC->ICER[i] = 0xFFFFFFFFUL;
        NVIC->ICPR[i] = 0xFFFFFFFFUL;
    }

    SCB->VTOR = app_address;

    __set_MSP(*(__IO uint32_t *)app_address);

    __enable_irq();

    uint32_t JumpAddress = *(__IO uint32_t *)(app_address + 4U);
    pFunction Jump = (pFunction)JumpAddress;
    Jump();
}

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
        Flash_SetActivePartition(fallback_id);
        UART_SendString("[ROLLBACK] Khoi phuc thanh cong! Jumping to App ");
        UART_SendDec(fallback_id);
        UART_SendString("...\r\n");
        Bootloader_JumpToApp(fallback_addr);
    }

    UART_SendString("[BOOT ERROR] Ca 2 phan vung App 1 va App 2 deu trong/loi!\r\n");
    UART_SendString("[BOOT] O lai che do Bootloader cho nhan ban cap nhat OTA qua CAN...\r\n");
}

void Bootloader_ToggleLED(void)
{
    GPIO_WriteBit(GPIOC, GPIO_Pin_13,
                  (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_13)));
}
