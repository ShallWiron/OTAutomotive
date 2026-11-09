#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>
#include "stm32f10x.h"

#define MOTOR_PORT           GPIOB
#define MOTOR_CLK            RCC_APB2Periph_GPIOB

#define MOTOR_RIGHT_FWD_PIN  GPIO_Pin_12
#define MOTOR_RIGHT_REV_PIN  GPIO_Pin_13
#define MOTOR_LEFT_FWD_PIN   GPIO_Pin_14
#define MOTOR_LEFT_REV_PIN   GPIO_Pin_15

#define MOTOR_PIN_MASK       (MOTOR_LEFT_FWD_PIN | MOTOR_LEFT_REV_PIN | \
                              MOTOR_RIGHT_FWD_PIN | MOTOR_RIGHT_REV_PIN)

void Motor_Init(void);
void Motor_Stop(void);
void Motor_Forward(void);
void Motor_Backward(void);
void Motor_TurnLeft(void);
void Motor_TurnRight(void);

void Motor_LeftFwd(void);
void Motor_RightFwd(void);
void Motor_LeftRev(void);
void Motor_RightRev(void);

#endif
