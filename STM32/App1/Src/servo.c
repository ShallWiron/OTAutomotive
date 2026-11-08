/**
 * @file    servo.c
 * @brief   Trien khai driver Timer 3 PWM dieu khien Servo
 */

#include "servo.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"

static uint16_t Angle_To_Us(int angle)
{
    if (angle < 0)   { angle = 0; }
    if (angle > 180) { angle = 180; }
    return (uint16_t)(SERVO_MIN_US + ((int32_t)angle * (SERVO_MAX_US - SERVO_MIN_US)) / 180L);
}

void Servo_Init(void)
{
    TIM_TimeBaseInitTypeDef tb;
    TIM_OCInitTypeDef       oc;
    GPIO_InitTypeDef        gpio;

    /* 1. Bat Clock TIM3 va cac Port GPIO liên quan */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

    /* 2. Cau hinh GPIO Alternate Function Push-Pull:
     *    PA6 = TIM3_CH1, PA7 = TIM3_CH2, PB0 = TIM3_CH3 */
    gpio.GPIO_Mode  = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;

    gpio.GPIO_Pin   = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOA, &gpio);

    gpio.GPIO_Pin   = GPIO_Pin_0;
    GPIO_Init(GPIOB, &gpio);

    /* 3. Cau hinh Timer 3 TimeBase:
     *    Prescaler: 72 MHz / 72 = 1 MHz (1 tick = 1 us)
     *    Period   : 20000 us = 20 ms -> 50 Hz */
    TIM_TimeBaseStructInit(&tb);
    tb.TIM_Prescaler     = (uint16_t)(SERVO_TIM_CLK_HZ / 1000000UL - 1UL);
    tb.TIM_Period        = SERVO_PERIOD_US - 1U;
    tb.TIM_ClockDivision = TIM_CKD_DIV1;
    tb.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &tb);

    /* 4. Cau hinh PWM Mode 1 tren ca 3 channel */
    TIM_OCStructInit(&oc);
    oc.TIM_OCMode      = TIM_OCMode_PWM1;
    oc.TIM_OutputState = TIM_OutputState_Enable;
    oc.TIM_OCPolarity  = TIM_OCPolarity_High;
    oc.TIM_Pulse       = Angle_To_Us(0);

    TIM_OC1Init(TIM3, &oc);
    TIM_OC2Init(TIM3, &oc);
    TIM_OC3Init(TIM3, &oc);

    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM3, ENABLE);

    /* 5. Cho phep Timer 3 dem */
    TIM_Cmd(TIM3, ENABLE);
}

void Servo_WriteUs(uint8_t ch, uint16_t us)
{
    if      (ch == SERVO_CH_LIFTER)    { TIM_SetCompare1(TIM3, us); }
    else if (ch == SERVO_CH_EXTENSION) { TIM_SetCompare2(TIM3, us); }
    else if (ch == SERVO_CH_GRIPPER)   { TIM_SetCompare3(TIM3, us); }
}

void Servo_WriteAngle(uint8_t ch, int angle)
{
    uint16_t us = Angle_To_Us(angle);
    Servo_WriteUs(ch, us);
}

