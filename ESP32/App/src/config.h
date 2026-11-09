#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include "driver/twai.h"

#define CAN_TX_PIN              GPIO_NUM_5
#define CAN_RX_PIN              GPIO_NUM_4

#define SD_CS_PIN               15

#define WIFI_AP_SSID            "AutoCar_Gateway"
#define WIFI_AP_PASSWORD        "12345678"
#define WIFI_SSID               "Shall"
#define WIFI_PASSWORD           "0123456789"
#define WEB_SERVER_PORT         80

#define SERIAL_BAUD_RATE        115200

#define CAN_ID_REQ_BOOT         0x600U
#define CAN_ID_QUERY_APP        0x620U
#define CAN_ID_START_OTA        0x640U
#define CAN_ID_START_FILE       0x680U
#define CAN_ID_START_CHUNK      0x6C0U
#define CAN_ID_DATA_CHUNK       0x700U
#define CAN_ID_END_CHUNK        0x740U
#define CAN_ID_END_FILE         0x780U
#define CAN_ID_RESPONSE         0x7C0U

#define OTA_ACK                 0x00U
#define OTA_NACK                0x01U

#define CHUNK_SIZE              256U
#define MAX_CHUNK_RETRIES       3U

#endif
