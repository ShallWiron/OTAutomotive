#ifndef CAR_CONTROLLER_H
#define CAR_CONTROLLER_H

#include <Arduino.h>
#include "CAN_Driver.h"
#include "Car_Protocol.h"

enum class CarMode {
    MODE_MOTOR = 0,     // Chế độ lái xe (Bánh xe)
    MODE_SERVO = 1      // Chế độ cánh tay gắp 3 bậc
};

/**
 * @brief Lớp dịch vụ điều khiển Robot Car thông qua mạng CAN Bus
 */
class Car_Controller {
public:
    explicit Car_Controller(CAN_Driver &can_driver);

    void begin();

    // ==================== Nhóm 0: An toàn & Chuyển chế độ ====================
    bool emergencyStop();
    bool setMode(CarMode mode);
    bool triggerBootloader();

    // ==================== Nhóm 1: Điều khiển Bánh xe ====================
    bool moveForward();
    bool moveBackward();
    bool turnLeft();
    bool turnRight();
    bool stop();
    bool leftWheelForward();
    bool leftWheelBackward();
    bool rightWheelForward();
    bool rightWheelBackward();

    // ==================== Nhóm 2: Điều khiển Cánh tay & Kẹp ====================
    bool armUp();
    bool armDown();
    bool armExtend();
    bool armRetract();
    bool armStop();
    bool gripperOpen();
    bool gripperClose();

    // ==================== Bộ điều phối lệnh chuẩn Opcode CAN ID ====================
    /**
     * @brief Thực thi lệnh trực tiếp bằng mã CAN ID (Opcode)
     * @param can_id Mã lệnh Hex (vd: 0x041: FORWARD, 0x040: STOP, 0x081: ARM_UP...)
     */
    bool executeCanId(uint16_t can_id);

    /**
     * @brief Thực thi lệnh dạng chuỗi (Hỗ trợ mã hex '0x041' hoặc từ khóa mô tả 'forward', 'stop'...)
     */
    bool executeCommand(const String &cmd);

    // ==================== Trạng thái hệ thống ====================
    CarMode getMode() const { return _currentMode; }
    String getModeString() const;
    String getLastAction() const { return _lastAction; }
    String getStatusJson() const;

private:
    CAN_Driver &_can;
    CarMode _currentMode;
    String _lastAction;

    bool sendCanCommand(uint32_t can_id, const String &action_name);
};

#endif // CAR_CONTROLLER_H
