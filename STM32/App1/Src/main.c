/**
 * @file    main.c
 * @brief   Template Blink LED & OTA Trigger cho App1 (STM32F103C8T6)
 *          - Tích hợp thư viện Delay (SysTick)
 *          - Lắng nghe lệnh OTA Bootloader qua CAN 500kbps (ID 0x600, Security Key 0xDEADBEEF)
 */

#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "delay.h"
#include "ota_trigger.h"

/* ========================================================================
 * CẤU HÌNH PHẦN CỨNG LED (Mặc định: PC13 - LED tích hợp board Blue Pill)
 * ======================================================================*/
#define LED_PIN             GPIO_Pin_13
#define LED_PORT            GPIOC
#define LED_RCC             RCC_APB2Periph_GPIOC
#define BLINK_INTERVAL_MS   500U

/**
 * @brief  Khởi tạo chân GPIO cho LED trạng thái
 */
static void LED_Init(void)
{
    GPIO_InitTypeDef gpioInit;

    /* Cấp clock cho ngoại vi GPIO của LED */
    RCC_APB2PeriphClockCmd(LED_RCC, ENABLE);

    /* Cấu hình chân LED dạng Output Push-Pull, tốc độ tối đa 50MHz */
    gpioInit.GPIO_Pin   = LED_PIN;
    gpioInit.GPIO_Mode  = GPIO_Mode_Out_PP;
    gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LED_PORT, &gpioInit);

    /* Mặc định tắt LED (LED PC13 trên Blue Pill là Active LOW: HIGH = Tắt, LOW = Sáng) */
    GPIO_SetBits(LED_PORT, LED_PIN);
}

/**
 * @brief  Đảo trạng thái LED
 */
static void LED_Toggle(void)
{
    LED_PORT->ODR ^= LED_PIN;
}

/* ========================================================================
 * CHƯƠNG TRÌNH CHÍNH
 * ======================================================================*/
int main(void)
{
    /* 1. Khởi tạo thư viện định thời trễ SysTick (1 ms) */
    Delay_Init();

    /* 2. Khởi tạo ngoại vi LED trạng thái */
    LED_Init();

    /* 3. Khởi tạo ngoại vi CAN (500 kbps) và bộ lọc phần cứng nhận lệnh OTA (ID 0x600, 0x620) */
    OTA_Trigger_Init();

    /* 4. Vòng lặp chính */
    while (1)
    {
        /* Kiểm tra và xử lý bản tin CAN kích hoạt Bootloader từ ESP32 */
        OTA_Trigger_Poll();

        /* Đảo trạng thái LED */
        LED_Toggle();

        /* Chờ trễ (Hàm Delay_Ms có liên tục gọi OTA_Trigger_Poll để đảm bảo không bỏ lỡ lệnh) */
        Delay_Ms(BLINK_INTERVAL_MS);
    }
}
