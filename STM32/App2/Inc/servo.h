#ifndef SERVO_H
#define SERVO_H

#include <stdint.h>
#include "stm32f10x.h"

#define SERVO_MIN_US         544L
#define SERVO_MAX_US         2400L

#define SERVO_LIFTER_MAX_DEG     70
#define SERVO_EXTENSION_MAX_DEG  70
#define SERVO_GRIPPER_OPEN_DEG   90
#define SERVO_GRIPPER_CLOSE_DEG  0

#define SERVO_STEP_DEG           3

void Servo_Init(void);

void Servo_SetLifterAngle(int16_t angle);
void Servo_SetExtensionAngle(int16_t angle);
void Servo_SetGripperAngle(int16_t angle);

void Servo_LifterStep(int8_t step);
void Servo_ExtensionStep(int8_t step);
void Servo_GripperOpen(void);
void Servo_GripperClose(void);

int16_t Servo_GetLifterAngle(void);
int16_t Servo_GetExtensionAngle(void);
int16_t Servo_GetGripperAngle(void);

#endif
