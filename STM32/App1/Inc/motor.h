/**
 * @file    motor.h
 * @brief   Driver dieu khien dong co DC qua mach cau H L298N (STM32F103)
 */

#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>
#include "stm32f10x.h"

/* ========================================================================
 * CAU HINH PHAN CUNG MOTOR L298N TREN GPIOB
 * ======================================================================*/
#define MOTOR_PORT           GPIOB
#define MOTOR_CLK            RCC_APB2Periph_GPIOB

#define MA1_PIN              GPIO_Pin_12    /* IN1: Cum banh TRAI tien (motorA1) */
#define MA2_PIN              GPIO_Pin_13    /* IN2: Cum banh TRAI lui  (motorA2) */
#define MB1_PIN              GPIO_Pin_14    /* IN3: Cum banh PHAI tien (motorB1) */
#define MB2_PIN              GPIO_Pin_15    /* IN4: Cum banh PHAI lui  (motorB2) */
#define MOTOR_PIN_MASK       (MA1_PIN | MA2_PIN | MB1_PIN | MB2_PIN)

/**
 * @brief  Khoi tao cac chan GPIO dieu khien dong co L298N (Output Push-Pull)
 */
void Motor_Init(void);

/**
 * @brief  Dieu khien 4 banh xe bang 4 gia tri trang thai doc lap
 * @param  fl: Banh Truoc Trai (Front-Left)  : 1 = Tien, -1 = Lui, 0 = Dung
 * @param  rl: Banh Sau Trai   (Rear-Left)   : 1 = Tien, -1 = Lui, 0 = Dung
 * @param  fr: Banh Truoc Phai (Front-Right) : 1 = Tien, -1 = Lui, 0 = Dung
 * @param  rr: Banh Sau Phai   (Rear-Right)  : 1 = Tien, -1 = Lui, 0 = Dung
 *
 * @note   Tren xe 4WD dung L298N 2 kenh:
 *         - Cum Trai (fl, rl) anh xa sang IN1, IN2
 *         - Cum Phai (fr, rr) anh xa sang IN3, IN4
 */
void Motor_Set4Wheels(int8_t fl, int8_t rl, int8_t fr, int8_t rr);

/**
 * @brief  Dieu khien 2 cum banh Trai / Phai
 * @param  left:  1 (Tien), -1 (Lui), 0 (Dung)
 * @param  right: 1 (Tien), -1 (Lui), 0 (Dung)
 */
void Motor_SetSides(int8_t left, int8_t right);

/**
 * @brief  Xuat truc tiep muc logic vao 4 chan IN1..IN4 cua L298N
 */
void Motor_SetRaw(uint8_t in1, uint8_t in2, uint8_t in3, uint8_t in4);

/* Cac ham di chuyen chuan */
void Motor_Stop(void);
void Motor_Forward(void);
void Motor_Backward(void);
void Motor_TurnLeft(void);
void Motor_TurnRight(void);

/* Cac ham danh rieng cho tung ben banh */
void Motor_LeftFwd(void);
void Motor_LeftRev(void);
void Motor_RightFwd(void);
void Motor_RightRev(void);

#endif /* MOTOR_H */

