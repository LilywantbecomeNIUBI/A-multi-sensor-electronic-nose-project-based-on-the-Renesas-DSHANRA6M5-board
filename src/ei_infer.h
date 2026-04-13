#ifndef EI_INFER_H_
#define EI_INFER_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 项目内固定模型参数（来自当前已确认的 EI 导出模型）
 * - 5 个输入轴
 * - 152 个时间点
 * - 760 个 DSP 原始输入长度
 * - 4 个分类标签
 */
#define EI_INFER_AXES_COUNT            (5U)
#define EI_INFER_WINDOW_SAMPLE_COUNT   (152U)
#define EI_INFER_FEATURE_FRAME_SIZE    (EI_INFER_AXES_COUNT * EI_INFER_WINDOW_SAMPLE_COUNT)
#define EI_INFER_LABEL_COUNT           (4U)
#define EI_INFER_INTERVAL_MS           (1000U)
#define EI_INFER_SCORE_THRESHOLD       (0.60f)

typedef enum e_ei_infer_state
{
    EI_INFER_STATE_IDLE = 0,
    EI_INFER_STATE_COLLECTING,
    EI_INFER_STATE_READY,
    EI_INFER_STATE_RUNNING,
    EI_INFER_STATE_ERROR
} ei_infer_state_t;

typedef enum e_ei_infer_err
{
    EI_INFER_OK = 0,
    EI_INFER_ERR_PARAM = -1,
    EI_INFER_ERR_NOT_INIT = -2,
    EI_INFER_ERR_WINDOW_NOT_READY = -3,
    EI_INFER_ERR_SIGNAL_FROM_BUFFER = -4,
    EI_INFER_ERR_RUN_CLASSIFIER = -5
} ei_infer_err_t;

typedef struct st_ei_input_sample
{
    float env_voc_index;
    float mq135_mv;
    float mq138_mv;
    float mq2_mv;
    float mq3_mv;
} ei_input_sample_t;

typedef struct st_ei_infer_result
{
    bool     valid;
    int32_t  infer_err;
    int32_t  backend_err;

    uint32_t window_count;
    bool     window_ready;
    bool     infer_ran;

    uint32_t timing_dsp_ms;
    uint32_t timing_classification_ms;
    uint32_t timing_anomaly_ms;

    float    scores[EI_INFER_LABEL_COUNT];
    uint32_t top1_index;
    float    top1_score;
    uint32_t top2_index;
    float    top2_score;
} ei_infer_result_t;

typedef struct st_ei_infer_context
{
    ei_infer_state_t state;
    bool             initialized;
    uint32_t         sample_count;
    uint32_t         write_index;
    ei_input_sample_t window[EI_INFER_WINDOW_SAMPLE_COUNT];
    ei_infer_result_t last_result;
} ei_infer_context_t;

int32_t EIInfer_Init(ei_infer_context_t * p_ctx);
int32_t EIInfer_Reset(ei_infer_context_t * p_ctx);
int32_t EIInfer_PushSample(ei_infer_context_t * p_ctx,
                           ei_input_sample_t const * p_sample);
bool EIInfer_IsWindowReady(ei_infer_context_t const * p_ctx);
int32_t EIInfer_Run(ei_infer_context_t * p_ctx,
                    ei_infer_result_t * p_result);
char const * EIInfer_GetLabelByIndex(uint32_t index);

#ifdef __cplusplus
}
#endif

#endif /* EI_INFER_H_ */
