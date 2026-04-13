/*
 * i2c_master.h
 *
 *  Created on: 2026年3月10日
 *      Author: leo
 */

#ifndef I2C_MASTER_H_
#define I2C_MASTER_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "hal_data.h"

/*
 * 对应硬件功能：
 * - 绑定当前 FSP 生成的 Common I2C 主机实例
 * 依赖资料：
 * - 当前工程 hal_data.h 中导出的 g_i2c_master0
 * 前提假设：
 * - 当前工程继续使用 r_iic_master + channel 2
 */
#define APP_I2C_MASTER      (g_i2c_master0)
#define APP_I2C_TIMEOUT_MS  (100U)

typedef enum e_app_i2c_event
{
    APP_I2C_EVENT_NONE = 0,
    APP_I2C_EVENT_TX_COMPLETE,
    APP_I2C_EVENT_RX_COMPLETE,
    APP_I2C_EVENT_ABORTED
} app_i2c_event_t;

/*
 * 对应硬件功能：
 * - 这些变量用于观察 I2C 中断事件、传输完成状态和恢复动作
 * 依赖资料：
 * - 教材第15章/第9章的 callback event + 标志位等待模式
 * 前提假设：
 * - 仅单线程前台调试使用
 */
extern volatile bool g_app_i2c_tx_cplt;
extern volatile bool g_app_i2c_rx_cplt;
extern volatile bool g_app_i2c_aborted;
extern volatile app_i2c_event_t g_app_i2c_last_event;

extern volatile uint32_t g_app_i2c_callback_count;
extern volatile uint32_t g_app_i2c_timeout_count;
extern volatile uint32_t g_app_i2c_recover_count;

void i2c2_callback(i2c_master_callback_args_t * p_args);

void I2CMaster_ClearFlags(void);

fsp_err_t I2CMaster_Open(void);
fsp_err_t I2CMaster_Close(void);

/*
 * 对应硬件功能：
 * - 对 I2C 控制器执行一次软件恢复：abort + close + open
 * 依赖资料：
 * - i2c_master_api_t 中提供 abort/open/close
 * 前提假设：
 * - 仅用于通信异常后的恢复，不表示本次事务成功
 */
fsp_err_t I2CMaster_Recover(void);

fsp_err_t I2CMaster_SlaveAddrSet(uint8_t slave_addr);

fsp_err_t I2CMaster_WaitTxComplete(uint32_t timeout_ms);
fsp_err_t I2CMaster_WaitRxComplete(uint32_t timeout_ms);

fsp_err_t I2CMaster_Write(const uint8_t * p_buf, uint32_t len, bool restart);
fsp_err_t I2CMaster_Read(uint8_t * p_buf, uint32_t len, bool restart);
fsp_err_t I2CMaster_WriteRead(const uint8_t * p_wbuf,
                              uint32_t wlen,
                              uint8_t * p_rbuf,
                              uint32_t rlen);

#endif /* I2C_MASTER_H_ */
