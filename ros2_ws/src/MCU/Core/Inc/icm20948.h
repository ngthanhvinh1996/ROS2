/**
  ******************************************************************************
  * @file           : icm20948.h
  * @brief          : Header for icm20948.c file.
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
#ifndef __ICM20948_H
#define __ICM20948_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "common.h"
#include "uart.h"
#include "math.h"

/* Private define ----------------------------------------------------------*/
/* I2C Address of ICM */
#define ICM20948_DEV    0x68

/* User bank register map */
#define REG_VAL_SELECT_BANK_0 0x00
#define REG_VAL_SELECT_BANK_1 0x10
#define REG_VAL_SELECT_BANK_2 0x20
#define REG_VAL_SELECT_BANK_3 0x30

/* Bank write */
#define REG_BANK_SEL 0x7F

/* Bank 0 */
#define WHO_AM_I     0x00  //��ȡ�üĴ���ֵ,����0xEA
#define USER_CTRL    0x03
#define LP_CONFIG    0x05
#define PWR_MGMT_1   0x06
#define PWR_MGMT_2   0x07
#define INT_PIN_CFG  0x0F
#define INT_ENABLE   0x10
#define INT_ENABLE_1 0x11
#define INT_ENABLE_2 0x12
#define INT_ENABLE_3 0x13
#define I2C_MST_STATUS 0x17
#define INT_STATUS   0x19
#define INT_STATUS_1 0x1A
#define INT_STATUS_2 0x1B
#define INT_STATUS_3 0x1C
#define DELAY_TIMEH  0x28
#define DELAY_TIMEL  0x29
#define ACCEL_XOUT_H 0x2D
#define ACCEL_XOUT_L 0x2E
#define ACCEL_YOUT_H 0x2F
#define ACCEL_YOUT_L 0x30
#define ACCEL_ZOUT_H 0x31
#define ACCEL_ZOUT_L 0x32
#define GYRO_XOUT_H  0x33
#define GYRO_XOUT_L  0x34
#define GYRO_YOUT_H  0x35
#define GYRO_YOUT_L  0x36
#define GYRO_ZOUT_H  0x37
#define GYRO_ZOUT_L  0x38
#define TEMP_OUT_H   0x39
#define TEMP_OUT_L   0x3A
#define EXT_SLV_SENS_DATA_00 0x3B
#define EXT_SLV_SENS_DATA_01 0x3C
#define EXT_SLV_SENS_DATA_02 0x3D
#define EXT_SLV_SENS_DATA_03 0x3E
#define EXT_SLV_SENS_DATA_04 0x3F
#define EXT_SLV_SENS_DATA_05 0x40
#define EXT_SLV_SENS_DATA_06 0x41
#define EXT_SLV_SENS_DATA_07 0x42
#define EXT_SLV_SENS_DATA_08 0x43
#define EXT_SLV_SENS_DATA_09 0x44
#define EXT_SLV_SENS_DATA_10 0x45
#define EXT_SLV_SENS_DATA_11 0x46
#define EXT_SLV_SENS_DATA_12 0x47
#define EXT_SLV_SENS_DATA_13 0x48
#define EXT_SLV_SENS_DATA_14 0x49
#define EXT_SLV_SENS_DATA_15 0x4A
#define EXT_SLV_SENS_DATA_16 0x4B
#define EXT_SLV_SENS_DATA_17 0x4C
#define EXT_SLV_SENS_DATA_18 0x4D
#define EXT_SLV_SENS_DATA_19 0x4E
#define EXT_SLV_SENS_DATA_20 0x4F
#define EXT_SLV_SENS_DATA_21 0x50
#define EXT_SLV_SENS_DATA_22 0x51
#define EXT_SLV_SENS_DATA_23 0x52
#define FIFO_EN_1       0x66
#define FIFO_EN_2       0x67
#define FIFO_RST        0x68
#define FIFO_MODE       0x69
#define FIFO_COUNTH     0x70
#define FIFO_COUNTL     0x71
#define FIFO_R_W        0x72
#define DATA_RDY_STATUS 0x74
#define FIFO_CFG        0x76

/* Bank 1 */
#define SELF_TEST_X_GYRO 0x02
#define SELF_TEST_Y_GYRO 0x03
#define SELF_TEST_Z_GYRO 0x04
#define SELF_TEST_X_ACCEL 0x0E
#define SELF_TEST_Y_ACCEL 0x0F
#define SELF_TEST_Z_ACCEL 0x10
#define XA_OFFS_H 0x14
#define XA_OFFS_L 0x15
#define YA_OFFS_H 0x17
#define YA_OFFS_L 0x18
#define ZA_OFFS_H 0x1A
#define ZA_OFFS_L 0x1B
#define TIMEBASE_CORRECTION_PLL 0x28

/* Bank 2 */
#define GYRO_SMPLRT_DIV 0x00
#define GYRO_CONFIG_1 0x01
#define GYRO_CONFIG_2 0x02
#define XG_OFFS_USRH 0x03
#define XG_OFFS_USRL 0x04
#define YG_OFFS_USRH 0x05
#define YG_OFFS_USRL 0x06
#define ZG_OFFS_USRH 0x07
#define ZG_OFFS_USRL 0x08
#define ODR_ALIGN_EN 0x09
#define ACCEL_SMPLRT_DIV_1 0x10
#define ACCEL_SMPLRT_DIV_2 0x11
#define ACCEL_INTEL_CTRL 0x12
#define ACCEL_WOM_THR    0x13
#define ACCEL_CONFIG 0x14
#define ACCEL_CONFIG_2 0x15
#define FSYNC_CONFIG 0x52
#define TEMP_CONFIG 0x53
#define MOD_CTRL_USR 0x54

/* Bank 3 */
#define I2C_MST_ODR_CONFIG 0x00
#define I2C_MST_CTRL 0x01
#define I2C_MST_DELAY_CTRL 0x02
#define I2C_SLV0_ADDR 0x03
#define I2C_SLV0_REG 0x04
#define I2C_SLV0_CTRL 0x05
#define I2C_SLV0_DO 0x06
#define I2C_SLV1_ADDR 0x07
#define I2C_SLV1_REG 0x08
#define I2C_SLV1_CTRL 0x09
#define I2C_SLV1_DO 0x0A
#define I2C_SLV2_ADDR 0x0B
#define I2C_SLV2_REG 0x0C
#define I2C_SLV2_CTRL 0x0D
#define I2C_SLV2_DO 0x0E
#define I2C_SLV3_ADDR 0x0F
#define I2C_SLV3_REG 0x10
#define I2C_SLV3_CTRL 0x11
#define I2C_SLV3_DO 0x12
#define I2C_SLV4_ADDR 0x13
#define I2C_SLV4_REG 0x14
#define I2C_SLV4_CTRL 0x15
#define I2C_SLV4_DO 0x16
#define I2C_SCL4_DI 0x17

#define AK09916_DEV 0x0C
#define WIA 0x01
#define ST1 0x10
#define HXL 0x11
#define HXH 0x12
#define HYL 0x13
#define HYH 0x14
#define HZL 0x15
#define HZH 0x16
#define ST2 0x18
#define CNTL2 0x31
#define CNTL3 0x32

#define SIIC_SDA_H  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET)
#define SIIC_SDA_L  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET)
#define SIIC_SCL_H  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET)
#define SIIC_SCL_L  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET)

#define userconfig_DELAY_TIME 25
/* Private variables ---------------------------------------------------------*/
extern I2C_HandleTypeDef hi2c1;

/* Exported types ------------------------------------------------------------*/
typedef struct
{
    float roll;
    float pitch;
    float yaw;
} Orientation_t;

typedef struct
{
  float x;
  float y;
  float z;
} PrivateBuf_t;

typedef struct 
{
    PrivateBuf_t gyro;
    PrivateBuf_t accel;
    PrivateBuf_t magn;
} Imu_Data_t;

typedef struct {
    void (*init)(void);
    HAL_StatusTypeDef (*write)(uint16_t DevAddr, uint8_t *pData, uint16_t Size, uint32_t Timeout);
    HAL_StatusTypeDef (*read)(uint16_t DevAddr, uint8_t *pData, uint16_t Size, uint32_t Timeout);
    HAL_StatusTypeDef (*write_reg)(uint16_t DevAddr, uint16_t MemAddr, uint8_t *pData, uint16_t Size, uint32_t Timeout);
    HAL_StatusTypeDef (*read_reg)(uint16_t DevAddr, uint16_t MemAddr, uint8_t *pData, uint16_t Size, uint32_t Timeout);
} ICMInterface_t;
/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/ 
extern ICMInterface_t User_sICMDev;

/* Exported functions prototypes ---------------------------------------------*/
uint8_t ICM20948_Init(void);
void Read_Imu_Data(Imu_Data_t *data);
void Read_Imu_Raw_Data(uint8_t *data);
void Read_Gyro_Angle(PrivateBuf_t *gyro);
void Update_Orientation(Orientation_t *orientation, 
                        float accX, 
                        float accY, 
                        float accZ, 
                        float gyroX, 
                        float gyroY, 
                        float gyroZ, 
                        float magX, 
                        float magY, 
                        float magZ);

#ifdef __cplusplus
}
#endif

#endif /* __ICM20948_H */
