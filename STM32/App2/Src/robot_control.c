#include "robot_control.h"
#include "motor.h"
#include "servo.h"
#include "buzzer.h"
#include "led.h"
#include "oled.h"
#include "can_driver.h"
#include "ota_trigger.h"
#include "delay.h"

static uint8_t s_dofMode = 0;
static uint16_t s_lastMovementCmd = CMD_MOTOR_STOP;

void Robot_Init(void)
{
    Delay_Init();
    LED_Init();
    Buzzer_Init();
    Motor_Init();
    Servo_Init();
    OLED_Init();
    CAN_Driver_Init();
    OTA_Trigger_Init();

    Buzzer_BeepStart();
    OLED_ShowWaitingWiFi();
}

void Robot_ProcessCommand(uint16_t cmdId)
{
    switch (cmdId)
    {
        case CMD_EMERGENCY_STOP:
            Motor_Stop();
            LED_Off();
            Buzzer_HornOff();
            s_lastMovementCmd = CMD_EMERGENCY_STOP;
            OLED_ShowState(s_dofMode, CMD_EMERGENCY_STOP, Buzzer_IsHornActive());
            break;

        case CMD_MODE_MOTOR:
            s_dofMode = 0;
            Motor_Stop();
            s_lastMovementCmd = CMD_MOTOR_STOP;
            OLED_ShowState(0, CMD_MOTOR_STOP, Buzzer_IsHornActive());
            break;

        case CMD_MODE_SERVO:
            s_dofMode = 1;
            Motor_Stop();
            s_lastMovementCmd = CMD_SERVO_STOP;
            OLED_ShowState(1, CMD_SERVO_STOP, Buzzer_IsHornActive());
            break;

        case CMD_LED_ON:
            LED_On();
            break;

        case CMD_LED_OFF:
            LED_Off();
            break;

        case CMD_WIFI_CONNECTED:
            Buzzer_BeepSuccess();
            OLED_ShowConnectedSuccess();
            Delay_Ms(1200);
            OLED_ShowState(s_dofMode, s_lastMovementCmd, Buzzer_IsHornActive());
            break;

        case CMD_MOTOR_STOP:
            Motor_Stop();
            s_lastMovementCmd = (s_dofMode == 0) ? CMD_MOTOR_STOP : CMD_SERVO_STOP;
            OLED_ShowState(s_dofMode, s_lastMovementCmd, Buzzer_IsHornActive());
            break;

        case CMD_MOTOR_FORWARD:
            if (s_dofMode == 0) {
                Motor_Forward();
                s_lastMovementCmd = CMD_MOTOR_FORWARD;
            } else {
                Servo_LifterStep(+SERVO_STEP_DEG);
                s_lastMovementCmd = CMD_SERVO_ARM_UP;
            }
            OLED_ShowState(s_dofMode, s_lastMovementCmd, Buzzer_IsHornActive());
            break;

        case CMD_MOTOR_BACKWARD:
            if (s_dofMode == 0) {
                Motor_Backward();
                s_lastMovementCmd = CMD_MOTOR_BACKWARD;
            } else {
                Servo_LifterStep(-SERVO_STEP_DEG);
                s_lastMovementCmd = CMD_SERVO_ARM_DOWN;
            }
            OLED_ShowState(s_dofMode, s_lastMovementCmd, Buzzer_IsHornActive());
            break;

        case CMD_MOTOR_TURN_LEFT:
            if (s_dofMode == 0) {
                Motor_TurnLeft();
                s_lastMovementCmd = CMD_MOTOR_TURN_LEFT;
            } else {
                Servo_ExtensionStep(+SERVO_STEP_DEG);
                s_lastMovementCmd = CMD_SERVO_ARM_EXTEND;
            }
            OLED_ShowState(s_dofMode, s_lastMovementCmd, Buzzer_IsHornActive());
            break;

        case CMD_MOTOR_TURN_RIGHT:
            if (s_dofMode == 0) {
                Motor_TurnRight();
                s_lastMovementCmd = CMD_MOTOR_TURN_RIGHT;
            } else {
                Servo_ExtensionStep(-SERVO_STEP_DEG);
                s_lastMovementCmd = CMD_SERVO_ARM_RETRACT;
            }
            OLED_ShowState(s_dofMode, s_lastMovementCmd, Buzzer_IsHornActive());
            break;

        case CMD_MOTOR_LEFT_FWD:
            Motor_LeftFwd();
            s_lastMovementCmd = CMD_MOTOR_LEFT_FWD;
            OLED_ShowState(0, CMD_MOTOR_LEFT_FWD, Buzzer_IsHornActive());
            break;

        case CMD_MOTOR_LEFT_REV:
            Motor_LeftRev();
            s_lastMovementCmd = CMD_MOTOR_LEFT_REV;
            OLED_ShowState(0, CMD_MOTOR_LEFT_REV, Buzzer_IsHornActive());
            break;

        case CMD_MOTOR_RIGHT_FWD:
            Motor_RightFwd();
            s_lastMovementCmd = CMD_MOTOR_RIGHT_FWD;
            OLED_ShowState(0, CMD_MOTOR_RIGHT_FWD, Buzzer_IsHornActive());
            break;

        case CMD_MOTOR_RIGHT_REV:
            Motor_RightRev();
            s_lastMovementCmd = CMD_MOTOR_RIGHT_REV;
            OLED_ShowState(0, CMD_MOTOR_RIGHT_REV, Buzzer_IsHornActive());
            break;

        case CMD_SERVO_STOP:
            s_lastMovementCmd = CMD_SERVO_STOP;
            OLED_ShowState(1, CMD_SERVO_STOP, Buzzer_IsHornActive());
            break;

        case CMD_SERVO_ARM_UP:
            Servo_LifterStep(+SERVO_STEP_DEG);
            s_lastMovementCmd = CMD_SERVO_ARM_UP;
            OLED_ShowState(1, CMD_SERVO_ARM_UP, Buzzer_IsHornActive());
            break;

        case CMD_SERVO_ARM_DOWN:
            Servo_LifterStep(-SERVO_STEP_DEG);
            s_lastMovementCmd = CMD_SERVO_ARM_DOWN;
            OLED_ShowState(1, CMD_SERVO_ARM_DOWN, Buzzer_IsHornActive());
            break;

        case CMD_SERVO_ARM_EXTEND:
            Servo_ExtensionStep(+SERVO_STEP_DEG);
            s_lastMovementCmd = CMD_SERVO_ARM_EXTEND;
            OLED_ShowState(1, CMD_SERVO_ARM_EXTEND, Buzzer_IsHornActive());
            break;

        case CMD_SERVO_ARM_RETRACT:
            Servo_ExtensionStep(-SERVO_STEP_DEG);
            s_lastMovementCmd = CMD_SERVO_ARM_RETRACT;
            OLED_ShowState(1, CMD_SERVO_ARM_RETRACT, Buzzer_IsHornActive());
            break;

        case CMD_HORN_ON:
            Buzzer_HornOn();
            OLED_ShowState(s_dofMode, s_lastMovementCmd, 1);
            break;

        case CMD_HORN_OFF:
            Buzzer_HornOff();
            OLED_ShowState(s_dofMode, s_lastMovementCmd, 0);
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
