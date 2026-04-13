#ifndef APP_BUTTON_H_
#define APP_BUTTON_H_

#include <stdint.h>
#include <stdbool.h>
#include "common_data.h"

/*
 * 对应硬件功能：
 * - 向业务层抛出“按下一次”的事件，而不是持续电平
 *
 * 依赖资料：
 * - 传感器设计16.0.md 中 P000 Active-Low 与边沿检测要求
 */
typedef enum e_app_button_event
{
    APP_BUTTON_EVENT_NONE = 0,
    APP_BUTTON_EVENT_PRESSED = 1
} app_button_event_t;

/*
 * 对应硬件功能：
 * - 初始化 P000 按键扫描器
 *
 * 依赖资料：
 * - P000 = BSP_IO_PORT_00_PIN_00
 * - R_IOPORT_PinRead()
 *
 * 前提假设：
 * - FSP 已把 P000 配置为 Input + Pull-up
 */
fsp_err_t AppButton_Init(void);

/*
 * 对应硬件功能：
 * - 非阻塞按键扫描与 20ms 去抖
 *
 * 依赖资料：
 * - 传感器设计16.0.md 中 20ms~30ms 非阻塞消抖要求
 * - chapter5.md 中 R_IOPORT_PinRead()
 *
 * 前提假设：
 * - now_ms 来自 AppSystick_GetMs()
 * - p_event 非 NULL
 */
fsp_err_t AppButton_Update(uint32_t now_ms, app_button_event_t * p_event);

#endif
