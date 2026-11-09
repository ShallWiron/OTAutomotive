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

static void Motor_SetSides(uint8_t left_fwd, uint8_t left_rev, uint8_t right_fwd, uint8_t right_rev)
{
    uint16_t val = (uint16_t)(GPIO_ReadOutputData(MOTOR_PORT) & (uint16_t)(~MOTOR_PIN_MASK));

    if (left_fwd)  { val |= MOTOR_LEFT_FWD_PIN; }
    if (left_rev)  { val |= MOTOR_LEFT_REV_PIN; }
    if (right_fwd) { val |= MOTOR_RIGHT_FWD_PIN; }
    if (right_rev) { val |= MOTOR_RIGHT_REV_PIN; }

    GPIO_Write(MOTOR_PORT, val);
}

void Motor_Stop(void)
{
    Motor_SetSides(0, 0, 0, 0);
}

void Motor_Forward(void)
{
    /* Hai bên cùng quay tiến */
    Motor_SetSides(1, 0, 1, 0);
}

void Motor_Backward(void)
{
    /* Hai bên cùng quay lùi */
    Motor_SetSides(0, 1, 0, 1);
}

void Motor_TurnLeft(void)
{
    /* Xoay trái: Cụm trái lùi, cụm phải tiến */
    Motor_SetSides(0, 1, 1, 0);
}

void Motor_TurnRight(void)
{
    /* Xoay phải: Cụm trái tiến, cụm phải lùi */
    Motor_SetSides(1, 0, 0, 1);
}

void Motor_LeftFwd(void)
{
    /* Tiến lệch trái: Cụm phải tiến, cụm trái đứng yên */
    Motor_SetSides(0, 0, 1, 0);
}

void Motor_RightFwd(void)
{
    /* Tiến lệch phải: Cụm trái tiến, cụm phải đứng yên */
    Motor_SetSides(1, 0, 0, 0);
}

void Motor_LeftRev(void)
{
    /* Lùi lệch trái: Cụm phải lùi, cụm trái đứng yên */
    Motor_SetSides(0, 0, 0, 1);
}

void Motor_RightRev(void)
{
    /* Lùi lệch phải: Cụm trái lùi, cụm phải đứng yên */
    Motor_SetSides(0, 1, 0, 0);
}
