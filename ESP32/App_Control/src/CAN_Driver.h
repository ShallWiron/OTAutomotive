#ifndef CAN_DRIVER_H
#define CAN_DRIVER_H

#include <Arduino.h>
#include "driver/twai.h"

/**
 * @brief Lớp điều khiển phần cứng CAN (TWAI) trên ESP32 tốc độ 500kbps
 */
class CAN_Driver {
public:
    CAN_Driver(gpio_num_t tx_pin, gpio_num_t rx_pin);

    /**
     * @brief Khởi tạo phần cứng TWAI (500 kbps, Accept All)
     */
    bool begin();

    /**
     * @brief Gửi một frame CAN 11-bit tiêu chuẩn
     */
    bool sendFrame(uint32_t id, const uint8_t* data = nullptr, uint8_t len = 0, uint32_t timeout_ms = 100);

    /**
     * @brief Nhận một frame CAN từ hàng đợi phần cứng
     */
    bool receiveFrame(twai_message_t &rx_msg, uint32_t timeout_ms = 10);

    /**
     * @brief Xả sạch bộ đệm hàng đợi nhận
     */
    void clearRxQueue();

    /**
     * @brief Đợi nhận phản hồi từ một ID cụ thể
     */
    bool waitResponse(uint32_t response_id, uint8_t expected_val, uint32_t timeout_ms = 500);

private:
    gpio_num_t _tx_pin;
    gpio_num_t _rx_pin;
};

#endif // CAN_DRIVER_H

