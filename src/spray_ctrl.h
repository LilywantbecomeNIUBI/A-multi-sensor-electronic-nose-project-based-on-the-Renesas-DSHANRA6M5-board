#ifndef SPRAY_CTRL_H_
#define SPRAY_CTRL_H_

#include <stdint.h>
#include <stdbool.h>
#include "common_data.h"
#include "app_button.h"
#include "sensor_uart.h"

/*
 * 对应硬件功能：
 * - 自动喷气业务状态机
 *
 * 依赖资料：
 * - 业务时序：IDLE -> SPRAY(2000ms) -> RECOVERY(180000ms)
 */
typedef enum e_spray_ctrl_state
{
    SPRAY_CTRL_STATE_IDLE = 0,
    SPRAY_CTRL_STATE_SPRAY,
    SPRAY_CTRL_STATE_RECOVERY
} spray_ctrl_state_t;

/*
 * 对应硬件功能：
 * - 初始化业务状态机为 IDLE
 * - 强制关闭 P612 雾化输出
 * - 强制串口保持人类友好模式
 */
fsp_err_t SprayCtrl_Init(void);

/*
 * 对应硬件功能：
 * - 轮询推进自动喷气状态机
 *
 * 依赖资料：
 * - P000 按键事件
 * - P612 雾化输出
 * - 毫秒 tick
 *
 * 前提假设：
 * - now_ms 来自 AppSystick_GetMs()
 */
fsp_err_t SprayCtrl_Update(uint32_t now_ms, app_button_event_t button_event);

spray_ctrl_state_t SprayCtrl_GetState(void);
bool SprayCtrl_IsIdle(void);

/*
 * 对应硬件功能：
 * - 控制 P612 驱动三极管导通/截止
 *
 * 依赖资料：
 * - 传感器设计16.0.md 中 P612 反相逻辑
 *
 * 前提假设：
 * - Atomizer_On()  = P612 输出 HIGH
 * - Atomizer_Off() = P612 输出 LOW
 */
fsp_err_t Atomizer_On(void);
fsp_err_t Atomizer_Off(void);

#endif
