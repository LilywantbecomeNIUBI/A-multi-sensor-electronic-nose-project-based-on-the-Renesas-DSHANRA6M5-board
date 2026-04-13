/*
 * mq_judge.h
 *
 *  Created on: 2026年3月12日
 *      Author: leo
 *
 *  模块定位：
 *  - MQ 数据质量判断层（第一批）
 *
 *  当前只负责：
 *  - 对单帧 mq_frame_t 做最小判断
 *  - 判断本轮是否完整
 *  - 判断是否命中：
 *      1) 不完整
 *      2) 四路全零
 *      3) 四路近零（阈值可配置）
 *      4) 四路完全相同
 *
 *  当前不负责：
 *  - baseline / delta / ratio / peak / recovery
 *  - 多帧趋势判断
 *  - 单路长期不变
 *  - 四路同步波动
 *  - RF / SVM 可用性判断
 */

#ifndef MQ_JUDGE_H_
#define MQ_JUDGE_H_

#include <stdbool.h>
#include <stdint.h>
#include "mq_acquire.h"

typedef enum e_mq_judge_level
{
    MQ_JUDGE_LEVEL_OK = 0,
    MQ_JUDGE_LEVEL_SUSPECT,
    MQ_JUDGE_LEVEL_ERROR
} mq_judge_level_t;

typedef enum e_mq_judge_layer
{
    MQ_JUDGE_LAYER_NONE = 0,
    MQ_JUDGE_LAYER_COMM,
    MQ_JUDGE_LAYER_VALUE,
    MQ_JUDGE_LAYER_RESPONSE
} mq_judge_layer_t;

typedef enum e_mq_judge_flag
{
    MQ_JUDGE_FLAG_NONE          = 0x0000,
    MQ_JUDGE_FLAG_INCOMPLETE    = 0x0001,
    MQ_JUDGE_FLAG_ALL_ZERO      = 0x0002,
    MQ_JUDGE_FLAG_ALL_NEAR_ZERO = 0x0004,
    MQ_JUDGE_FLAG_ALL_EQUAL     = 0x0008
} mq_judge_flag_t;

typedef struct st_mq_judge_cfg
{
    int32_t near_zero_mv_threshold;
} mq_judge_cfg_t;

typedef struct st_mq_judge_result
{
    bool              complete;
    mq_judge_level_t  level;
    mq_judge_layer_t  layer;
    uint32_t          flags;
} mq_judge_result_t;

void MQJudge_GetDefaultCfg(mq_judge_cfg_t * p_cfg);
void MQJudge_ClearResult(mq_judge_result_t * p_result);
fsp_err_t MQJudge_EvaluateFrame(mq_frame_t const * p_frame,
                                mq_judge_cfg_t const * p_cfg,
                                mq_judge_result_t * p_result);
bool MQJudge_HasFlag(mq_judge_result_t const * p_result,
                     mq_judge_flag_t flag);

#endif /* MQ_JUDGE_H_ */
