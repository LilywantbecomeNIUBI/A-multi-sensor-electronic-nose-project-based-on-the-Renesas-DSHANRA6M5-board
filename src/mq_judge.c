/*
 * mq_judge.c
 *
 *  Created on: 2026年3月12日
 *      Author: leo
 */

/*  模块定位：
 *  - MQ 数据质量判断层（第一批）
 *
 *  当前只负责：
 *  - 对单帧 mq_frame_t 做最小判断
 *  - 输出：complete / level / layer / flags
 *
 *  当前不负责：
 *  - 历史帧缓存
 *  - 预热状态机
 *  - baseline / response / recovery
 *  - 训练集可用性总评
 */
#include <stdbool.h>
#include <stddef.h>
#include "mq_judge.h"

/*
 * 对应功能：
 * - 计算 int32_t 的绝对值
 *
 * 依赖资料：
 * - 无额外硬件资料；仅依赖 C 语言整型运算
 *
 * 前提假设：
 * - 当前 mV 数值范围来自 ADS1115 满量程换算，不会接近 int32_t 极限
 */
static int32_t MQJudge_AbsInt32(int32_t value)
{
    if (value < 0)
    {
        return -value;
    }

    return value;
}

/*
 * 对应功能：
 * - 判断当前帧的 4 路 raw/mV 是否全部为 0
 *
 * 依赖资料：
 * - mq_frame_t 中 raw[] / mv[] 的定义
 *
 * 前提假设：
 * - p_frame 非 NULL
 * - 通道数固定为 ADS1115_CHANNEL_MAX
 */
static bool MQJudge_IsAllZero(mq_frame_t const * p_frame)
{
    uint32_t ch;

    for (ch = 0U; ch < (uint32_t) ADS1115_CHANNEL_MAX; ch++)
    {
        if ((0 != p_frame->raw[ch]) || (0 != p_frame->mv[ch]))
        {
            return false;
        }
    }

    return true;
}

/*
 * 对应功能：
 * - 判断当前帧的 4 路 mV 是否都在“近零阈值”内
 *
 * 依赖资料：
 * - mq_frame_t 中 mv[] 的定义
 * - mq_judge_cfg_t 中 near_zero_mv_threshold 的定义
 *
 * 前提假设：
 * - p_frame 非 NULL
 * - threshold >= 0
 */
static bool MQJudge_IsAllNearZero(mq_frame_t const * p_frame, int32_t threshold)
{
    uint32_t ch;

    for (ch = 0U; ch < (uint32_t) ADS1115_CHANNEL_MAX; ch++)
    {
        if (MQJudge_AbsInt32(p_frame->mv[ch]) > threshold)
        {
            return false;
        }
    }

    return true;
}

/*
 * 对应功能：
 * - 判断当前帧的 4 路 raw/mV 是否完全相同
 *
 * 依赖资料：
 * - mq_frame_t 中 raw[] / mv[] 的定义
 *
 * 前提假设：
 * - p_frame 非 NULL
 * - 通道数固定为 ADS1115_CHANNEL_MAX
 */
static bool MQJudge_IsAllEqual(mq_frame_t const * p_frame)
{
    uint32_t ch;
    int16_t raw_ref;
    int32_t mv_ref;

    raw_ref = p_frame->raw[0];
    mv_ref  = p_frame->mv[0];

    for (ch = 1U; ch < (uint32_t) ADS1115_CHANNEL_MAX; ch++)
    {
        if ((p_frame->raw[ch] != raw_ref) || (p_frame->mv[ch] != mv_ref))
        {
            return false;
        }
    }

    return true;
}

/*
 * 对应功能：
 * - 获取默认判断配置
 *
 * 依赖资料：
 * - 无额外硬件资料；仅依赖本模块配置结构定义
 *
 * 前提假设：
 * - 当前缺少项目级 near-zero 阈值，因此默认配置采用保守值 0 mV
 * - 上层后续可按实验结果覆盖 near_zero_mv_threshold
 */
void MQJudge_GetDefaultCfg(mq_judge_cfg_t * p_cfg)
{
    if (NULL == p_cfg)
    {
        return;
    }

    p_cfg->near_zero_mv_threshold = 0;
}

/*
 * 对应功能：
 * - 清空一次判断结果
 *
 * 依赖资料：
 * - 无额外硬件资料；仅依赖本模块结果结构定义
 *
 * 前提假设：
 * - p_result 指向有效对象
 */
void MQJudge_ClearResult(mq_judge_result_t * p_result)
{
    if (NULL == p_result)
    {
        return;
    }

    p_result->complete = false;
    p_result->level    = MQ_JUDGE_LEVEL_OK;
    p_result->layer    = MQ_JUDGE_LAYER_NONE;
    p_result->flags    = (uint32_t) MQ_JUDGE_FLAG_NONE;
}

/*
 * 对应功能：
 * - 对单帧 mq_frame_t 做第一批最小规则判断
 *
 * 判断规则优先级：
 * 1. 帧不完整 -> ERROR / COMM / INCOMPLETE
 * 2. 四路全零 -> ERROR / VALUE / ALL_ZERO
 * 3. 四路近零 -> SUSPECT / VALUE / ALL_NEAR_ZERO
 * 4. 四路全相同 -> SUSPECT / VALUE / ALL_EQUAL
 * 5. 其余 -> OK / NONE / NONE
 *
 * 依赖资料：
 * - mq_acquire.h/.c 中 mq_frame_t 结构与 MQAcquire_IsFrameComplete()
 *
 * 前提假设：
 * - p_frame 已由 MQAcquire_ReadFrame() 填充
 * - 当前只做单帧判断，不做历史帧比较
 */
fsp_err_t MQJudge_EvaluateFrame(mq_frame_t const * p_frame,
                                mq_judge_cfg_t const * p_cfg,
                                mq_judge_result_t * p_result)
{
    if ((NULL == p_frame) || (NULL == p_cfg) || (NULL == p_result))
    {
        return FSP_ERR_ASSERTION;
    }

    MQJudge_ClearResult(p_result);

    p_result->complete = MQAcquire_IsFrameComplete(p_frame);

    if (false == p_result->complete)
    {
        p_result->level = MQ_JUDGE_LEVEL_ERROR;
        p_result->layer = MQ_JUDGE_LAYER_COMM;
        p_result->flags = (uint32_t) MQ_JUDGE_FLAG_INCOMPLETE;
        return FSP_SUCCESS;
    }

    if (MQJudge_IsAllZero(p_frame))
    {
        p_result->level = MQ_JUDGE_LEVEL_ERROR;
        p_result->layer = MQ_JUDGE_LAYER_VALUE;
        p_result->flags = (uint32_t) MQ_JUDGE_FLAG_ALL_ZERO;
        return FSP_SUCCESS;
    }

    if ((p_cfg->near_zero_mv_threshold >= 0) &&
        MQJudge_IsAllNearZero(p_frame, p_cfg->near_zero_mv_threshold))
    {
        p_result->level = MQ_JUDGE_LEVEL_SUSPECT;
        p_result->layer = MQ_JUDGE_LAYER_VALUE;
        p_result->flags = (uint32_t) MQ_JUDGE_FLAG_ALL_NEAR_ZERO;
        return FSP_SUCCESS;
    }

    if (MQJudge_IsAllEqual(p_frame))
    {
        p_result->level = MQ_JUDGE_LEVEL_SUSPECT;
        p_result->layer = MQ_JUDGE_LAYER_VALUE;
        p_result->flags = (uint32_t) MQ_JUDGE_FLAG_ALL_EQUAL;
        return FSP_SUCCESS;
    }

    p_result->level = MQ_JUDGE_LEVEL_OK;
    p_result->layer = MQ_JUDGE_LAYER_NONE;
    p_result->flags = (uint32_t) MQ_JUDGE_FLAG_NONE;

    return FSP_SUCCESS;
}

/*
 * 对应功能：
 * - 查询判断结果里是否包含某个现象标志
 *
 * 依赖资料：
 * - 无额外硬件资料；仅依赖 flags 位定义
 *
 * 前提假设：
 * - p_result 可以为 NULL；NULL 时返回 false
 */
bool MQJudge_HasFlag(mq_judge_result_t const * p_result,
                     mq_judge_flag_t flag)
{
    if (NULL == p_result)
    {
        return false;
    }

    return (0U != (p_result->flags & (uint32_t) flag));
}
