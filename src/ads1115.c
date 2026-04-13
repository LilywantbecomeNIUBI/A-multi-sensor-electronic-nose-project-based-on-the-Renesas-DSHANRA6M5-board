/*
 * ads1115.c
 *
 *  Created on: 2026年3月11日
 *      Author: leo
 */
/*
 * ads1115.c
 *
 *  对应硬件功能：
 *  - ADS1115 单器件驱动（单端、single-shot、四通道轮询）
 *
 *  依赖资料：
 *  - ADS1115 数据手册：Pointer/Config 寄存器、MUX/PGA/MODE/DR、OS 位、数据格式
 *  - chapter15 Common I2C API 风格
 *  - 当前工程 i2c_master.h/.c 平台封装
 *
 *  前提假设：
 *  - 当前项目仅使用 1 块 ADS1115，默认地址为 0x48
 *  - 不使用 ALERT/RDY 引脚
 *  - I2C 总线平台层已由 i2c_master 模块统一封装
 */

#include "ads1115.h"

static bool g_ads1115_opened = false;
static uint8_t g_ads1115_slave_addr = ADS1115_I2C_ADDR_DEFAULT;
static ads1115_pga_t g_ads1115_pga = ADS1115_PGA_6_144V;
static ads1115_data_rate_t g_ads1115_dr = ADS1115_DR_128SPS;

/*
 * 对应硬件功能：
 * - 将当前器件地址切换到 ADS1115
 * 依赖资料：
 * - 当前平台层 I2CMaster_SlaveAddrSet()
 * 前提假设：
 * - 总线已经由 I2CMaster_Open() 打开
 */
static fsp_err_t ADS1115_SelectDevice(void)
{
    return I2CMaster_SlaveAddrSet(g_ads1115_slave_addr);
}

/*
 * 对应硬件功能：
 * - 根据单端通道生成 MUX 位
 * 依赖资料：
 * - ADS1115 数据手册 MUX[14:12] 单端定义
 * 前提假设：
 * - channel 已经做过范围校验
 */
static uint16_t ADS1115_GetSingleEndedMuxBits(ads1115_channel_t channel)
{
    switch (channel)
    {
        case ADS1115_CHANNEL_AIN0:
            return ADS1115_CONFIG_MUX_AIN0_GND;

        case ADS1115_CHANNEL_AIN1:
            return ADS1115_CONFIG_MUX_AIN1_GND;

        case ADS1115_CHANNEL_AIN2:
            return ADS1115_CONFIG_MUX_AIN2_GND;

        case ADS1115_CHANNEL_AIN3:
            return ADS1115_CONFIG_MUX_AIN3_GND;

        default:
            return ADS1115_CONFIG_MUX_AIN0_GND;
    }
}

/*
 * 对应硬件功能：
 * - 根据 DR 计算 single-shot 最大等待时间
 * 依赖资料：
 * - ADS1115 数据手册 DR 枚举值（8/16/32/64/128/250/475/860 SPS）
 * 前提假设：
 * - 返回值是软件超时上限，不是硬件寄存器值
 */
static uint32_t ADS1115_GetConversionTimeoutMs(ads1115_data_rate_t dr)
{
    switch (dr)
    {
        case ADS1115_DR_8SPS:
            return 130U;

        case ADS1115_DR_16SPS:
            return 70U;

        case ADS1115_DR_32SPS:
            return 40U;

        case ADS1115_DR_64SPS:
            return 20U;

        case ADS1115_DR_128SPS:
            return 12U;

        case ADS1115_DR_250SPS:
            return 8U;

        case ADS1115_DR_475SPS:
            return 6U;

        case ADS1115_DR_860SPS:
            return 4U;

        default:
            return 20U;
    }
}

/*
 * 对应硬件功能：
 * - 轮询 Config.OS 位，等待单次转换完成
 * 依赖资料：
 * - ADS1115 数据手册：OS=0 表示转换中，OS=1 表示转换完成/空闲
 * 前提假设：
 * - 本版使用 single-shot 模式
 */
static fsp_err_t ADS1115_WaitConversionComplete(void)
{
    uint32_t timeout_ms = ADS1115_GetConversionTimeoutMs(g_ads1115_dr);
    uint16_t config = 0U;
    fsp_err_t err;

    while (timeout_ms > 0U)
    {
        err = ADS1115_ReadRegister(ADS1115_REG_CONFIG, &config);
        if (FSP_SUCCESS != err)
        {
            return err;
        }

        if ((config & ADS1115_CONFIG_OS_MASK) != 0U)
        {
            return FSP_SUCCESS;
        }

        R_BSP_SoftwareDelay(1U, BSP_DELAY_UNITS_MILLISECONDS);
        timeout_ms--;
    }

    return FSP_ERR_TIMEOUT;
}

/*
 * 对应硬件功能：
 * - 组装 single-shot + 单端输入 配置字
 * 依赖资料：
 * - ADS1115 数据手册 Config 寄存器位定义
 * 前提假设：
 * - 不使用比较器，因此 COMP_QUE 直接 disable
 */
static uint16_t ADS1115_BuildSingleEndedConfig(ads1115_channel_t channel)
{
    uint16_t config = 0U;

    config |= ADS1115_CONFIG_OS_SINGLE;
    config |= ADS1115_GetSingleEndedMuxBits(channel);
    config |= (uint16_t) g_ads1115_pga;
    config |= ADS1115_CONFIG_MODE_SINGLE;
    config |= (uint16_t) g_ads1115_dr;

    config |= ADS1115_CONFIG_COMP_MODE_TRAD;
    config |= ADS1115_CONFIG_COMP_POL_LOW;
    config |= ADS1115_CONFIG_COMP_LAT_NON;
    config |= ADS1115_CONFIG_COMP_DISABLE;

    return config;
}

fsp_err_t ADS1115_Open(void)
{
    fsp_err_t err;

    if (g_ads1115_opened)
    {
        return FSP_SUCCESS;
    }

    err = I2CMaster_Open();
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = ADS1115_SelectDevice();
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    g_ads1115_opened = true;
    g_ads1115_pga = ADS1115_PGA_6_144V;
    g_ads1115_dr = ADS1115_DR_128SPS;

    return FSP_SUCCESS;
}

/*
 * 对应硬件功能：
 * - 本模块本地关闭
 * 依赖资料：
 * - 当前系统为共享 I2C 总线
 * 前提假设：
 * - Close 不主动关闭底层 I2C 控制器，避免影响其他器件驱动
 */
fsp_err_t ADS1115_Close(void)
{
    g_ads1115_opened = false;
    return FSP_SUCCESS;
}

fsp_err_t ADS1115_SetAddress(uint8_t slave_addr)
{
    if ((ADS1115_I2C_ADDR_GND != slave_addr) &&
        (ADS1115_I2C_ADDR_VDD != slave_addr) &&
        (ADS1115_I2C_ADDR_SDA != slave_addr) &&
        (ADS1115_I2C_ADDR_SCL != slave_addr))
    {
        return FSP_ERR_ASSERTION;
    }

    g_ads1115_slave_addr = slave_addr;

    if (g_ads1115_opened)
    {
        return ADS1115_SelectDevice();
    }

    return FSP_SUCCESS;
}

uint8_t ADS1115_GetAddress(void)
{
    return g_ads1115_slave_addr;
}

fsp_err_t ADS1115_SetGain(ads1115_pga_t pga)
{
    switch (pga)
    {
        case ADS1115_PGA_6_144V:
        case ADS1115_PGA_4_096V:
        case ADS1115_PGA_2_048V:
        case ADS1115_PGA_1_024V:
        case ADS1115_PGA_0_512V:
        case ADS1115_PGA_0_256V:
            g_ads1115_pga = pga;
            return FSP_SUCCESS;

        default:
            return FSP_ERR_ASSERTION;
    }
}

ads1115_pga_t ADS1115_GetGain(void)
{
    return g_ads1115_pga;
}

fsp_err_t ADS1115_SetDataRate(ads1115_data_rate_t dr)
{
    switch (dr)
    {
        case ADS1115_DR_8SPS:
        case ADS1115_DR_16SPS:
        case ADS1115_DR_32SPS:
        case ADS1115_DR_64SPS:
        case ADS1115_DR_128SPS:
        case ADS1115_DR_250SPS:
        case ADS1115_DR_475SPS:
        case ADS1115_DR_860SPS:
            g_ads1115_dr = dr;
            return FSP_SUCCESS;

        default:
            return FSP_ERR_ASSERTION;
    }
}

ads1115_data_rate_t ADS1115_GetDataRate(void)
{
    return g_ads1115_dr;
}

/*
 * 对应硬件功能：
 * - 向 ADS1115 某 16 位寄存器写入数据
 * 依赖资料：
 * - ADS1115 数据手册：写寄存器格式 = Pointer + MSB + LSB
 * - 平台层 I2CMaster_Write()
 * 前提假设：
 * - 器件寄存器为大端序传输
 */
fsp_err_t ADS1115_WriteRegister(uint8_t reg, uint16_t value)
{
    uint8_t txbuf[3];
    fsp_err_t err;

    if (!g_ads1115_opened)
    {
        return FSP_ERR_NOT_OPEN;
    }

    txbuf[0] = reg;
    txbuf[1] = (uint8_t) ((value >> 8) & 0xFFU);
    txbuf[2] = (uint8_t) (value & 0xFFU);

    err = ADS1115_SelectDevice();
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    return I2CMaster_Write(txbuf, 3U, false);
}

/*
 * 对应硬件功能：
 * - 从 ADS1115 某 16 位寄存器读取数据
 * 依赖资料：
 * - ADS1115 数据手册：先写 Pointer，再读 2 字节
 * - 平台层 I2CMaster_WriteRead()
 * 前提假设：
 * - 当前平台层 write/read 之间为 stop + start
 * - ADS1115 手册允许 Pointer 写完后 STOP 或 repeated START
 */
fsp_err_t ADS1115_ReadRegister(uint8_t reg, uint16_t * p_value)
{
    uint8_t txbuf[1];
    uint8_t rxbuf[2];
    fsp_err_t err;

    if ((!g_ads1115_opened) || (NULL == p_value))
    {
        return FSP_ERR_ASSERTION;
    }

    txbuf[0] = reg;

    err = ADS1115_SelectDevice();
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = I2CMaster_WriteRead(txbuf, 1U, rxbuf, 2U);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    *p_value = (uint16_t)((((uint16_t) rxbuf[0]) << 8) | ((uint16_t) rxbuf[1]));
    return FSP_SUCCESS;
}

fsp_err_t ADS1115_ReadSingleEnded(ads1115_channel_t channel, int16_t * p_raw)
{
    uint16_t config;
    uint16_t conv_reg;
    fsp_err_t err;

    if ((!g_ads1115_opened) || (NULL == p_raw))
    {
        return FSP_ERR_ASSERTION;
    }

    if (channel >= ADS1115_CHANNEL_MAX)
    {
        return FSP_ERR_ASSERTION;
    }

    config = ADS1115_BuildSingleEndedConfig(channel);

    err = ADS1115_WriteRegister(ADS1115_REG_CONFIG, config);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = ADS1115_WaitConversionComplete();
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = ADS1115_ReadRegister(ADS1115_REG_CONVERSION, &conv_reg);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    *p_raw = (int16_t) conv_reg;
    return FSP_SUCCESS;
}

fsp_err_t ADS1115_ReadSingleEndedMilliVolts(ads1115_channel_t channel, int32_t * p_mv)
{
    int16_t raw = 0;
    fsp_err_t err;

    if (NULL == p_mv)
    {
        return FSP_ERR_ASSERTION;
    }

    err = ADS1115_ReadSingleEnded(channel, &raw);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    *p_mv = ADS1115_RawToMilliVolts(raw);
    return FSP_SUCCESS;
}

/*
 * 对应硬件功能：
 * - 轮询采完 4 个单端通道
 * 依赖资料：
 * - ADS1115 4 路单端输入
 * 前提假设：
 * - 扫描顺序固定为 AIN0 -> AIN1 -> AIN2 -> AIN3
 */
fsp_err_t ADS1115_ReadAllChannels(ads1115_scan_result_t * p_result)
{
    fsp_err_t err;

    if (NULL == p_result)
    {
        return FSP_ERR_ASSERTION;
    }

    err = ADS1115_ReadSingleEnded(ADS1115_CHANNEL_AIN0, &p_result->raw[ADS1115_CHANNEL_AIN0]);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = ADS1115_ReadSingleEnded(ADS1115_CHANNEL_AIN1, &p_result->raw[ADS1115_CHANNEL_AIN1]);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = ADS1115_ReadSingleEnded(ADS1115_CHANNEL_AIN2, &p_result->raw[ADS1115_CHANNEL_AIN2]);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = ADS1115_ReadSingleEnded(ADS1115_CHANNEL_AIN3, &p_result->raw[ADS1115_CHANNEL_AIN3]);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    return FSP_SUCCESS;
}

/*
 * 对应硬件功能：
 * - 返回当前 PGA 对应的满量程 mV
 * 依赖资料：
 * - ADS1115 数据手册 Table 3
 * 前提假设：
 * - 返回的是“ADC 缩放满量程”，不是输入脚允许超过 VDD 的意思
 */
uint16_t ADS1115_GetFullScaleMilliVolts(void)
{
    switch (g_ads1115_pga)
    {
        case ADS1115_PGA_6_144V:
            return 6144U;

        case ADS1115_PGA_4_096V:
            return 4096U;

        case ADS1115_PGA_2_048V:
            return 2048U;

        case ADS1115_PGA_1_024V:
            return 1024U;

        case ADS1115_PGA_0_512V:
            return 512U;

        case ADS1115_PGA_0_256V:
            return 256U;

        default:
            return 6144U;
    }
}

/*
 * 对应硬件功能：
 * - 将 ADS1115 原始码换算为 mV
 * 依赖资料：
 * - ADS1115 输出为 16 位二补码
 * - 满量程与 PGA 对应关系
 * 前提假设：
 * - 公式按 raw / 32768 * FS(mV) 计算
 * - 本项目单端输入下，正常读数应主要位于非负区间
 */
int32_t ADS1115_RawToMilliVolts(int16_t raw)
{
    int64_t temp;
    int32_t fs_mv = (int32_t) ADS1115_GetFullScaleMilliVolts();

    temp = ((int64_t) raw) * ((int64_t) fs_mv);

    if (temp >= 0)
    {
        temp += 16384LL;
    }
    else
    {
        temp -= 16384LL;
    }

    return (int32_t) (temp / 32768LL);
}

