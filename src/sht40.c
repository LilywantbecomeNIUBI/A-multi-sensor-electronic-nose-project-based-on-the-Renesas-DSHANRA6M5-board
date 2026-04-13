/*
 * sht40.c
 *
 *  Created on: 2026年3月12日
 *      Author: leo
 */

#include "sht40.h"
#include "i2c_master.h"
#include "hal_data.h"

/*
 * 硬件功能：
 * - SHT40 读回数据的 CRC8 校验
 *
 * 依赖手册：
 * - SHT4x Datasheet 4.3 Checksum Calculation
 *
 * 前提假设：
 * - SHT40 读数据的 CRC 参数为：
 *   Polynomial = 0x31
 *   Init       = 0xFF
 *   RefIn/RefOut = false
 *   Final XOR    = 0x00
 */
static uint8_t SHT40_CalcCRC8(uint8_t const * data, uint32_t len)
{
    uint8_t crc = 0xFFU;
    uint32_t j;
    uint32_t i;

    if (NULL == data)
    {
        return 0U;
    }

    for (j = 0U; j < len; j++)
    {
        crc ^= data[j];
        for (i = 0U; i < 8U; i++)
        {
            if (0U != (crc & 0x80U))
            {
                crc = (uint8_t) ((crc << 1) ^ 0x31U);
            }
            else
            {
                crc = (uint8_t) (crc << 1);
            }
        }
    }

    return crc;
}

/*
 * 硬件功能：
 * - 初始化 SHT40，发送 soft reset(0x94)
 *
 * 依赖手册：
 * - SHT4x Datasheet 4.4 Command Overview
 * - SHT4x Datasheet 4.7 Reset & Abort
 * - soft reset time 最大 1ms
 *
 * 前提假设：
 * - I2C 平台层已经正确配置为 g_i2c_master0 / channel 2
 * - 当前使用 7-bit 地址 0x44
 */
static int SHT40DrvInit(struct SHT40Dev * ptdev)
{
    fsp_err_t err;
    uint8_t cmd = SHT40_CMD_SOFT_RESET;

    if (NULL == ptdev)
    {
        return -1;
    }

    err = I2CMaster_SlaveAddrSet(SHT40_I2C_ADDR);
    if (FSP_SUCCESS != err)
    {
        return -10;
    }

    /*
     * 发送 soft reset 命令，发送完成后产生 Stop
     *
     * 注意：
     * - 不再额外调用 I2CMaster_WaitTxComplete()
     * - 因为你当前 i2c_master.c 的 I2CMaster_Write() 内部已经等待完成
     */
    err = I2CMaster_Write(&cmd, 1U, false);
    if (FSP_SUCCESS != err)
    {
        return -11;
    }

    /* 手册要求 soft reset 后最大 1ms 进入 idle，这里保守等待 2ms */
    R_BSP_SoftwareDelay(2U, BSP_DELAY_UNITS_MILLISECONDS);

    return 0;
}

/*
 * 硬件功能：
 * - 触发 SHT40 一次高精度测量并读取温湿度
 *
 * 依赖手册：
 * - SHT4x Datasheet 4.4 Command Overview
 *   0xFD -> high precision measurement
 * - SHT4x Datasheet 4.2 Data type & length
 *   返回 6 字节：T(2B)+CRC + RH(2B)+CRC
 * - SHT4x Datasheet 4.5 Conversion of Signal Output
 * - SHT4x Datasheet Table 4
 *   high repeatability 最大测量时间 8.2ms
 *
 * 前提假设：
 * - 当前工程按 1 次命令 + 延时 + 1 次读 的最小阻塞方式联调
 */
static int SHT40DrvRead(struct SHT40Dev * ptdev)
{
    fsp_err_t err;
    uint8_t cmd = SHT40_CMD_MEASURE_HIGH_RES;
    uint8_t rx_buf[6] = {0};
    uint16_t t_ticks;
    uint16_t rh_ticks;
    float temp_c;
    float rh_p;

    if (NULL == ptdev)
    {
        return -1;
    }

    /*
     * 先切到 SHT40 的 7-bit 地址 0x44
     * 对应硬件功能：避免总线之前被其他器件改到别的从机地址
     */
    err = I2CMaster_SlaveAddrSet(SHT40_I2C_ADDR);
    if (FSP_SUCCESS != err)
    {
        return -20;
    }

    /*
     * 发送高精度测量命令 0xFD
     * 对应硬件功能：启动 SHT40 一次 RH/T 转换
     *
     * 注意：
     * - 不再额外手动 WaitTx
     * - 由 I2CMaster_Write() 内部完成等待
     */
    err = I2CMaster_Write(&cmd, 1U, false);
    if (FSP_SUCCESS != err)
    {
        return -21;
    }

    /*
     * high repeatability 最大测量时间 8.2ms
     * 这里保守等待 10ms，便于最小联调
     */
    R_BSP_SoftwareDelay(10U, BSP_DELAY_UNITS_MILLISECONDS);

    /*
     * 读取 6 字节：
     * T_MSB, T_LSB, T_CRC, RH_MSB, RH_LSB, RH_CRC
     *
     * 注意：
     * - 不再额外手动 WaitRx
     * - 由 I2CMaster_Read() 内部完成等待
     */
    err = I2CMaster_Read(rx_buf, 6U, false);
    if (FSP_SUCCESS != err)
    {
        return -22;
    }

    /* 温度 CRC */
    if (SHT40_CalcCRC8(&rx_buf[0], 2U) != rx_buf[2])
    {
        return -2;
    }

    /* 湿度 CRC */
    if (SHT40_CalcCRC8(&rx_buf[3], 2U) != rx_buf[5])
    {
        return -3;
    }

    t_ticks  = (uint16_t) ((((uint16_t) rx_buf[0]) << 8) | rx_buf[1]);
    rh_ticks = (uint16_t) ((((uint16_t) rx_buf[3]) << 8) | rx_buf[4]);

    /*
     * 物理量换算，严格按 datasheet 4.5
     * T  = -45 + 175 * ST  / 65535
     * RH = -6  + 125 * SRH / 65535
     */
    temp_c = -45.0f + 175.0f * ((float) t_ticks / 65535.0f);
    rh_p   = -6.0f  + 125.0f * ((float) rh_ticks / 65535.0f);

    /*
     * 手册明确说 RH 公式可能得到 <0 或 >100 的非物理值，
     * 如果不需要保留 uncropped 值，建议裁剪到 0~100
     */
    if (rh_p < 0.0f)
    {
        rh_p = 0.0f;
    }
    else if (rh_p > 100.0f)
    {
        rh_p = 100.0f;
    }

    ptdev->temperature = temp_c;
    ptdev->humidity    = rh_p;

    return 0;
}

/*
 * 硬件功能：
 * - SHT40 对象实例化与注册
 *
 * 依赖资料：
 * - 参考你当前工程“设备对象 + 函数指针”的教材风格
 *
 * 前提假设：
 * - sht40.h 中结构体定义与这里匹配
 */
static SHT40Device gSHT40Dev =
{
    .name        = "SHT40",
    .temperature = 0.0f,
    .humidity    = 0.0f,
    .Init        = SHT40DrvInit,
    .Read        = SHT40DrvRead
};

SHT40Device * SHT40GetDevice(void)
{
    return &gSHT40Dev;
}
