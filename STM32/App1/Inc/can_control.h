#ifndef CAN_CONTROL_H
#define CAN_CONTROL_H

#include <stdint.h>

/* ========================================================================
 * DINH NGHIA CAN ID DIEU KHIEN XE (11-bit Standard ID)
 *
 * Cau truc 11-bit CAN ID:
 * - Bit [10:9] = 00 : Luong Control (Loc phan cung bxCAN vao FIFO0)
 * - Bit [8:6]  : Nhom lenh (3 bit)
 *                000 (0x0): He thong, An toan & Chuyen trang thai
 *                001 (0x1): Dong co di chuyen banh xe
 *                010 (0x2): Servo canh tay & Kep
 * - Bit [5:0]  : Ma lenh chi tiet (DLC = 0)
 * ======================================================================*/

/* Bit-mask phan tich CAN ID */
#define CAN_STREAM_MASK         0b11000000000U  /* Mask 2 bit [10:9] phan luong */
#define CAN_GROUP_MASK          0b00111000000U  /* Mask 3 bit [8:6] phan nhom   */
#define CAN_CMD_MASK            0b00000111111U  /* Mask 6 bit [5:0] ma lenh     */

#define STREAM_CONTROL          0b00000000000U  /* Luong Control */
#define GROUP_SYSTEM            0b00000000000U  /* Nhom 0: 0x000 */
#define GROUP_MOTOR             0b00001000000U  /* Nhom 1: 0x040 */
#define GROUP_SERVO             0b00010000000U  /* Nhom 2: 0x080 */

/* ========================================================================
 * Nhom 0: He thong, An toan & Chuyen trang thai (Base 0x000)
 * ======================================================================*/
#define CMD_EMERGENCY_STOP      0b00000000000U  /* 0x000: Dung khan cap toan bo xe & tay gap */
#define CMD_MODE_MOTOR          0b00000000001U  /* 0x001: Chuyen sang che do lai xe (DOF_C = 0) */
#define CMD_MODE_SERVO          0b00000000010U  /* 0x002: Chuyen sang che do tay gap (DOF_C = 1) */
#define CMD_LED_OFF             0b00000000011U  /* 0x003: Tat LED tren board ('U') */
#define CMD_LED_ON              0b00000000100U  /* 0x004: Bat LED tren board ('u') */
#define CMD_RELAY_OFF           0b00000000101U  /* 0x005: Tat Relay ('v') */
#define CMD_RELAY_ON            0b00000000110U  /* 0x006: Bat Relay ('V') */
#define CMD_BUZZER_BEEP         0b00000000111U  /* 0x007: Phat tieng coi Buzzer */

/* ========================================================================
 * Nhom 1: Dieu khien Dong co banh xe (Base 0x040)
 * ======================================================================*/
#define CMD_MOTOR_STOP          0b00001000000U  /* 0x040: Dung 4 banh xe */
#define CMD_MOTOR_FORWARD       0b00001000001U  /* 0x041: Xe tien thang */
#define CMD_MOTOR_BACKWARD      0b00001000010U  /* 0x042: Xe lui thang */
#define CMD_MOTOR_TURN_LEFT     0b00001000011U  /* 0x043: Quay trai tai cho */
#define CMD_MOTOR_TURN_RIGHT    0b00001000100U  /* 0x044: Quay phai tai cho */
#define CMD_MOTOR_LEFT_FWD      0b00001000101U  /* 0x045: Chi cum banh trai tien */
#define CMD_MOTOR_LEFT_REV      0b00001000110U  /* 0x046: Chi cum banh trai lui */
#define CMD_MOTOR_RIGHT_FWD     0b00001000111U  /* 0x047: Chi cum banh phai tien */
#define CMD_MOTOR_RIGHT_REV     0b00001001000U  /* 0x048: Chi cum banh phai lui */

/* ========================================================================
 * Nhom 2: Dieu khien Servo Canh tay & Kep (Base 0x080)
 * ======================================================================*/
#define CMD_SERVO_STOP          0b00010000000U  /* 0x080: Dung buoc servo, giu nguyen vi tri */
#define CMD_SERVO_ARM_UP        0b00010000001U  /* 0x081: Nang tay gap (Servo Trai PA6 +1 do) */
#define CMD_SERVO_ARM_DOWN      0b00010000010U  /* 0x082: Ha tay gap (Servo Trai PA6 -1 do) */
#define CMD_SERVO_ARM_EXTEND    0b00010000011U  /* 0x083: Vuon dai tay (Servo Phai PA7 +1 do) */
#define CMD_SERVO_ARM_RETRACT   0b00010000100U  /* 0x084: Thu tay ve (Servo Phai PA7 -1 do) */
#define CMD_SERVO_GRIP_OPEN     0b00010000101U  /* 0x085: Mo kep gap (90 do) */
#define CMD_SERVO_GRIP_CLOSE    0b00010000110U  /* 0x086: Dong kep gap (0 do) */

#endif /* CAN_CONTROL_H */

