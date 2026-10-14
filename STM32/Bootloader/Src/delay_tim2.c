#include "delay_tim2.h"
#include "stm32f10x.h"                  // Device header
#include "stm32f10x_rcc.h"   
#include "stm32f10x_tim.h"

void Delay_Init()
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	TIM_TimeBaseInitTypeDef timerInit;
	timerInit.TIM_CounterMode = TIM_CounterMode_Up;
	timerInit.TIM_Period = 0xFFFF;
	timerInit.TIM_Prescaler = 72 - 1;
	
	TIM_TimeBaseInit(TIM2, &timerInit);
	
	TIM_Cmd(TIM2, ENABLE);
}

void Delay1Ms(void)
{
	
	TIM_SetCounter(TIM2, 0);
	while (TIM_GetCounter(TIM2) < 1000) {
	}
}

void Delay_Ms(uint32_t u32DelayInMs)
{
	
	while (u32DelayInMs) {
		Delay1Ms();
		--u32DelayInMs;
	}
}