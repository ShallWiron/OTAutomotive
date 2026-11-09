#include "robot_control.h"
#include "motor.h"
#include "servo.h"
#include "led.h"
#include "can_driver.h"
#include "ota_trigger.h"
#include "delay.h"

static uint8_t s_dofMode = 0; /* 0: Car (Motor), 1: Arm (Servo) */

void Robot_Init(void)
{
    Delay_Init();
    LED_Init();
    Motor_Init();
    Servo_Init();
    CAN_Driver_Init();
    OTA_Trigger_Init();

    /* Blink diagnostic LED once on startup */
    LED_On();
    Delay_Ms(150);
    LED_Off();
}

void Robot_ProcessCommand(uint16_t cmdId)
{
    switch (cmdId)
    {
        /* System & Mode */
        case CMD_EMERGENCY_STOP:
            Motor_Stop();
            LED_Off();
            break;

        case CMD_MODE_MOTOR:
            s_dofMode = 0;
            Motor_Stop();
            break;

        case CMD_MODE_SERVO:
            s_dofMode = 1;
            Motor_Stop();
            break;

        case CMD_LED_ON:
            LED_On();
            break;

        case CMD_LED_OFF:
            LED_Off();
            break;

        case CMD_WIFI_CONNECTED:
            /* Quick double blink to acknowledge WiFi connection */
            LED_On(); Delay_Ms(80); LED_Off(); Delay_Ms(80);
            LED_On(); Delay_Ms(80); LED_Off();
            break;

        /* Motor / Directional Navigation */
        case CMD_MOTOR_STOP:
            Motor_Stop();
            break;

        case CMD_MOTOR_FORWARD:
            if (s_dofMode == 0) {
                Motor_Forward();
            } else {
                Servo_LifterStep(+SERVO_STEP_DEG);
            }
            break;

        case CMD_MOTOR_BACKWARD:
            if (s_dofMode == 0) {
                Motor_Backward();
            } else {
                Servo_LifterStep(-SERVO_STEP_DEG);
            }
            break;

        case CMD_MOTOR_TURN_LEFT:
            if (s_dofMode == 0) {
                Motor_TurnLeft();
            } else {
                Servo_ExtensionStep(+SERVO_STEP_DEG);
            }
            break;

        case CMD_MOTOR_TURN_RIGHT:
            if (s_dofMode == 0) {
                Motor_TurnRight();
            } else {
                Servo_ExtensionStep(-SERVO_STEP_DEG);
            }
            break;

        case CMD_MOTOR_LEFT_FWD:
            Motor_LeftFwd();
            break;

        case CMD_MOTOR_LEFT_REV:
            Motor_LeftRev();
            break;

        case CMD_MOTOR_RIGHT_FWD:
            Motor_RightFwd();
            break;

        case CMD_MOTOR_RIGHT_REV:
            Motor_RightRev();
            break;

        /* Robotic Arm direct opcodes */
        case CMD_SERVO_ARM_UP:
            Servo_LifterStep(+SERVO_STEP_DEG);
            break;

        case CMD_SERVO_ARM_DOWN:
            Servo_LifterStep(-SERVO_STEP_DEG);
            break;

        case CMD_SERVO_ARM_EXTEND:
            Servo_ExtensionStep(+SERVO_STEP_DEG);
            break;

        case CMD_SERVO_ARM_RETRACT:
            Servo_ExtensionStep(-SERVO_STEP_DEG);
            break;

        /* Horn commands - not equipped in App1 */
        case CMD_HORN_ON:
        case CMD_HORN_OFF:
            break;

        default:
            break;
    }
}

void Robot_Poll(void)
{
    CanRxMsg rx;
    if (CAN_Driver_Receive(&rx))
    {
        if (rx.StdId == ID_REQ_BOOT || rx.StdId == ID_QUERY_APP)
        {
            OTA_Trigger_Check(&rx);
        }
        else
        {
            Robot_ProcessCommand(rx.StdId);
        }
    }
}
