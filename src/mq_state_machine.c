#include <stddef.h>
#include "mq_state_machine.h"
#include "mq_judge.h"

static void MQStateMachine_ClearWindow(mq_state_machine_t * p_sm)
{
    uint32_t i;
    uint32_t ch;

    p_sm->stable_window_count = 0U;

    for (i = 0U; i < MQ_STATE_MACHINE_WINDOW_CAPACITY; i++)
    {
        for (ch = 0U; ch < (uint32_t) ADS1115_CHANNEL_MAX; ch++)
        {
            p_sm->stable_window_mv[i][ch] = 0;
        }
    }
}

static void MQStateMachine_ResetRuntimeCounters(mq_state_machine_t * p_sm)
{
    p_sm->consecutive_suspect_count = 0U;
    p_sm->consecutive_error_count = 0U;
}

static mq_state_machine_quality_t MQStateMachine_MapQuality(mq_judge_result_t const * p_result)
{
    if (MQ_JUDGE_LEVEL_ERROR == p_result->level)
    {
        return MQ_STATE_MACHINE_QUALITY_ERROR;
    }

    if (MQ_JUDGE_LEVEL_SUSPECT == p_result->level)
    {
        return MQ_STATE_MACHINE_QUALITY_SUSPECT;
    }

    return MQ_STATE_MACHINE_QUALITY_OK;
}

static uint32_t MQStateMachine_MapFlags(mq_judge_result_t const * p_result)
{
    return p_result->flags;
}

static void MQStateMachine_PushWindowFrame(mq_state_machine_t * p_sm,
                                           mq_frame_t const * p_frame)
{
    uint32_t i;
    uint32_t ch;
    uint32_t window_size = p_sm->cfg.stable_window_size;

    if (p_sm->stable_window_count < window_size)
    {
        for (ch = 0U; ch < (uint32_t) ADS1115_CHANNEL_MAX; ch++)
        {
            p_sm->stable_window_mv[p_sm->stable_window_count][ch] = p_frame->mv[ch];
        }

        p_sm->stable_window_count++;
        return;
    }

    for (i = 1U; i < window_size; i++)
    {
        for (ch = 0U; ch < (uint32_t) ADS1115_CHANNEL_MAX; ch++)
        {
            p_sm->stable_window_mv[i - 1U][ch] = p_sm->stable_window_mv[i][ch];
        }
    }

    for (ch = 0U; ch < (uint32_t) ADS1115_CHANNEL_MAX; ch++)
    {
        p_sm->stable_window_mv[window_size - 1U][ch] = p_frame->mv[ch];
    }
}

static bool MQStateMachine_IsWindowStable(mq_state_machine_t const * p_sm)
{
    uint32_t ch;
    uint32_t i;
    int32_t min_mv;
    int32_t max_mv;

    if (p_sm->stable_window_count < p_sm->cfg.stable_window_size)
    {
        return false;
    }

    for (ch = 0U; ch < (uint32_t) ADS1115_CHANNEL_MAX; ch++)
    {
        min_mv = p_sm->stable_window_mv[0][ch];
        max_mv = p_sm->stable_window_mv[0][ch];

        for (i = 1U; i < p_sm->cfg.stable_window_size; i++)
        {
            if (p_sm->stable_window_mv[i][ch] < min_mv)
            {
                min_mv = p_sm->stable_window_mv[i][ch];
            }

            if (p_sm->stable_window_mv[i][ch] > max_mv)
            {
                max_mv = p_sm->stable_window_mv[i][ch];
            }
        }

        if ((max_mv - min_mv) >= p_sm->cfg.stable_range_mv)
        {
            return false;
        }
    }

    return true;
}

static void MQStateMachine_SortInt32Ascending(int32_t * p_values, uint32_t count)
{
    uint32_t i;
    uint32_t j;
    int32_t key;

    for (i = 1U; i < count; i++)
    {
        key = p_values[i];
        j = i;

        while ((j > 0U) && (p_values[j - 1U] > key))
        {
            p_values[j] = p_values[j - 1U];
            j--;
        }

        p_values[j] = key;
    }
}

static fsp_err_t MQStateMachine_BuildMedianBaseline(mq_state_machine_t const * p_sm,
                                                    int32_t baseline_mv[ADS1115_CHANNEL_MAX])
{
    uint32_t ch;
    uint32_t i;
    uint32_t count;
    uint32_t mid;
    int32_t temp[MQ_STATE_MACHINE_WINDOW_CAPACITY];
    int64_t median_sum;

    if (NULL == p_sm)
    {
        return FSP_ERR_ASSERTION;
    }

    count = p_sm->cfg.stable_window_size;
    if ((0U == count) || (count > MQ_STATE_MACHINE_WINDOW_CAPACITY))
    {
        return FSP_ERR_INVALID_ARGUMENT;
    }

    for (ch = 0U; ch < (uint32_t) ADS1115_CHANNEL_MAX; ch++)
    {
        for (i = 0U; i < count; i++)
        {
            temp[i] = p_sm->stable_window_mv[i][ch];
        }

        MQStateMachine_SortInt32Ascending(temp, count);

        mid = count / 2U;
        if (0U != (count & 0x01U))
        {
            baseline_mv[ch] = temp[mid];
        }
        else
        {
            median_sum = (int64_t) temp[mid - 1U] + (int64_t) temp[mid];
            baseline_mv[ch] = (int32_t) (median_sum / 2);
        }
    }

    return FSP_SUCCESS;
}

static void MQStateMachine_ResetToWarmup(mq_state_machine_t * p_sm,
                                         mq_metrics_t * p_metrics)
{
    MQMetrics_Clear(p_metrics);
    MQStateMachine_ClearWindow(p_sm);
    MQStateMachine_ResetRuntimeCounters(p_sm);
    p_sm->warmup_elapsed_frames = 0U;
    p_sm->state = MQ_STATE_MACHINE_STATE_WARMUP;
}

void MQStateMachine_Clear(mq_state_machine_t * p_sm)
{
    if (NULL == p_sm)
    {
        return;
    }

    p_sm->state = MQ_STATE_MACHINE_STATE_WARMUP;
    p_sm->last_quality = MQ_STATE_MACHINE_QUALITY_NONE;
    p_sm->last_event = MQ_STATE_MACHINE_EVENT_NONE;
    p_sm->last_flags = (uint32_t) MQ_STATE_MACHINE_FLAG_NONE;
    p_sm->last_metrics_err = FSP_SUCCESS;
    p_sm->warmup_elapsed_frames = 0U;
    p_sm->stable_window_count = 0U;
    p_sm->consecutive_suspect_count = 0U;
    p_sm->consecutive_error_count = 0U;
    p_sm->cfg.warmup_min_frames = 0U;
    p_sm->cfg.stable_window_size = 0U;
    p_sm->cfg.stable_range_mv = 0;
    p_sm->cfg.running_suspect_limit = 0U;
    p_sm->cfg.near_zero_mv_threshold = 0;
    MQStateMachine_ClearWindow(p_sm);
}

void MQStateMachine_GetDefaultConfig(mq_state_machine_config_t * p_cfg)
{
    if (NULL == p_cfg)
    {
        return;
    }

    p_cfg->warmup_min_frames = 60U;
    p_cfg->stable_window_size = 20U;
    p_cfg->stable_range_mv = 10;
    p_cfg->running_suspect_limit = 5U;
    p_cfg->near_zero_mv_threshold = 0;
}

fsp_err_t MQStateMachine_Init(mq_state_machine_t * p_sm,
                              mq_state_machine_config_t const * p_cfg)
{
    if ((NULL == p_sm) || (NULL == p_cfg))
    {
        return FSP_ERR_ASSERTION;
    }

    if ((0U == p_cfg->warmup_min_frames) ||
        (0U == p_cfg->stable_window_size) ||
        (p_cfg->stable_window_size > MQ_STATE_MACHINE_WINDOW_CAPACITY) ||
        (p_cfg->stable_range_mv <= 0))
    {
        return FSP_ERR_INVALID_ARGUMENT;
    }

    MQStateMachine_Clear(p_sm);
    p_sm->cfg = *p_cfg;
    p_sm->state = MQ_STATE_MACHINE_STATE_WARMUP;

    return FSP_SUCCESS;
}

fsp_err_t MQStateMachine_ProcessFrame(mq_state_machine_t * p_sm,
                                      mq_frame_t const * p_frame,
                                      mq_metrics_t * p_metrics,
                                      mq_metrics_config_t const * p_metrics_cfg)
{
    mq_judge_cfg_t judge_cfg;
    mq_judge_result_t judge_result;
    int32_t baseline_mv[ADS1115_CHANNEL_MAX];
    fsp_err_t err;

    if ((NULL == p_sm) || (NULL == p_frame) || (NULL == p_metrics) || (NULL == p_metrics_cfg))
    {
        return FSP_ERR_ASSERTION;
    }

    if (MQ_STATE_MACHINE_STATE_BASELINE_READY == p_sm->state)
    {
        p_sm->state = MQ_STATE_MACHINE_STATE_RUNNING;
    }

    p_sm->last_event = MQ_STATE_MACHINE_EVENT_NONE;
    p_sm->last_flags = (uint32_t) MQ_STATE_MACHINE_FLAG_NONE;
    p_sm->last_metrics_err = FSP_SUCCESS;

    judge_cfg.near_zero_mv_threshold = p_sm->cfg.near_zero_mv_threshold;
    err = MQJudge_EvaluateFrame(p_frame, &judge_cfg, &judge_result);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    p_sm->last_quality = MQStateMachine_MapQuality(&judge_result);
    p_sm->last_flags = MQStateMachine_MapFlags(&judge_result);

    if (MQ_STATE_MACHINE_STATE_WARMUP == p_sm->state)
    {
        p_sm->warmup_elapsed_frames++;

        if (p_sm->warmup_elapsed_frames < p_sm->cfg.warmup_min_frames)
        {
            p_sm->last_event = MQ_STATE_MACHINE_EVENT_WARMUP_HOLD;
            return FSP_SUCCESS;
        }

        p_sm->state = MQ_STATE_MACHINE_STATE_WAIT_STABLE;
        p_sm->last_event = MQ_STATE_MACHINE_EVENT_WARMUP_TO_WAIT_STABLE;
        MQStateMachine_ClearWindow(p_sm);
        MQStateMachine_ResetRuntimeCounters(p_sm);
    }

    if (MQ_STATE_MACHINE_STATE_WAIT_STABLE == p_sm->state)
    {
        if (MQ_STATE_MACHINE_QUALITY_OK != p_sm->last_quality)
        {
            MQStateMachine_ClearWindow(p_sm);
            p_sm->last_event = MQ_STATE_MACHINE_EVENT_WAIT_STABLE_RESET;
            return FSP_SUCCESS;
        }

        MQStateMachine_PushWindowFrame(p_sm, p_frame);

        if (p_sm->stable_window_count < p_sm->cfg.stable_window_size)
        {
            p_sm->last_event = MQ_STATE_MACHINE_EVENT_WAIT_STABLE_FILLING;
            return FSP_SUCCESS;
        }

        if (!MQStateMachine_IsWindowStable(p_sm))
        {
            p_sm->last_event = MQ_STATE_MACHINE_EVENT_WAIT_STABLE_NOT_STABLE;
            return FSP_SUCCESS;
        }

        err = MQStateMachine_BuildMedianBaseline(p_sm, baseline_mv);
        if (FSP_SUCCESS != err)
        {
            return err;
        }

        err = MQMetrics_SetBaselineFromMilliVolts(p_metrics, baseline_mv);
        p_sm->last_metrics_err = err;
        if (FSP_SUCCESS != err)
        {
            return err;
        }

        MQMetrics_ResetPeaks(p_metrics);
        MQStateMachine_ResetRuntimeCounters(p_sm);
        p_sm->state = MQ_STATE_MACHINE_STATE_BASELINE_READY;
        p_sm->last_event = MQ_STATE_MACHINE_EVENT_BASELINE_LOCKED;
        return FSP_SUCCESS;
    }

    if (MQ_STATE_MACHINE_STATE_RUNNING == p_sm->state)
    {
        if (MQ_STATE_MACHINE_QUALITY_OK == p_sm->last_quality)
        {
            MQStateMachine_ResetRuntimeCounters(p_sm);
            err = MQMetrics_Update(p_metrics, p_frame, p_metrics_cfg);
            p_sm->last_metrics_err = err;
            if (FSP_SUCCESS != err)
            {
                return err;
            }

            p_sm->last_event = MQ_STATE_MACHINE_EVENT_RUNNING_UPDATED;
            return FSP_SUCCESS;
        }

        if (MQ_STATE_MACHINE_QUALITY_SUSPECT == p_sm->last_quality)
        {
            p_sm->consecutive_suspect_count++;
            p_sm->consecutive_error_count = 0U;

            if ((p_sm->cfg.running_suspect_limit > 0U) &&
                (p_sm->consecutive_suspect_count > p_sm->cfg.running_suspect_limit))
            {
                MQStateMachine_ResetToWarmup(p_sm, p_metrics);
                p_sm->last_event = MQ_STATE_MACHINE_EVENT_RUNNING_RESET_TO_WARMUP;
            }
            else
            {
                p_sm->last_event = MQ_STATE_MACHINE_EVENT_RUNNING_DROP_SUSPECT;
            }

            return FSP_SUCCESS;
        }

        p_sm->consecutive_error_count++;
        p_sm->consecutive_suspect_count = 0U;
        p_sm->last_event = MQ_STATE_MACHINE_EVENT_RUNNING_DROP_ERROR;
        return FSP_SUCCESS;
    }

    return FSP_SUCCESS;
}
