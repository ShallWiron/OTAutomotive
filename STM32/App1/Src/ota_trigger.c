#include "ota_trigger.h"
#include "stm32f10x_can.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_bkp.h"

void OTA_Trigger_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
}

void OTA_Trigger_Check(CanRxMsg *rx)
{
    if (rx == 0) return;

    /* 0x600: Bootloader request */
    if (rx->StdId == ID_REQ_BOOT && rx->DLC == 4)
    {
        if (rx->Data[0] == 0xDE && rx->Data[1] == 0xAD &&
            rx->Data[2] == 0xBE && rx->Data[3] == 0xEF)
        {
            CanTxMsg tx;
            tx.StdId = ID_RESPONSE;
            tx.ExtId = 0;
            tx.RTR = CAN_RTR_DATA;
            tx.IDE = CAN_ID_STD;
            tx.DLC = 1;
            tx.Data[0] = OTA_ACK;
            CAN_Transmit(CAN1, &tx);

            for (volatile uint32_t i = 0; i < 72000; i++) { __NOP(); }

            PWR_BackupAccessCmd(ENABLE);
            BKP_WriteBackupRegister(BKP_DR1, BKP_OTA_FLAG_MAGIC);

            NVIC_SystemReset();
        }
    }
    /* 0x620: Current App query */
    else if (rx->StdId == ID_QUERY_APP)
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
