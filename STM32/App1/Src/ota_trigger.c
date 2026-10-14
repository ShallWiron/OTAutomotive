#include "ota_trigger.h"
#include "stm32f10x_can.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_bkp.h"

void OTA_Trigger_Init(void)
{
    /* 1. Bật Clock CAN1, GPIOA, AFIO */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);

    /* 2. Cấu hình GPIO cho CAN: PA11 = RX (Input Pull-Up), PA12 = TX (AF Push-Pull) */
    GPIO_InitTypeDef gpioInit;
    gpioInit.GPIO_Speed = GPIO_Speed_50MHz;

    gpioInit.GPIO_Pin = GPIO_Pin_11;
    gpioInit.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &gpioInit);

    gpioInit.GPIO_Pin = GPIO_Pin_12;
    gpioInit.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &gpioInit);

    /* 3. Cấu hình CAN Timing 500 kbps (khớp 100% với Bootloader và ESP32) */
    CAN_InitTypeDef canInit;
    CAN_DeInit(CAN1);
    CAN_StructInit(&canInit);
    canInit.CAN_ABOM = ENABLE;
    canInit.CAN_NART = DISABLE;
    canInit.CAN_Mode = CAN_Mode_Normal;
    canInit.CAN_SJW = CAN_SJW_1tq;
    canInit.CAN_BS1 = CAN_BS1_5tq;
    canInit.CAN_BS2 = CAN_BS2_2tq;
    canInit.CAN_Prescaler = 9; /* 36MHz / 9 / (1 + 5 + 2) = 500 kbps */
    CAN_Init(CAN1, &canInit);

    /* 4. Cấu hình Filter nhận các bản tin CAN (chấp nhận cả Control và OTA) */
    CAN_FilterInitTypeDef filterInit;
    filterInit.CAN_FilterNumber = 0;
    filterInit.CAN_FilterMode = CAN_FilterMode_IdMask;
    filterInit.CAN_FilterScale = CAN_FilterScale_32bit;
    filterInit.CAN_FilterIdHigh = 0x0000;
    filterInit.CAN_FilterIdLow = 0x0000;
    filterInit.CAN_FilterMaskIdHigh = 0x0000; /* Không che mask -> nhận tất cả */
    filterInit.CAN_FilterMaskIdLow = 0x0000;
    filterInit.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
    filterInit.CAN_FilterActivation = ENABLE;
    CAN_FilterInit(&filterInit);
}

void OTA_Trigger_Poll(void)
{
    /* Kiểm tra xem FIFO0 có bản tin không */
    if (CAN_MessagePending(CAN1, CAN_FIFO0) == 0)
    {
        return;
    }

    CanRxMsg rx;
    CAN_Receive(CAN1, CAN_FIFO0, &rx);

    /* Kiểm tra ID 0x600 và đúng 4 byte Security Key [0xDE, 0xAD, 0xBE, 0xEF] */
    if (rx.StdId == ID_REQ_BOOT && rx.DLC == 4)
    {
        if (rx.Data[0] == 0xDE && rx.Data[1] == 0xAD &&
            rx.Data[2] == 0xBE && rx.Data[3] == 0xEF)
        {
            /* 1. Gửi phản hồi ACK (0x7C0, Data=0x00) về ESP32 */
            CanTxMsg tx;
            tx.StdId = ID_RESPONSE;
            tx.ExtId = 0;
            tx.RTR = CAN_RTR_DATA;
            tx.IDE = CAN_ID_STD;
            tx.DLC = 1;
            tx.Data[0] = OTA_ACK;

            CAN_Transmit(CAN1, &tx);

            /* Đợi một khoảng ngắn (~10ms) để frame ACK kịp phát ra Bus */
            for (volatile uint32_t i = 0; i < 72000; i++) { __NOP(); }

            /* 2. Mở khóa Backup Domain và ghi cờ Magic Key */
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
            PWR_BackupAccessCmd(ENABLE);
            BKP_WriteBackupRegister(BKP_DR1, BKP_OTA_FLAG_MAGIC);

            /* 3. Soft-Reset MCU sang Bootloader */
            NVIC_SystemReset();
        }
    }
    /* Kiểm tra ID 0x620: ESP32 truy vấn trạng thái App hiện tại */
    else if (rx.StdId == ID_QUERY_APP)
    {
        CanTxMsg tx;
        tx.StdId = ID_RESPONSE;
        tx.ExtId = 0;
        tx.RTR = CAN_RTR_DATA;
        tx.IDE = CAN_ID_STD;
        tx.DLC = 2;
        tx.Data[0] = OTA_ACK;
        tx.Data[1] = CURRENT_APP_ID;

        CAN_Transmit(CAN1, &tx);
    }
}

