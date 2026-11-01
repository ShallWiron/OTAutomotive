/*============================================================================
 *  ROBOT XE TAY GAP 3 BAC - PHIEN BAN TOI GIAN CHO HOC TAP
 *  - Dieu khien dong co DC (L298N): PB12, PB13, PB14, PB15
 *  - Dieu khien Servo (TIM3 PWM 50Hz): PA6 (Trai), PA7 (Phai), PB0 (Kep)
 *  - LED trang thai (PC13), Relay (PA4), Buzzer (PA5)
 *  - Giao tiep CAN 500 kbps:
 *      + FIFO 0: Nhan cac lenh dieu khien truc tiep (Motor, Servo, Relay...)
 *      + FIFO 1: Nhan lenh kich hoat OTA Bootloader (0x600)
 *===========================================================================*/

#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "misc.h"
#include "ota_trigger.h"
#include "robot_control.h"

/* ========================================================================
 * 1. CAU HINH THONG SO
 * ======================================================================*/
#define F_TIM3_HZ            72000000UL /* Clock cap cho TIM3 = 72 MHz */

/* Canh tay Servo */
#define ARM_MIN_DEG          0          /* Gioi han goc toi thieu */
#define ARM_MAX_DEG          70         /* Gioi han goc toi da    */
#define ARM_DELAY_FAST_MS    20UL       /* Toc do buoc servo nang/ha (20ms/do) */
#define ARM_DELAY_SLOW_MS    40UL       /* Toc do buoc servo keo dai/thu (40ms/do) */
#define GRIP_OPEN_DEG        90         /* Goc mo kep  */
#define GRIP_CLOSE_DEG       0          /* Goc dong kep */

/* Do rong xung PWM Servo (544 us ~ 0 do, 2400 us ~ 180 do) */
#define SERVO_MIN_US         544L
#define SERVO_MAX_US         2400L

#define RELAY_ACTIVE_HIGH    1          /* 1: Relay muc CAO, 0: Relay muc THAP */

/* ========================================================================
 * 2. KHAI BAO CHAN PHAN CUNG
 * ======================================================================*/
/* Dong co L298N - GPIOB */
#define MOTOR_PORT           GPIOB
#define MA1_PIN              GPIO_Pin_12    /* IN1: Banh TRAI tien  (motorA1) */
#define MA2_PIN              GPIO_Pin_13    /* IN2: Banh TRAI lui   (motorA2) */
#define MB1_PIN              GPIO_Pin_14    /* IN3: Banh PHAI tien  (motorB1) */
#define MB2_PIN              GPIO_Pin_15    /* IN4: Banh PHAI lui   (motorB2) */
#define MOTOR_MASK           (MA1_PIN | MA2_PIN | MB1_PIN | MB2_PIN)

/* Servo - TIM3 (PWM 50 Hz) */
#define SERVO_TRAI           0              /* TIM3_CH1 = PA6 : Nang / ha tay     */
#define SERVO_PHAI           1              /* TIM3_CH2 = PA7 : Keo dai / thu tay */
#define SERVO_KEP            2              /* TIM3_CH3 = PB0 : Kep               */

/* Ngoai vi phu tro */
#define LED_PORT             GPIOC
#define LED_PIN              GPIO_Pin_13    /* LED tren board Blue Pill (Active LOW) */
#define RELAY_PORT           GPIOA
#define RELAY_PIN            GPIO_Pin_4
#define BUZZER_PORT          GPIOA
#define BUZZER_PIN           GPIO_Pin_0    /* Noi vao PA0 theo so do */

/* ========================================================================
 * 3. BIEN TOAN CUC
 * ======================================================================*/
volatile uint32_t g_ms = 0U;                /* Tang moi 1 ms boi SysTick_Handler */

static uint8_t  DOF_C                  = 0U; /* 0: Che do banh xe, 1: Che do tay   */

/* Trang thai Servo */
static uint16_t current_arm_cmd        = CMD_SERVO_STOP;
static int      step_servo_Trai        = 0;  /* Goc hien tai servo nang/ha (PA6)   */
static int      step_servo_Phai        = 0;  /* Goc hien tai servo duoi/thu (PA7)  */
static uint32_t arm_trai_tick          = 0U; /* Thoi diem buoc servo Trai          */
static uint32_t arm_phai_tick          = 0U; /* Thoi diem buoc servo Phai          */

/* Trang thai Buzzer */
static uint8_t  buzzer_active          = 0U;
static uint32_t buzzer_end_ms          = 0U;

/* ========================================================================
 * 4. HAM TIEN ICH GPIO & DIEU KHIEN DONG CO
 * ======================================================================*/
void Pin_Init(GPIO_TypeDef *port, uint16_t pin, GPIOMode_TypeDef mode, GPIOSpeed_TypeDef speed)
{
    GPIO_InitTypeDef g;
    g.GPIO_Pin   = pin;
    g.GPIO_Mode  = mode;
    g.GPIO_Speed = speed;
    GPIO_Init(port, &g);
}

/**
 * @brief  Dieu khien trang thai 4 chan cau H L298N
 *         a1, a2: Dong co trai | b1, b2: Dong co phai
 */
void Motor_Set(uint8_t a1, uint8_t a2, uint8_t b1, uint8_t b2)
{
    uint16_t v = (uint16_t)(GPIO_ReadOutputData(MOTOR_PORT) & (uint16_t)(~MOTOR_MASK));

    if (a1) { v |= MA1_PIN; }
    if (a2) { v |= MA2_PIN; }
    if (b1) { v |= MB1_PIN; }
    if (b2) { v |= MB2_PIN; }

    GPIO_Write(MOTOR_PORT, v);
}

void Motor_Stop(void)
{
    Motor_Set(0U, 0U, 0U, 0U);
}

void Motor_Forward(void)
{
    Motor_Set(1U, 0U, 1U, 0U);
}

void Motor_Backward(void)
{
    Motor_Set(0U, 1U, 0U, 1U);
}

void Motor_TurnLeft(void)
{
    Motor_Set(1U, 0U, 0U, 1U);
}

void Motor_TurnRight(void)
{
    Motor_Set(0U, 1U, 1U, 0U);
}

void Led_Set(uint8_t on)
{
    if (on) { GPIO_ResetBits(LED_PORT, LED_PIN); }
    else    { GPIO_SetBits(LED_PORT, LED_PIN); }
}

void Relay_Set(uint8_t on)
{
#if RELAY_ACTIVE_HIGH
    if (on) { GPIO_SetBits(RELAY_PORT, RELAY_PIN); }
    else    { GPIO_ResetBits(RELAY_PORT, RELAY_PIN); }
#else
    if (on) { GPIO_ResetBits(RELAY_PORT, RELAY_PIN); }
    else    { GPIO_SetBits(RELAY_PORT, RELAY_PIN); }
#endif
}

void Buzzer_Beep(uint32_t ms)
{
    GPIO_SetBits(BUZZER_PORT, BUZZER_PIN);
    buzzer_active = 1U;
    buzzer_end_ms = g_ms + ms;
}

/* ========================================================================
 * 5. DIEU KHIEN SERVO (TIM3 - PWM 50 Hz)
 * ======================================================================*/
static uint16_t Angle_To_Us(int angle)
{
    if (angle < 0)   { angle = 0; }
    if (angle > 180) { angle = 180; }
    return (uint16_t)(SERVO_MIN_US + ((int32_t)angle * (SERVO_MAX_US - SERVO_MIN_US)) / 180L);
}

void Servo_Write(uint8_t ch, int angle)
{
    uint16_t us = Angle_To_Us(angle);

    if      (ch == SERVO_TRAI) { TIM_SetCompare1(TIM3, us); }
    else if (ch == SERVO_PHAI) { TIM_SetCompare2(TIM3, us); }
    else if (ch == SERVO_KEP)  { TIM_SetCompare3(TIM3, us); }
}

void Servo_Init(void)
{
    TIM_TimeBaseInitTypeDef tb;
    TIM_OCInitTypeDef       oc;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    Pin_Init(GPIOA, GPIO_Pin_6, GPIO_Mode_AF_PP, GPIO_Speed_50MHz);   /* PA6 = TIM3_CH1 */
    Pin_Init(GPIOA, GPIO_Pin_7, GPIO_Mode_AF_PP, GPIO_Speed_50MHz);   /* PA7 = TIM3_CH2 */
    Pin_Init(GPIOB, GPIO_Pin_0, GPIO_Mode_AF_PP, GPIO_Speed_50MHz);   /* PB0 = TIM3_CH3 */

    /* Clock timer: 72 MHz / 72 = 1 MHz (1 tick = 1 us), period 20000 us = 50 Hz */
    TIM_TimeBaseStructInit(&tb);
    tb.TIM_Prescaler     = (uint16_t)(F_TIM3_HZ / 1000000UL - 1UL);
    tb.TIM_Period        = 20000U - 1U;
    tb.TIM_ClockDivision = TIM_CKD_DIV1;
    tb.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &tb);

    /* PWM mode 1, khoi dau o 0 do */
    TIM_OCStructInit(&oc);
    oc.TIM_OCMode      = TIM_OCMode_PWM1;
    oc.TIM_OutputState = TIM_OutputState_Enable;
    oc.TIM_OCPolarity  = TIM_OCPolarity_High;
    oc.TIM_Pulse       = Angle_To_Us(0);

    TIM_OC1Init(TIM3, &oc);
    TIM_OC2Init(TIM3, &oc);
    TIM_OC3Init(TIM3, &oc);

    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM3, ENABLE);

    TIM_Cmd(TIM3, ENABLE);
}

void Arm_Step(uint8_t ch, int *angle, int dir, uint32_t delay_ms)
{
    (void)delay_ms;
    if (dir > 0) { if (*angle < ARM_MAX_DEG) { (*angle)++; } }
    else         { if (*angle > ARM_MIN_DEG) { (*angle)--; } }
    Servo_Write(ch, *angle);
}

/* ========================================================================
 * 6. LOGIC DIEU KHIEN CHUYEN DONG
 * ======================================================================*/
void Robot_ExecuteCommand(uint16_t can_id)
{
    /* 1. Nhom 0: Chuyen che do va ngoai vi phu tro */
    if (can_id == CMD_MODE_MOTOR)
    {
        DOF_C = 0U;
        Motor_Stop();
        current_arm_cmd = CMD_SERVO_STOP;
        Buzzer_Beep(50U);
        return;
    }
    else if (can_id == CMD_MODE_SERVO)
    {
        DOF_C = 1U;
        Motor_Stop();
        current_arm_cmd = CMD_SERVO_STOP;
        Buzzer_Beep(50U);
        return;
    }
    else if (can_id == CMD_LED_ON)
    {
        Led_Set(1U);
        return;
    }
    else if (can_id == CMD_LED_OFF)
    {
        Led_Set(0U);
        return;
    }
    else if (can_id == CMD_RELAY_ON)
    {
        Relay_Set(1U);
        return;
    }
    else if (can_id == CMD_RELAY_OFF)
    {
        Relay_Set(0U);
        return;
    }
    else if (can_id == CMD_BUZZER_BEEP)
    {
        Buzzer_Beep(100U);
        return;
    }

    /* 2. Dong / Mo kep (kha dung tren ca 2 che do) */
    if (can_id == CMD_SERVO_GRIP_OPEN)
    {
        Servo_Write(SERVO_KEP, GRIP_OPEN_DEG);
        return;
    }
    else if (can_id == CMD_SERVO_GRIP_CLOSE)
    {
        Servo_Write(SERVO_KEP, GRIP_CLOSE_DEG);
        return;
    }

    /* 3. Che do Banh xe (DOF_C == 0) */
    if (!DOF_C)
    {
        switch (can_id)
        {
            case CMD_MOTOR_STOP:
                Motor_Stop();
                break;

            case CMD_MOTOR_FORWARD:
                Motor_Forward();
                break;

            case CMD_MOTOR_BACKWARD:
                Motor_Backward();
                break;

            case CMD_MOTOR_TURN_LEFT:
                Motor_TurnLeft();
                break;

            case CMD_MOTOR_TURN_RIGHT:
                Motor_TurnRight();
                break;

            case CMD_MOTOR_LEFT_FWD:
                Motor_Set(1U, 0U, 0U, 0U);
                break;

            case CMD_MOTOR_LEFT_REV:
                Motor_Set(0U, 1U, 0U, 0U);
                break;

            case CMD_MOTOR_RIGHT_FWD:
                Motor_Set(0U, 0U, 1U, 0U);
                break;

            case CMD_MOTOR_RIGHT_REV:
                Motor_Set(0U, 0U, 0U, 1U);
                break;

            default:
                break;
        }
    }
    /* 4. Che do Tay gap (DOF_C == 1) */
    else
    {
        switch (can_id)
        {
            case CMD_SERVO_STOP:
                current_arm_cmd = CMD_SERVO_STOP;
                break;

            case CMD_SERVO_ARM_UP:
            case CMD_SERVO_ARM_DOWN:
            case CMD_SERVO_ARM_EXTEND:
            case CMD_SERVO_ARM_RETRACT:
                current_arm_cmd = can_id;
                break;

            /* Cho phep dung xe ngay neu nhan lenh dung */
            case CMD_MOTOR_STOP:
                Motor_Stop();
                break;

            default:
                break;
        }
    }
}

/**
 * @brief  Cap nhat cac tien trinh dinh ky:
 *         - Buoc chay lien tuc cua servo khi giu nut
 *         - Tu dong tat coi Buzzer sau khi het thoi gian
 */
void Robot_Poll(void)
{
    /* 1. Tu dong tat tieng coi Buzzer sau khi het thoi gian */
    if (buzzer_active)
    {
        if ((int32_t)(g_ms - buzzer_end_ms) >= 0)
        {
            GPIO_ResetBits(BUZZER_PORT, BUZZER_PIN);
            buzzer_active = 0U;
        }
    }

    /* 2. Cap nhat chuyen dong servo khi nguoi dung dang nhan giu nut */
    if (DOF_C && (current_arm_cmd != CMD_SERVO_STOP))
    {
        switch (current_arm_cmd)
        {
            case CMD_SERVO_ARM_UP:
                if ((uint32_t)(g_ms - arm_trai_tick) >= ARM_DELAY_FAST_MS)
                {
                    arm_trai_tick = g_ms;
                    if (step_servo_Trai < ARM_MAX_DEG)
                    {
                        step_servo_Trai++;
                        Servo_Write(SERVO_TRAI, step_servo_Trai);
                    }
                }
                break;

            case CMD_SERVO_ARM_DOWN:
                if ((uint32_t)(g_ms - arm_trai_tick) >= ARM_DELAY_SLOW_MS)
                {
                    arm_trai_tick = g_ms;
                    if (step_servo_Trai > ARM_MIN_DEG)
                    {
                        step_servo_Trai--;
                        Servo_Write(SERVO_TRAI, step_servo_Trai);
                    }
                }
                break;

            case CMD_SERVO_ARM_EXTEND:
                if ((uint32_t)(g_ms - arm_phai_tick) >= ARM_DELAY_SLOW_MS)
                {
                    arm_phai_tick = g_ms;
                    if (step_servo_Phai < ARM_MAX_DEG)
                    {
                        step_servo_Phai++;
                        Servo_Write(SERVO_PHAI, step_servo_Phai);
                    }
                }
                break;

            case CMD_SERVO_ARM_RETRACT:
                if ((uint32_t)(g_ms - arm_phai_tick) >= ARM_DELAY_SLOW_MS)
                {
                    arm_phai_tick = g_ms;
                    if (step_servo_Phai > ARM_MIN_DEG)
                    {
                        step_servo_Phai--;
                        Servo_Write(SERVO_PHAI, step_servo_Phai);
                    }
                }
                break;

            default:
                break;
        }
    }
}

/**
 * @brief  Kiem tra va tiep nhan ban tin CAN dieu khien tu FIFO0 (non-blocking)
 */
void Robot_Control_Poll(void)
{
    if (CAN_MessagePending(CAN1, CAN_FIFO0) == 0)
    {
        return;
    }

    CanRxMsg rx;
    CAN_Receive(CAN1, CAN_FIFO0, &rx);

    Robot_ExecuteCommand(rx.StdId);
}

void Delay_Ms(uint32_t ms)
{
    uint32_t start = g_ms;
    while ((uint32_t)(g_ms - start) < ms)
    {
        Robot_Control_Poll();
        OTA_Trigger_Poll();
    }
}

/* ========================================================================
 * 7. KHOI TAO HE THONG
 * ======================================================================*/
void Robot_Init(void)
{
    /* 1. Ngat SysTick 1 ms */
    SysTick_Config(SystemCoreClock / 1000UL);

    /* 2. Bat Clock cac ngoai vi GPIO */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
                           RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);

    /* 3. Khoi tao 4 chan dong co L298N (muc 0 truoc khi xuat output) */
    GPIO_ResetBits(MOTOR_PORT, MOTOR_MASK);
    Pin_Init(MOTOR_PORT, MOTOR_MASK, GPIO_Mode_Out_PP, GPIO_Speed_50MHz);

    /* 4. Khoi tao LED, Relay, Buzzer */
    Led_Set(0U);
    Pin_Init(LED_PORT, LED_PIN, GPIO_Mode_Out_PP, GPIO_Speed_2MHz);

    Relay_Set(0U);
    Pin_Init(RELAY_PORT, RELAY_PIN, GPIO_Mode_Out_PP, GPIO_Speed_2MHz);

    GPIO_ResetBits(BUZZER_PORT, BUZZER_PIN);
    Pin_Init(BUZZER_PORT, BUZZER_PIN, GPIO_Mode_Out_PP, GPIO_Speed_2MHz);

    /* 5. Khoi tao Servo (ca 3 servo ve 0 do) */
    Servo_Init();
    step_servo_Trai   = 0;
    step_servo_Phai   = 0;
    current_arm_cmd   = CMD_SERVO_STOP;
    Servo_Write(SERVO_TRAI, 0);
    Servo_Write(SERVO_PHAI, 0);
    Servo_Write(SERVO_KEP, 0);

    /* 6. Khoi tao ngoai vi CAN va bo loc phan luong (FIFO0: Control, FIFO1: OTA) */
    OTA_Trigger_Init();

    /* 7. Am thanh pip khoi dong he thong thanh cong */
    Buzzer_Beep(100U);
}

/* ========================================================================
 * 8. CHUONG TRINH CHINH
 * ======================================================================*/
int main(void)
{
    Robot_Init();

    /* Chay vong lap he thong */
   while (1)
    {
        /* 1. Kiem tra va thuc thi ban tin CAN dieu khien tu FIFO0 */
        Robot_Control_Poll();

        /* 2. Cap nhat buoc servo va buzzer */
        Robot_Poll();

        /* 3. Thuc hien kiem tra frame OTA tu FIFO1 (non-blocking) */
        OTA_Trigger_Poll();
    
    }
}
