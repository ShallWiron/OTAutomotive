#include "led.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

void LED_Init(void)
{
    RCC_APB2PeriphClockCmd(LED_CLK, ENABLE);

    GPIO_InitTypeDef gpio;
    gpio.GPIO_Pin   = LED_PIN;
    gpio.GPIO_Mode  = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LED_PORT, &gpio);

    LED_Off();
}

void LED_On(void)
{
    GPIO_ResetBits(LED_PORT, LED_PIN);
}

void LED_Off(void)
{
    GPIO_SetBits(LED_PORT, LED_PIN);
}

void LED_Toggle(void)
{
    if (GPIO_ReadOutputDataBit(LED_PORT, LED_PIN))
    {
        GPIO_ResetBits(LED_PORT, LED_PIN);
    }
    else
    {
        GPIO_SetBits(LED_PORT, LED_PIN);
    }
}
