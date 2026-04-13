#include "sensirion_i2c.h"
#include "sensirion_common.h"
#include "i2c_master.h"

/*
 * 对应硬件功能：
 * - 记录 Sensirion 平台适配层是否已经打开 RA6M5 的 IIC2 主机
 * 依赖资料：
 * - i2c_master.c/.h 里 I2CMaster_Open()/Close()/SlaveAddrSet()/Write()/Read()
 * - hal_data.h 里 g_i2c_master0 已由 FSP 生成
 * 前提假设：
 * - 当前工程只有一条给 SGP40 使用的 I2C 总线，即 IIC2
 */
static bool g_sensirion_i2c_inited = false;

/*
 * 对应硬件功能：
 * - 单总线工程下选择 I2C 总线
 * 依赖资料：
 * - sensirion_i2c.h 要求提供 select_bus 接口；但文档明确单总线时可选实现
 * 前提假设：
 * - 当前 SGP40/SHT40 都会挂在同一条 IIC2 总线上
 */
int16_t sensirion_i2c_select_bus(uint8_t bus_idx)
{
    FSP_PARAMETER_NOT_USED(bus_idx);
    return NO_ERROR;
}

/*
 * 对应硬件功能：
 * - 初始化 RA6M5 的 IIC2 主机，为 Sensirion 驱动提供底层 I2C 入口
 * 依赖资料：
 * - i2c_master.c/.h 的 I2CMaster_Open()
 * - FSP 自动生成实例 g_i2c_master0
 * 前提假设：
 * - IIC2 的 Pin/Stack 已在 FSP 中配置正确；当前使用 7-bit 地址模式
 */
void sensirion_i2c_init(void)
{
    if (!g_sensirion_i2c_inited)
    {
        if (FSP_SUCCESS == I2CMaster_Open())
        {
            g_sensirion_i2c_inited = true;
        }
    }
}

/*
 * 对应硬件功能：
 * - 释放 RA6M5 的 IIC2 主机资源
 * 依赖资料：
 * - i2c_master.c/.h 的 I2CMaster_Close()
 * 前提假设：
 * - 当前工程允许在退出时关闭 I2C；失败时保持原状态等待上层复位或重试
 */
void sensirion_i2c_release(void)
{
    if (g_sensirion_i2c_inited)
    {
        if (FSP_SUCCESS == I2CMaster_Close())
        {
            g_sensirion_i2c_inited = false;
        }
    }
}

/*
 * 对应硬件功能：
 * - 在每次读写前，确保 IIC2 已打开并切换到目标从机地址
 * 依赖资料：
 * - I2CMaster_Open()
 * - I2CMaster_SlaveAddrSet()
 * - SGP40 数据手册：I2C 地址为 0x59，按 7-bit 地址使用
 * 前提假设：
 * - 所有 Sensirion API 传入的 address 都是 7-bit 地址，不做左移
 */
static int8_t sensirion_i2c_prepare(uint8_t address)
{
    fsp_err_t err;

    if (!g_sensirion_i2c_inited)
    {
        err = I2CMaster_Open();
        if (FSP_SUCCESS != err)
        {
            return STATUS_FAIL;
        }

        g_sensirion_i2c_inited = true;
    }

    err = I2CMaster_SlaveAddrSet(address);
    if (FSP_SUCCESS != err)
    {
        return STATUS_FAIL;
    }

    return NO_ERROR;
}

/*
 * 对应硬件功能：
 * - 通过 RA6M5 IIC2 从当前从机地址读取指定字节数
 */
int8_t sensirion_i2c_read(uint8_t address, uint8_t* data, uint16_t count)
{
    fsp_err_t err;

    if ((NULL == data) || (0U == count))
    {
        return STATUS_FAIL;
    }

    /*
     * 【关键修复】：读操作之前同样必须切地址！
     */
    err = g_i2c_master0.p_api->slaveAddressSet(g_i2c_master0.p_ctrl, address, I2C_MASTER_ADDR_MODE_7BIT);
    if (FSP_SUCCESS != err)
    {
        return STATUS_FAIL;
    }

    if (NO_ERROR != sensirion_i2c_prepare(address))
    {
        return STATUS_FAIL;
    }

    /* 调用你底层的读封装，获取数据 */
    err = I2CMaster_Read(data, (uint32_t) count, false);
    if (FSP_SUCCESS != err)
    {
        return STATUS_FAIL;
    }

    return NO_ERROR;
}

/*
 * 对应硬件功能：
 * - 通过 RA6M5 IIC2 向当前从机地址发送指定字节数
 * 依赖资料：
 * - sensirion_i2c.h 的 write 原型
 * - i2c_master.c/.h 的 I2CMaster_Write()
 * - SGP40 数据手册：命令本体 2 字节不附加 CRC，后续参数 word 需要附加 CRC
 *   这些数据组帧由 sensirion_common.c 完成，这里只负责底层发送
 * 前提假设：
 * - 这里发送结束后直接 STOP；后续如需读数据，由上层延时后再发单独读事务
 */
int8_t sensirion_i2c_write(uint8_t address, const uint8_t* data, uint16_t count)
{
    fsp_err_t err;

    if ((NULL == data) || (0U == count))
    {
        return STATUS_FAIL;
    }

    /*
     * 【关键修复】：每次发送前，必须强制底层 FSP 切换到目标从机地址。
     * 这一步绕过任何封装，直接操作底层 API，确保地址肯定对得上！
     */
    err = g_i2c_master0.p_api->slaveAddressSet(g_i2c_master0.p_ctrl, address, I2C_MASTER_ADDR_MODE_7BIT);
    if (FSP_SUCCESS != err)
    {
        return STATUS_FAIL;
    }

    /* 保留你原有的 prepare 逻辑（如果有清空状态机等需要） */
    if (NO_ERROR != sensirion_i2c_prepare(address))
    {
        return STATUS_FAIL;
    }

    /* 执行底层真正的发送事务 */
    err = I2CMaster_Write((uint8_t *)data, (uint32_t) count, false);
    if (FSP_SUCCESS != err)
    {
        return STATUS_FAIL;
    }

    return NO_ERROR;
}

/*
 * 对应硬件功能：
 * - 为 Sensirion 驱动提供延时函数
 * 依赖资料：
 * - sensirion_i2c.h 注释：硬件 I2C 情况下，小于 10 ms 级精度即可
 * - RA6M5 BSP 的 R_BSP_SoftwareDelay()
 * 前提假设：
 * - 当前只基于你已确认存在的 BSP_DELAY_UNITS_MILLISECONDS 使用毫秒延时
 * - 对于 500 us / 1000 us 这类等待，向上取整到 1 ms 是允许的
 */
void sensirion_sleep_usec(uint32_t useconds)
{
    uint32_t delay_ms;

    if (0U == useconds)
    {
        return;
    }

    delay_ms = useconds / 1000U;
    if ((useconds % 1000U) != 0U)
    {
        delay_ms++;
    }

    if (0U == delay_ms)
    {
        delay_ms = 1U;
    }

    R_BSP_SoftwareDelay(delay_ms, BSP_DELAY_UNITS_MILLISECONDS);
}
