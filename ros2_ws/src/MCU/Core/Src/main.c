/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
#include "main.h"

/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
osThreadId_t UartRxTaskHandle;
const osThreadAttr_t vUartRxTask_attributes = {
    .name = "vUartRxTask",
    .stack_size = 1024 * 2,
    .priority = (osPriority_t)osPriorityAboveNormal,
};

osThreadId_t MotorControlTaskHandle;
const osThreadAttr_t vMotorControlTask_attributes = {
    .name = "vMotorControlTask",
    .stack_size = 1024 * 2,
    .priority = (osPriority_t)osPriorityRealtime,
};

osThreadId_t UartTxTaskHandle;
const osThreadAttr_t vUartTxTask_attributes = {
    .name = "vUartTxTask",
    .stack_size = 1024 * 2,
    .priority = (osPriority_t)osPriorityNormal,
};

PID_Controller right_robotPID;
PID_Controller left_robotPID;

uint8_t Write_Msg_Index = 0U;
uint8_t Read_Msg_Index = 0U;
Motion_type Motion = MOTION_STOP;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void UartRxTask(void *argument);
void MotorControlTask(void *argument);
void UartTxTask(void *argument);

/* Private user code ---------------------------------------------------------*/

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    uint8_t pData = 0U;
    uint32_t EncoderA = 0U;
    uint32_t EncoderB = 0U;
    Imu_Data_t imu_data = {0};
    uint8_t imu_raw_data[21U] = {0U};
    uint8_t index = 0U;
    uint8_t key = 0;
    uint8_t Lidar_Start[5U] = {0xA5, 0x60, 0x00, 0x00, 0x00};
    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* Configure the system clock */
    SystemClock_Config();

    /* Initialize led*/
    Led_Init();
    Led_Set(GPIO_PIN_RESET);

    /* Init scheduler */
    osKernelInitialize();

    /* Init UART3 for communication with RDK X5 */
    if(true != Uart3_Init(115200U))
    {
        /* Return error when UART3 init fail */
        return -1;
    }

    /* Create the thread(s) */
    UartRxTaskHandle = osThreadNew(UartRxTask, NULL, &vUartRxTask_attributes);
    UartTxTaskHandle = osThreadNew(UartTxTask, NULL, &vUartTxTask_attributes);
    MotorControlTaskHandle = osThreadNew(MotorControlTask, NULL, &vMotorControlTask_attributes);
    

    /* Start scheduler */
    osKernelStart();
    
    /* Infinite loop */
    while (1)
    {

    }

    return 0;
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
     */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 16;
    RCC_OscInitStruct.PLL.PLLN = 192;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
    {
        Error_Handler();
    }
}

/*
* @brief Handle command from RDK X5
*/
void UartRxTask(void *argument)
{
    while(1)
    {
        
    }
}

/*
* @brief Handle motor control
*/
void MotorControlTask(void *argument)
{
    /* Initialize the Encoder TIM2 for the MotorA */
    MotorA_TIM2_Encoder_Init();

    /* Initialize the Encoder TIM3 for the MotorB */
    MotorB_TIM3_Encoder_Init();

    /* Initialize the PWM for the Motor A */
    MotorA_PWM_Init();

    /* Initialize the PWM for the Motor B */
    MotorB_PWM_Init();

    /* Initialize the PWM for the Servo */
    Servo_PWM_Init();

    TIM4_Init();

    if(0 != ICM20948_Init())
    {
        return;
    }

    while(1)
    {
        
    }
}

/*
* @brief Handle UART TX
*/
void UartTxTask(void *argument)
{
    SensorDataPacket sensor_data = {0};
    uint8_t checksum = 0;
    Imu_Data_t *imu_data;
    Orientation_t *orientation;
    Odometry_t *robot_odometry;
    while(1)
    {
        Read_Imu_Data(imu_data);
        Update_Orientation(orientation,
                           imu_data->accel.x, 
                           imu_data->accel.y, 
                           imu_data->accel.z, 
                           imu_data->gyro.x, 
                           imu_data->gyro.y, 
                           imu_data->gyro.z, 
                           imu_data->magn.x, 
                           imu_data->magn.y, 
                           imu_data->magn.z);
        
        update_odometry(&robot_odometry, Read_Encoder(&htim2), Read_Encoder(&htim3), orientation->yaw); 
        
        sensor_data.header1 = 0xAA;
        sensor_data.header2 = 0x55;
        sensor_data.type = 0x02;
        sensor_data.length = 24;
        sensor_data.robot_x = robot_odometry->x;
        sensor_data.robot_y = robot_odometry->y;
        sensor_data.robot_theta = robot_odometry->theta;
        sensor_data.imu_roll = orientation->roll;
        sensor_data.imu_pitch = orientation->pitch;
        sensor_data.imu_yaw = orientation->yaw;
        checksum = 0;
        for(uint8_t i = 0; i < 24; i++)
        {
            checksum += ((uint8_t*)&sensor_data)[i];
        }
        sensor_data.checksum = checksum;
        Uart_Send_Data(&huart3, (uint8_t*)&sensor_data, sizeof(sensor_data));
        osDelay(100);
    }
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1)
    {
    }
}
#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
