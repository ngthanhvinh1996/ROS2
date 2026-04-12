/**
 ******************************************************************************
 * @file           : uart.c
 * @brief          : UART program body
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
#include "uart.h"

/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
UART_HandleTypeDef huart4;
UART_HandleTypeDef huart5;
DMA_HandleTypeDef hdma_uart2_rx;

// uint8_t Ctrl_Data[3U] = {0U};
uint8_t Ctrl_Data = 0U;
uint8_t Ctrl_Data_pre = 0U;
uint8_t Lidar_Data[58U] = {0U};
TLVMessage Control_msg[50U] = {0};
volatile uint8_t Bluetooth_Data[DMA_UART2_BUF_SIZE] = {0};
int dma_prev_pos = 0U;
int index_num = 0;

/* Private function prototypes -----------------------------------------------*/

#ifdef USE_UART_DMA
static HAL_StatusTypeDef DMA_UART2_RX_Init(void);


static HAL_StatusTypeDef DMA_UART2_RX_Init(void)
{
    __HAL_RCC_DMA1_CLK_ENABLE();

    hdma_uart2_rx.Instance = DMA1_Stream5;
    hdma_uart2_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_uart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_uart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_uart2_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_uart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_uart2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_uart2_rx.Init.Mode = DMA_CIRCULAR;
    hdma_uart2_rx.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_uart2_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

    if(HAL_OK != HAL_DMA_Init(&hdma_uart2_rx))
    {
        return HAL_ERROR;
    }

    __HAL_LINKDMA(&huart2, hdmarx, hdma_uart2_rx);

    HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);

    return HAL_OK;
}
#endif

/* Global user code ---------------------------------------------------------*/
/**
 * @brief Initializes the USART2 function for Bluetooth HC
 * @retval None
 */
void Uart2_Init(int baud)
{
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_USART2_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
#ifdef USE_UART_DMA
    if(HAL_OK != DMA_UART2_RX_Init())
    {
        LOG_ERR("DMA for UART2 init failed.\r\n");
        return;
    }
#endif

    huart2.Instance = USART2;
    huart2.Init.BaudRate = baud;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    if(HAL_OK != HAL_UART_Init(&huart2))
    {
        LOG_ERR("UART2 init failed.\r\n");
        return;
    }

#ifdef USE_UART_DMA
    if(HAL_OK != HAL_UART_Receive_DMA(&huart2, &Bluetooth_Data, DMA_UART2_BUF_SIZE))
    {
        LOG_ERR("Received DMA for UART2 failed.\r\n");
        return;
    }
#else
    HAL_NVIC_SetPriority(USART2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);

    HAL_UART_Receive_IT(&huart2, &Ctrl_Data, 1U);
#endif

    LOG_INFO("UART2 for Bluetooth HC initialized.\r\n");
}

#ifdef USE_UART_DMA
int uart_dma_process(void)
{
    bool end_flag = false;
    int pos = (DMA_UART2_BUF_SIZE - __HAL_DMA_GET_COUNTER(huart2.hdmarx));

    if((pos > dma_prev_pos))
    {
        LOG_INFO("Normal\r\n");
        write_ring_buf(Bluetooth_Data, dma_prev_pos, pos);
        dma_prev_pos = pos;
        index_num++;
    }
    else if(pos < dma_prev_pos)
    {
        LOG_INFO("Back\r\n");
        write_ring_buf(Bluetooth_Data, dma_prev_pos, DMA_UART2_BUF_SIZE);
        write_ring_buf(Bluetooth_Data, 0, pos);
        dma_prev_pos = pos;
        index_num++;
    }
    else 
    {
        LOG_INFO("Dont receive buffer.\r\n");
    }

    if(index_num == 10U)
    {
        LOG_INFO("FINISHED.\r\n");
        return -1;
    }

    return 0;
}
#else
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART2)
    {
        HAL_UART_Receive_IT(&huart2, &Ctrl_Data, 1U);
    }
}
#endif

/**
 * @brief Initializes the USART3 function for console debug log
 * @retval None
 */
void Uart3_Init(int baud)
{
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_USART3_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    huart3.Instance = USART3;
    huart3.Init.BaudRate = baud;
    huart3.Init.WordLength = UART_WORDLENGTH_8B;
    huart3.Init.StopBits = UART_STOPBITS_1;
    huart3.Init.Parity = UART_PARITY_NONE;
    huart3.Init.Mode = UART_MODE_TX;
    huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart3.Init.OverSampling = UART_OVERSAMPLING_16;
    if(HAL_OK != HAL_UART_Init(&huart3))
    {
        LOG_ERR("UART3 init failed.\r\n");
        return;
    }
    LOG_INFO("UART3 for debug console initialized.\r\n");
}

/**
 * @brief Initializes the USART4 function for Lidar
 * @retval None
 */
void Uart4_Init(int baud)
{
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_UART4_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    huart4.Instance = UART4;
    huart4.Init.BaudRate = baud;
    huart4.Init.WordLength = UART_WORDLENGTH_8B;
    huart4.Init.StopBits = UART_STOPBITS_1;
    huart4.Init.Parity = UART_PARITY_NONE;
    huart4.Init.Mode = UART_MODE_TX_RX;
    huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart4.Init.OverSampling = UART_OVERSAMPLING_16;
    if(HAL_OK != HAL_UART_Init(&huart4))
    {
        LOG_ERR("UART4 init failed.\r\n");
        return;
    }
    LOG_INFO("UART4 for ESP32 initialized.\r\n");
}

/**
 * @brief Initializes the USART5 function for Lidar
 * @retval None
 */
void Uart5_Init(int baud)
{
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_UART5_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_UART5;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    huart5.Instance = UART5;
    huart5.Init.BaudRate = baud;
    huart5.Init.WordLength = UART_WORDLENGTH_8B;
    huart5.Init.StopBits = UART_STOPBITS_1;
    huart5.Init.Parity = UART_PARITY_NONE;
    huart5.Init.Mode = UART_MODE_TX_RX;
    huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart5.Init.OverSampling = UART_OVERSAMPLING_16;
    if(HAL_OK != HAL_UART_Init(&huart5))
    {
        LOG_ERR("UART5 init failed.\r\n");
        return;
    }

    // HAL_NVIC_SetPriority(UART5_IRQn, 2, 0);
    // HAL_NVIC_EnableIRQ(UART5_IRQn);

    // HAL_UART_Receive_IT(&huart5, &Lidar_Data, 1);
    // __HAL_UART_ENABLE_IT(&huart5, UART_IT_RXNE);

    LOG_INFO("UART5 for Lidar initialized.\r\n");
}

/**
 * @brief Send data polling
 * @retval None
*/
void Uart_Send_Data(UART_HandleTypeDef *huart, const char *buf, uint16_t size)
{
    HAL_UART_Transmit(huart, (uint8_t *)buf, size, HAL_MAX_DELAY);
}

/**
 * @brief Send char polling
 * @retval None
*/
void Uart_Send_Char(UART_HandleTypeDef *huart, const char *buf)
{
    HAL_UART_Transmit(huart, (uint8_t *)buf, 1U, HAL_MAX_DELAY);
}

/**
 * @brief Receive data polling
 * @retval None
*/
void Uart_Receive_Data(UART_HandleTypeDef *huart, char *buf, uint16_t size)
{
    HAL_UART_Receive(huart, buf, size, HAL_MAX_DELAY);
}

void UART_Get_Line(UART_HandleTypeDef *huart, char *buf, uint16_t max_len)
{
    uint8_t rx_char;
    uint16_t index = 0;

    memset(buf, 0, max_len);

    while(1)
    {
        if(HAL_OK == HAL_UART_Receive(&huart3, rx_char, 1U, HAL_MAX_DELAY))
        {
            if(rx_char == '\n' || rx_char == '\r')
            {
                HAL_UART_Transmit(&huart3, (uint8_t *)"\r\n", 2, 100);
                break;
            }
            else if(rx_char == 8 || rx_char == 127)
            {
                if(index > 0)
                {
                    index--;
                    buf[index] = 0;
                    HAL_UART_Transmit(&huart3, (uint8_t *)"\b\b", 2, 100);
                }
            }
            else
            {
                if(index < max_len - 1)
                {
                    buf[index] = rx_char;
                    HAL_UART_Transmit(&huart3, &rx_char, 1, 100);
                }
            }
        }
    }
}

/**
 * @brief LOG_Print
 * @retval None
 */
void LOG_Print(const char *fmt, ...)
{
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    HAL_UART_Transmit(&huart3, (uint8_t*)buf, strlen(buf), HAL_MAX_DELAY);
}
