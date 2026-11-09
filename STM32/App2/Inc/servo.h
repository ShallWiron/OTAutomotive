#ifndef SERVO_H
#define SERVO_H

#include <stdint.h>
#include "stm32f10x.h"

/*
 * CẤU HÌNH SERVO MG90S QUA TIMER 3 PWM 50Hz (Chu kỳ 20ms = 20000us)
 * A6 - D10 L298P : TIM3_CH1 (Servo Nâng - Trái, 0..70 độ)
 * A7 - D11 L298P : TIM3_CH2 (Servo Duỗi - Phải, 0..70 độ)
 * B0 - D12 L298P : TIM3_CH3 (Servo Kẹp, 0 độ hoặc 90 độ)
 */
#define SERVO_MIN_US         544L
#define SERVO_MAX_US         2400L

#define SERVO_LIFTER_MAX_DEG     70
#define SERVO_EXTENSION_MAX_DEG  70
#define SERVO_GRIPPER_OPEN_DEG   90
#define SERVO_GRIPPER_CLOSE_DEG  0

/* Bước quay mỗi lần nhận lệnh (tăng từ 1 lên 3 độ để quay nhanh và mượt hơn) */
#define SERVO_STEP_DEG           3

void Servo_Init(void);

/* Điều khiển trực tiếp góc (độ) */
void Servo_SetLifterAngle(int16_t angle);      /* Servo Trái - Nâng (0..70) */
void Servo_SetExtensionAngle(int16_t angle);   /* Servo Phải - Duỗi (0..70) */
void Servo_SetGripperAngle(int16_t angle);     /* Servo Kẹp (0 hoặc 90) */

/* Lệnh tăng giảm bước (+1, -1) */
void Servo_LifterStep(int8_t step);
void Servo_ExtensionStep(int8_t step);
void Servo_GripperOpen(void);
void Servo_GripperClose(void);

int16_t Servo_GetLifterAngle(void);
int16_t Servo_GetExtensionAngle(void);
int16_t Servo_GetGripperAngle(void);

#endif /* SERVO_H */
