/**
  ******************************************************************************
  * @file           : motor.h
  * @brief          : Header for motor.c file.
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
#ifndef __MOTOR_H
#define __MOTOR_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "common.h"
#include "uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

/* Private defines ----------------------------------------------------------*/
#define ENCODER_TIM_PERIOD  0xFFFF  
#define PWM_PERIOD          9599U

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  MOTION_STOP = 0U,
  MOTION_GO_FORWARD,
  MOTION_BACK_FORWARD,
  MOTION_TURN_LEFT,
  MOTION_TURN_RIGHT
} Motion_type;

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim9;
extern TIM_HandleTypeDef htim10;
extern TIM_HandleTypeDef htim11;

extern int Encoder;
extern float Kp;
extern float Ki;
extern float Kd;

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/

/* Private defines -----------------------------------------------------------*/
void MotorA_TIM2_Encoder_Init(void);
void MotorA_PWM_Init(void);
void MotorB_TIM3_Encoder_Init(void);
void MotorB_PWM_Init(void);
void Servo_PWM_Init(void);
void Set_Motor_PWM(int motor_a, int motor_b);
bool Process_Motor(int right_pwm, int left_pwm);
void Set_Servo_PWM(int servo);
void Set_Motor_duty(int motor_a, int motor_b);
uint32_t Read_Encoder(TIM_HandleTypeDef *htim);
void Set_Encoder(TIM_HandleTypeDef *htim, uint16_t value);
void TIM4_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __MOTOR_H */
