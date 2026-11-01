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

    // ==================== Nhóm 0: An toàn & Hệ thống ====================
    bool emergencyStop();
    bool setMode(CarMode mode);
    bool triggerBootloader();

    // ==================== Nhóm 0: Phụ trợ & Ngoại vi ====================
    bool ledOn();
    bool ledOff();
    bool relayOn();
    bool relayOff();
    bool buzzerBeep();

    bool isLedOn() const { return _ledState; }
    bool isRelayOn() const { return _relayState; }

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

    // ==================== Bộ điều phối lệnh (Dùng cho Web & Serial) ====================
    /**
     * @brief Thực thi lệnh dạng chuỗi (Hỗ trợ cả từ khóa Web API và ký tự phím tắt)
     * @param cmd Chuỗi lệnh (vd: "forward", "stop", "arm_up", "F", "S", "X"...)
     * @return true nếu gửi frame CAN thành công
     */
    bool executeCommand(const String &cmd);

    // ==================== Trạng thái hệ thống (Chuẩn bị cho Web Dashboard) ====================
    CarMode getMode() const { return _currentMode; }
    String getModeString() const;
    String getLastAction() const { return _lastAction; }
    String getStatusJson() const;

private:
    CAN_Driver &_can;
    CarMode _currentMode;
    String _lastAction;
    bool _ledState = false;
    bool _relayState = false;

    bool sendCanCommand(uint32_t can_id, const String &action_name);
};

#endif // CAR_CONTROLLER_H

