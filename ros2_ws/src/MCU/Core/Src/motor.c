/**
 ******************************************************************************
 * @file           : motor.c
 * @brief          : Motor program body
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
#include "motor.h"

/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/
int servo = 1500U;
int Encoder = 0;
float Kp = 0.0f;
float Ki = 0.0f;
float Kd = 0.0f;

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
TaskHandle_t xControlTaskHandle = NULL;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim8;
TIM_HandleTypeDef htim9;
TIM_HandleTypeDef htim10;
TIM_HandleTypeDef htim11;

/* Private function prototypes -----------------------------------------------*/

/* Global user code ---------------------------------------------------------*/
/**
 * @brief Initializes the Encoder TIM2 function for the MotorA 
 * @retval None
 */
void MotorA_TIM2_Encoder_Init(void)
{
    __HAL_RCC_TIM2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0x0U};
    TIM_Encoder_InitTypeDef sConfig = {0x0U};

    GPIO_InitStruct.Pin = GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 0x0U;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = ENCODER_TIM_PERIOD;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    sConfig.EncoderMode = TIM_ENCODERMODE_TI12;

    sConfig.IC1Polarity = TIM_ENCODERINPUTPOLARITY_RISING;
    sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
    sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
    sConfig.IC1Filter = 6U;

    sConfig.IC2Polarity = TIM_ENCODERINPUTPOLARITY_RISING;
    sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
    sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
    sConfig.IC2Filter = 6U;

    if (HAL_TIM_Encoder_Init(&htim2, &sConfig) != HAL_OK)
    {
        return;
    }

    if(HAL_OK != HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL))
    {
        return;
    }
}



/**
 * @brief Initializes the Encoder TIM3 function for the MotorB 
 * @retval None
 */
void MotorB_TIM3_Encoder_Init(void)
{
    __HAL_RCC_TIM3_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0U};
    TIM_Encoder_InitTypeDef sConfig = {0U};

    GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 0x0U;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = ENCODER_TIM_PERIOD;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    sConfig.EncoderMode = TIM_ENCODERMODE_TI12;

    sConfig.IC1Polarity = TIM_ENCODERINPUTPOLARITY_RISING;
    sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
    sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
    sConfig.IC1Filter = 6U;

    sConfig.IC2Polarity = TIM_ENCODERINPUTPOLARITY_RISING;
    sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
    sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
    sConfig.IC2Filter = 6U;
    
    if(HAL_TIM_Encoder_Init(&htim3, &sConfig) != HAL_OK)
    {
        return;
    }

    if(HAL_OK != HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL))
    {
        return;
    }
}

/**
 * @brief Initializes the PWM function for the MotorA
 */
void MotorA_PWM_Init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_TIM10_CLK_ENABLE();
    __HAL_RCC_TIM11_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0U};
    TIM_OC_InitTypeDef TIM_OCInitStruct = {0U};

    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF3_TIM10;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    htim10.Instance = TIM10;
    htim10.Init.Prescaler = 0U;
    htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim10.Init.Period = PWM_PERIOD;
    htim10.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim10.Init.RepetitionCounter = 0U;
    if(HAL_OK != HAL_TIM_PWM_Init(&htim10))
    {
        return;
    }

    TIM_OCInitStruct.OCMode = TIM_OCMODE_PWM1;
    TIM_OCInitStruct.Pulse = 0U;
    TIM_OCInitStruct.OCPolarity = TIM_OCPOLARITY_HIGH;
    TIM_OCInitStruct.OCFastMode = TIM_OCFAST_DISABLE;
    if(HAL_OK != HAL_TIM_PWM_ConfigChannel(&htim10, &TIM_OCInitStruct, TIM_CHANNEL_1))
    {
        return;
    }

    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF3_TIM11;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    htim11.Instance = TIM11;
    htim11.Init.Prescaler = 0U;
    htim11.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim11.Init.Period = PWM_PERIOD;
    htim11.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim11.Init.RepetitionCounter = 0U;
    if(HAL_OK != HAL_TIM_PWM_Init(&htim11))
    {
        return;
    }

    if(HAL_OK != HAL_TIM_PWM_ConfigChannel(&htim11, &TIM_OCInitStruct, TIM_CHANNEL_1))
    {
        return;
    }

    if(HAL_OK != HAL_TIM_PWM_Start(&htim10, TIM_CHANNEL_1))
    {
        return;
    }
    
    if(HAL_OK != HAL_TIM_PWM_Start(&htim11, TIM_CHANNEL_1))
    {
        return;
    }
}

/**
 * @brief Initializes the PWM function for the MotorB
 */
void MotorB_PWM_Init(void)
{
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_TIM9_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0U};
    TIM_OC_InitTypeDef TIM_OCInitStruct = {0U};

    GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF3_TIM9;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    htim9.Instance = TIM9;
    htim9.Init.Prescaler = 0U;
    htim9.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim9.Init.Period = PWM_PERIOD;
    htim9.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim9.Init.RepetitionCounter = 0U;
    if(HAL_OK != HAL_TIM_PWM_Init(&htim9))
    {
        return;
    }

    TIM_OCInitStruct.OCMode = TIM_OCMODE_PWM1;
    TIM_OCInitStruct.OCPolarity = TIM_OCPOLARITY_HIGH;
    TIM_OCInitStruct.OCFastMode = TIM_OCFAST_DISABLE;
    if(HAL_OK != HAL_TIM_PWM_ConfigChannel(&htim9, &TIM_OCInitStruct, TIM_CHANNEL_1))
    {
        return;
    }
    
    if(HAL_OK != HAL_TIM_PWM_ConfigChannel(&htim9, &TIM_OCInitStruct, TIM_CHANNEL_2))
    {
        return;
    }

    if(HAL_OK != HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_1))
    {
        return;
    }

    if(HAL_OK != HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_2))
    {
        return;
    }
}

/**
 * @brief Initializes the PWM function for the Servo
 */
void Servo_PWM_Init(void)
{
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_TIM8_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0U};
    TIM_OC_InitTypeDef TIM_OCInitStruct = {0U};

    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF3_TIM8;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    htim8.Instance = TIM8;
    htim8.Init.Prescaler = 95;
    htim8.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim8.Init.Period = 19999;
    htim8.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim8.Init.RepetitionCounter = 0U;
    if(HAL_OK != HAL_TIM_PWM_Init(&htim8))
    {
        return;
    }

    TIM_OCInitStruct.OCMode = TIM_OCMODE_PWM1;
    TIM_OCInitStruct.Pulse = 0U;
    TIM_OCInitStruct.OCPolarity = TIM_OCPOLARITY_HIGH;
    TIM_OCInitStruct.OCFastMode = TIM_OCFAST_DISABLE;
    if(HAL_OK != HAL_TIM_PWM_ConfigChannel(&htim8, &TIM_OCInitStruct, TIM_CHANNEL_4))
    {
        return;
    }

    if(HAL_OK != HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_4))
    {
        return;
    }
}

/**
 * @brief This function to run PWM
 * @retval None
*/
void Set_Motor_PWM(int motor_a, int motor_b)
{
    int MotorA_PWM = 0;
    int MotorB_PWM = 0;

    
    MotorA_PWM = abs(motor_a);
    MotorB_PWM = abs(motor_b);

    if(MotorA_PWM > PWM_PERIOD)  
    {
        MotorA_PWM = PWM_PERIOD;
    }
    else if(MotorA_PWM < 0) 
    {
        MotorA_PWM = 0;
    }

    if(MotorB_PWM > PWM_PERIOD)  
    {
        MotorB_PWM = PWM_PERIOD;
    }
    else if(MotorB_PWM < 0) 
    {
        MotorB_PWM = 0;
    }

    /* Time between two Set Compare Minimum 100ms */
    // HAL_Delay(20);

    if(motor_a < 0)
    {
        __HAL_TIM_SetCompare(&htim10, TIM_CHANNEL_1, MotorA_PWM);
        __HAL_TIM_SetCompare(&htim11, TIM_CHANNEL_1, 0);
    }
    else
    {
        __HAL_TIM_SetCompare(&htim10, TIM_CHANNEL_1, 0);
        __HAL_TIM_SetCompare(&htim11, TIM_CHANNEL_1, MotorA_PWM);
    }

    if(motor_b < 0)
    {
        __HAL_TIM_SetCompare(&htim9, TIM_CHANNEL_1, MotorB_PWM);
        __HAL_TIM_SetCompare(&htim9, TIM_CHANNEL_2, 0U);
    }
    else
    {
        __HAL_TIM_SetCompare(&htim9, TIM_CHANNEL_1, 0U);
        __HAL_TIM_SetCompare(&htim9, TIM_CHANNEL_2, MotorB_PWM);
    }
}

void Set_Servo_PWM(int servo)
{
    if(servo < 167)
    {
        servo = 167;
    }
    else if(servo > 9999)
    {
        servo = 9999;
    }

    __HAL_TIM_SetCompare(&htim8, TIM_CHANNEL_4, servo);
}

void Set_Motor_duty(int motor_a, int motor_b)
{
    int MotorA_PWM = 0;
    int MotorB_PWM = 0;

    if(motor_a > 100)  
    {
        motor_a = 100;
    }
    else if(motor_a < -100) 
    {
        motor_a = -100;
    }

    if(motor_b > 100)  
    {
        motor_b = 100;
    }
    else if(motor_b < -100) 
    {
        motor_b = -100;
    }
    
    MotorA_PWM = (PWM_PERIOD * abs(motor_a)) / 100;
    MotorB_PWM = (PWM_PERIOD * abs(motor_b)) / 100;

    Set_Motor_PWM(MotorA_PWM, MotorB_PWM);
}

bool Process_Motor(int right_pwm, int left_pwm)
{
    bool ret = true;
    int servo = 1500;

    switch (Ctrl_Data)
    {
        case 0x41:
            servo = 1500;
            break;
        case 0x45:
            right_pwm = -right_pwm;
            left_pwm = -left_pwm;
            servo = 1500;
            break;
        case 0x43:
            servo = 2000;
            break;
        case 0x47:
            servo = 1000;
            break;
        case 0x42:
            servo = 1750;
            break;
        case 0x44:
            right_pwm = -right_pwm;
            left_pwm = -left_pwm;
            servo = 1750;
            break;
        case 0x46:
            right_pwm = -right_pwm;
            left_pwm = -left_pwm;
            servo = 1250;
            break;
        case 0x48:
            servo = 1250;
            break;
        case 0x5A:
            right_pwm = 0;
            left_pwm = 0;
            servo = 1500;
            ret = false;
            break;
        default:
            right_pwm = 0;
            left_pwm = 0;
            servo = 1500;
            ret = false;
            break;
    }

    Set_Motor_PWM(left_pwm, right_pwm);
    Set_Servo_PWM(servo); 
    return ret;
}

void Set_Encoder(TIM_HandleTypeDef *htim, uint16_t value)
{
    __HAL_TIM_SetCounter(htim, value);
}

uint32_t Read_Encoder(TIM_HandleTypeDef *htim)
{
    return (uint32_t)__HAL_TIM_GetCounter(htim);
}

void TIM4_Init(void)
{
    __HAL_RCC_TIM4_CLK_ENABLE();

    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim4.Instance = TIM4;

    htim4.Init.Prescaler = 83; 
    
    htim4.Init.Period = 19999; 
    
    htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    
    if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
    {
        Error_Handler();
    }

    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_NVIC_SetPriority(TIM4_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(TIM4_IRQn);

    HAL_TIM_Base_Start_IT(&htim4);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM4)
    {
        if (MotorControlTaskHandle != NULL) 
        {
            osThreadFlagsSet(MotorControlTaskHandle, 0x01);
        }
    }
}

void update_odometry(Odometry_t *robot, 
                     int delta_tick_L, 
                     int delta_tick_R, 
                     float imu_yaw_degree)
{
    // 1. Theta angle update from IMU (Absolute and accurate)
    robot->theta = imu_yaw_degree * (PI / 180.0f);
    
    // Maintain the angle within the range of -PI to PI (ROS standard)
    if (robot->theta > PI) robot->theta -= 2.0f * PI;
    if (robot->theta < -PI) robot->theta += 2.0f * PI;

    // 2. Calculate the distance traveled by each wheel.
    float distance_L = ((float)delta_tick_L / TICKS_PER_REV) * WHEEL_CIRCUMFERENCE;
    float distance_R = ((float)delta_tick_R / TICKS_PER_REV) * WHEEL_CIRCUMFERENCE;

    // 3. Calculate the displacement of the robot's center.
    float delta_S = (distance_L + distance_R) / 2.0f;

    // 4. Add to X, Y
    robot->x += delta_S * cos(robot->theta);
    robot->y += delta_S * sin(robot->theta);
}
