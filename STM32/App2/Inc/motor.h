#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>
#include "stm32f10x.h"

/*
 * CẤU HÌNH PHẦN CỨNG L298P TRÊN STM32 (GPIOB)
 * Đã đảo cặp Trái - Phải để khớp đúng chiều thực tế của xe:
 * PB12 (D3) : Cụm bánh phải - Tiến (IN1)
 * PB13 (D5) : Cụm bánh phải - Lùi  (IN2)
 * PB14 (D6) : Cụm bánh trái - Tiến (IN3)
 * PB15 (D9) : Cụm bánh trái - Lùi  (IN4)
 */
#define MOTOR_PORT           GPIOB
#define MOTOR_CLK            RCC_APB2Periph_GPIOB

#define MOTOR_RIGHT_FWD_PIN  GPIO_Pin_12   /* PB12 -> IN1 (Bánh Phải Tiến) */
#define MOTOR_RIGHT_REV_PIN  GPIO_Pin_13   /* PB13 -> IN2 (Bánh Phải Lùi) */
#define MOTOR_LEFT_FWD_PIN   GPIO_Pin_14   /* PB14 -> IN3 (Bánh Trái Tiến) */
#define MOTOR_LEFT_REV_PIN   GPIO_Pin_15   /* PB15 -> IN4 (Bánh Trái Lùi) */

#define MOTOR_PIN_MASK       (MOTOR_LEFT_FWD_PIN | MOTOR_LEFT_REV_PIN | \
                              MOTOR_RIGHT_FWD_PIN | MOTOR_RIGHT_REV_PIN)

void Motor_Init(void);
void Motor_Stop(void);
void Motor_Forward(void);
void Motor_Backward(void);
void Motor_TurnLeft(void);
void Motor_TurnRight(void);

/* Các hướng lệch kết hợp */
void Motor_LeftFwd(void);   /* Tiến lệch trái: Cụm phải tiến */
void Motor_RightFwd(void);  /* Tiến lệch phải: Cụm trái tiến */
void Motor_LeftRev(void);   /* Lùi lệch trái: Cụm phải lùi */
void Motor_RightRev(void);  /* Lùi lệch phải: Cụm trái lùi */

#endif /* MOTOR_H */
