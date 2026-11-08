/**
 * @file    can_control.h
 * @brief   Dinh nghia ma lenh CAN ID dieu khien xe va tay gap (11-bit Standard ID)
 *
 * Cau truc 11-bit CAN ID:
 * - Bit [10:9] = 00 : Luong Control (Loc phan cung bxCAN vao FIFO0)
 * - Bit [8:6]  : Nhom chuc nang (3 bit)
 *                000 (0x000): He thong & An toan
 *                001 (0x040): Dong co banh xe (Motor)
 *                010 (0x080): Servo canh tay & Kep
 * - Bit [5:0]  : Ma lenh chi tiet (DLC = 0)
 */

#ifndef CAN_CONTROL_H
#define CAN_CONTROL_H

#include <stdint.h>

/* ========================================================================
 * Nhom 0: He thong & An toan (Base 0x000)
 * ======================================================================*/
#define CMD_EMERGENCY_STOP      0x000U  /* 0x000: Dung khan cap toan bo xe & tay gap */
#define CMD_MODE_MOTOR          0x001U  /* 0x001: Chuyen sang che do lai xe (Motor) */
#define CMD_MODE_SERVO          0x002U  /* 0x002: Chuyen sang che do tay gap (Servo) */

/* ========================================================================
 * Nhom 1: Dieu khien Dong co banh xe (Base 0x040)
 * ======================================================================*/
#define CMD_MOTOR_STOP          0x040U  /* 0x040: Dung 4 banh xe */
#define CMD_MOTOR_FORWARD       0x041U  /* 0x041: Xe tien thang */
#define CMD_MOTOR_BACKWARD      0x042U  /* 0x042: Xe lui thang */
#define CMD_MOTOR_TURN_LEFT     0x043U  /* 0x043: Quay trai tai cho */
#define CMD_MOTOR_TURN_RIGHT    0x044U  /* 0x044: Quay phai tai cho */
#define CMD_MOTOR_LEFT_FWD      0x045U  /* 0x045: Banh trai tien */
#define CMD_MOTOR_LEFT_REV      0x046U  /* 0x046: Banh trai lui */
#define CMD_MOTOR_RIGHT_FWD     0x047U  /* 0x047: Banh phai tien */
#define CMD_MOTOR_RIGHT_REV     0x048U  /* 0x048: Banh phai lui */

/* ========================================================================
 * Nhom 2: Dieu khien Servo Canh tay & Kep (Base 0x080)
 * ======================================================================*/
#define CMD_SERVO_STOP          0x080U  /* 0x080: Dung buoc servo, giu nguyen vi tri */
#define CMD_SERVO_ARM_UP        0x081U  /* 0x081: Nang tay gap (Servo Trai PA6 +1 do) */
#define CMD_SERVO_ARM_DOWN      0x082U  /* 0x082: Ha tay gap (Servo Trai PA6 -1 do) */
#define CMD_SERVO_ARM_EXTEND    0x083U  /* 0x083: Vuon dai tay (Servo Phai PA7 +1 do) */
#define CMD_SERVO_ARM_RETRACT   0x084U  /* 0x084: Thu tay ve (Servo Phai PA7 -1 do) */
#define CMD_SERVO_GRIP_OPEN     0x085U  /* 0x085: Mo kep gap (90 do) */
#define CMD_SERVO_GRIP_CLOSE    0x086U  /* 0x086: Dong kep gap (0 do) */

#endif /* CAN_CONTROL_H */
