#include "can_driver.h"
#include "stm32f10x_can.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

void CAN_Driver_Init(void)
{
    /* 1. Bật Clock CAN1, GPIOA, AFIO */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
    RCC_APB2PeriphClockCmd(CAN_GPIO_CLK | RCC_APB2Periph_AFIO, ENABLE);

    /* 2. Cấu hình GPIO: PA11 = Input Pull-Up, PA12 = Alternate Function Push-Pull */
    GPIO_InitTypeDef gpioInit;
    gpioInit.GPIO_Speed = GPIO_Speed_50MHz;

    gpioInit.GPIO_Pin = CAN_RX_PIN;
    gpioInit.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(CAN_GPIO_PORT, &gpioInit);

    gpioInit.GPIO_Pin = CAN_TX_PIN;
    gpioInit.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(CAN_GPIO_PORT, &gpioInit);

    /* 3. Cấu hình CAN Timing 500 kbps (Clock PCLK1 = 36MHz, Prescaler = 9, 1tq + 5tq + 2tq = 8tq) */
    CAN_InitTypeDef canInit;
    CAN_DeInit(CAN1);
    CAN_StructInit(&canInit);
    canInit.CAN_ABOM = ENABLE;
    canInit.CAN_NART = DISABLE;
    canInit.CAN_Mode = CAN_Mode_Normal;
    canInit.CAN_SJW = CAN_SJW_1tq;
    canInit.CAN_BS1 = CAN_BS1_5tq;
    canInit.CAN_BS2 = CAN_BS2_2tq;
    canInit.CAN_Prescaler = 9;
    CAN_Init(CAN1, &canInit);

    /* 4. Filter mở toàn bộ để nhận cả Control (0x000-0x0FF) và OTA (0x600-0x7FF) */
    CAN_FilterInitTypeDef filterInit;
    filterInit.CAN_FilterNumber = 0;
    filterInit.CAN_FilterMode = CAN_FilterMode_IdMask;
    filterInit.CAN_FilterScale = CAN_FilterScale_32bit;
    filterInit.CAN_FilterIdHigh = 0x0000;
    filterInit.CAN_FilterIdLow = 0x0000;
    filterInit.CAN_FilterMaskIdHigh = 0x0000;
    filterInit.CAN_FilterMaskIdLow = 0x0000;
    filterInit.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
    filterInit.CAN_FilterActivation = ENABLE;
    CAN_FilterInit(&filterInit);
}

uint8_t CAN_Driver_Receive(CanRxMsg *rxMsg)
{
    if (CAN_MessagePending(CAN1, CAN_FIFO0) > 0)
    {
        CAN_Receive(CAN1, CAN_FIFO0, rxMsg);
        return 1;
    }
    return 0;
}
