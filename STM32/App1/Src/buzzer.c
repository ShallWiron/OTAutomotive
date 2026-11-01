/**
 * @file    buzzer.c
 * @brief   Hiện thực driver điều khiển Passive Buzzer qua Hardware PWM (TIM2_CH1)
 *          và hàm trễ chuẩn SysTick cho STM32F103.
 */

#include "buzzer.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"

/* ----------------- HÀM TẠO TRỄ SYSTICK ----------------- */

void Delay_Init(void)
{
    /* Cấu hình SysTick tạo chu kỳ 1ms tại tần số nhịp hệ thống (72MHz) */
    SysTick->LOAD = (SystemCoreClock / 1000U) - 1U;
    SysTick->VAL = 0U;
    /* Sử dụng xung lõi CPU, không bật ngắt (polling cờ COUNTFLAG), bật counter */
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
}

void Delay_Ms(uint32_t u32DelayInMs)
{
    while (u32DelayInMs > 0U) {
        SysTick->VAL = 0U;
        /* Đọc CTRL để xóa cờ COUNTFLAG trước khi chờ */
        (void)SysTick->CTRL;
        while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0U) {
            /* Chờ đúng 1ms */
        }
        --u32DelayInMs;
    }
}

/* ----------------- DRIVER PASSIVE BUZZER ----------------- */

void Buzzer_Init(void)
{
    GPIO_InitTypeDef gpioInit;
    TIM_TimeBaseInitTypeDef timerInit;
    TIM_OCInitTypeDef ocInit;

    /* 1. Cấp clock cho GPIOA và TIM2 */
    RCC_APB2PeriphClockCmd(BUZZER_GPIO_CLK, ENABLE);
    RCC_APB1PeriphClockCmd(BUZZER_TIM_CLK, ENABLE);

    /* 2. Cấu hình chân PA0 chế độ Alternate Function Push-Pull (AF_PP) */
    gpioInit.GPIO_Pin = BUZZER_PIN;
    gpioInit.GPIO_Mode = GPIO_Mode_AF_PP;
    gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(BUZZER_PORT, &gpioInit);

    /* 3. Cấu hình Timer Base cho TIM2 với tần số đếm 1MHz (1 us / tick) */
    TIM_TimeBaseStructInit(&timerInit);
    timerInit.TIM_CounterMode = TIM_CounterMode_Up;
    timerInit.TIM_Prescaler = 72U - 1U;            /* 72 MHz / 72 = 1 MHz */
    timerInit.TIM_Period = 1000U - 1U;             /* Mặc định 1kHz */
    timerInit.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(BUZZER_TIM, &timerInit);

    /* 4. Cấu hình Kênh 1 (TIM2_CH1) ở chế độ PWM Mode 1 */
    TIM_OCStructInit(&ocInit);
    ocInit.TIM_OCMode = TIM_OCMode_PWM1;
    ocInit.TIM_OutputState = TIM_OutputState_Enable;
    ocInit.TIM_Pulse = 0U;                         /* Mặc định tắt (0% duty) */
    ocInit.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC1Init(BUZZER_TIM, &ocInit);

    /* Bật tính năng Preload cho thanh ghi ARR và CCR1 */
    TIM_OC1PreloadConfig(BUZZER_TIM, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(BUZZER_TIM, ENABLE);

    /* Kích hoạt TIM2 */
    TIM_Cmd(BUZZER_TIM, ENABLE);

    /* Đảm bảo ban đầu còi không kêu */
    Buzzer_Stop();
}

void Buzzer_SetTone(uint16_t freq_hz, uint8_t duty_percent)
{
    uint32_t period_us;
    uint32_t pulse_us;

    if ((freq_hz == 0U) || (duty_percent == 0U)) {
        Buzzer_Stop();
        return;
    }

    if (duty_percent > 100U) {
        duty_percent = 100U;
    }

    /* Tính chu kỳ Period (tương ứng giá trị nạp thanh ghi ARR) */
    /* Với timer chạy ở 1MHz: period_us = 1.000.000 / freq_hz */
    period_us = 1000000UL / freq_hz;
    if (period_us > 65536UL) {
        period_us = 65536UL;
    }
    if (period_us < 2UL) {
        period_us = 2UL;
    }

    /* Tính độ rộng xung tương ứng duty cycle */
    pulse_us = (period_us * duty_percent) / 100UL;

    /* Cập nhật tần số và duty cycle */
    TIM_SetAutoreload(BUZZER_TIM, (uint16_t)(period_us - 1U));
    TIM_SetCompare1(BUZZER_TIM, (uint16_t)pulse_us);

    /* Bật ngõ ra Channel 1 */
    TIM_CCxCmd(BUZZER_TIM, TIM_Channel_1, TIM_CCx_Enable);
    TIM_Cmd(BUZZER_TIM, ENABLE);
}

void Buzzer_Stop(void)
{
    TIM_SetCompare1(BUZZER_TIM, 0U);
    TIM_CCxCmd(BUZZER_TIM, TIM_Channel_1, TIM_CCx_Disable);
}

void Buzzer_Beep(uint16_t freq_hz, uint32_t duration_ms)
{
    Buzzer_SetTone(freq_hz, 50U);
    Delay_Ms(duration_ms);
    Buzzer_Stop();
}

void Buzzer_PlayStartup(void)
{
    /* Chuỗi nốt khởi động vui tai: Do (C5) -> Mi (E5) -> Sol (G5) */
    Buzzer_Beep(NOTE_C5, 80U);
    Delay_Ms(40U);
    Buzzer_Beep(NOTE_E5, 80U);
    Delay_Ms(40U);
    Buzzer_Beep(NOTE_G5, 120U);
}

