#ifndef APP_INFER_H_
#define APP_INFER_H_

/*
 * 对应功能：
 * - 应用层推理封装接口
 * - 从 sensor_hub_record_t 中提取模型输入，驱动 EI 推理层运行
 *
 * 依赖资料：
 * - sensor_hub_record_t 提供 env.voc_index 与 mq.mv[0..3]
 * - 外层以 1 秒周期调用 SensorHub_SampleOnce()
 *
 * 前提假设：
 * - 本层不直接包含任何 Edge Impulse SDK 头文件
 */

#include <stdint.h>
#include <stdbool.h>

#include "sensor_hub.h"
#include "ei_infer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define APP_INFER_OK                        (0)
#define APP_INFER_ERR_PARAM                 (-4200)
#define APP_INFER_ERR_NOT_INIT              (-4201)
#define APP_INFER_ERR_SAMPLE_REJECTED       (-4202)

typedef enum e_app_infer_decision
{
    APP_INFER_DECISION_NONE = 0,
    APP_INFER_DECISION_UNCERTAIN,
    APP_INFER_DECISION_AIR,
    APP_INFER_DECISION_ALCOHOL,
    APP_INFER_DECISION_PERFUME,
    APP_INFER_DECISION_VINEGAR
} app_infer_decision_t;

/*
 * 对应功能：
 * - 给上层/串口调试层提供统一输出
 */
typedef struct st_app_infer_output
{
    bool                 valid;
    bool                 sample_accepted;
    bool                 infer_ran_this_frame;

    int32_t              process_err;
    uint32_t             sequence;
    uint32_t             window_count;
    bool                 window_ready;

    ei_input_sample_t    input_sample;
    ei_infer_result_t    raw_result;

    app_infer_decision_t decision;
    uint32_t             decision_index;
    float                decision_score;
} app_infer_output_t;

typedef struct st_app_infer_context
{
    bool               initialized;
    ei_infer_context_t ei;
} app_infer_context_t;

int32_t AppInfer_Init(app_infer_context_t * p_ctx);
int32_t AppInfer_Reset(app_infer_context_t * p_ctx);
bool    AppInfer_IsRecordAccepted(sensor_hub_record_t const * p_record);
int32_t AppInfer_ExtractInputSample(sensor_hub_record_t const * p_record,
                                    ei_input_sample_t * p_sample);
int32_t AppInfer_ProcessRecord(app_infer_context_t * p_ctx,
                               sensor_hub_record_t const * p_record,
                               app_infer_output_t * p_output);
char const * AppInfer_GetDecisionName(app_infer_decision_t decision);

#ifdef __cplusplus
}
#endif

#endif /* APP_INFER_H_ */
