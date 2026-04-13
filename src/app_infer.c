#include "app_infer.h"

#include <string.h>

static void app_infer_clear_output(app_infer_output_t * p_output)
{
    if (p_output == NULL)
    {
        return;
    }

    (void) memset(p_output, 0, sizeof(app_infer_output_t));
    p_output->process_err = APP_INFER_OK;
    p_output->decision = APP_INFER_DECISION_NONE;
}

static app_infer_decision_t app_infer_map_index_to_decision(uint32_t index)
{
    switch (index)
    {
        case 0U:
            return APP_INFER_DECISION_AIR;
        case 1U:
            return APP_INFER_DECISION_ALCOHOL;
        case 2U:
            return APP_INFER_DECISION_PERFUME;
        case 3U:
            return APP_INFER_DECISION_VINEGAR;
        default:
            return APP_INFER_DECISION_NONE;
    }
}

static void app_infer_fill_decision(app_infer_output_t * p_output)
{
    if (NULL == p_output)
    {
        return;
    }

    p_output->decision = APP_INFER_DECISION_NONE;
    p_output->decision_index = 0U;
    p_output->decision_score = 0.0f;

    if (!p_output->raw_result.valid)
    {
        return;
    }

    p_output->decision_index = p_output->raw_result.top1_index;
    p_output->decision_score = p_output->raw_result.top1_score;

    if (p_output->raw_result.top1_score < EI_INFER_SCORE_THRESHOLD)
    {
        p_output->decision = APP_INFER_DECISION_UNCERTAIN;
        return;
    }

    p_output->decision = app_infer_map_index_to_decision(p_output->raw_result.top1_index);
}

int32_t AppInfer_Init(app_infer_context_t * p_ctx)
{
    if (NULL == p_ctx)
    {
        return APP_INFER_ERR_PARAM;
    }

    (void) memset(p_ctx, 0, sizeof(app_infer_context_t));
    p_ctx->initialized = true;
    return EIInfer_Init(&p_ctx->ei);
}

int32_t AppInfer_Reset(app_infer_context_t * p_ctx)
{
    if (NULL == p_ctx)
    {
        return APP_INFER_ERR_PARAM;
    }

    if (!p_ctx->initialized)
    {
        return APP_INFER_ERR_NOT_INIT;
    }

    return EIInfer_Reset(&p_ctx->ei);
}

bool AppInfer_IsRecordAccepted(sensor_hub_record_t const * p_record)
{
    if (NULL == p_record)
    {
        return false;
    }

    /*
     * 对应硬件功能：
     * - 仅在本帧环境链路有效、且 MQ 本帧采样完成时，将样本送入模型窗口
     *
     * 依赖资料：
     * - env.valid 表示当前 VOC 输出有效
     * - mq.frame_complete 表示当前 MQ 采样帧完整
     *
     * 前提假设：
     * - 这里故意不使用 mq.metrics_valid 作为门控，因为模型输入是 mq.mv 原始电压，
     *   不是 baseline/delta/ratio 等派生统计量
     */
    return (p_record->env.valid && p_record->mq.frame_complete);
}

int32_t AppInfer_ExtractInputSample(sensor_hub_record_t const * p_record,
                                    ei_input_sample_t * p_sample)
{
    if ((NULL == p_record) || (NULL == p_sample))
    {
        return APP_INFER_ERR_PARAM;
    }

    p_sample->env_voc_index = (float) p_record->env.voc_index;
    p_sample->mq135_mv      = (float) p_record->mq.mv[0];
    p_sample->mq138_mv      = (float) p_record->mq.mv[1];
    p_sample->mq2_mv        = (float) p_record->mq.mv[2];
    p_sample->mq3_mv        = (float) p_record->mq.mv[3];

    return APP_INFER_OK;
}

int32_t AppInfer_ProcessRecord(app_infer_context_t * p_ctx,
                               sensor_hub_record_t const * p_record,
                               app_infer_output_t * p_output)
{
    int32_t err;
    int32_t run_err;

    if ((NULL == p_ctx) || (NULL == p_record) || (NULL == p_output))
    {
        return APP_INFER_ERR_PARAM;
    }

    if (!p_ctx->initialized)
    {
        return APP_INFER_ERR_NOT_INIT;
    }

    app_infer_clear_output(p_output);
    p_output->sequence = p_record->sequence;
    p_output->raw_result = p_ctx->ei.last_result;
    p_output->window_count = p_ctx->ei.sample_count;
    p_output->window_ready = EIInfer_IsWindowReady(&p_ctx->ei);

    err = AppInfer_ExtractInputSample(p_record, &p_output->input_sample);
    if (APP_INFER_OK != err)
    {
        p_output->process_err = err;
        return err;
    }

    if (!AppInfer_IsRecordAccepted(p_record))
    {
        p_output->process_err = APP_INFER_ERR_SAMPLE_REJECTED;
        app_infer_fill_decision(p_output);
        p_output->valid = true;
        return APP_INFER_OK;
    }

    p_output->sample_accepted = true;

    err = EIInfer_PushSample(&p_ctx->ei, &p_output->input_sample);
    if (EI_INFER_OK != err)
    {
        p_output->process_err = err;
        return err;
    }

    p_output->window_count = p_ctx->ei.sample_count;
    p_output->window_ready = EIInfer_IsWindowReady(&p_ctx->ei);
    p_output->raw_result = p_ctx->ei.last_result;

    if (p_output->window_ready)
    {
        run_err = EIInfer_Run(&p_ctx->ei, &p_output->raw_result);
        p_output->infer_ran_this_frame = (run_err == EI_INFER_OK) && p_output->raw_result.infer_ran;
        if ((run_err != EI_INFER_OK) && (run_err != EI_INFER_ERR_WINDOW_NOT_READY))
        {
            p_output->process_err = run_err;
        }
    }

    p_output->window_count = p_output->raw_result.window_count;
    p_output->window_ready = p_output->raw_result.window_ready;
    app_infer_fill_decision(p_output);
    p_output->valid = true;
    return p_output->process_err;
}

char const * AppInfer_GetDecisionName(app_infer_decision_t decision)
{
    switch (decision)
    {
        case APP_INFER_DECISION_NONE:
            return "none";
        case APP_INFER_DECISION_UNCERTAIN:
            return "uncertain";
        case APP_INFER_DECISION_AIR:
            return "air";
        case APP_INFER_DECISION_ALCOHOL:
            return "alcohol";
        case APP_INFER_DECISION_PERFUME:
            return "perfume";
        case APP_INFER_DECISION_VINEGAR:
            return "vinegar";
        default:
            return "unknown";
    }
}
