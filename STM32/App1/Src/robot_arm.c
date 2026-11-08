/**
 * @file    robot_arm.c
 * @brief   Trien khai logic dieu khien tay gap robot 3-DOF
 */

#include "robot_arm.h"
#include "servo.h"

/* Trang thai goc hien tai */
static int      g_angleLifter    = 0;
static int      g_angleExtension = 0;
static int      g_angleGripper   = 0;

/* Lenh dieu khien va thoi diem buoc gan nhat */
static uint16_t g_currentArmCmd  = CMD_SERVO_STOP;
static uint32_t g_lastLifterTick = 0U;
static uint32_t g_lastExtendTick = 0U;

void Arm_Init(void)
{
    Servo_Init();

    g_angleLifter    = 0;
    g_angleExtension = 0;
    g_angleGripper   = 0;
    g_currentArmCmd  = CMD_SERVO_STOP;

    Servo_WriteAngle(SERVO_CH_LIFTER,    g_angleLifter);
    Servo_WriteAngle(SERVO_CH_EXTENSION, g_angleExtension);
    Servo_WriteAngle(SERVO_CH_GRIPPER,   g_angleGripper);
}

void Arm_SetCommand(uint16_t cmd)
{
    g_currentArmCmd = cmd;
}

void Arm_Stop(void)
{
    g_currentArmCmd = CMD_SERVO_STOP;
}

void Arm_GripOpen(void)
{
    g_angleGripper = GRIP_OPEN_DEG;
    Servo_WriteAngle(SERVO_CH_GRIPPER, g_angleGripper);
}

void Arm_GripClose(void)
{
    g_angleGripper = GRIP_CLOSE_DEG;
    Servo_WriteAngle(SERVO_CH_GRIPPER, g_angleGripper);
}

void Arm_Poll(uint32_t current_ms)
{
    if (g_currentArmCmd == CMD_SERVO_STOP)
    {
        return;
    }

    switch (g_currentArmCmd)
    {
        case CMD_SERVO_ARM_UP:
            if ((uint32_t)(current_ms - g_lastLifterTick) >= ARM_DELAY_FAST_MS)
            {
                g_lastLifterTick = current_ms;
                if (g_angleLifter < ARM_MAX_DEG)
                {
                    g_angleLifter++;
                    Servo_WriteAngle(SERVO_CH_LIFTER, g_angleLifter);
                }
            }
            break;

        case CMD_SERVO_ARM_DOWN:
            if ((uint32_t)(current_ms - g_lastLifterTick) >= ARM_DELAY_SLOW_MS)
            {
                g_lastLifterTick = current_ms;
                if (g_angleLifter > ARM_MIN_DEG)
                {
                    g_angleLifter--;
                    Servo_WriteAngle(SERVO_CH_LIFTER, g_angleLifter);
                }
            }
            break;

        case CMD_SERVO_ARM_EXTEND:
            if ((uint32_t)(current_ms - g_lastExtendTick) >= ARM_DELAY_SLOW_MS)
            {
                g_lastExtendTick = current_ms;
                if (g_angleExtension < ARM_MAX_DEG)
                {
                    g_angleExtension++;
                    Servo_WriteAngle(SERVO_CH_EXTENSION, g_angleExtension);
                }
            }
            break;

        case CMD_SERVO_ARM_RETRACT:
            if ((uint32_t)(current_ms - g_lastExtendTick) >= ARM_DELAY_SLOW_MS)
            {
                g_lastExtendTick = current_ms;
                if (g_angleExtension > ARM_MIN_DEG)
                {
                    g_angleExtension--;
                    Servo_WriteAngle(SERVO_CH_EXTENSION, g_angleExtension);
                }
            }
            break;

        default:
            break;
    }
}

int Arm_GetLifterAngle(void)
{
    return g_angleLifter;
}

int Arm_GetExtensionAngle(void)
{
    return g_angleExtension;
}

int Arm_GetGripperAngle(void)
{
    return g_angleGripper;
}

