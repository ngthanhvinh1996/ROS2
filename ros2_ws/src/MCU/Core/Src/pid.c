/**
 ******************************************************************************
 * @file           : pid.c
 * @brief          : PID program body
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

/* Includes ------------------------------------------------------------------*/
#include "pid.h"

/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define WHEEL_DIAMETER      0.065f   // 6.5 cm = 0.065 m
#define PI                  3.14159f
#define WHEEL_CIRCUMFERENCE (WHEEL_DIAMETER * PI) // ~0.2042 m
#define PULSE_PER_REV       1560.0f  // 13 * 30 * 4
#define SAMPLE_TIME         0.02f    // 20ms
#define PWM_MAX_LIMIT       9669     // ARR Timer
#define PWM_DEADZONE        5100     // Range Init
#define PWM_GAIN_SLOPE      0.524f   // (PWM / (xung/s))
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Global user code ---------------------------------------------------------*/
void PID_Init(PID_Controller *pid, float kp, float ki, float kd, int min, int max, float t_sample)
{
    pid->Kp = kp;
    pid->Ki = ki;
    pid->Kd = kd;
    pid->outMin = min;
    pid->outMax = max;
    pid->T_sample = t_sample;
    
    pid->integral = 0.0f;
    pid->prevError = 0.0f;
}

int PID_Compute(PID_Controller *pid, int setpoint, int measurement)
{
    float error = (float)(setpoint - measurement);
    
    /* --- P term --- */
    float Pout = pid->Kp * error;
    
    /* --- I term --- */
    pid->integral += error * pid->T_sample;
    float Iout = pid->Ki * pid->integral;
    
    if (Iout > pid->outMax) Iout = pid->outMax;
    else if (Iout < pid->outMin) Iout = pid->outMin;

    /* --- D term --- */
    float derivative = (error - pid->prevError) / pid->T_sample;
    float Dout = pid->Kd * derivative;

    float output_f = Pout + Iout + Dout;

    if (output_f > pid->outMax) output_f = pid->outMax;
    else if (output_f < pid->outMin) output_f = pid->outMin;

    pid->prevError = error;
    return (int)output_f;
}

int Convert_Ms_To_Pulse(float ms)
{
    float pulse = 0;

    pulse = (ms * PULSE_PER_REV) / WHEEL_CIRCUMFERENCE;

    return (int)round(pulse);
}

int Convert_Pulse_To_Pwm(int pulse)
{
    return ((pulse * PWM_GAIN_SLOPE) + PWM_DEADZONE);
}
