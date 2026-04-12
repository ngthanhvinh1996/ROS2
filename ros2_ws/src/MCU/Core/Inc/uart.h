/**
  ******************************************************************************
  * @file           : uart.h
  * @brief          : Header for uart.c file.
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
#ifndef __UART_H
#define __UART_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "common.h"

/* Private defines ----------------------------------------------------------*/
#define ENABLE_LOG_INFO  1
#define ENABLE_LOG_WARN  1
#define ENABLE_LOG_ERR   1
#define ENABLE_LOG_BUF   1

#define USE_UART_DMA 1

#define DMA_UART2_BUF_SIZE      2048U
#define START_DELIMITER         0xAA
#define END_DELIMITER           0x55

#define TLV_STOP        0x00
#define TLV_ANGLE       0x01
#define TLV_SPEED       0x04
#define TLV_MOTOR       0x08
#define TLV_SERVO       0x10
#define TLV_BATERRY     0x20
#define TLV_SENSOR      0x40
#define TLV_PING        0x80
#define TLV_ALL_EVENT   0xFF
/* Exported types ------------------------------------------------------------*/
typedef enum {
    STATE_WAIT_START = 0U,
    STATE_READ_EVENT_TYPE,
    STATE_READ_LENGTH_LOW,
    STATE_READ_LENGTH_HIGH,
    STATE_READ_PAYLOAD,
    STATE_WAIT_END,
    STATE_DEFAULT,
} ParserState;

typedef struct {
    uint8_t eventType;
    uint16_t payloadLength;
    uint8_t payload[256U];
} TLVMessage;

typedef struct {
  uint16_t head;
  uint16_t tail;
  uint8_t buf[1024U];
} RingBuf_t;

extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;
extern DMA_HandleTypeDef hdma_uart2_rx;

// extern uint8_t Ctrl_Data[3U];
extern uint8_t Ctrl_Data;
extern uint8_t Ctrl_Data_pre;
extern volatile uint8_t Bluetooth_Data[DMA_UART2_BUF_SIZE];
extern TLVMessage Control_msg[50U];
/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
void Uart2_Init(int baud);
void Uart3_Init(int baud);
void Uart4_Init(int baud);
void Uart5_Init(int baud);
void Uart_Send_Data(UART_HandleTypeDef *huart, const char *buf, uint16_t size);
void Uart_Send_Char(UART_HandleTypeDef *huart, const char *buf);
void Uart_Receive_Data(UART_HandleTypeDef *huart, char *buf, uint16_t size);
void UART_Get_Line(UART_HandleTypeDef *huart, char *buf, uint16_t max_len);
int uart_dma_process(void);
void LOG_Print(const char *fmt, ...);

#if ENABLE_LOG_INFO
#define LOG_INFO(fmt, ...)  LOG_Print("[INFO] " fmt, ##__VA_ARGS__)
#else
#define LOG_INFO(fmt, ...)  do {} while(0)
#endif

#if ENABLE_LOG_WARN
#define LOG_WARN(fmt, ...)  LOG_Print("[WARN] " fmt, ##__VA_ARGS__)
#else
#define LOG_WARN(fmt, ...)  do {} while(0)
#endif

#if ENABLE_LOG_ERR
#define LOG_ERR(fmt, ...)   LOG_Print("[ERROR] " fmt, ##__VA_ARGS__)
#else
#define LOG_ERR(fmt, ...)   do {} while(0)
#endif

#if ENABLE_LOG_BUF
#define LOG_BUF(fmt, ...)   LOG_Print(fmt, ##__VA_ARGS__)
#else
#define LOG_BUF(fmt, ...)   do {} while(0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __UART_H */
