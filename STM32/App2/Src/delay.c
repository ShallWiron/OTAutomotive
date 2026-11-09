#include "delay.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"

void Delay_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_TimeBaseInitTypeDef tb;
    TIM_TimeBaseStructInit(&tb);
    tb.TIM_CounterMode   = TIM_CounterMode_Up;
    tb.TIM_Period        = 0xFFFF;
    tb.TIM_Prescaler     = 72 - 1;
    TIM_TimeBaseInit(TIM2, &tb);
    TIM_Cmd(TIM2, ENABLE);
}

void Delay_Us(uint32_t us)
{
    while (us > 0)
    {
        uint16_t chunk = (us > 50000) ? 50000 : (uint16_t)us;
        TIM_SetCounter(TIM2, 0);
        while (TIM_GetCounter(TIM2) < chunk) { }
        us -= chunk;
    }
}

void Delay_Ms(uint32_t ms)
{
    while (ms--)
    {
        Delay_Us(1000);
    }
}
