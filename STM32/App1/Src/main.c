#include "stm32f10x.h"
#include "ota_trigger.h"

void Delay1Ms(void);
void Delay_Ms(uint32_t u32DelayInMs);

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

int main(void)
{
	GPIO_InitTypeDef gpioInit;
	TIM_TimeBaseInitTypeDef timerInit;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	
	gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioInit.GPIO_Pin = GPIO_Pin_13;
	gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOC, &gpioInit);
	
	gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioInit.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOB, &gpioInit);
	
	timerInit.TIM_CounterMode = TIM_CounterMode_Up;
	timerInit.TIM_Period = 0xFFFF;
	timerInit.TIM_Prescaler = 72 - 1;
	
	TIM_TimeBaseInit(TIM2, &timerInit);
	TIM_Cmd(TIM2, ENABLE);
	
	/* Khởi tạo lắng nghe lệnh OTA qua CAN */
	OTA_Trigger_Init();
	
	while (1) {
		OTA_Trigger_Poll();
		GPIO_SetBits(GPIOC, GPIO_Pin_13);
		Delay_Ms(500);
		OTA_Trigger_Poll();
		GPIO_ResetBits(GPIOC, GPIO_Pin_13);
		Delay_Ms(500);
	}
}
