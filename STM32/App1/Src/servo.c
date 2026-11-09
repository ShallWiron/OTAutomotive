#include "servo.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"

static int16_t s_lifterAngle = 0;
static int16_t s_extensionAngle = 0;
static int16_t s_gripperAngle = 0;

static uint16_t Angle_To_Compare(int16_t angle)
{
    if (angle < 0)   { angle = 0; }
    if (angle > 180) { angle = 180; }
    return (uint16_t)(SERVO_MIN_US + ((int32_t)angle * (SERVO_MAX_US - SERVO_MIN_US)) / 180L);
}

void Servo_Init(void)
{
    /* 1. Cấp clock TIM3, GPIOA, GPIOB */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

    /* 2. Cấu hình GPIO PA6, PA7, PB0 (AF Push-Pull 50MHz) */
    GPIO_InitTypeDef gpio;
    gpio.GPIO_Mode  = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;

    gpio.GPIO_Pin   = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOA, &gpio);

    gpio.GPIO_Pin   = GPIO_Pin_0;
    GPIO_Init(GPIOB, &gpio);

    /* 3. Cấu hình Timer 3 Base: 72MHz / 72 = 1MHz (1us/tick), Period = 20000us = 50Hz */
    TIM_TimeBaseInitTypeDef tb;
    TIM_TimeBaseStructInit(&tb);
    tb.TIM_Prescaler     = 72 - 1;
    tb.TIM_Period        = 20000 - 1;
    tb.TIM_ClockDivision = TIM_CKD_DIV1;
    tb.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &tb);

    /* 4. Cấu hình PWM Mode 1 cho CH1, CH2, CH3 */
    TIM_OCInitTypeDef oc;
    TIM_OCStructInit(&oc);
    oc.TIM_OCMode      = TIM_OCMode_PWM1;
    oc.TIM_OutputState = TIM_OutputState_Enable;
    oc.TIM_OCPolarity  = TIM_OCPolarity_High;
    oc.TIM_Pulse       = Angle_To_Compare(0);

    TIM_OC1Init(TIM3, &oc);
    TIM_OC2Init(TIM3, &oc);
    TIM_OC3Init(TIM3, &oc);

    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM3, ENABLE);

    TIM_Cmd(TIM3, ENABLE);

    /* Đặt góc ban đầu về 0 độ */
    Servo_SetLifterAngle(0);
    Servo_SetExtensionAngle(0);
    Servo_SetGripperAngle(0);
}

void Servo_SetLifterAngle(int16_t angle)
{
    if (angle < 0) { angle = 0; }
    if (angle > SERVO_LIFTER_MAX_DEG) { angle = SERVO_LIFTER_MAX_DEG; }
    s_lifterAngle = angle;
    TIM_SetCompare1(TIM3, Angle_To_Compare(s_lifterAngle));
}

void Servo_SetExtensionAngle(int16_t angle)
{
    if (angle < 0) { angle = 0; }
    if (angle > SERVO_EXTENSION_MAX_DEG) { angle = SERVO_EXTENSION_MAX_DEG; }
    s_extensionAngle = angle;
    TIM_SetCompare2(TIM3, Angle_To_Compare(s_extensionAngle));
}

void Servo_SetGripperAngle(int16_t angle)
{
    if (angle < 0) { angle = 0; }
    if (angle > 180) { angle = 180; }
    s_gripperAngle = angle;
    TIM_SetCompare3(TIM3, Angle_To_Compare(s_gripperAngle));
}

void Servo_LifterStep(int8_t step)
{
    Servo_SetLifterAngle(s_lifterAngle + step);
}

void Servo_ExtensionStep(int8_t step)
{
    Servo_SetExtensionAngle(s_extensionAngle + step);
}

void Servo_GripperOpen(void)
{
    Servo_SetGripperAngle(SERVO_GRIPPER_OPEN_DEG);
}

void Servo_GripperClose(void)
{
    Servo_SetGripperAngle(SERVO_GRIPPER_CLOSE_DEG);
}

int16_t Servo_GetLifterAngle(void)
{
    return s_lifterAngle;
}

int16_t Servo_GetExtensionAngle(void)
{
    return s_extensionAngle;
}

int16_t Servo_GetGripperAngle(void)
{
    return s_gripperAngle;
}
