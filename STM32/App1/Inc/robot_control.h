#ifndef ROBOT_CONTROL_H
#define ROBOT_CONTROL_H

#include <stdint.h>
#include "stm32f10x.h"
#include "can_control.h"

/* Thoi gian watchdog ngat an toan phong khi mat song WiFi / CAN khi dang giu nut (ms) */
#define CMD_WATCHDOG_TIMEOUT_MS  600UL

/* Cac ham dieu khien dong co L298N */
void Motor_Set(uint8_t a1, uint8_t a2, uint8_t b1, uint8_t b2);
void Motor_Stop(void);
void Motor_Forward(void);
void Motor_Backward(void);
void Motor_TurnLeft(void);
void Motor_TurnRight(void);

/* Cac ham dieu khien Servo (TIM3 PWM 50Hz) */
void Servo_Init(void);
void Servo_Write(uint8_t ch, int angle);
void Arm_Step(uint8_t ch, int *angle, int dir, uint32_t delay_ms);

/* Ngoai vi phu tro */
void Led_Set(uint8_t on);
void Relay_Set(uint8_t on);
void Buzzer_Beep(uint32_t ms);

/* Khoi tao, lang nghe CAN & thuc thi lenh */
void Robot_Init(void);
void Robot_Control_Poll(void);
void Robot_ExecuteCommand(uint16_t can_id);
void Robot_Poll(void);
void Delay_Ms(uint32_t ms);

#endif /* ROBOT_CONTROL_H */
