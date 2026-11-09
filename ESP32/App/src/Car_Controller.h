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
    bool ledOn();
    bool ledOff();
    bool toggleLed();
    bool isLedOn() const { return _ledState; }

    // ==================== Nhóm 1: Điều khiển Bánh xe ====================
    bool moveForward();
    bool moveBackward();
    bool turnLeft();
    bool turnRight();
    bool stop();
    bool leftWheelForward();     // Tiến lệch trái (NW)
    bool leftWheelBackward();    // Lùi lệch trái (SW)
    bool rightWheelForward();    // Tiến lệch phải (NE)
    bool rightWheelBackward();   // Lùi lệch phải (SE)

    // ==================== Nhóm 2: Điều khiển Cánh tay 2-DOF ====================
    bool armUp();
    bool armDown();
    bool armExtend();
    bool armRetract();
    bool armStop();

    // ==================== Nhóm 3: Còi báo hiệu ====================
    bool hornOn();
    bool hornOff();
    bool isHornOn() const { return _hornState; }

    // ==================== Bộ điều phối lệnh chuẩn Opcode CAN ID ====================
    bool executeCanId(uint16_t can_id);
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
    uint32_t _lastCanId = 0x040;
    bool _ledState;
    bool _hornState = false;

    bool sendCanCommand(uint32_t can_id, const String &action_name);
};

#endif // CAR_CONTROLLER_H
