/*
 * sensor_uart.h
 *
 *  Created on: 2026年3月16日
 *      Author: leo
 */

#ifndef SENSOR_UART_H_
#define SENSOR_UART_H_

#include "sensor_hub.h"
#include "app_debug.h"

fsp_err_t SensorUart_Init(void);

/*
 * 对应硬件功能：
 * - 切回人类友好串口输出模式
 *
 * 依赖资料：
 * - 当前项目已跑通的人类友好文本输出版本
 */
fsp_err_t SensorUart_EnableHumanMode(void);

/*
 * 对应硬件功能：
 * - 切换到 CSV 输出模式
 * - 切换瞬间输出一次 CSV 表头
 *
 * 依赖资料：
 * - 传感器设计14.0 中对 CSV 模式切换的要求
 */
fsp_err_t SensorUart_EnableCsvMode(void);

bool SensorUart_IsCsvMode(void);
fsp_err_t SensorUart_PrintBanner(void);
fsp_err_t SensorUart_PrintInitResult(sensor_hub_t const * p_hub, int32_t hub_err);
fsp_err_t SensorUart_PrintRecord(sensor_hub_record_t const * p_record);

#endif /* SENSOR_UART_H_ */
