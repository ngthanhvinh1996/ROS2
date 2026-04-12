/**
  ******************************************************************************
  * @file           : pid.h
  * @brief          : Header for pid.c file.
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
#ifndef __PID_H
#define __PID_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "common.h"
#include <math.h>

/* Private defines ----------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
typedef struct {
    float Kp;
    float Ki;
    float Kd;
    int outMin;
    int outMax;
    float integral;
    float prevError;
    float T_sample;
} PID_Controller;

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
void PID_Init(PID_Controller *pid, float kp, float ki, float kd, int min, int max, float t_sample);
int PID_Compute(PID_Controller *pid, int setpoint, int measurement);

/* Private defines -----------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* __OLED_H */
