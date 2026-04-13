#ifndef MQ_STATE_MACHINE_H_
#define MQ_STATE_MACHINE_H_

#include <stdint.h>
#include <stdbool.h>
#include "mq_acquire.h"
#include "mq_metrics.h"

#define MQ_STATE_MACHINE_WINDOW_CAPACITY    (30U)

typedef enum e_mq_state_machine_state
{
    MQ_STATE_MACHINE_STATE_WARMUP = 0,
    MQ_STATE_MACHINE_STATE_WAIT_STABLE,
    MQ_STATE_MACHINE_STATE_BASELINE_READY,
    MQ_STATE_MACHINE_STATE_RUNNING
} mq_state_machine_state_t;

typedef enum e_mq_state_machine_quality
{
    MQ_STATE_MACHINE_QUALITY_NONE = 0,
    MQ_STATE_MACHINE_QUALITY_OK,
    MQ_STATE_MACHINE_QUALITY_SUSPECT,
    MQ_STATE_MACHINE_QUALITY_ERROR
} mq_state_machine_quality_t;

typedef enum e_mq_state_machine_flag
{
    MQ_STATE_MACHINE_FLAG_NONE          = 0x0000,
    MQ_STATE_MACHINE_FLAG_INCOMPLETE    = 0x0001,
    MQ_STATE_MACHINE_FLAG_ALL_ZERO      = 0x0002,
    MQ_STATE_MACHINE_FLAG_ALL_NEAR_ZERO = 0x0004,
    MQ_STATE_MACHINE_FLAG_ALL_EQUAL     = 0x0008
} mq_state_machine_flag_t;

typedef enum e_mq_state_machine_event
{
    MQ_STATE_MACHINE_EVENT_NONE = 0,
    MQ_STATE_MACHINE_EVENT_WARMUP_HOLD,
    MQ_STATE_MACHINE_EVENT_WARMUP_TO_WAIT_STABLE,
    MQ_STATE_MACHINE_EVENT_WAIT_STABLE_FILLING,
    MQ_STATE_MACHINE_EVENT_WAIT_STABLE_RESET,
    MQ_STATE_MACHINE_EVENT_WAIT_STABLE_NOT_STABLE,
    MQ_STATE_MACHINE_EVENT_BASELINE_LOCKED,
    MQ_STATE_MACHINE_EVENT_RUNNING_UPDATED,
    MQ_STATE_MACHINE_EVENT_RUNNING_DROP_SUSPECT,
    MQ_STATE_MACHINE_EVENT_RUNNING_DROP_ERROR,
    MQ_STATE_MACHINE_EVENT_RUNNING_RESET_TO_WARMUP
} mq_state_machine_event_t;

typedef struct st_mq_state_machine_config
{
    uint32_t warmup_min_frames;
    uint32_t stable_window_size;
    int32_t  stable_range_mv;
    uint32_t running_suspect_limit;
    int32_t  near_zero_mv_threshold;
} mq_state_machine_config_t;

typedef struct st_mq_state_machine
{
    mq_state_machine_state_t   state;
    mq_state_machine_quality_t last_quality;
    mq_state_machine_event_t   last_event;
    uint32_t                   last_flags;
    fsp_err_t                  last_metrics_err;

    uint32_t                   warmup_elapsed_frames;
    uint32_t                   stable_window_count;
    uint32_t                   consecutive_suspect_count;
    uint32_t                   consecutive_error_count;

    mq_state_machine_config_t  cfg;
    int32_t                    stable_window_mv[MQ_STATE_MACHINE_WINDOW_CAPACITY][ADS1115_CHANNEL_MAX];
} mq_state_machine_t;

void MQStateMachine_Clear(mq_state_machine_t * p_sm);
void MQStateMachine_GetDefaultConfig(mq_state_machine_config_t * p_cfg);
fsp_err_t MQStateMachine_Init(mq_state_machine_t * p_sm,
                              mq_state_machine_config_t const * p_cfg);
fsp_err_t MQStateMachine_ProcessFrame(mq_state_machine_t * p_sm,
                                      mq_frame_t const * p_frame,
                                      mq_metrics_t * p_metrics,
                                      mq_metrics_config_t const * p_metrics_cfg);

#endif /* MQ_STATE_MACHINE_H_ */
