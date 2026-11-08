/**
 * @file    servo.h
 * @brief   Driver phan cung dieu khien Servo su dung Timer 3 PWM 50Hz (STM32F103)
 */

#ifndef SERVO_H
#define SERVO_H

#include <stdint.h>
#include "stm32f10x.h"

/* Tan so PWM chuan cho RC Servo: 50Hz (Chu ky 20ms = 20000us) */
#define SERVO_TIM_CLK_HZ     72000000UL /* Clock TIM3 = 72 MHz */
#define SERVO_PERIOD_US      20000U     /* 20 ms */

/* Gioi han do rong xung microsecond (544us ~ 0 do, 2400us ~ 180 do) */
#define SERVO_MIN_US         544L
#define SERVO_MAX_US         2400L

/* Dinh nghia cac kenh phan cung tren Timer 3 */
#define SERVO_CH_LIFTER      0          /* TIM3_CH1 = PA6 : Nang / ha tay     */
#define SERVO_CH_EXTENSION   1          /* TIM3_CH2 = PA7 : Keo dai / thu tay */
#define SERVO_CH_GRIPPER     2          /* TIM3_CH3 = PB0 : Mo / dong kep     */

/**
 * @brief  Khoi tao Timer 3 va cac chan GPIO (PA6, PA7, PB0) o che do PWM 50Hz
 */
void Servo_Init(void);

/**
 * @brief  Xuat goc quay (0 - 180 do) cho kenh servo chi dinh
 * @param  ch: SERVO_CH_LIFTER, SERVO_CH_EXTENSION, hoac SERVO_CH_GRIPPER
 * @param  angle: Goc tu 0 den 180 do
 */
void Servo_WriteAngle(uint8_t ch, int angle);

/**
 * @brief  Xuat do rong xung truc tiep (microsecond) cho kenh servo
 * @param  ch: Kenh servo
 * @param  us: Do rong xung (544 - 2400 us)
 */
void Servo_WriteUs(uint8_t ch, uint16_t us);

#endif /* SERVO_H */

