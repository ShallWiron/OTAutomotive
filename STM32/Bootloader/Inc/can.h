#ifndef CAN_H
#define CAN_H

#include <stdint.h>
#include "stm32f10x_can.h"

void CAN_Setup(void);
void CAN_ReceiveMessagePolling(CanRxMsg *RxMsg);
void CAN_SendMessagePolling(uint32_t ID, const uint8_t *data, uint8_t length);

// --- CÁC HÀM TRUYỀN NHẬN MẢNG CƠ BẢN ---
void CAN_SendArray(const uint8_t *data, uint16_t length);
uint8_t CAN_ReceiveArray(uint8_t *buffer, uint16_t *out_length, uint32_t timeout_loops);

#endif