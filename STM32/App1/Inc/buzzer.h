/**
 * @file    buzzer.h
 * @brief   Driver điều khiển Passive Buzzer (còi chip bị động) trên chân PA0 (TIM2_CH1)
 *          cho STM32F103 (chuẩn SPL - Standard Peripheral Library).
 */

#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>
#include "stm32f10x.h"

/* Cấu hình phần cứng PA0 - TIM2_CH1 */
#define BUZZER_PIN               GPIO_Pin_0
#define BUZZER_PORT              GPIOA
#define BUZZER_GPIO_CLK          RCC_APB2Periph_GPIOA
#define BUZZER_TIM               TIM2
#define BUZZER_TIM_CLK           RCC_APB1Periph_TIM2

/* Tần số cơ bản thông dụng của còi chíp */
#define BUZZER_FREQ_DEFAULT      2700U  /* Tần số cộng hưởng tiêu chuẩn (2.7 kHz) */
#define BUZZER_FREQ_BEEP         2000U  /* Tiếng beep đanh gọn (2.0 kHz) */
#define BUZZER_FREQ_ALERT        4000U  /* Tiếng cảnh báo khẩn cấp (4.0 kHz) */

/* Danh sách nốt nhạc chuẩn (Hz) để tạo giai điệu */
#define NOTE_C4                  262U
#define NOTE_D4                  294U
#define NOTE_E4                  330U
#define NOTE_F4                  349U
#define NOTE_G4                  392U
#define NOTE_A4                  440U
#define NOTE_B4                  494U
#define NOTE_C5                  523U
#define NOTE_D5                  587U
#define NOTE_E5                  659U
#define NOTE_F5                  698U
#define NOTE_G5                  784U
#define NOTE_A5                  880U
#define NOTE_B5                  988U
#define NOTE_C6                  1047U

/**
 * @brief  Khởi tạo ngoại vi PA0 (Alternate Function Push-Pull) và Timer 2 Channel 1 ở chế độ PWM.
 */
void Buzzer_Init(void);

/**
 * @brief  Phát âm thanh liên tục với tần số và duty cycle tùy chỉnh (Non-blocking).
 * @param  freq_hz: Tần số âm thanh mong muốn (20Hz - 20000Hz). Nếu bằng 0 sẽ tắt còi.
 * @param  duty_percent: Độ rộng xung (1 - 100%). Chuẩn 50% cho âm lượng to và rõ nhất.
 */
void Buzzer_SetTone(uint16_t freq_hz, uint8_t duty_percent);

/**
 * @brief  Tắt buzzer ngay lập tức.
 */
void Buzzer_Stop(void);

/**
 * @brief  Phát một tiếng beep với tần số và thời gian xác định (Blocking qua Delay_Ms).
 * @param  freq_hz: Tần số âm thanh (Hz).
 * @param  duration_ms: Thời gian phát âm thanh (ms).
 */
void Buzzer_Beep(uint16_t freq_hz, uint32_t duration_ms);

/**
 * @brief  Phát một giai điệu ngắn báo hiệu khởi động thành công.
 */
void Buzzer_PlayStartup(void);

/**
 * @brief  Khởi tạo SysTick timer phục vụ hàm Delay_Ms (thay thế delay ngắt quãng bằng TIM2).
 */
void Delay_Init(void);

/**
 * @brief  Tạo khoảng trễ theo mili-giây dùng SysTick.
 * @param  u32DelayInMs: Số mili-giây cần chờ.
 */
void Delay_Ms(uint32_t u32DelayInMs);

#endif /* BUZZER_H */

