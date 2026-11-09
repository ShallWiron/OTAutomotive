#ifndef CAR_PROTOCOL_H
#define CAR_PROTOCOL_H

#include <stdint.h>

/* ========================================================================
 * ĐỊNH NGHĨA CAN ID ĐIỀU KHIỂN XE (11-BIT STANDARD ID)
 * Khớp 100% với STM32 App1 (robot_control.h) và Bootloader
 * ======================================================================*/

// ==================== Nhóm 0: Chế độ & Điều khiển cơ bản ====================
#define CMD_EMERGENCY_STOP      0x000U  // Dừng khẩn cấp toàn bộ hệ thống
#define CMD_MODE_MOTOR          0x001U  // Chuyển sang chế độ lái xe (Motor - DOF_C = 0)
#define CMD_MODE_SERVO          0x002U  // Chuyển sang chế độ tay gắp (Servo - DOF_C = 1)
#define CMD_LED_ON              0x003U  // Bật LED PC13 trên STM32
#define CMD_LED_OFF             0x004U  // Tắt LED PC13 trên STM32
#define CMD_WIFI_CONNECTED      0x005U  // Báo kết nối WiFi thành công (STM32 bíp + OLED)

// ==================== Nhóm 1: Động cơ di chuyển bánh xe (L298) ====================
#define CMD_MOTOR_STOP          0x040U  // Dừng 4 bánh xe
#define CMD_MOTOR_FORWARD       0x041U  // Xe tiến thẳng
#define CMD_MOTOR_BACKWARD      0x042U  // Xe lùi thẳng
#define CMD_MOTOR_TURN_LEFT     0x043U  // Quay trái tại chỗ
#define CMD_MOTOR_TURN_RIGHT    0x044U  // Quay phải tại chỗ
#define CMD_MOTOR_LEFT_FWD      0x045U  // Bánh trái tiến / Tiến lệch trái
#define CMD_MOTOR_LEFT_REV      0x046U  // Bánh trái lùi / Lùi lệch trái
#define CMD_MOTOR_RIGHT_FWD     0x047U  // Bánh phải tiến / Tiến lệch phải
#define CMD_MOTOR_RIGHT_REV     0x048U  // Bánh phải lùi / Lùi lệch phải

// ==================== Nhóm 2: Servo Cánh tay & Ngàm kẹp ====================
#define CMD_SERVO_STOP          0x080U  // Dừng servo
#define CMD_SERVO_ARM_UP        0x081U  // Nâng cánh tay (+1 độ)
#define CMD_SERVO_ARM_DOWN      0x082U  // Hạ cánh tay (-1 độ)
#define CMD_SERVO_ARM_EXTEND    0x083U  // Vươn cánh tay (+1 độ)
#define CMD_SERVO_ARM_RETRACT   0x084U  // Thu cánh tay (-1 độ)
#define CMD_SERVO_GRIP_OPEN     0x085U  // Mở ngàm kẹp (90 độ)
#define CMD_SERVO_GRIP_CLOSE    0x086U  // Đóng ngàm kẹp (0 độ)

// ==================== Nhóm 3: Còi báo hiệu (Horn / Buzzer) ====================
#define CMD_HORN_OFF            0x0C0U  // Tắt còi
#define CMD_HORN_ON             0x0C1U  // Bật còi

// ==================== Luồng OTA Trigger (Chuyển sang Bootloader) ====================
#define CAN_ID_REQ_BOOT         0x600U  // Yêu cầu STM32 chuyển sang Bootloader
#define CAN_ID_RESPONSE         0x7C0U  // Phản hồi từ STM32 -> ESP32

#endif // CAR_PROTOCOL_H
