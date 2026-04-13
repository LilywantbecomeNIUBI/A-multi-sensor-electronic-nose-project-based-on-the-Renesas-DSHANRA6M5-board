/*
 * app_debug.h
 *
 *  Created on: 2026年3月16日
 *      Author: leo
 */

#ifndef APP_DEBUG_H_
#define APP_DEBUG_H_

#include <stdint.h>
#include <stdbool.h>
#include "hal_data.h"
#include "i2c_master.h"

/*
 * 对应硬件功能：
 * - 打开 SCI7 调试串口
 *
 * 依赖资料：
 * - hal_data.h 中 g_uart7
 *
 * 前提假设：
 * - SCI7 已在 FSP 中配置完成
 * - 本工程仅由这一套统一调试层使用 uart7_callback
 */
fsp_err_t AppDebug_UartOpen(void);

/*
 * 对应硬件功能：
 * - 通过 SCI7 发送指定长度字节流
 *
 * 依赖资料：
 * - g_uart7.p_api->write()
 * - UART_EVENT_TX_COMPLETE 回调
 *
 * 前提假设：
 * - 已先调用 AppDebug_UartOpen()
 */
fsp_err_t AppDebug_WriteBytes(uint8_t const * p_data, uint32_t len);

/*
 * 对应硬件功能：
 * - 通过 SCI7 发送 '\0' 结尾字符串
 *
 * 依赖资料：
 * - AppDebug_WriteBytes()
 *
 * 前提假设：
 * - p_str 非 NULL
 */
fsp_err_t AppDebug_WriteString(char const * p_str);

/* 文本缓冲区拼接工具：沿用你现有调试版“手工拼串，不依赖 printf”风格 */
uint32_t AppDebug_BufAppendChar(char * p_buf, uint32_t buf_size, uint32_t pos, char ch);
uint32_t AppDebug_BufAppendString(char * p_buf, uint32_t buf_size, uint32_t pos, char const * p_str);
uint32_t AppDebug_BufAppendUInt32(char * p_buf, uint32_t buf_size, uint32_t pos, uint32_t value);
uint32_t AppDebug_BufAppendInt32(char * p_buf, uint32_t buf_size, uint32_t pos, int32_t value);
uint32_t AppDebug_BufAppendFloat1(char * p_buf, uint32_t buf_size, uint32_t pos, float value);
uint32_t AppDebug_BufAppendHex8(char * p_buf, uint32_t buf_size, uint32_t pos, uint8_t value);

/*
 * 对应硬件功能：
 * - 输出统一格式错误码
 *
 * 依赖资料：
 * - SCI7 发送接口
 *
 * 前提假设：
 * - prefix 为 '\0' 结尾字符串
 */
fsp_err_t AppDebug_PrintError(char const * prefix, int32_t err_code);

/*
 * 对应硬件功能：
 * - 输出 I2C 底层诊断计数与最近事件
 *
 * 依赖资料：
 * - i2c_master.h 中导出的 g_app_i2c_xxx 变量
 *
 * 前提假设：
 * - 这些诊断变量仍由你当前 i2c_master.c 维护
 */
fsp_err_t AppDebug_PrintI2CDiag(void);

#endif /* APP_DEBUG_H_ */
