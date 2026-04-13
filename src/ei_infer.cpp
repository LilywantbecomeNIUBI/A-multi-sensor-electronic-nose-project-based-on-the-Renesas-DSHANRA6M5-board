#include "edge-impulse-sdk/classifier/ei_run_classifier.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "hal_data.h"

extern "C" {
#include "app_debug.h"
#include "app_systick.h"
#include "ei_infer.h"
}

namespace {

static const char * const g_ei_labels[EI_INFER_LABEL_COUNT] = {
    "air",
    "alcohol",
    "perfume",
    "vinegar"
};

static void ei_infer_clear_result(ei_infer_result_t * p_result)
{
    if (p_result == NULL)
    {
        return;
    }

    (void) memset(p_result, 0, sizeof(ei_infer_result_t));
    p_result->infer_err = EI_INFER_OK;
    p_result->backend_err = 0;
}

static void ei_infer_fill_scores(ei_infer_result_t * p_out,
                                 ei_impulse_result_t const * p_ei_result)
{
    uint32_t i;
    uint32_t top1 = 0U;
    uint32_t top2 = 0U;

    for (i = 0U; i < EI_INFER_LABEL_COUNT; i++)
    {
        p_out->scores[i] = p_ei_result->classification[i].value;
    }

    for (i = 1U; i < EI_INFER_LABEL_COUNT; i++)
    {
        if (p_out->scores[i] > p_out->scores[top1])
        {
            top2 = top1;
            top1 = i;
        }
        else if ((i != top1) &&
                 ((top1 == top2) || (p_out->scores[i] > p_out->scores[top2])))
        {
            top2 = i;
        }
    }

    p_out->top1_index = top1;
    p_out->top1_score = p_out->scores[top1];
    p_out->top2_index = top2;
    p_out->top2_score = p_out->scores[top2];
}

static void ei_infer_materialize_ordered_frame(ei_infer_context_t const * p_ctx,
                                               float * p_frame)
{
    uint32_t out_ix = 0U;
    uint32_t start_index = p_ctx->write_index;

    for (uint32_t i = 0U; i < EI_INFER_WINDOW_SAMPLE_COUNT; i++)
    {
        uint32_t idx = (start_index + i) % EI_INFER_WINDOW_SAMPLE_COUNT;
        ei_input_sample_t const * p_sample = &p_ctx->window[idx];

        p_frame[out_ix++] = p_sample->env_voc_index;
        p_frame[out_ix++] = p_sample->mq135_mv;
        p_frame[out_ix++] = p_sample->mq138_mv;
        p_frame[out_ix++] = p_sample->mq2_mv;
        p_frame[out_ix++] = p_sample->mq3_mv;
    }
}

} /* namespace */

extern "C" {

int32_t EIInfer_Init(ei_infer_context_t * p_ctx)
{
    if (p_ctx == NULL)
    {
        return EI_INFER_ERR_PARAM;
    }

    (void) memset(p_ctx, 0, sizeof(ei_infer_context_t));
    p_ctx->initialized = true;
    p_ctx->state = EI_INFER_STATE_COLLECTING;
    ei_infer_clear_result(&p_ctx->last_result);

    return EI_INFER_OK;
}

int32_t EIInfer_Reset(ei_infer_context_t * p_ctx)
{
    return EIInfer_Init(p_ctx);
}

int32_t EIInfer_PushSample(ei_infer_context_t * p_ctx,
                           ei_input_sample_t const * p_sample)
{
    if ((p_ctx == NULL) || (p_sample == NULL))
    {
        return EI_INFER_ERR_PARAM;
    }

    if (!p_ctx->initialized)
    {
        return EI_INFER_ERR_NOT_INIT;
    }

    p_ctx->window[p_ctx->write_index] = *p_sample;
    p_ctx->write_index = (p_ctx->write_index + 1U) % EI_INFER_WINDOW_SAMPLE_COUNT;

    if (p_ctx->sample_count < EI_INFER_WINDOW_SAMPLE_COUNT)
    {
        p_ctx->sample_count++;
    }

    p_ctx->state = (p_ctx->sample_count >= EI_INFER_WINDOW_SAMPLE_COUNT) ?
        EI_INFER_STATE_READY : EI_INFER_STATE_COLLECTING;

    p_ctx->last_result.window_count = p_ctx->sample_count;
    p_ctx->last_result.window_ready = (p_ctx->sample_count >= EI_INFER_WINDOW_SAMPLE_COUNT);
    p_ctx->last_result.infer_ran = false;

    return EI_INFER_OK;
}

bool EIInfer_IsWindowReady(ei_infer_context_t const * p_ctx)
{
    if (p_ctx == NULL)
    {
        return false;
    }

    return (p_ctx->initialized &&
            (p_ctx->sample_count >= EI_INFER_WINDOW_SAMPLE_COUNT));
}

int32_t EIInfer_Run(ei_infer_context_t * p_ctx,
                    ei_infer_result_t * p_result)
{
    static float features[EI_INFER_FEATURE_FRAME_SIZE] = {0.0f};
    signal_t signal;
    int signal_err;
    EI_IMPULSE_ERROR ei_err;
    ei_impulse_result_t ei_result = {0};

    if ((p_ctx == NULL) || (p_result == NULL))
    {
        return EI_INFER_ERR_PARAM;
    }

    if (!p_ctx->initialized)
    {
        return EI_INFER_ERR_NOT_INIT;
    }

    ei_infer_clear_result(p_result);
    p_result->window_count = p_ctx->sample_count;
    p_result->window_ready = EIInfer_IsWindowReady(p_ctx);

    if (!p_result->window_ready)
    {
        p_ctx->last_result = *p_result;
        return EI_INFER_ERR_WINDOW_NOT_READY;
    }

    p_ctx->state = EI_INFER_STATE_RUNNING;

    /* 将环形窗口按时间顺序展开为 EI 需要的 760 长度原始输入 */
    ei_infer_materialize_ordered_frame(p_ctx, features);

    signal_err = numpy::signal_from_buffer(features,
                                           (uint32_t) EI_INFER_FEATURE_FRAME_SIZE,
                                           &signal);
    if (signal_err != 0)
    {
        p_result->infer_err = EI_INFER_ERR_SIGNAL_FROM_BUFFER;
        p_result->backend_err = signal_err;
        p_ctx->state = EI_INFER_STATE_ERROR;
        p_ctx->last_result = *p_result;
        return EI_INFER_ERR_SIGNAL_FROM_BUFFER;
    }

    ei_err = run_classifier(&signal, &ei_result, false);
    if (ei_err != EI_IMPULSE_OK)
    {
        p_result->infer_err = EI_INFER_ERR_RUN_CLASSIFIER;
        p_result->backend_err = (int32_t) ei_err;
        p_ctx->state = EI_INFER_STATE_ERROR;
        p_ctx->last_result = *p_result;
        return EI_INFER_ERR_RUN_CLASSIFIER;
    }

    p_result->valid = true;
    p_result->infer_ran = true;
    p_result->timing_dsp_ms = ei_result.timing.dsp;
    p_result->timing_classification_ms = ei_result.timing.classification;
    p_result->timing_anomaly_ms = ei_result.timing.anomaly;

    ei_infer_fill_scores(p_result, &ei_result);

    p_ctx->state = EI_INFER_STATE_READY;
    p_ctx->last_result = *p_result;

    return EI_INFER_OK;
}

char const * EIInfer_GetLabelByIndex(uint32_t index)
{
    if (index >= EI_INFER_LABEL_COUNT)
    {
        return "unknown";
    }

    return g_ei_labels[index];
}

} /* <--- 修复点：这里提前闭合了 extern "C" */

/* =========================
 * Edge Impulse porting
 * 这里只保留一份实现
 * 以下函数处于 extern "C" 外部，保留 C++ linkage 以匹配 EI SDK
 * ========================= */
#undef stderr
extern "C" FILE * const stderr = NULL;

extern "C" void _exit(int status)
{
    (void) status;
    while (1)
    {
        ;
    }
}

void ei_printf(const char * format, ...)
{
    char print_buf[256];
    va_list args;

    va_start(args, format);
    (void) vsnprintf(print_buf, sizeof(print_buf), format, args);
    va_end(args);

    (void) AppDebug_WriteString(print_buf);
}

void ei_printf_float(float f)
{
    char print_buf[48];
    (void) snprintf(print_buf, sizeof(print_buf), "%.6f", (double) f);
    (void) AppDebug_WriteString(print_buf);
}

void * ei_malloc(size_t size)
{
    return malloc(size);
}

void * ei_calloc(size_t nitems, size_t size)
{
    return calloc(nitems, size);
}

void ei_free(void * ptr)
{
    free(ptr);
}

uint64_t ei_read_timer_ms(void)
{
    return (uint64_t) AppSystick_GetMs();
}

uint64_t ei_read_timer_us(void)
{
    return ((uint64_t) AppSystick_GetMs()) * 1000ULL;
}

EI_IMPULSE_ERROR ei_sleep(int32_t time_ms)
{
    R_BSP_SoftwareDelay((uint32_t) time_ms, BSP_DELAY_UNITS_MILLISECONDS);
    return EI_IMPULSE_OK;
}

EI_IMPULSE_ERROR ei_run_impulse_check_canceled(void)
{
    return EI_IMPULSE_OK;
}
