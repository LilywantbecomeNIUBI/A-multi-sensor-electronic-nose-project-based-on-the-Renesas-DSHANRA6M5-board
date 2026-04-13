#ifndef APP_SYSTICK_H_
#define APP_SYSTICK_H_

#include <stdint.h>
#include "hal_data.h"

/*
 * 对应硬件功能：
 * - 初始化 Cortex-M33 SysTick 为 1ms 节拍
 *
 * 依赖资料：
 * - chapter11.md 中 SysTick_Config() / R_BSP_SourceClockHzGet(FSP_PRIV_CLOCK_PLL)
 * - 你当前工程的 hal_data.h / bsp_api.h
 *
 * 前提假设：
 * - 当前工程尚未实现其他 SysTick_Handler
 */
fsp_err_t AppSystick_Init(void);

/*
 * 对应硬件功能：
 * - 读取单调递增的毫秒计数
 *
 * 依赖资料：
 * - chapter11.md 中 HAL_GetTick 思路
 *
 * 前提假设：
 * - 已先调用 AppSystick_Init()
 */
uint32_t AppSystick_GetMs(void);

#endif
