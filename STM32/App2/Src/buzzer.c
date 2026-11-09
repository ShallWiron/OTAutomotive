#include "buzzer.h"
#include "delay.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "misc.h"

static volatile uint8_t s_hornActive = 0;
static volatile uint32_t s_beepTicks = 0;

void Buzzer_Init(void)
{
    /* 1. GPIO Pin PA0 Configuration */
    RCC_APB2PeriphClockCmd(BUZZER_CLK, ENABLE);

    GPIO_InitTypeDef gpio;
    gpio.GPIO_Pin   = BUZZER_PIN;
    gpio.GPIO_Mode  = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(BUZZER_PORT, &gpio);

    GPIO_ResetBits(BUZZER_PORT, BUZZER_PIN);

    /* 2. TIM4 Hardware Timer Configuration (5 kHz toggle rate = 2.5 kHz Tone) */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    TIM_TimeBaseInitTypeDef tb;
    TIM_TimeBaseStructInit(&tb);
    tb.TIM_Prescaler     = 72 - 1;  /* 1 MHz clock */
    tb.TIM_Period        = 200 - 1; /* 200 us period -> 5000 Hz ISR */
    tb.TIM_CounterMode   = TIM_CounterMode_Up;
    tb.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM4, &tb);

    TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);

    /* 3. NVIC Interrupt Configuration */
    NVIC_InitTypeDef nvic;
    nvic.NVIC_IRQChannel = TIM4_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 2;
    nvic.NVIC_IRQChannelSubPriority = 0;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);

    TIM_Cmd(TIM4, DISABLE);
}

void Buzzer_HornOn(void)
{
    s_hornActive = 1;
    TIM_Cmd(TIM4, ENABLE);
}

void Buzzer_HornOff(void)
{
    s_hornActive = 0;
    s_beepTicks = 0;
    TIM_Cmd(TIM4, DISABLE);
    GPIO_ResetBits(BUZZER_PORT, BUZZER_PIN);
}

uint8_t Buzzer_IsHornActive(void)
{
    return (s_hornActive || (s_beepTicks > 0)) ? 1U : 0U;
}

void Buzzer_Beep(uint32_t duration_ms)
{
    s_beepTicks = duration_ms * 5U;
    TIM_Cmd(TIM4, ENABLE);
}

void Buzzer_Tone(uint16_t freq_hz, uint32_t duration_ms)
{
    if (freq_hz == 0)
    {
        Delay_Ms(duration_ms);
        return;
    }

    uint32_t half_period_us = 1000000UL / (2UL * freq_hz);
    uint32_t cycles = ((uint32_t)freq_hz * duration_ms) / 1000UL;

    for (uint32_t i = 0; i < cycles; i++)
    {
        GPIO_SetBits(BUZZER_PORT, BUZZER_PIN);
        Delay_Us(half_period_us);
        GPIO_ResetBits(BUZZER_PORT, BUZZER_PIN);
        Delay_Us(half_period_us);
    }
}

void Buzzer_BeepStart(void)
{
    Buzzer_Tone(2500, 150);
}

void Buzzer_BeepSuccess(void)
{
    Buzzer_Tone(2700, 100);
    Delay_Ms(80);
    Buzzer_Tone(2700, 150);
}

void TIM4_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);

        if (s_hornActive)
        {
            BUZZER_PORT->ODR ^= BUZZER_PIN;
        }
        else if (s_beepTicks > 0)
        {
            s_beepTicks--;
            BUZZER_PORT->ODR ^= BUZZER_PIN;
            if (s_beepTicks == 0)
            {
                BUZZER_PORT->BRR = BUZZER_PIN;
                TIM_Cmd(TIM4, DISABLE);
            }
        }
        else
        {
            BUZZER_PORT->BRR = BUZZER_PIN;
            TIM_Cmd(TIM4, DISABLE);
        }
    }
}
