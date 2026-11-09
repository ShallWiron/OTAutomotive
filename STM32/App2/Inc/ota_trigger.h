#ifndef OTA_TRIGGER_H
#define OTA_TRIGGER_H

#include <stdint.h>
#include "stm32f10x.h"

#define ID_REQ_BOOT         0x600U
#define ID_QUERY_APP        0x620U
#define ID_RESPONSE         0x7C0U
#define OTA_ACK             0x00U

#define CURRENT_APP_ID      2

#define BKP_OTA_FLAG_MAGIC  0x5A5AU

void OTA_Trigger_Init(void);
void OTA_Trigger_Check(CanRxMsg *rx);

#endif
