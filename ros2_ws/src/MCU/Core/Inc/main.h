/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "common.h"
#include "motor.h"
#include "uart.h"
#include "oled.h"
#include "icm20948.h"
#include "pid.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/

/* Structure ----------------------------------------------------------*/
#pragma pack(push, 1)
typedef struct {
    uint8_t header1;    // 0xAA
    uint8_t header2;    // 0x55
    uint8_t type;       // 0x01
    uint8_t length;     // 12 bytes
    float linear_x;     // Forward/backward speed (m/s)
    float linear_y;     // Lateral sliding velocity (m/s) - Usually = 0 if it is a 2-wheeled robot
    float angular_z;    // Rotational speed (rad/s)
    uint8_t checksum;
} CmdVelPacket;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    uint8_t header1;    // 0xAA
    uint8_t header2;    // 0x55
    uint8_t type;       // 0x02
    uint8_t length;     // 24 bytes
    
    // Odom Data (Encoder-derived data)
    float robot_x;      // Current X-coordinate (m)
    float robot_y;      // Current Y coordinate (m)
    float robot_theta;  // Current angle of rotation (rad)
    
    // IMU Data (Roll, Pitch, Yaw hoặc Quaternion)
    float imu_roll;
    float imu_pitch;
    float imu_yaw;
    
    uint8_t checksum;
} SensorDataPacket;
#pragma pack(pop)

/* Exported types ------------------------------------------------------------*/
extern osThreadId_t UartRxTaskHandle;
extern osThreadId_t MotorControlTaskHandle;
extern osThreadId_t UartTxTaskHandle;
extern uint8_t Write_Msg_Index;
extern uint8_t Read_Msg_Index;

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/ 
#define FALSE 0U
#define TRUE  1U

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
