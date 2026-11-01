//
// Created by wangy on 2026/10/29.
//

#include "PAW3327.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "PAW3327_Ins.h"
#include <stddef.h>

extern SPI_HandleTypeDef hspi2;
static PAW3327_Diagnostics diagnostics;
const PAW3327_Diagnostics *PAW3327_GetDiagnostics(void)
{
    return &diagnostics;
}

/**
 * @brief 拉低对应从机片选电频，使能芯片
 */
void PAW3327_Enable(void) {
    HAL_GPIO_WritePin(W25Q64_NCS_GPIO_Port, W25Q64_NCS_Pin,GPIO_PIN_SET);
    HAL_GPIO_WritePin(PAW3327_NCS_GPIO_Port, PAW3327_NCS_Pin,GPIO_PIN_RESET);
}

/**
 * @brief 拉高对应芯片片选电平，失能芯片
 */
void PAW3327_Disable(void) {
    HAL_GPIO_WritePin(PAW3327_NCS_GPIO_Port, PAW3327_NCS_Pin,GPIO_PIN_SET);
}

/**
 * @brief 模拟SPI发送过程：双向循环移位发送
 * @param SendData 要发送给从机的数据
 * @return 接收来自从机的数据
 */
uint8_t PAW3327_SwapByte(uint8_t SendData) {
    taskENTER_CRITICAL();
    uint8_t ReceiveData = 0xFF;
    HAL_SPI_TransmitReceive(&hspi2, &SendData, &ReceiveData, 1, 100);
    taskEXIT_CRITICAL();
    return  ReceiveData;
}


/**
 * @brief 读取一个ID寄存器
 * @note  使用毫秒延时，适用于初始化和调试读取ID
 */
static uint8_t PAW3327_ReadIDRegister(uint8_t address)
{
    uint8_t value = 0;
    PAW3327_Enable();
    PAW3327_SwapByte(address & PAW3327_READ_ADDRESS_MASK);
    value = PAW3327_SwapByte(PAW3327_DUMMY_BYTE);
    PAW3327_Disable();
    return value;
}

/**
 * @brief 读取PAW3327产品ID及其反码
 * @param MID 产品ID，正常值为0x4C
 * @param DID 产品ID反码，正常值为0x00B3
 */
void PAW3327_GetID(uint8_t *MID, uint16_t *DID)
{
    if ((MID == NULL) || (DID == NULL))
    {
        return;
    }

    *MID = PAW3327_ReadIDRegister(PAW3327_REG_PRODUCT_ID);

    *DID = (uint16_t)PAW3327_ReadIDRegister(
        PAW3327_REG_INV_PRODUCT_ID);
}

void PAW3327_WriteRegister(uint8_t address, uint8_t value)
{
    PAW3327_Enable();
    (void)PAW3327_SwapByte((address & PAW3327_READ_ADDRESS_MASK) | PAW3327_WRITE_FLAG);
    (void)PAW3327_SwapByte(value);
    PAW3327_Disable();
}

uint8_t PAW3327_ReadRegister(uint8_t address)
{
    uint8_t value;
    PAW3327_Enable();
    (void)PAW3327_SwapByte(address & PAW3327_READ_ADDRESS_MASK);
    value = PAW3327_SwapByte(PAW3327_DUMMY_BYTE);
    PAW3327_Disable();

    return value;
}

/* 返回1：成功；返回0：ID或厂家初始化校验不通过 */
uint8_t PAW3327_Init(void)
{
    uint8_t product_id;
    uint16_t inverse_id;
    uint8_t var_a;
    uint8_t var_b;
    uint8_t status;
    uint32_t errors_before = diagnostics.spi_errors;

    diagnostics.init_status = PAW3327_INIT_NOT_STARTED;
    diagnostics.product_id = 0xFFU;
    diagnostics.inverse_id = 0xFFU;
    diagnostics.calibration_status = 0;

    /* 先让NCS为高；随后WriteRegister会拉低NCS */
    PAW3327_Disable();
    PAW3327_WriteRegister(PAW3327_REG_POWER_UP_RESET,
                         PAW3327_RESET_VALUE);
    HAL_Delay(PAW3327_RESET_WAIT_MS);

    /* 无论MOT是否置位，都按手册顺序读取一次 */
    (void)PAW3327_ReadRegister(PAW3327_REG_MOTION);
    (void)PAW3327_ReadRegister(PAW3327_REG_DELTA_X_L);
    (void)PAW3327_ReadRegister(PAW3327_REG_DELTA_X_H);
    (void)PAW3327_ReadRegister(PAW3327_REG_DELTA_Y_L);
    (void)PAW3327_ReadRegister(PAW3327_REG_DELTA_Y_H);

    /* 额外检查基本通信 */
    PAW3327_GetID(&product_id, &inverse_id);
    if (diagnostics.spi_errors != errors_before)
    {
        diagnostics.init_status = PAW3327_INIT_SPI_ERROR;
        return 0;
    }
    if ((product_id != PAW3327_EXPECTED_PRODUCT_ID) ||
        (inverse_id != PAW3327_EXPECTED_INV_ID))
    {
        diagnostics.init_status = PAW3327_INIT_ID_ERROR;
        return 0;
    }

    /* 以下严格对应手册第22页第6步 */
    PAW3327_WriteRegister(PAW3327_REG_AXIS_CONTROL, 0x40U);
    PAW3327_WriteRegister(PAW3327_REG_BANK_SELECT, PAW3327_BANK_1);
    PAW3327_WriteRegister(PAW3327_REG_INIT_1A, 0x50U);
    PAW3327_WriteRegister(PAW3327_REG_INIT_6F, 0xFFU);
    PAW3327_WriteRegister(PAW3327_REG_INIT_6C, 0x3BU);
    PAW3327_WriteRegister(PAW3327_REG_INIT_6D, 0x2BU);
    PAW3327_WriteRegister(PAW3327_REG_INIT_7A, 0x4FU);
    PAW3327_WriteRegister(PAW3327_REG_INIT_49, 0x40U);

    PAW3327_WriteRegister(PAW3327_REG_BANK_SELECT, PAW3327_BANK_0);
    PAW3327_WriteRegister(PAW3327_REG_INIT_4A, 0x16U);
    PAW3327_WriteRegister(PAW3327_REG_INIT_4D, 0x53U);
    PAW3327_WriteRegister(PAW3327_REG_RESOLUTION, PAW3327_CPI_1500);
    PAW3327_WriteRegister(PAW3327_REG_INIT_62, 0x16U);
    PAW3327_WriteRegister(PAW3327_REG_INIT_1A, 0x11U);

    PAW3327_WriteRegister(PAW3327_REG_BANK_SELECT, PAW3327_BANK_3);
    (void)PAW3327_ReadRegister(PAW3327_REG_INIT_1C);
    var_a = PAW3327_ReadRegister(PAW3327_REG_INIT_1A);
    var_b = (uint8_t)(((var_a >> 1) + 0x23U) ^ 0x5AU);
    PAW3327_WriteRegister(PAW3327_REG_INIT_1B, var_b);
    status = PAW3327_ReadRegister(PAW3327_REG_INIT_1C);

    diagnostics.calibration_status = status;
    if (diagnostics.spi_errors != errors_before)
    {
        diagnostics.init_status = PAW3327_INIT_SPI_ERROR;
        return 0;
    }
    if ((status & PAW3327_INIT_OK_MASK) == 0U)
    {
        /* 手册要求断电重上电；本函数无法替代物理断电 */
        diagnostics.init_status = PAW3327_INIT_CAL_ERROR;
        return 0;
    }

    PAW3327_WriteRegister(PAW3327_REG_BANK_SELECT, PAW3327_BANK_0);
    if (diagnostics.spi_errors != errors_before)
    {
        diagnostics.init_status = PAW3327_INIT_SPI_ERROR;
        return 0;
    }
    diagnostics.init_status = PAW3327_INIT_OK;
    return 1;
}

/* 返回1：有运动；返回0：无运动或参数为空 */
uint8_t PAW3327_ReadMotion(int16_t *dx, int16_t *dy)
{
    uint8_t data[PAW3327_BURST_BYTES];
    uint8_t i;
    uint32_t errors_before = diagnostics.spi_errors;

    if ((dx == NULL) || (dy == NULL))
    {
        return 0;
    }

    *dx = 0;
    *dy = 0;

    PAW3327_Enable();
    (void)PAW3327_SwapByte(
        PAW3327_REG_MOTION_BURST & PAW3327_READ_ADDRESS_MASK);

    /* No turnaround delay after the Motion Burst address. */
    for (i = 0; i < PAW3327_BURST_BYTES; i++)
    {
        data[i] = PAW3327_SwapByte(PAW3327_DUMMY_BYTE);
    }

    PAW3327_Disable();

    if ((diagnostics.spi_errors != errors_before) ||
        ((data[0] & PAW3327_MOTION_FLAG) == 0U))
    {
        return 0;
    }

    /* data[1]是Observation，不是X低字节 */
    *dx = (int16_t)(((uint16_t)data[3] << 8) | data[2]);
    *dy = (int16_t)(((uint16_t)data[5] << 8) | data[4]);

    return 1;
}