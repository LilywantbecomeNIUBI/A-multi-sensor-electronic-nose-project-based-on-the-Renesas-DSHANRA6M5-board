/*
 * mq_metrics.c
 *
 *  Created on: 2026年3月15日
 *      Author: leo
 */

/*
 * mq_metrics.c
 *
 *  模块定位：
 *  - MQ 指标整理层（第二批）
 */

#include "mq_metrics.h"

static int32_t MQMetrics_AbsInt32(int32_t value)
{
    if (value < 0)
    {
        return -value;
    }

    return value;
}

static bool MQMetrics_IsFrameUsable(mq_frame_t const * p_frame)
{
    if (NULL == p_frame)
    {
        return false;
    }

    return MQAcquire_IsFrameComplete(p_frame);
}

static void MQMetrics_FillCurrent(mq_metrics_t * p_metrics, mq_frame_t const * p_frame)
{
    uint32_t ch;

    for (ch = 0U; ch < (uint32_t) ADS1115_CHANNEL_MAX; ch++)
    {
        p_metrics->current_mv[ch] = p_frame->mv[ch];
    }
}

static void MQMetrics_FillDelta(mq_metrics_t * p_metrics)
{
    uint32_t ch;

    for (ch = 0U; ch < (uint32_t) ADS1115_CHANNEL_MAX; ch++)
    {
        p_metrics->delta_mv[ch] = p_metrics->current_mv[ch] - p_metrics->baseline_mv[ch];
    }
}

static void MQMetrics_FillRatioPermille(mq_metrics_t * p_metrics)
{
    uint32_t ch;
    int32_t baseline;

    for (ch = 0U; ch < (uint32_t) ADS1115_CHANNEL_MAX; ch++)
    {
        baseline = p_metrics->baseline_mv[ch];

        if (0 == baseline)
        {
            p_metrics->ratio_permille[ch] = 0;
        }
        else
        {
            p_metrics->ratio_permille[ch] = (p_metrics->delta_mv[ch] * 1000) / baseline;
        }
    }
}

static void MQMetrics_UpdatePeaks(mq_metrics_t * p_metrics)
{
    uint32_t ch;

    for (ch = 0U; ch < (uint32_t) ADS1115_CHANNEL_MAX; ch++)
    {
        if (p_metrics->delta_mv[ch] > p_metrics->peak_delta_mv[ch])
        {
            p_metrics->peak_delta_mv[ch] = p_metrics->delta_mv[ch];
        }
    }
}

static bool MQMetrics_CheckResponding(mq_metrics_t const * p_metrics,
                                      mq_metrics_config_t const * p_cfg)
{
    uint32_t ch;

    for (ch = 0U; ch < (uint32_t) ADS1115_CHANNEL_MAX; ch++)
    {
        if (p_metrics->delta_mv[ch] >= (int32_t) p_cfg->response_threshold_mv)
        {
            return true;
        }
    }

    return false;
}

static bool MQMetrics_CheckRecovered(mq_metrics_t const * p_metrics,
                                     mq_metrics_config_t const * p_cfg)
{
    uint32_t ch;

    for (ch = 0U; ch < (uint32_t) ADS1115_CHANNEL_MAX; ch++)
    {
        if (MQMetrics_AbsInt32(p_metrics->delta_mv[ch]) > (int32_t) p_cfg->recovery_band_mv)
        {
            return false;
        }
    }

    return true;
}

void MQMetrics_Clear(mq_metrics_t * p_metrics)
{
    uint32_t ch;

    if (NULL == p_metrics)
    {
        return;
    }

    p_metrics->valid = false;
    p_metrics->baseline_ready = false;
    p_metrics->responding = false;
    p_metrics->recovered = false;
    p_metrics->update_count = 0U;

    for (ch = 0U; ch < (uint32_t) ADS1115_CHANNEL_MAX; ch++)
    {
        p_metrics->baseline_mv[ch] = 0;
        p_metrics->current_mv[ch] = 0;
        p_metrics->delta_mv[ch] = 0;
        p_metrics->peak_delta_mv[ch] = 0;
        p_metrics->ratio_permille[ch] = 0;
    }
}

void MQMetrics_GetDefaultConfig(mq_metrics_config_t * p_cfg)
{
    if (NULL == p_cfg)
    {
        return;
    }

    p_cfg->response_threshold_mv = 10U;
    p_cfg->recovery_band_mv = 5U;
}

fsp_err_t MQMetrics_SetBaselineFromMilliVolts(mq_metrics_t * p_metrics,
                                              int32_t const p_baseline_mv[ADS1115_CHANNEL_MAX])
{
    uint32_t ch;

    if ((NULL == p_metrics) || (NULL == p_baseline_mv))
    {
        return FSP_ERR_ASSERTION;
    }

    for (ch = 0U; ch < (uint32_t) ADS1115_CHANNEL_MAX; ch++)
    {
        p_metrics->baseline_mv[ch] = p_baseline_mv[ch];
        p_metrics->current_mv[ch] = p_baseline_mv[ch];
        p_metrics->delta_mv[ch] = 0;
        p_metrics->peak_delta_mv[ch] = 0;
        p_metrics->ratio_permille[ch] = 0;
    }

    p_metrics->baseline_ready = true;
    p_metrics->valid = true;
    p_metrics->responding = false;
    p_metrics->recovered = true;
    p_metrics->update_count = 0U;

    return FSP_SUCCESS;
}

fsp_err_t MQMetrics_SetBaselineFromFrame(mq_metrics_t * p_metrics,
                                         mq_frame_t const * p_frame)
{
    if ((NULL == p_metrics) || (NULL == p_frame))
    {
        return FSP_ERR_ASSERTION;
    }

    if (!MQMetrics_IsFrameUsable(p_frame))
    {
        return FSP_ERR_INVALID_ARGUMENT;
    }

    return MQMetrics_SetBaselineFromMilliVolts(p_metrics, p_frame->mv);
}

void MQMetrics_ResetPeaks(mq_metrics_t * p_metrics)
{
    uint32_t ch;

    if (NULL == p_metrics)
    {
        return;
    }

    for (ch = 0U; ch < (uint32_t) ADS1115_CHANNEL_MAX; ch++)
    {
        p_metrics->peak_delta_mv[ch] = 0;
    }
}

fsp_err_t MQMetrics_Update(mq_metrics_t * p_metrics,
                           mq_frame_t const * p_frame,
                           mq_metrics_config_t const * p_cfg)
{
    if ((NULL == p_metrics) || (NULL == p_frame) || (NULL == p_cfg))
    {
        return FSP_ERR_ASSERTION;
    }

    if (!MQMetrics_IsFrameUsable(p_frame))
    {
        p_metrics->valid = false;
        return FSP_ERR_INVALID_ARGUMENT;
    }

    if (!p_metrics->baseline_ready)
    {
        p_metrics->valid = false;
        return FSP_ERR_NOT_OPEN;
    }

    MQMetrics_FillCurrent(p_metrics, p_frame);
    MQMetrics_FillDelta(p_metrics);
    MQMetrics_FillRatioPermille(p_metrics);
    MQMetrics_UpdatePeaks(p_metrics);

    p_metrics->responding = MQMetrics_CheckResponding(p_metrics, p_cfg);
    p_metrics->recovered = MQMetrics_CheckRecovered(p_metrics, p_cfg);
    p_metrics->valid = true;
    p_metrics->update_count++;

    return FSP_SUCCESS;
}
