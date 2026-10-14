#include "can.h"
#include "uart.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_can.h"

void CAN_Setup(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef GPIO_Structure;
    GPIO_Structure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Structure.GPIO_Pin = GPIO_Pin_11; // RX
    GPIO_Structure.GPIO_Mode = GPIO_Mode_IPU; // Input Pull-Up
    GPIO_Init(GPIOA, &GPIO_Structure);
    GPIO_Structure.GPIO_Pin = GPIO_Pin_12; // TX
    GPIO_Structure.GPIO_Mode = GPIO_Mode_AF_PP; // Alternate Function Push-Pull
    GPIO_Init(GPIOA, &GPIO_Structure);

    CAN_InitTypeDef CAN_Structure;
    CAN_DeInit(CAN1);
    CAN_StructInit(&CAN_Structure);
    CAN_Structure.CAN_ABOM = ENABLE;  // Tự phục hồi khi Bus-Off
    CAN_Structure.CAN_NART = DISABLE; // Tự gửi lại khi lỗi
    CAN_Structure.CAN_Mode = CAN_Mode_Normal;
    CAN_Structure.CAN_SJW = CAN_SJW_1tq;
    CAN_Structure.CAN_BS1 = CAN_BS1_5tq;
    CAN_Structure.CAN_BS2 = CAN_BS2_2tq;
    CAN_Structure.CAN_Prescaler = 9;  // 36MHz / 9 / (1 + 5 + 2) = 500 kbps
    CAN_Init(CAN1, &CAN_Structure);

    CAN_FilterInitTypeDef Filter_Structure;
    // Filter 0: Luồng Control (2 bit đầu [10:9] = 00) -> FIFO 0
    Filter_Structure.CAN_FilterNumber = 0;
    Filter_Structure.CAN_FilterMode = CAN_FilterMode_IdMask;
    Filter_Structure.CAN_FilterScale = CAN_FilterScale_32bit;
    Filter_Structure.CAN_FilterIdHigh = 0x0000;
    Filter_Structure.CAN_FilterIdLow = 0x0000;
    Filter_Structure.CAN_FilterMaskIdHigh = (0b11 << 14);
    Filter_Structure.CAN_FilterMaskIdLow = 0x0000;
    Filter_Structure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
    Filter_Structure.CAN_FilterActivation = ENABLE;
    CAN_FilterInit(&Filter_Structure);

    // Filter 1: Luồng OTA (2 bit đầu [10:9] = 11) -> FIFO 1
    Filter_Structure.CAN_FilterNumber = 1;
    Filter_Structure.CAN_FilterMode = CAN_FilterMode_IdMask;
    Filter_Structure.CAN_FilterScale = CAN_FilterScale_32bit;
    Filter_Structure.CAN_FilterIdHigh = (0b11 << 14);
    Filter_Structure.CAN_FilterIdLow = 0x0000;
    Filter_Structure.CAN_FilterMaskIdHigh = (0b11 << 14);
    Filter_Structure.CAN_FilterMaskIdLow = 0x0000;
    Filter_Structure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO1;
    Filter_Structure.CAN_FilterActivation = ENABLE;
    CAN_FilterInit(&Filter_Structure);
}

void CAN_ReceiveMessagePolling(CanRxMsg *RxMsg)
{
    CAN_Receive(CAN1, CAN_FIFO0, RxMsg);
    UART_SendString("Received CAN message: StdId=0x");
    UART_SendHex16((uint16_t)RxMsg->StdId);
    UART_SendString(", Data=");
    UART_SendHexArray8(RxMsg->Data, RxMsg->DLC);
    UART_SendString("\r\n");
}

void CAN_SendMessagePolling(uint32_t ID, const uint8_t *data, uint8_t length)
{
    if (length > 8)
    {
        length = 8;
    }

    CanTxMsg tx;
    tx.StdId = ID;
    tx.ExtId = 0;
    tx.RTR = CAN_RTR_DATA;
    tx.IDE = CAN_ID_STD;
    tx.DLC = length;

    if (data != 0)
    {
        for (uint8_t i = 0; i < length; i++)
        {
            tx.Data[i] = data[i];
        }
    }

    uint8_t mbox = CAN_Transmit(CAN1, &tx);
    uint32_t wait_mbox = 10000;
    while (mbox == CAN_TxStatus_NoMailBox && wait_mbox--)
    {
        mbox = CAN_Transmit(CAN1, &tx);
    }

    if (mbox != CAN_TxStatus_NoMailBox)
    {
        uint32_t timeout = 100000;
        while (CAN_TransmitStatus(CAN1, mbox) != CAN_TxStatus_Ok && timeout--);
    }
}

void CAN_SendArray(const uint8_t *data, uint16_t length)
{
    uint8_t start_cmd = 0xAA;
    CAN_SendMessagePolling(0x100, &start_cmd, 1);

    for (volatile int i = 0; i < 20000; i++);

    uint16_t sent = 0;
    while (sent < length)
    {
        uint8_t chunk_len = (length - sent) > 8 ? 8 : (length - sent);
        CAN_SendMessagePolling(0x101, &data[sent], chunk_len);
        sent += chunk_len;

        for (volatile int i = 0; i < 15000; i++);
    }

    uint8_t end_cmd = 0xBB;
    CAN_SendMessagePolling(0x102, &end_cmd, 1);
}

uint8_t CAN_ReceiveArray(uint8_t *buffer, uint16_t *out_length, uint32_t timeout_loops)
{
    uint8_t is_receiving = 0;
    uint16_t index = 0;
    uint32_t timeout = timeout_loops;

    while (timeout > 0)
    {
        if (CAN_MessagePending(CAN1, CAN_FIFO0) > 0)
        {
            CanRxMsg rx;
            CAN_Receive(CAN1, CAN_FIFO0, &rx);

            if (rx.StdId == 0x100 && rx.Data[0] == 0xAA)
            {
                is_receiving = 1;
                index = 0;
            }
            else if (rx.StdId == 0x101 && is_receiving == 1)
            {
                for (int i = 0; i < rx.DLC; i++)
                {
                    buffer[index++] = rx.Data[i];
                }
            }
            else if (rx.StdId == 0x102 && rx.Data[0] == 0xBB && is_receiving == 1)
            {
                *out_length = index;
                return 1;
            }

            timeout = timeout_loops;
        }
        else
        {
            timeout--;
        }
    }
    return 0;
}
