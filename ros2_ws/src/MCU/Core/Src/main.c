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
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
    .name = "defaultTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};

osThreadId_t controlTaskHandle;
const osThreadAttr_t vControlLoopTask_attributes = {
    .name = "vControlLoopTask",
    .stack_size = 1024 * 2,
    .priority = (osPriority_t)osPriorityRealtime,
};

osThreadId_t vDataBluetoothProcessTaskHandle;
const osThreadAttr_t vDataBluetoothProcessTask_attributes = {
    .name = "vDataBluetoothProcessTask",
    .stack_size = 1024 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};

PID_Controller right_robotPID;
PID_Controller left_robotPID;

uint8_t Write_Msg_Index = 0U;
uint8_t Read_Msg_Index = 0U;
Motion_type Motion = MOTION_STOP;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void StartDefaultTask(void *argument);
void vControlLoopTask(void *argument);
void vDataBluetoothProcessTask(void *argument);
void UartRxTask(void *argument);

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
    char msg_str[16] = "\r\n=== ROBOT BOOT CONFIG ===\r\nPress 'c' to Configure PID, or wait 3s to run...\r\n";
    uint8_t key = 0;
    uint8_t Lidar_Start[5U] = {0xA5, 0x60, 0x00, 0x00, 0x00};
    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* Configure the system clock */
    SystemClock_Config();

    /* Initialize the UART3 for debug log */
    Uart3_Init(115200U);

    /* Initialize the UART2 for bluetooth */
    Uart2_Init(9600U);

    /* Initialize the UART5 for Lidar */
    Uart5_Init(460800);
    // Uart_Send_Data(&huart5, &Lidar_Start, 5U);

    /* Initialize the UART4 for ESP32 */
    Uart4_Init(115200U);

    /* Initialize led*/
    Led_Init();
    Led_Set(GPIO_PIN_RESET);

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

    if(0 != ICM20948_Init())
    {
        LOG_ERR("Error when init ICM\r\n");
        return;
    }

    TIM4_Init();

    HAL_UART_Transmit(&huart3, &msg_str, strlen(msg_str), 1000);

    if(HAL_OK == HAL_UART_Receive(&huart3, &key,1, 3000) && (key == 'c' || key == 'C'))
    {
        char temp_input[32];
        char temp_msg[64];

        HAL_UART_Transmit(&huart3, (uint8_t *)"\r\n[CONFIG MODE ACTIVATED]\r\n", 28, 1000);

        /* Enter PWM */
        HAL_UART_Transmit(&huart3, (uint8_t *)"Enter Encoder pulses/s: ", 23, 1000);
        UART_Get_Line(&huart3, temp_input, 32);
        if(strlen(temp_input) > 0)
        {
            Encoder = atof(temp_input);
        }

        /* Enter Kp */
        HAL_UART_Transmit(&huart3, (uint8_t *)"Enter Kp: ", 11, 1000);
        UART_Get_Line(&huart3, temp_input, 32);
        if(strlen(temp_input) > 0)
        {
            Kp = atof(temp_input);
        }

        /* Enter Ki */
        HAL_UART_Transmit(&huart3, (uint8_t *)"Enter Ki: ", 11, 1000);
        UART_Get_Line(&huart3, temp_input, 32);
        if(strlen(temp_input) > 0)
        {
            Ki = atof(temp_input);
        }

        /* Enter Kd */
        HAL_UART_Transmit(&huart3, (uint8_t *)"Enter Kd: ", 11, 1000);
        UART_Get_Line(&huart3, temp_input, 32);
        if(strlen(temp_input) > 0)
        {
            Kd = atof(temp_input);
        }

        sprintf(temp_msg, "Encoder: %d - NEW PID: Kp=%.2f, Ki=%.2f, Kd=%.2f\r\n", Encoder, Kp, Ki, Kd);
        HAL_UART_Transmit(&huart3, (uint8_t*)temp_msg, strlen(temp_msg), 1000);
    }
    else
    {
        HAL_UART_Transmit(&huart3, (uint8_t*)"\r\n[DEFAULT LOADED] Starting FreeRTOS...\r\n", 42, 1000);
    }
    /* Init scheduler */
    osKernelInitialize();

    /* Create the thread(s) */
    /* creation of defaultTask */
    // defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);
    controlTaskHandle = osThreadNew(vControlLoopTask, NULL, &vControlLoopTask_attributes);
    // vDataBluetoothProcessTaskHandle = osThreadNew(vDataBluetoothProcessTask, NULL, &vDataBluetoothProcessTask_attributes);

    /* Start scheduler */
    osKernelStart();
    
    /* Infinite loop */
    while (1)
    {

    }
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

/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
void StartDefaultTask(void *argument)
{
    /* Infinite loop */
    for (;;)
    {
        osDelay(1);
    }
}

void vControlLoopTask(void *argument)
{
    int left_Encoder = 0U;
    int right_Encoder = 0U;
    int left_Encoder_pre = 0U;
    int right_Encoder_pre = 0U;
    bool first = true;
    int index = 0;
    int left_pid = 0;
    int right_pid = 0;
    int left_final_pwm = 0;
    int right_final_pwm = 0;

    PID_Init(&right_robotPID, Kp, Ki, Kd, -9599, 9599, 0.02f);
    PID_Init(&left_robotPID, Kp, Ki, Kd, -9599, 9599, 0.02f);
    LOG_INFO("Run task vControlLoopTask\r\n");

    Set_Motor_PWM(7000, 7000);
    // Set_Motor_duty(70, 70);
    LOG_INFO("Encoder_1ms_L,PWM_L,Encoder_1ms_R,PWM_R\r\n");
    while(1)
    {
        osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);
        
        left_Encoder = Read_Encoder(&htim2);
        right_Encoder = Read_Encoder(&htim3);
        
        left_pid = abs(left_Encoder_pre - left_Encoder) / 0.02;
        if(first)
        {
            right_pid = abs(0xFFFF - right_Encoder_pre) / 0.02;
        } 
        else
        {
            right_pid = abs(right_Encoder_pre - right_Encoder)/ 0.02;
            left_final_pwm = 7000 + (PID_Compute(&left_robotPID, Encoder, left_pid));
            right_final_pwm = 7000 + (PID_Compute(&right_robotPID, Encoder, right_pid));
            Set_Motor_PWM(left_final_pwm, right_final_pwm);
            // LOG_INFO("[%d]left_Encoder[%d], right_Encoder[%d]\r\n", index, left_Encoder, right_Encoder);
            // LOG_INFO("left_pid[%d], right_pid[%d]\r\n", left_pid, right_pid);
            // LOG_INFO("left_final_pwm[%d], right_final_pwm[%d]\r\n", left_final_pwm, right_final_pwm);
            LOG_BUF("%d,%d,%d,%d\r\n", 
                    left_pid,
                    left_final_pwm,
                    right_pid,
                    right_final_pwm
                    );
        }

        first = false;
        left_Encoder_pre = left_Encoder;
        right_Encoder_pre = right_Encoder;

        index++;
        if(index == 100)
        {
            // left_Encoder = Read_Encoder(&htim2);
            // right_Encoder = Read_Encoder(&htim3);
            // LOG_INFO("[%d]left_Encoder[%d], right_Encoder[%d]\r\n", index, left_Encoder, right_Encoder);
            LOG_INFO("END TASK\r\n");
            Set_Motor_PWM(0, 0);
            break;
        }
    }
}

void vDataBluetoothProcessTask(void *argument)
{
    int dma_prev_pos = 0;
    int current_check = 0;
    int index = 0;
    int dma_use_len = 0;
    ParserState state = STATE_WAIT_START;
    uint16_t payloadIndex = 0U;
    bool first = true;

    LOG_INFO("Run task vDataBluetoothProcessTask\r\n");
    while(1)
    {
        int pos = (DMA_UART2_BUF_SIZE - __HAL_DMA_GET_COUNTER(huart2.hdmarx));
        
        while((current_check != pos) && (0 != pos))
        {
            if(current_check >= DMA_UART2_BUF_SIZE)
            {
                current_check = 0;
            }

            switch (state)
            {
                case STATE_WAIT_START:
                    if(START_DELIMITER == Bluetooth_Data[current_check])
                    {
                        current_check++;
                        state = STATE_READ_EVENT_TYPE;
                        memset(&Control_msg[Write_Msg_Index], 0, sizeof(TLVMessage));
                    }
                    break;
                case STATE_READ_EVENT_TYPE:
                    Control_msg[Write_Msg_Index].eventType = Bluetooth_Data[current_check];
                    current_check++;
                    state = STATE_READ_LENGTH_LOW;
                    break;
                case STATE_READ_LENGTH_LOW:
                    Control_msg[Write_Msg_Index].payloadLength = Bluetooth_Data[current_check];
                    current_check++;
                    state = STATE_READ_LENGTH_HIGH;
                    break;
                case STATE_READ_LENGTH_HIGH:
                    Control_msg[Write_Msg_Index].payloadLength |= (Bluetooth_Data[current_check] << 8);
                    current_check++;
                    state = STATE_READ_PAYLOAD;
                    break;
                case STATE_READ_PAYLOAD:
                    Control_msg[Write_Msg_Index].payload[payloadIndex] = Bluetooth_Data[current_check];
                    payloadIndex++;
                    current_check++;
                    if(payloadIndex >= Control_msg[Write_Msg_Index].payloadLength)
                    {
                        payloadIndex = 0U;
                        state = STATE_WAIT_END;
                    }

                    break;
                case STATE_WAIT_END:
                    if(END_DELIMITER == Bluetooth_Data[current_check])
                    {
                        current_check++;
                        state = STATE_WAIT_START;
                        LOG_INFO("Write_Msg_Index[%d] Event 0x%x, Length=%d, current_check=%d\r\n", 
                                    Write_Msg_Index, Control_msg[Write_Msg_Index].eventType, Control_msg[Write_Msg_Index].payloadLength, current_check);
                        for(index = 0; index < Control_msg[Write_Msg_Index].payloadLength; index++)
                            LOG_BUF("0x%x ", Control_msg[Write_Msg_Index].payload[index]);
                        LOG_BUF("\r\n");
                        Write_Msg_Index++;
                        if(Write_Msg_Index >= 50)
                        {
                            Write_Msg_Index = 0;
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

void vDataHandleTask(void *argument)
{
    while(1)
    {
        if(Read_Msg_Index < Write_Msg_Index)
        {
            if(TLV_STOP == (Control_msg[Read_Msg_Index].eventType & TLV_ALL_EVENT))
            {
                Motion = MOTION_STOP;
            }
            // else if(TL)

        }
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
