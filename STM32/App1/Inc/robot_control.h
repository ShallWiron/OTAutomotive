#ifndef ROBOT_CONTROL_H
#define ROBOT_CONTROL_H

#include <stdint.h>
#include "stm32f10x.h"

#define CMD_EMERGENCY_STOP      0x000U
#define CMD_MODE_MOTOR          0x001U
#define CMD_MODE_SERVO          0x002U
#define CMD_LED_ON              0x003U
#define CMD_LED_OFF             0x004U
#define CMD_WIFI_CONNECTED      0x005U

#define CMD_MOTOR_STOP          0x040U
#define CMD_MOTOR_FORWARD       0x041U
#define CMD_MOTOR_BACKWARD      0x042U
#define CMD_MOTOR_TURN_LEFT     0x043U
#define CMD_MOTOR_TURN_RIGHT    0x044U
#define CMD_MOTOR_LEFT_FWD      0x045U
#define CMD_MOTOR_LEFT_REV      0x046U
#define CMD_MOTOR_RIGHT_FWD     0x047U
#define CMD_MOTOR_RIGHT_REV     0x048U

#define CMD_SERVO_STOP          0x080U
#define CMD_SERVO_ARM_UP        0x081U
#define CMD_SERVO_ARM_DOWN      0x082U
#define CMD_SERVO_ARM_EXTEND    0x083U
#define CMD_SERVO_ARM_RETRACT   0x084U

#define CMD_HORN_OFF            0x0C0U
#define CMD_HORN_ON             0x0C1U

void Robot_Init(void);
void Robot_ProcessCommand(uint16_t cmdId);
void Robot_Poll(void);

#endif
