#ifndef CAN_DRIVER_H
#define CAN_DRIVER_H

#include <Arduino.h>
#include "driver/gpio.h"
#include "driver/twai.h"

/**
 * @brief Lớp điều khiển phần cứng CAN (TWAI) trên ESP32 tốc độ 500kbps.
 *        Phục vụ đồng thời cho cả Car Controller và OTA Protocol.
 */
class CAN_Driver {
public:
    CAN_Driver(gpio_num_t tx_pin, gpio_num_t rx_pin);

    /**
     * @brief Khởi tạo phần cứng TWAI (500 kbps, Accept All)
     */
    bool begin();

    /**
     * @brief Gửi một frame CAN 11-bit tiêu chuẩn (tối đa 8 bytes dữ liệu)
     */
    bool sendFrame(uint32_t id, const uint8_t* data = nullptr, uint8_t len = 0, uint32_t timeout_ms = 100);

    /**
     * @brief Gửi frame và chờ nhận ACK xác nhận (hỗ trợ tự động thử lại khi lỗi)
     */
    bool sendWithAck(uint32_t tx_id, const uint8_t* data, uint8_t len,
                     uint32_t ack_id, uint8_t expected_ack,
                     uint32_t timeout_ms = 1000, uint8_t max_retries = 3);

    /**
     * @brief Nhận một frame CAN từ hàng đợi phần cứng (Non-blocking hoặc Timeout)
     */
    bool receiveFrame(twai_message_t &rx_msg, uint32_t timeout_ms = 10);

    /**
     * @brief Xả sạch bộ đệm hàng đợi nhận (RX Queue)
     */
    void clearRxQueue();

    /**
     * @brief Chờ nhận phản hồi từ một ID cụ thể với giá trị byte 0 mong đợi
     */
    bool waitResponse(uint32_t response_id, uint8_t expected_val, uint32_t timeout_ms = 1000);

private:
    gpio_num_t _tx_pin;
    gpio_num_t _rx_pin;
};

#endif // CAN_DRIVER_H

