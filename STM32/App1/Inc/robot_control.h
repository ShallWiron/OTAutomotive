/**
 * @file    robot_control.h
 * @brief   Header tong hop he thong robot dieu khien xe va tay gap
 */

#ifndef ROBOT_CONTROL_H
#define ROBOT_CONTROL_H

#include <stdint.h>
#include "stm32f10x.h"
#include "can_control.h"
#include "motor.h"
#include "servo.h"
#include "robot_arm.h"

/* Cac ham dieu phoi toan cuc */
void Robot_Init(void);
void Robot_Control_Poll(void);
void Robot_ExecuteCommand(uint16_t can_id);
void Delay_Ms(uint32_t ms);

#endif /* ROBOT_CONTROL_H */
