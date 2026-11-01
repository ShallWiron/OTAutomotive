#include "stm32f10x.h"
#include "ota_trigger.h"
#include "buzzer.h"

int main(void)
{
	GPIO_InitTypeDef gpioInit;
	
	/* Khởi tạo hàm trễ SysTick chuẩn Cortex-M3 */
	Delay_Init();
	
	/* Cấp clock cho GPIO Port B và C */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	/* Cấu hình LED báo trạng thái PC13 */
	gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioInit.GPIO_Pin = GPIO_Pin_13;
	gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &gpioInit);
	
	/* Cấu hình các chân điều khiển động cơ PB12-PB15 */
	gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioInit.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &gpioInit);
	
	/* Khởi tạo ngoại vi điều khiển Buzzer bị động trên chân PA0 (TIM2_CH1 PWM) */
	Buzzer_Init();
	
	/* Khởi tạo lắng nghe lệnh OTA qua CAN */
	OTA_Trigger_Init();
	
	/* Phát âm báo khởi động App 1 thành công */
	Buzzer_PlayStartup();
	
	while (1) {
		OTA_Trigger_Poll();
		GPIO_SetBits(GPIOC, GPIO_Pin_13);
		Delay_Ms(500);
		
		OTA_Trigger_Poll();
		GPIO_ResetBits(GPIOC, GPIO_Pin_13);
		/* Beep nhẹ 30ms theo nhịp nhấp nháy LED */
		Buzzer_Beep(BUZZER_FREQ_DEFAULT, 30);
		Delay_Ms(470);
	}
}
