/*
 * i2c_master.c
 *
 *  Created on: 2026年3月10日
 *      Author: leo
 */
#include "i2c_master.h"

/*
 * 对应硬件功能：
 * - 这些标志位用于同步 I2C 中断回调和前台读写流程
 * 依赖资料：
 * - 教材第15章/第9章的“发送完成/接收完成标志位 + 等待函数”风格
 * 前提假设：
 * - 当前工程使用 Common I2C + 中断回调
 */
volatile bool g_app_i2c_tx_cplt = false;
volatile bool g_app_i2c_rx_cplt = false;
volatile bool g_app_i2c_aborted = false;
volatile app_i2c_event_t g_app_i2c_last_event = APP_I2C_EVENT_NONE;

/*
 * 对应硬件功能：
 * - 调试计数器：统计 callback、超时、恢复尝试次数
 * 依赖资料：
 * - 无额外 FSP 依赖，仅用于串口调试
 * 前提假设：
 * - 计数器溢出不作为错误处理条件
 */
volatile uint32_t g_app_i2c_callback_count = 0U;
volatile uint32_t g_app_i2c_timeout_count  = 0U;
volatile uint32_t g_app_i2c_recover_count  = 0U;

static bool g_app_i2c_opened = false;

/*
 * 对应硬件功能：
 * - IIC2 中断回调
 * 依赖资料：
 * - hal_data.h 中已声明 i2c2_callback
 * - 教材第15章给出的 I2C_MASTER_EVENT_TX_COMPLETE / RX_COMPLETE / ABORTED
 * 前提假设：
 * - FSP 已将 g_i2c_master0_cfg.p_callback 绑定为 i2c2_callback
 */
void i2c2_callback(i2c_master_callback_args_t * p_args)
{
    g_app_i2c_callback_count++;

    if (NULL == p_args)
    {
        g_app_i2c_last_event = APP_I2C_EVENT_ABORTED;
        g_app_i2c_aborted = true;
        g_app_i2c_tx_cplt = false;
        g_app_i2c_rx_cplt = false;
        return;
    }

    switch (p_args->event)
    {
        case I2C_MASTER_EVENT_TX_COMPLETE:
        {
            g_app_i2c_tx_cplt = true;
            g_app_i2c_rx_cplt = false;
            g_app_i2c_aborted = false;
            g_app_i2c_last_event = APP_I2C_EVENT_TX_COMPLETE;
            break;
        }

        case I2C_MASTER_EVENT_RX_COMPLETE:
        {
            g_app_i2c_tx_cplt = false;
            g_app_i2c_rx_cplt = true;
            g_app_i2c_aborted = false;
            g_app_i2c_last_event = APP_I2C_EVENT_RX_COMPLETE;
            break;
        }

        case I2C_MASTER_EVENT_ABORTED:
        default:
        {
            g_app_i2c_tx_cplt = false;
            g_app_i2c_rx_cplt = false;
            g_app_i2c_aborted = true;
            g_app_i2c_last_event = APP_I2C_EVENT_ABORTED;
            break;
        }
    }
}

void I2CMaster_ClearFlags(void)
{
    g_app_i2c_tx_cplt = false;
    g_app_i2c_rx_cplt = false;
    g_app_i2c_aborted = false;
    g_app_i2c_last_event = APP_I2C_EVENT_NONE;
}

/*
 * 对应硬件功能：
 * - 打开 RA6M5 的 IIC2 主机控制器
 * 依赖资料：
 * - 教材第15章 open 用法
 * - 当前 hal_data.c 中 g_i2c_master0 / g_i2c_master0_cfg
 * 前提假设：
 * - Pins 与 Stack 已经由 FSP 正确生成
 */
fsp_err_t I2CMaster_Open(void)
{
    fsp_err_t err;

    if (g_app_i2c_opened)
    {
        return FSP_SUCCESS;
    }

    I2CMaster_ClearFlags();

    err = APP_I2C_MASTER.p_api->open(APP_I2C_MASTER.p_ctrl, APP_I2C_MASTER.p_cfg);
    if (FSP_SUCCESS == err)
    {
        g_app_i2c_opened = true;
    }

    return err;
}

fsp_err_t I2CMaster_Close(void)
{
    fsp_err_t err;

    if (!g_app_i2c_opened)
    {
        I2CMaster_ClearFlags();
        return FSP_SUCCESS;
    }

    err = APP_I2C_MASTER.p_api->close(APP_I2C_MASTER.p_ctrl);
    if (FSP_SUCCESS == err)
    {
        g_app_i2c_opened = false;
        I2CMaster_ClearFlags();
    }

    return err;
}

/*
 * 对应硬件功能：
 * - 异常后重置 I2C 控制器软件状态
 * 依赖资料：
 * - 教材第15章/第9章中 i2c_master_api_t 提供 abort/open/close
 * 前提假设：
 * - 这是“恢复控制器状态”的动作，不保证外部总线硬件问题被消除
 */
fsp_err_t I2CMaster_Recover(void)
{
    fsp_err_t err;

    g_app_i2c_recover_count++;

    if (!g_app_i2c_opened)
    {
        err = APP_I2C_MASTER.p_api->open(APP_I2C_MASTER.p_ctrl, APP_I2C_MASTER.p_cfg);
        if (FSP_SUCCESS == err)
        {
            g_app_i2c_opened = true;
        }
        return err;
    }

    /* 先终止当前事务，再关闭并重新打开控制器 */
    (void) APP_I2C_MASTER.p_api->abort(APP_I2C_MASTER.p_ctrl);
    (void) APP_I2C_MASTER.p_api->close(APP_I2C_MASTER.p_ctrl);

    g_app_i2c_opened = false;

    R_BSP_SoftwareDelay(1U, BSP_DELAY_UNITS_MILLISECONDS);

    err = APP_I2C_MASTER.p_api->open(APP_I2C_MASTER.p_ctrl, APP_I2C_MASTER.p_cfg);
    if (FSP_SUCCESS == err)
    {
        g_app_i2c_opened = true;
    }

    return err;
}

/*
 * 对应硬件功能：
 * - 切换当前 I2C 从机地址
 * 依赖资料：
 * - 教材 API: slaveAddressSet
 * 前提假设：
 * - 所有设备都使用 7-bit 地址
 */
fsp_err_t I2CMaster_SlaveAddrSet(uint8_t slave_addr)
{
    if (!g_app_i2c_opened)
    {
        return FSP_ERR_NOT_OPEN;
    }

    return APP_I2C_MASTER.p_api->slaveAddressSet(APP_I2C_MASTER.p_ctrl,
                                                 (uint32_t) slave_addr,
                                                 I2C_MASTER_ADDR_MODE_7BIT);
}

/*
 * 对应硬件功能：
 * - 等待发送完成中断
 * 依赖资料：
 * - 教材第9章/15章“标志位 + 软件延时轮询”风格
 * 前提假设：
 * - BSP 延时函数可用
 */
fsp_err_t I2CMaster_WaitTxComplete(uint32_t timeout_ms)
{
    while (timeout_ms > 0U)
    {
        if (g_app_i2c_aborted)
        {
            (void) I2CMaster_Recover();
            return FSP_ERR_ABORTED;
        }

        if (g_app_i2c_tx_cplt)
        {
            g_app_i2c_tx_cplt = false;
            return FSP_SUCCESS;
        }

        R_BSP_SoftwareDelay(1U, BSP_DELAY_UNITS_MILLISECONDS);
        timeout_ms--;
    }

    g_app_i2c_timeout_count++;
    (void) I2CMaster_Recover();
    return FSP_ERR_TIMEOUT;
}

fsp_err_t I2CMaster_WaitRxComplete(uint32_t timeout_ms)
{
    while (timeout_ms > 0U)
    {
        if (g_app_i2c_aborted)
        {
            (void) I2CMaster_Recover();
            return FSP_ERR_ABORTED;
        }

        if (g_app_i2c_rx_cplt)
        {
            g_app_i2c_rx_cplt = false;
            return FSP_SUCCESS;
        }

        R_BSP_SoftwareDelay(1U, BSP_DELAY_UNITS_MILLISECONDS);
        timeout_ms--;
    }

    g_app_i2c_timeout_count++;
    (void) I2CMaster_Recover();
    return FSP_ERR_TIMEOUT;
}

/*
 * 对应硬件功能：
 * - 主机发送一帧数据
 * 依赖资料：
 * - 教材第15章 write 接口与 restart 参数语义
 * 前提假设：
 * - p_buf 非空，len > 0
 */
fsp_err_t I2CMaster_Write(const uint8_t * p_buf, uint32_t len, bool restart)
{
    fsp_err_t err;

    if (!g_app_i2c_opened)
    {
        return FSP_ERR_NOT_OPEN;
    }

    if ((NULL == p_buf) || (0U == len))
    {
        return FSP_ERR_ASSERTION;
    }

    I2CMaster_ClearFlags();

    err = APP_I2C_MASTER.p_api->write(APP_I2C_MASTER.p_ctrl,
                                      (uint8_t *) p_buf,
                                      len,
                                      restart);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    return I2CMaster_WaitTxComplete(APP_I2C_TIMEOUT_MS);
}

fsp_err_t I2CMaster_Read(uint8_t * p_buf, uint32_t len, bool restart)
{
    fsp_err_t err;

    if (!g_app_i2c_opened)
    {
        return FSP_ERR_NOT_OPEN;
    }

    if ((NULL == p_buf) || (0U == len))
    {
        return FSP_ERR_ASSERTION;
    }

    I2CMaster_ClearFlags();

    err = APP_I2C_MASTER.p_api->read(APP_I2C_MASTER.p_ctrl,
                                     p_buf,
                                     len,
                                     restart);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    return I2CMaster_WaitRxComplete(APP_I2C_TIMEOUT_MS);
}

/*
 * 对应硬件功能：
 * - 典型“先写寄存器地址，再读数据”事务
 * 依赖资料：
 * - 教材先 write 再 read 的组合用法
 * 前提假设：
 * - 当前版本先保持 stop + start 组合，不在这里改事务语义
 */
fsp_err_t I2CMaster_WriteRead(const uint8_t * p_wbuf,
                              uint32_t wlen,
                              uint8_t * p_rbuf,
                              uint32_t rlen)
{
    fsp_err_t err;

    if (!g_app_i2c_opened)
    {
        return FSP_ERR_NOT_OPEN;
    }

    if ((NULL == p_wbuf) || (0U == wlen) || (NULL == p_rbuf) || (0U == rlen))
    {
        return FSP_ERR_ASSERTION;
    }

    err = I2CMaster_Write(p_wbuf, wlen, false);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = I2CMaster_Read(p_rbuf, rlen, false);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    return FSP_SUCCESS;
}
