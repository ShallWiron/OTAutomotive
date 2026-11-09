#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>
#include "stm32f10x.h"

#define BUZZER_PORT     GPIOA
#define BUZZER_PIN      GPIO_Pin_0
#define BUZZER_CLK      RCC_APB2Periph_GPIOA

void Buzzer_Init(void);
void Buzzer_HornOn(void);
void Buzzer_HornOff(void);
uint8_t Buzzer_IsHornActive(void);
void Buzzer_Beep(uint32_t duration_ms);
void Buzzer_Tone(uint16_t freq_hz, uint32_t duration_ms);
void Buzzer_BeepStart(void);
void Buzzer_BeepSuccess(void);

#endif /* BUZZER_H */
