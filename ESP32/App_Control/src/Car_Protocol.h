#ifndef CAR_PROTOCOL_H
#define CAR_PROTOCOL_H

#include <stdint.h>

/* ========================================================================
 * ĐỊNH NGHĨA CAN ID ĐIỀU KHIỂN XE (11-BIT STANDARD ID)
 * Khớp 100% với STM32 App1 (can_control.h) và Bootloader
 *
 * Bit [10:9] = 00 : Luồng Control (Lọc phần cứng vào FIFO0)
 * Bit [8:6]  : Nhóm chức năng (3-bit)
 * Bit [5:0]  : Lệnh chi tiết (DLC = 0)
 * ======================================================================*/

// ==================== Nhóm 0: An toàn & Chuyển trạng thái ====================
#define CMD_EMERGENCY_STOP      0x000U  // 0b00_000_000000: Dừng khẩn cấp toàn bộ hệ thống
#define CMD_MODE_MOTOR          0x001U  // 0b00_000_000001: Chuyển sang chế độ lái xe (DOF_C = 0)
#define CMD_MODE_SERVO          0x002U  // 0b00_000_000010: Chuyển sang chế độ tay gắp (DOF_C = 1)
#define CMD_LED_OFF             0x003U  // 0b00_000_000011: Tắt LED trên board ('U')
#define CMD_LED_ON              0x004U  // 0b00_000_000100: Bật LED trên board ('u')
#define CMD_RELAY_OFF           0x005U  // 0b00_000_000101: Tắt Relay ('v')
#define CMD_RELAY_ON            0x006U  // 0b00_000_000110: Bật Relay ('V')
#define CMD_BUZZER_BEEP         0x007U  // 0b00_000_000111: Bíp còi Buzzer

// ==================== Nhóm 1: Động cơ di chuyển bánh xe ====================
#define CMD_MOTOR_STOP          0x040U  // 0b00_001_000000: Dừng 4 bánh xe
#define CMD_MOTOR_FORWARD       0x041U  // 0b00_001_000001: Xe tiến thẳng
#define CMD_MOTOR_BACKWARD      0x042U  // 0b00_001_000010: Xe lùi thẳng
#define CMD_MOTOR_TURN_LEFT     0x043U  // 0b00_001_000011: Quay trái tại chỗ
#define CMD_MOTOR_TURN_RIGHT    0x044U  // 0b00_001_000100: Quay phải tại chỗ
#define CMD_MOTOR_LEFT_FWD      0x045U  // 0b00_001_000101: Bánh trái tiến
#define CMD_MOTOR_LEFT_REV      0x046U  // 0b00_001_000110: Bánh trái lùi
#define CMD_MOTOR_RIGHT_FWD     0x047U  // 0b00_001_000111: Bánh phải tiến
#define CMD_MOTOR_RIGHT_REV     0x048U  // 0b00_001_001000: Bánh phải lùi

// ==================== Nhóm 2: Servo Cánh tay & Kẹp ====================
#define CMD_SERVO_STOP          0x080U  // 0b00_010_000000: Dừng bước servo, giữ nguyên vị trí
#define CMD_SERVO_ARM_UP        0x081U  // 0b00_010_000001: Nâng cánh tay (Servo Trái PA6 +1 độ)
#define CMD_SERVO_ARM_DOWN      0x082U  // 0b00_010_000010: Hạ cánh tay (Servo Trái PA6 -1 độ)
#define CMD_SERVO_ARM_EXTEND    0x083U  // 0b00_010_000011: Vươn dài cánh tay (Servo Phải PA7 +1 độ)
#define CMD_SERVO_ARM_RETRACT   0x084U  // 0b00_010_000100: Thu cánh tay về (Servo Phải PA7 -1 độ)
#define CMD_SERVO_GRIP_OPEN     0x085U  // 0b00_010_000101: Mở kẹp gắp (90 độ)
#define CMD_SERVO_GRIP_CLOSE    0x086U  // 0b00_010_000110: Đóng kẹp gắp (0 độ)

// ==================== Luồng OTA Trigger (Nhảy vào Bootloader) ====================
#define CAN_ID_REQ_BOOT         0x600U  // 0b11_000_000000: Yêu cầu STM32 chuyển sang Bootloader
#define CAN_ID_RESPONSE         0x7C0U  // 0b11_111_000000: Phản hồi từ STM32 -> ESP32

#endif // CAR_PROTOCOL_H

