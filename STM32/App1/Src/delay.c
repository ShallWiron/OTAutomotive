#include "delay.h"
#include "ota_trigger.h"

/* Biến đếm thời gian hệ thống được tăng mỗi 1ms trong SysTick_Handler (stm32f10x_it.c) */
volatile uint32_t g_ms = 0U;

void Delay_Init(void)
{
    /* Cấu hình SysTick ngắt chu kỳ 1ms theo xung nhịp hệ thống (72MHz -> 1kHz) */
    SysTick_Config(SystemCoreClock / 1000UL);
}

uint32_t Delay_GetTick(void)
{
    return g_ms;
}

void Delay_Ms(uint32_t ms)
{
    uint32_t start = g_ms;
    while ((uint32_t)(g_ms - start) < ms)
    {
        /* Thường xuyên kiểm tra bản tin CAN OTA trong lúc delay để phản hồi tức thì về ESP32 */
        OTA_Trigger_Poll();
    }
}

