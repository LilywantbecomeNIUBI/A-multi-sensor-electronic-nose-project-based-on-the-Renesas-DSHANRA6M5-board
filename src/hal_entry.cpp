/*
 * hal_entry.cpp
 *
 * 调试版主调度文件（方案 A）：
 * - 只负责系统初始化、1Hz 采样调度、调用 AppInfer_ProcessRecord()、打印调试信息
 * - 不直接包含 Edge Impulse 头文件
 * - 不定义任何 EI porting 函数
 */

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "hal_data.h"

extern "C" {
#include "sensor_hub.h"
#include "sensor_uart.h"
#include "app_debug.h"
#include "app_systick.h"
#include "app_button.h"
#include "spray_ctrl.h"
#include "app_infer.h"
#include "lvgl.h"
#include "lv_port_disp.h"
#include "drv_gpt_timer.h"
#include "face_simple.h"

}

#if (1 == BSP_MULTICORE_PROJECT) && BSP_TZ_SECURE_BUILD
bsp_ipc_semaphore_handle_t g_core_start_semaphore =
{
    .semaphore_num = 0
};
#endif

static sensor_hub_t        g_sensor_hub;
static sensor_hub_record_t g_sensor_record;
static app_infer_context_t g_app_infer;
static app_infer_output_t  g_app_infer_output;

/* 新增：AI 采集启动门控 */
static bool     g_ai_capture_enabled = false;
static uint32_t g_ai_start_ms        = 0U;

static void hal_entry_trap(void)
{
    while (1)
    {
        ;
    }
}

static void hal_entry_update_atomizer_flag(sensor_hub_record_t * p_record)
{
    if (NULL == p_record)
    {
        return;
    }

    /* 当前已不使用自动喷射，这里固定为 false 也可以 */
    p_record->atomizer_on = false;
}

static void hal_entry_print_ai_waiting(void)
{
    (void) AppDebug_WriteString("[AI] waiting trigger, press P000 to start capture\r\n");
}

static void hal_entry_print_ai_started(uint32_t now_ms)
{
    char msg[96];
    (void) snprintf(msg,
                    sizeof(msg),
                    "[AI] trigger accepted, start capture at %lu ms\r\n",
                    (unsigned long) now_ms);
    (void) AppDebug_WriteString(msg);
}

static void hal_entry_debug_print_infer_output(app_infer_output_t const * p_output)
{
    if (NULL == p_output)
    {
        return;
    }

    if (!p_output->sample_accepted)
    {
        (void) AppDebug_WriteString("[AI] sample rejected\r\n");
        return;
    }

    if (!p_output->window_ready)
    {
        char msg[160];
        (void) snprintf(msg,
                        sizeof(msg),
                        "[AI] seq=%lu collecting %lu/%u\r\n",
                        (unsigned long) p_output->sequence,
                        (unsigned long) p_output->window_count,
                        (unsigned int) EI_INFER_WINDOW_SAMPLE_COUNT);
        (void) AppDebug_WriteString(msg);
        return;
    }

    if (!p_output->infer_ran_this_frame)
    {
        char msg[160];
        (void) snprintf(msg,
                        sizeof(msg),
                        "[AI] seq=%lu ready but infer not run, err=%ld backend=%ld\r\n",
                        (unsigned long) p_output->sequence,
                        (long) p_output->process_err,
                        (long) p_output->raw_result.backend_err);
        (void) AppDebug_WriteString(msg);
        return;
    }

    {
        char msg[256];
        (void) snprintf(msg,
                        sizeof(msg),
                        "[AI] seq=%lu top1=%s(%.2f%%) top2=%s(%.2f%%) decision=%s dsp=%lu ms cls=%lu ms\r\n",
                        (unsigned long) p_output->sequence,
                        EIInfer_GetLabelByIndex(p_output->raw_result.top1_index),
                        p_output->raw_result.top1_score * 100.0f,
                        EIInfer_GetLabelByIndex(p_output->raw_result.top2_index),
                        p_output->raw_result.top2_score * 100.0f,
                        AppInfer_GetDecisionName(p_output->decision),
                        (unsigned long) p_output->raw_result.timing_dsp_ms,
                        (unsigned long) p_output->raw_result.timing_classification_ms);
        (void) AppDebug_WriteString(msg);
    }

    {
        char msg[192];
        (void) snprintf(msg,
                        sizeof(msg),
                        "[AI] prob air=%.2f alcohol=%.2f perfume=%.2f vinegar=%.2f\r\n",
                        p_output->raw_result.scores[0] * 100.0f,
                        p_output->raw_result.scores[1] * 100.0f,
                        p_output->raw_result.scores[2] * 100.0f,
                        p_output->raw_result.scores[3] * 100.0f);
        (void) AppDebug_WriteString(msg);
    }
}

void hal_entry(void)
{
    fsp_err_t uart_err;
    fsp_err_t tick_err;
    fsp_err_t button_err;
    fsp_err_t spray_err;
    int32_t   hub_err;
    int32_t   infer_err;
    uint32_t  now_ms;
    uint32_t  last_sample_ms = 0U;
    app_button_event_t button_event = APP_BUTTON_EVENT_NONE;

    R_BSP_SoftwareDelay(2000, BSP_DELAY_UNITS_MILLISECONDS);

    uart_err = SensorUart_Init();
    if (FSP_SUCCESS != uart_err)
    {
        hal_entry_trap();
    }

    (void) SensorUart_EnableHumanMode();
    (void) SensorUart_PrintBanner();

    hub_err = SensorHub_Init(&g_sensor_hub);
    (void) SensorUart_PrintInitResult(&g_sensor_hub, hub_err);
    if (SENSOR_HUB_OK != hub_err)
    {
        hal_entry_trap();
    }

    infer_err = AppInfer_Init(&g_app_infer);
    if (APP_INFER_OK != infer_err)
    {
        (void) AppDebug_PrintError("[AI] init err=", infer_err);
        hal_entry_trap();
    }
    else
    {
        (void) AppDebug_WriteString("[AI] app infer init ok\r\n");
    }

    tick_err = AppSystick_Init();
    if (FSP_SUCCESS != tick_err)
    {
        hal_entry_trap();
    }

    button_err = AppButton_Init();
    if (FSP_SUCCESS != button_err)
    {
        hal_entry_trap();
    }

    /* 保留初始化，避免影响原工程链接关系；但后续不再推进自动喷射状态机 */
    spray_err = SprayCtrl_Init();
    if (FSP_SUCCESS != spray_err)
    {
        hal_entry_trap();
    }

    hub_err = SensorHub_SampleOnce(&g_sensor_hub, &g_sensor_record);
    if (SENSOR_HUB_OK == hub_err)
    {
        hal_entry_update_atomizer_flag(&g_sensor_record);
        (void) SensorUart_PrintRecord(&g_sensor_record);
        hal_entry_print_ai_waiting();
    }
    else if (!SensorUart_IsCsvMode())
    {
        (void) AppDebug_PrintError("[HUB] sample api err=", hub_err);
        (void) AppDebug_PrintI2CDiag();
    }

    last_sample_ms = AppSystick_GetMs();
    lv_init();

    /* 2. 初始化显示接口 (ST7789 和 SPI 配置) */
    lv_port_disp_init();

    /* 3. 初始化硬件定时器，给 LVGL 提供心跳 (lv_tick_inc) */
    drv_gpt_timer_init();

    /* 4. 初始化你的表情包 UI */
    face_simple_cfg_t face_cfg = {
      .p_parent = lv_scr_act(), // 画在当前活动屏幕上
      .width = 160,
      .height = 160,
      .pos_x = 40,
      .pos_y = 60,
      .auto_blink = true,       // 开启自动眨眼
      .blink_period_ms = 2500   // 每 2.5 秒眨眼一次
    };
    face_simple_init(&face_cfg);

    /* 5. 初始状态设为空气（悠闲吹口哨/待机状态） */
    face_simple_set_emotion(FACE_SIMPLE_AIR);

    while (1)
    {
        uint32_t time_till_next = lv_task_handler();
        now_ms = AppSystick_GetMs();
        button_event = APP_BUTTON_EVENT_NONE;

        button_err = AppButton_Update(now_ms, &button_event);

        if (FSP_SUCCESS != button_err)
        {
            hal_entry_trap();
        }

        if (APP_BUTTON_EVENT_PRESSED == button_event)
        {
            infer_err = AppInfer_Reset(&g_app_infer);
            if (APP_INFER_OK != infer_err)
            {
                (void) AppDebug_PrintError("[AI] reset err=", infer_err);
                hal_entry_trap();
            }

            g_ai_capture_enabled = true;
            g_ai_start_ms = now_ms;
            (void) g_ai_start_ms;
            hal_entry_print_ai_started(now_ms);
        }

        if ((uint32_t) (now_ms - last_sample_ms) >= 1000U)
        {
            last_sample_ms = now_ms;

            hub_err = SensorHub_SampleOnce(&g_sensor_hub, &g_sensor_record);
            if (SENSOR_HUB_OK == hub_err)
            {
                hal_entry_update_atomizer_flag(&g_sensor_record);
                (void) SensorUart_PrintRecord(&g_sensor_record);

                if (!g_ai_capture_enabled)
                {
                    hal_entry_print_ai_waiting();
                    face_simple_set_emotion(FACE_SIMPLE_THINKING);
                }
                else
                {
                    infer_err = AppInfer_ProcessRecord(&g_app_infer, &g_sensor_record, &g_app_infer_output);
                    if ((APP_INFER_OK != infer_err) && (APP_INFER_ERR_SAMPLE_REJECTED != infer_err))
                    {
                        (void) AppDebug_PrintError("[AI] process err=", infer_err);
                    }
                    hal_entry_debug_print_infer_output(&g_app_infer_output);
                    switch (g_app_infer_output.decision){
                        case APP_INFER_DECISION_AIR:
                            // 识别结果为空气时，切换为吹口哨表情
                            face_simple_set_emotion(FACE_SIMPLE_AIR);
                            break;
                        case APP_INFER_DECISION_ALCOHOL:
                            face_simple_set_emotion(FACE_SIMPLE_ALERT);
                            break;
                        case APP_INFER_DECISION_VINEGAR:
                            face_simple_set_emotion(FACE_SIMPLE_SAD);
                            break;
                        case APP_INFER_DECISION_PERFUME:
                            face_simple_set_emotion(FACE_SIMPLE_HAPPY);
                            break;
                        default:
                            face_simple_set_emotion(FACE_SIMPLE_THINKING);
                            break;
                    }
                }
            }
            else if (!SensorUart_IsCsvMode())
            {
                (void) AppDebug_PrintError("[HUB] sample api err=", hub_err);
                (void) AppDebug_PrintI2CDiag();
            }
        }
    }

#if (0 == _RA_CORE) && (1 == BSP_MULTICORE_PROJECT) && !BSP_TZ_NONSECURE_BUILD
#if BSP_TZ_SECURE_BUILD
    R_BSP_IpcSemaphoreTake(&g_core_start_semaphore);
#endif
    R_BSP_SecondaryCoreStart();
#if BSP_TZ_SECURE_BUILD
    while (FSP_ERR_IN_USE == R_BSP_IpcSemaphoreTake(&g_core_start_semaphore))
    {
        ;
    }
#endif
#endif

#if (1 == _RA_CORE) && (1 == BSP_MULTICORE_PROJECT) && BSP_TZ_SECURE_BUILD
    R_BSP_IpcSemaphoreGive(&g_core_start_semaphore);
#endif

#if BSP_TZ_SECURE_BUILD
    R_BSP_NonSecureEnter();
#endif
}
