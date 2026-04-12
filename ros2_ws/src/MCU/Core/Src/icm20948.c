/**
 ******************************************************************************
 * @file           : icm20948.c
 * @brief          : ICM20948 program body
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
#include "icm20948.h"

/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define GYRO_SENSITIVITY        16.4f
#define ACCEL_SENSITIVITY       16384

#define DEG_TO_RAD              0.01745329251f
#define EARTH_GRAVITY           9.81f

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

/* Private function prototypes -----------------------------------------------*/
static void siic_init(void);
static void SDA_IN(void);
static void SDA_OUT(void);
static uint8_t Read_SDA(void);
static void siic_Start(void);
static void siic_Stop(void);
static uint8_t siic_WaitACK(uint32_t timeout);
static void siic_Ack(void);
static void siic_NAck(void);
static void siic_SendByte(uint8_t byte);
static uint8_t siic_ReadByte(uint8_t ack);
static HAL_StatusTypeDef IIC_Master_Transmit(uint16_t dev_addr, uint8_t *data, uint16_t size,uint32_t timeout);
static HAL_StatusTypeDef IIC_Master_Receive(uint16_t dev_addr, uint8_t *data, uint16_t size,uint32_t timeout);
static HAL_StatusTypeDef IIC_Mem_Write(uint16_t dev_addr, uint16_t mem_addr, uint8_t *data, uint16_t size,uint32_t timeout);
static HAL_StatusTypeDef IIC_Mem_Read(uint16_t dev_addr, uint16_t mem_addr, uint8_t *data, uint16_t size,uint32_t timeout);

static void siic_init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
    // __HAL_RCC_I2C2_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // hi2c1.Instance = I2C2;
    // hi2c1.Init.ClockSpeed = 100000;
    // hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    // hi2c1.Init.OwnAddress1 = 0;
    // hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    // hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLED;
    // hi2c1.Init.OwnAddress2 = 0;
    // hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    // hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    // if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    // {
    //     LOG_ERR("I2C init failed./r/n");
    // }
}

static void SDA_IN(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

static void SDA_OUT(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

static uint8_t Read_SDA(void)
{
    return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11);
}

static void siic_Start(void)
{
    uint8_t delay = 25;

    SDA_OUT();

    SIIC_SDA_H;
    SIIC_SCL_H;

    while(delay--);

    SIIC_SDA_L;

    delay = 25;
    while(delay--);

    SIIC_SCL_L;
}

static void siic_Stop(void)
{
    uint8_t delay = 25;

    SDA_OUT();

    SIIC_SCL_L;
    SIIC_SDA_L;

    while(delay--);

    SIIC_SCL_H;

    delay = 25;
    while(delay--);

    SIIC_SDA_H;
}

static uint8_t siic_WaitACK(uint32_t timeout)
{
    uint32_t time = 0;

    SDA_IN();
    SIIC_SDA_H;
	
    uint8_t delay = userconfig_DELAY_TIME;
    while(delay--);
	
    SIIC_SCL_H;
	
    delay = userconfig_DELAY_TIME;
    while(delay--);
	
    while (Read_SDA()) { 
        time++;
        if (time > timeout) {
            siic_Stop();
            return 0;
        }
    }

    SIIC_SCL_L;
    return 1;
}

static void siic_Ack(void)
{
    SIIC_SCL_L;
    SDA_OUT();
    SIIC_SDA_L;
	
    uint8_t delay = userconfig_DELAY_TIME;
    while(delay--);
	
    SIIC_SCL_H;
	
    delay = userconfig_DELAY_TIME;
    while(delay--);
	
    SIIC_SCL_L;
}

static void siic_NAck(void)
{
    SIIC_SCL_L;
    SDA_OUT();
    SIIC_SDA_H;
	
    uint8_t delay = userconfig_DELAY_TIME;
    while(delay--);
	
    SIIC_SCL_H;
	
    delay = userconfig_DELAY_TIME;
    while(delay--);
	
    SIIC_SCL_L;
}

static void siic_SendByte(uint8_t byte)
{
    uint8_t i;

    SDA_OUT();
    SIIC_SCL_L;

    for (i = 0; i < 8; i++) {
        if (byte & 0x80) {
            SIIC_SDA_H;
        } else {
            SIIC_SDA_L;
        }
        byte <<= 1;
		
			uint8_t delay = userconfig_DELAY_TIME;
			while(delay--);
		
        SIIC_SCL_H;
		
			delay = userconfig_DELAY_TIME;
			while(delay--);
		
        SIIC_SCL_L;

    }
}

static uint8_t siic_ReadByte(uint8_t ack)
{
    uint8_t i, byte = 0;

    SDA_IN();

    for (i = 0; i < 8; i++) {
        SIIC_SCL_L;

        uint8_t delay = userconfig_DELAY_TIME;
        while(delay--);
		
        SIIC_SCL_H;
		
        delay = userconfig_DELAY_TIME;
        while(delay--);
		
        byte <<= 1;
        if (Read_SDA()) {
            byte |= 0x01;
        }
    }

    if (!ack) {
        siic_NAck();
    } else {
        siic_Ack();
    }

    return byte;
}

static HAL_StatusTypeDef IIC_Master_Transmit(uint16_t dev_addr, uint8_t *data, uint16_t size,uint32_t timeout)
{
    siic_Start();

    siic_SendByte(dev_addr);
    if (!siic_WaitACK(timeout)) {
        siic_Stop();
        return HAL_TIMEOUT;
    }

    for (uint16_t i = 0; i < size; i++) {
        siic_SendByte(data[i]);
        if (!siic_WaitACK(timeout)) {
            siic_Stop();
            return HAL_TIMEOUT;
        }
    }

    siic_Stop();
    return HAL_OK;
}

static HAL_StatusTypeDef IIC_Master_Receive(uint16_t dev_addr, uint8_t *data, uint16_t size,uint32_t timeout)
{
    siic_Start();

    siic_SendByte(dev_addr | 0x01);
    if (!siic_WaitACK(timeout)) {
        siic_Stop();
        return HAL_TIMEOUT;
    }

    for (uint16_t i = 0; i < size; i++) {
        data[i] = siic_ReadByte(i == (size - 1) ? 0 : 1);
    }

    siic_Stop();
    return HAL_OK;
}

static HAL_StatusTypeDef IIC_Mem_Write(uint16_t dev_addr, uint16_t mem_addr, uint8_t *data, uint16_t size,uint32_t timeout)
{
    siic_Start();
    
    siic_SendByte(dev_addr);
    if (!siic_WaitACK(timeout)) return HAL_TIMEOUT;

    siic_SendByte(mem_addr);
    if (!siic_WaitACK(timeout)) return HAL_TIMEOUT;

    for (uint16_t i = 0; i < size; i++) {
        siic_SendByte(data[i]);
        if (!siic_WaitACK(timeout)) return HAL_TIMEOUT;
    }
    
    siic_Stop();
    return HAL_OK;
}

static HAL_StatusTypeDef IIC_Mem_Read(uint16_t dev_addr, uint16_t mem_addr, uint8_t *data, uint16_t size,uint32_t timeout)
{
    siic_Start();
    
    siic_SendByte(dev_addr);
    if (!siic_WaitACK(timeout)) return HAL_TIMEOUT;

    siic_SendByte(mem_addr);
    if (!siic_WaitACK(timeout)) return HAL_TIMEOUT;
    
    siic_Start();
    siic_SendByte(dev_addr | 0x01);
    if (!siic_WaitACK(timeout)) return HAL_TIMEOUT;

    for (uint16_t i = 0; i < size; i++) {
        data[i] = siic_ReadByte(i == (size - 1) ? 0 : 1);
    }

    siic_Stop();
    return HAL_OK;
}

ICMInterface_t User_sICMDev = {
    .init = siic_init,
    .write = IIC_Master_Transmit,
    .read = IIC_Master_Receive,
    .write_reg = IIC_Mem_Write,
    .read_reg = IIC_Mem_Read,
};

/* Global user code ---------------------------------------------------------*/

uint8_t ICM20948_Init(void)
{
    uint8_t buf_w = 0U;
    ICMInterface_t *icmDev = &User_sICMDev;

    icmDev->init();

    buf_w = REG_VAL_SELECT_BANK_0;
    if(HAL_OK != icmDev->write_reg(ICM20948_DEV << 1U, REG_BANK_SEL, &buf_w, 1U, 500U))
    {
        LOG_ERR("Error REG_BANK_SEL 1\r\n");
        return 1;
    }

    if(HAL_OK != icmDev->read_reg(ICM20948_DEV << 1U, WHO_AM_I, &buf_w, 1U, 500U))
    {
        LOG_ERR("Error WHO_AM_I\r\n");
        return 1;
    }

    if(0xEA != buf_w)
    {
        LOG_ERR("Error buf_w=0x%x\r\n", buf_w);
        return 1;
    }

    buf_w = (1U << 7U);
    if(HAL_OK != icmDev->write_reg(ICM20948_DEV << 1U, PWR_MGMT_1, &buf_w, 1U, 500U))
    {
        LOG_ERR("Error PWR_MGMT_1\r\n");
        return 1;
    }
    HAL_Delay(100);

    buf_w = 0x00U;
    if(HAL_OK != icmDev->write_reg(ICM20948_DEV << 1U, USER_CTRL, &buf_w, 1U, 500U))
    {
        LOG_ERR("Error USER_CTRL\r\n");
        return 1;
    }

    buf_w = 0x01U;
    if(HAL_OK != icmDev->write_reg(ICM20948_DEV << 1U, PWR_MGMT_1, &buf_w, 1U, 100U))
    {
        LOG_ERR("Error PWR_MGMT_1\r\n");
        return 1;
    }

    buf_w = REG_VAL_SELECT_BANK_2;
    if(HAL_OK != icmDev->write_reg(ICM20948_DEV << 1U, REG_BANK_SEL, &buf_w, 1U, 100U))
    {
        LOG_ERR("Error REG_BANK_SEL\r\n");
        return 1;
    }

    buf_w = 0x04U;
    if(HAL_OK != icmDev->write_reg(ICM20948_DEV << 1U, GYRO_SMPLRT_DIV, &buf_w, 1U, 100U))
    {
        LOG_ERR("Error GYRO_SMPLRT_DIV\r\n");
        return 1;
    }

    buf_w = (3U << 1U) | (1U << 0U) | (3U << 3U);
    if(HAL_OK != icmDev->write_reg(ICM20948_DEV << 1U, GYRO_CONFIG_1, &buf_w, 1U, 100U))
    {
        LOG_ERR("Error GYRO_CONFIG_1\r\n");
        return 1;
    }

    buf_w = 0x04U;
    if(HAL_OK != icmDev->write_reg(ICM20948_DEV << 1U, ACCEL_SMPLRT_DIV_2, &buf_w, 1U, 100U))
    {
        LOG_ERR("Error ACCEL_SMPLRT_DIV_2\r\n");
        return 1;
    }

    buf_w = (0U << 1U) | (1U << 0U) | (5U << 3U);
    if(HAL_OK != icmDev->write_reg(ICM20948_DEV << 1U, ACCEL_CONFIG, &buf_w, 1U, 100U))
    {
        LOG_ERR("Error ACCEL_CONFIG\r\n");
        return 1;
    }

    buf_w = REG_VAL_SELECT_BANK_0;
    if(HAL_OK != icmDev->write_reg(ICM20948_DEV << 1U, REG_BANK_SEL, &buf_w, 1U, 100U))
    {
        LOG_ERR("Error REG_BANK_SEL\r\n");
        return 1;
    }

    buf_w = (1U << 1U);
    if(HAL_OK != icmDev->write_reg(ICM20948_DEV << 1U, INT_PIN_CFG, &buf_w, 1U, 100U))
    {
        LOG_ERR("Error INT_PIN_CFG\r\n");
        return 1;
    }

    icmDev->read_reg(AK09916_DEV << 1U, WIA, &buf_w, 1U, 100U);
    if(0x09 != buf_w)
    {
        LOG_ERR("Error WIA\r\n");
        return 1;
    }

    buf_w = (1U << 3U);
    if(HAL_OK != icmDev->write_reg(AK09916_DEV << 1U, CNTL2, &buf_w, 1U, 100U))
    {
        LOG_ERR("Error CNTL2\r\n");
        return 1;
    }

    return 0;
}

void Read_Imu_Data(Imu_Data_t *data)
{
    uint8_t buf_tmp[12U] = {0U};
    uint8_t buf_magn[8U] = {0U};

    ICMInterface_t *icmDev = &User_sICMDev;

    if(HAL_OK != icmDev->read_reg(ICM20948_DEV << 1U, ACCEL_XOUT_H, &buf_tmp, 12U, 100U))
    {
        return;
    }

    data->accel.x = (short)(buf_tmp[0]<<8 | buf_tmp[1]);
	data->accel.y = (short)(buf_tmp[2]<<8 | buf_tmp[3]);
	data->accel.z = (short)(buf_tmp[4]<<8 | buf_tmp[5]);
	
	// data->accel.x *= 0.00059814453125f;
	// data->accel.y *= 0.00059814453125f;
	// data->accel.z *= 0.00059814453125f;

    data->accel.x = (data->accel.x / ACCEL_SENSITIVITY) * EARTH_GRAVITY;
	data->accel.y = (data->accel.y / ACCEL_SENSITIVITY) * EARTH_GRAVITY;
	data->accel.z = (data->accel.z / ACCEL_SENSITIVITY) * EARTH_GRAVITY;

    data->gyro.x = (short)(buf_tmp[6]<<8 | buf_tmp[7]);
	data->gyro.y = (short)(buf_tmp[8]<<8 | buf_tmp[9]);
	data->gyro.z = (short)(buf_tmp[10]<<8 | buf_tmp[11]);
	
	// data->gyro.x *= 0.01745329252f;
	// data->gyro.y *= 0.01745329252f;
	// data->gyro.z *= 0.01745329252f;

    data->gyro.x = (data->gyro.x / GYRO_SENSITIVITY) * DEG_TO_RAD;
	data->gyro.y = (data->gyro.y / GYRO_SENSITIVITY) * DEG_TO_RAD;
	data->gyro.z = (data->gyro.z / GYRO_SENSITIVITY) * DEG_TO_RAD;

    icmDev->read_reg(AK09916_DEV << 1U, HXL, &buf_magn, 8U, 500);
    if( 0 == ((buf_magn[7]>>3)&0x01) ) 
	{
		data->magn.x = (float)(buf_magn[1]<<8 | buf_magn[0]);
		data->magn.y = (float)(buf_magn[3]<<8 | buf_magn[2]);
		data->magn.z = (float)(buf_magn[5]<<8 | buf_magn[4]);
		
		// data->magn.x *= 0.1495361328125f;
		// data->magn.y *= 0.1495361328125f;
		// data->magn.z *= 0.1495361328125f;
	}
}

void Read_Imu_Raw_Data(uint8_t *data)
{
    uint8_t index = 0U;
    uint8_t buf_tmp[12U] = {0U};
    uint8_t buf_magn[8U] = {0U};

    ICMInterface_t *icmDev = &User_sICMDev;

    if(HAL_OK != icmDev->read_reg(ICM20948_DEV << 1U, ACCEL_XOUT_H, &buf_tmp, 12U, 100U))
    {
        return;
    }
	
    for(index = 0U; index < 12U; index++)
    {
        data[index] = buf_tmp[index];
    }

    icmDev->read_reg(AK09916_DEV << 1U, HXL, &buf_magn, 8U, 500);
    if( 0 == ((buf_magn[7]>>3)&0x01) ) 
	{
		for(index = 0U; index < 8U; index++)
        {
            data[index + 12U] = buf_magn[index];
        }
	}
}

void Read_Gyro_Angle(PrivateBuf_t *gyro)
{
    uint8_t buf_tmp[6U] = {0U};

    ICMInterface_t *icmDev = &User_sICMDev;

    if(HAL_OK != icmDev->read_reg(ICM20948_DEV << 1U, GYRO_XOUT_H, &buf_tmp, 6U, 100U))
    {
        return;
    }

    gyro->x = (short)(buf_tmp[0]<<8 | buf_tmp[1]);
	gyro->y = (short)(buf_tmp[2]<<8 | buf_tmp[3]);
	gyro->z = (short)(buf_tmp[4]<<8 | buf_tmp[5]);
}
