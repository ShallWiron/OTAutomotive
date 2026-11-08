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
#define CMD_EMERGENCY_STOP      0x000U  // Dừng khẩn cấp toàn bộ hệ thống
#define CMD_MODE_MOTOR          0x001U  // Chuyển sang chế độ lái xe (Motor)
#define CMD_MODE_SERVO          0x002U  // Chuyển sang chế độ tay gắp (Servo)

// ==================== Nhóm 1: Động cơ di chuyển bánh xe ====================
#define CMD_MOTOR_STOP          0x040U  // Dừng 4 bánh xe
#define CMD_MOTOR_FORWARD       0x041U  // Xe tiến thẳng
#define CMD_MOTOR_BACKWARD      0x042U  // Xe lùi thẳng
#define CMD_MOTOR_TURN_LEFT     0x043U  // Quay trái tại chỗ
#define CMD_MOTOR_TURN_RIGHT    0x044U  // Quay phải tại chỗ
#define CMD_MOTOR_LEFT_FWD      0x045U  // Bánh trái tiến
#define CMD_MOTOR_LEFT_REV      0x046U  // Bánh trái lùi
#define CMD_MOTOR_RIGHT_FWD     0x047U  // Bánh phải tiến
#define CMD_MOTOR_RIGHT_REV     0x048U  // Bánh phải lùi

// ==================== Nhóm 2: Servo Cánh tay & Kẹp ====================
#define CMD_SERVO_STOP          0x080U  // Dừng bước servo, giữ nguyên vị trí
#define CMD_SERVO_ARM_UP        0x081U  // Nâng cánh tay (Servo Trái PA6 +1 độ)
#define CMD_SERVO_ARM_DOWN      0x082U  // Hạ cánh tay (Servo Trái PA6 -1 độ)
#define CMD_SERVO_ARM_EXTEND    0x083U  // Vươn dài cánh tay (Servo Phải PA7 +1 độ)
#define CMD_SERVO_ARM_RETRACT   0x084U  // Thu cánh tay về (Servo Phải PA7 -1 độ)
#define CMD_SERVO_GRIP_OPEN     0x085U  // Mở kẹp gắp (90 độ)
#define CMD_SERVO_GRIP_CLOSE    0x086U  // Đóng kẹp gắp (0 độ)

// ==================== Luồng OTA Trigger (Nhảy vào Bootloader) ====================
#define CAN_ID_REQ_BOOT         0x600U  // Yêu cầu STM32 chuyển sang Bootloader
#define CAN_ID_RESPONSE         0x7C0U  // Phản hồi từ STM32 -> ESP32

#endif // CAR_PROTOCOL_H
