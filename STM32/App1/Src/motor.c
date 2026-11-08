/**
 * @file    motor.c
 * @brief   Dinh nghia cac ham dieu khien dong co DC qua L298N
 */

#include "motor.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

void Motor_Init(void)
{
    RCC_APB2PeriphClockCmd(MOTOR_CLK, ENABLE);

    GPIO_InitTypeDef gpio;
    gpio.GPIO_Pin   = MOTOR_PIN_MASK;
    gpio.GPIO_Mode  = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MOTOR_PORT, &gpio);

    Motor_Stop();
}

void Motor_SetRaw(uint8_t in1, uint8_t in2, uint8_t in3, uint8_t in4)
{
    uint16_t val = (uint16_t)(GPIO_ReadOutputData(MOTOR_PORT) & (uint16_t)(~MOTOR_PIN_MASK));

    if (in1) { val |= MA1_PIN; }
    if (in2) { val |= MA2_PIN; }
    if (in3) { val |= MB1_PIN; }
    if (in4) { val |= MB2_PIN; }

    GPIO_Write(MOTOR_PORT, val);
}

void Motor_SetSides(int8_t left, int8_t right)
{
    uint8_t in1 = 0U, in2 = 0U, in3 = 0U, in4 = 0U;

    /* Cum banh Trai (IN1, IN2) */
    if (left > 0)       { in1 = 1U; in2 = 0U; }
    else if (left < 0)  { in1 = 0U; in2 = 1U; }

    /* Cum banh Phai (IN3, IN4) */
    if (right > 0)      { in3 = 1U; in4 = 0U; }
    else if (right < 0) { in3 = 0U; in4 = 1U; }

    Motor_SetRaw(in1, in2, in3, in4);
}

void Motor_Set4Wheels(int8_t fl, int8_t rl, int8_t fr, int8_t rr)
{
    int8_t left = 0;
    int8_t right = 0;

    /* Tong hop huong ben Trai tu banh Truoc-Trai (fl) va Sau-Trai (rl) */
    if (fl > 0 || rl > 0)
    {
        if (fl >= 0 && rl >= 0) { left = 1; }
    }
    else if (fl < 0 || rl < 0)
    {
        if (fl <= 0 && rl <= 0) { left = -1; }
    }

    /* Tong hop huong ben Phai tu banh Truoc-Phai (fr) va Sau-Phai (rr) */
    if (fr > 0 || rr > 0)
    {
        if (fr >= 0 && rr >= 0) { right = 1; }
    }
    else if (fr < 0 || rr < 0)
    {
        if (fr <= 0 && rr <= 0) { right = -1; }
    }

    Motor_SetSides(left, right);
}

void Motor_Stop(void)
{
    Motor_Set4Wheels(0, 0, 0, 0);
}

void Motor_Forward(void)
{
    Motor_Set4Wheels(1, 1, 1, 1);
}

void Motor_Backward(void)
{
    Motor_Set4Wheels(-1, -1, -1, -1);
}

void Motor_TurnLeft(void)
{
    /* Quay tai cho sang trai: Banh trai lui, banh phai tien */
    /* Theo phan cung xe hien tai: in1=1, in2=0, in3=0, in4=1 (left=1, right=-1) */
    Motor_SetSides(1, -1);
}

void Motor_TurnRight(void)
{
    /* Quay tai cho sang phai: in1=0, in2=1, in3=1, in4=0 (left=-1, right=1) */
    Motor_SetSides(-1, 1);
}

void Motor_LeftFwd(void)
{
    /* Tien - Re Trai: Banh PHAI tien de bo cua sang Trai */
    Motor_SetSides(0, 1);
}

void Motor_LeftRev(void)
{
    /* Lui - Re Trai: Banh PHAI lui de duoi xe bo sang Trai */
    Motor_SetSides(0, -1);
}

void Motor_RightFwd(void)
{
    /* Tien - Re Phai: Banh TRAI tien de bo cua sang Phai */
    Motor_SetSides(1, 0);
}

void Motor_RightRev(void)
{
    /* Lui - Re Phai: Banh TRAI lui de duoi xe bo sang Phai */
    Motor_SetSides(-1, 0);
}

