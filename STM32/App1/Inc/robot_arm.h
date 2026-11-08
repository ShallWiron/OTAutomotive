/**
 * @file    robot_arm.h
 * @brief   Module quan ly logic tay gap robot 3 bac tu do (3-DOF Robot Arm)
 */

#ifndef ROBOT_ARM_H
#define ROBOT_ARM_H

#include <stdint.h>
#include "can_control.h"

/* Gioi han goc co khi an toan */
#define ARM_MIN_DEG          0          /* Goc toi thieu (Nang/Ha & Vuon/Thu) */
#define ARM_MAX_DEG          70         /* Goc toi da    (Nang/Ha & Vuon/Thu) */
#define GRIP_OPEN_DEG        90         /* Goc mo kep  */
#define GRIP_CLOSE_DEG       0          /* Goc dong kep */

/* Toc do buoc servo khi giu nut dieu khien (ms / 1 do) */
#define ARM_DELAY_FAST_MS    20UL       /* Buoc nhanh (20ms/do) - Nang tay */
#define ARM_DELAY_SLOW_MS    40UL       /* Buoc cham (40ms/do)  - Ha / Vuon / Thu tay */

/**
 * @brief  Khoi tao cac goc ban dau cua tay gap ve vi tri an toan (0 do)
 */
void Arm_Init(void);

/**
 * @brief  Thiet lap lenh dieu khien tay gap (nhan tu CAN)
 * @param  cmd: CMD_SERVO_STOP, CMD_SERVO_ARM_UP, CMD_SERVO_ARM_DOWN,
 *              CMD_SERVO_ARM_EXTEND, CMD_SERVO_ARM_RETRACT
 */
void Arm_SetCommand(uint16_t cmd);

/**
 * @brief  Dung ngay lap tuc moi buoc di chuyen cua tay gap
 */
void Arm_Stop(void);

/**
 * @brief  Mo kep gap
 */
void Arm_GripOpen(void);

/**
 * @brief  Dong kep gap
 */
void Arm_GripClose(void);

/**
 * @brief  Tien trinh cap nhat buoc chuyen dong muot ma (Non-blocking)
 *         Can duoc goi dinh ky trong main loop
 * @param  current_ms: Thoi gian SysTick hien tai (g_ms)
 */
void Arm_Poll(uint32_t current_ms);

/* Lay goc hien tai de giam sat telemetry */
int Arm_GetLifterAngle(void);
int Arm_GetExtensionAngle(void);
int Arm_GetGripperAngle(void);

#endif /* ROBOT_ARM_H */

