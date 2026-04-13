/*
 * sensor_hub.c
 *
 *  Created on: 2026年3月16日
 *      Author: leo
 */

#include "sensor_hub.h"

static void sensor_hub_snapshot_mq(sensor_hub_t const * p_hub,
                                   sensor_hub_record_t * p_record,
                                   fsp_err_t acquire_err,
                                   fsp_err_t process_err)
{
    uint32_t ch;

    p_record->mq.frame_complete  = p_hub->mq_frame.complete;
    p_record->mq.metrics_valid   = p_hub->mq_metrics.valid;
    p_record->mq.baseline_ready  = p_hub->mq_metrics.baseline_ready;
    p_record->mq.responding      = p_hub->mq_metrics.responding;
    p_record->mq.recovered       = p_hub->mq_metrics.recovered;

    p_record->mq.acquire_err      = (int32_t) acquire_err;
    p_record->mq.process_err      = (int32_t) process_err;
    p_record->mq.last_metrics_err = (int32_t) p_hub->mq_sm.last_metrics_err;

    p_record->mq.state   = p_hub->mq_sm.state;
    p_record->mq.quality = p_hub->mq_sm.last_quality;
    p_record->mq.event   = p_hub->mq_sm.last_event;
    p_record->mq.flags   = p_hub->mq_sm.last_flags;

    p_record->mq.warmup_elapsed_frames     = p_hub->mq_sm.warmup_elapsed_frames;
    p_record->mq.stable_window_count       = p_hub->mq_sm.stable_window_count;
    p_record->mq.consecutive_suspect_count = p_hub->mq_sm.consecutive_suspect_count;
    p_record->mq.consecutive_error_count   = p_hub->mq_sm.consecutive_error_count;
    p_record->mq.update_count              = p_hub->mq_metrics.update_count;

    for (ch = 0U; ch < (uint32_t) ADS1115_CHANNEL_MAX; ch++)
    {
        p_record->mq.raw[ch]             = p_hub->mq_frame.raw[ch];
        p_record->mq.mv[ch]              = p_hub->mq_frame.mv[ch];
        p_record->mq.baseline_mv[ch]     = p_hub->mq_metrics.baseline_mv[ch];
        p_record->mq.delta_mv[ch]        = p_hub->mq_metrics.delta_mv[ch];
        p_record->mq.ratio_permille[ch]  = p_hub->mq_metrics.ratio_permille[ch];
        p_record->mq.peak_delta_mv[ch]   = p_hub->mq_metrics.peak_delta_mv[ch];
    }
}

static void sensor_hub_snapshot_env(sensor_hub_t const * p_hub,
                                    sensor_hub_record_t * p_record,
                                    int32_t service_err)
{
    sht40_sgp40_service_data_t const * p_latest;

    p_record->env.service_err = service_err;
    p_record->env.last_stage  = SHT40SGP40_ServiceGetLastStage(&p_hub->env_service);
    p_record->env.valid       = (SHT40_SGP40_SERVICE_OK == service_err);

    p_latest = SHT40SGP40_ServiceGetLatest(&p_hub->env_service);
    if (NULL == p_latest)
    {
        return;
    }

    p_record->env.temperature_c       = p_latest->temperature_c;
    p_record->env.humidity_rh         = p_latest->humidity_rh;
    p_record->env.temperature_milli_c = p_latest->temperature_milli_c;
    p_record->env.humidity_milli_rh   = p_latest->humidity_milli_rh;
    p_record->env.sraw                = p_latest->sraw;
    p_record->env.voc_index           = p_latest->voc_index;
    p_record->env.sample_count        = p_latest->sample_count;
    p_record->env.blackout            = p_latest->blackout;
}

static void sensor_hub_snapshot_i2c(sensor_hub_record_t * p_record)
{
    p_record->i2c_callback_count = g_app_i2c_callback_count;
    p_record->i2c_timeout_count  = g_app_i2c_timeout_count;
    p_record->i2c_recover_count  = g_app_i2c_recover_count;
    p_record->i2c_aborted        = g_app_i2c_aborted;
    p_record->i2c_last_event     = g_app_i2c_last_event;
}

void SensorHub_Clear(sensor_hub_t * p_hub)
{
    if (NULL == p_hub)
    {
        return;
    }

    memset(p_hub, 0, sizeof(sensor_hub_t));
    MQAcquire_ClearFrame(&p_hub->mq_frame);
    MQMetrics_Clear(&p_hub->mq_metrics);
    MQMetrics_GetDefaultConfig(&p_hub->mq_metrics_cfg);
    MQStateMachine_GetDefaultConfig(&p_hub->mq_sm_cfg);
    MQStateMachine_Clear(&p_hub->mq_sm);

    p_hub->last_hub_error     = SENSOR_HUB_OK;
    p_hub->last_mq_init_error = SENSOR_HUB_OK;
    p_hub->last_env_init_error = SENSOR_HUB_OK;
    p_hub->initialized = false;
}

void SensorHub_ClearRecord(sensor_hub_record_t * p_record)
{
    if (NULL == p_record)
    {
        return;
    }

    memset(p_record, 0, sizeof(sensor_hub_record_t));
    p_record->env.last_stage = SHT40_SGP40_STAGE_NONE;
    p_record->i2c_last_event = APP_I2C_EVENT_NONE;
}

int32_t SensorHub_Init(sensor_hub_t * p_hub)
{
    fsp_err_t err;
    int32_t env_err;

    if (NULL == p_hub)
    {
        return SENSOR_HUB_ERR_PARAM;
    }

    SensorHub_Clear(p_hub);

#if SENSOR_HUB_ENABLE_EXPLICIT_I2C_OPEN
    /*
     * 对应硬件功能：
     * - 可选地由统一层显式打开 I2C 主机
     *
     * 依赖资料：
     * - I2CMaster_Open()
     *
     * 前提假设：
     * - 你当前工程允许统一层先 Open
     */
    err = I2CMaster_Open();
    if (FSP_SUCCESS != err)
    {
        p_hub->last_mq_init_error = (int32_t) err;
        p_hub->last_hub_error = SENSOR_HUB_ERR_I2C_OPEN;
        return p_hub->last_hub_error;
    }
#endif

    /*
     * 对应硬件功能：
     * - 初始化 MQ 状态机本身
     *
     * 依赖资料：
     * - MQStateMachine_Init()
     *
     * 前提假设：
     * - 默认配置继续沿用你当前调通版本
     */
    err = MQStateMachine_Init(&p_hub->mq_sm, &p_hub->mq_sm_cfg);
    if (FSP_SUCCESS != err)
    {
        p_hub->last_mq_init_error = (int32_t) err;
        p_hub->last_hub_error = SENSOR_HUB_ERR_MQ_SM_INIT;
        return p_hub->last_hub_error;
    }

    /*
     * 对应硬件功能：
     * - 打开 ADS1115 所在采样链
     *
     * 依赖资料：
     * - ADS1115_Open()
     *
     * 前提假设：
     * - 沿用你当前 MQ 调试版已验证通过的打开路径
     */
    err = ADS1115_Open();
    if (FSP_SUCCESS != err)
    {
        p_hub->last_mq_init_error = (int32_t) err;
        p_hub->last_hub_error = SENSOR_HUB_ERR_MQ_OPEN;
        return p_hub->last_hub_error;
    }

    /*
     * 对应硬件功能：
     * - 固定选择当前已调通的 ADS1115#1 = 0x48
     *
     * 依赖资料：
     * - ads1115.h 中地址宏
     *
     * 前提假设：
     * - 本轮仍只接入第一块 ADS1115 的 4 路 MQ
     */
    err = ADS1115_SetAddress(ADS1115_I2C_ADDR_GND);
    if (FSP_SUCCESS != err)
    {
        p_hub->last_mq_init_error = (int32_t) err;
        p_hub->last_hub_error = SENSOR_HUB_ERR_MQ_SET_ADDRESS;
        return p_hub->last_hub_error;
    }

    /*
     * 对应硬件功能：
     * - 固定保持当前已验证通过的 PGA = 6.144V
     *
     * 依赖资料：
     * - ads1115.h 中 ads1115_pga_t
     *
     * 前提假设：
     * - 与你当前 MQ 调试版一致
     */
    err = ADS1115_SetGain(ADS1115_PGA_6_144V);
    if (FSP_SUCCESS != err)
    {
        p_hub->last_mq_init_error = (int32_t) err;
        p_hub->last_hub_error = SENSOR_HUB_ERR_MQ_SET_GAIN;
        return p_hub->last_hub_error;
    }

    /*
     * 对应硬件功能：
     * - 固定保持当前已验证通过的 DR = 128SPS
     *
     * 依赖资料：
     * - ads1115.h 中 ads1115_data_rate_t
     *
     * 前提假设：
     * - 与你当前 MQ 调试版一致
     */
    err = ADS1115_SetDataRate(ADS1115_DR_128SPS);
    if (FSP_SUCCESS != err)
    {
        p_hub->last_mq_init_error = (int32_t) err;
        p_hub->last_hub_error = SENSOR_HUB_ERR_MQ_SET_DR;
        return p_hub->last_hub_error;
    }

    /*
     * 对应硬件功能：
     * - 初始化 SHT40 + SGP40 + VOC 联合服务
     *
     * 依赖资料：
     * - SHT40SGP40_ServiceInit()
     *
     * 前提假设：
     * - 继续复用你当前已经跑通的联合服务层
     */
    env_err = SHT40SGP40_ServiceInit(&p_hub->env_service);
    p_hub->last_env_init_error = env_err;
    if (SHT40_SGP40_SERVICE_OK != env_err)
    {
        p_hub->last_hub_error = SENSOR_HUB_ERR_ENV_INIT;
        return p_hub->last_hub_error;
    }

    p_hub->sequence = 0U;
    p_hub->initialized = true;
    p_hub->last_hub_error = SENSOR_HUB_OK;
    return SENSOR_HUB_OK;
}

int32_t SensorHub_SampleOnce(sensor_hub_t * p_hub, sensor_hub_record_t * p_record)
{
    fsp_err_t acquire_err;
    fsp_err_t process_err;
    int32_t env_err;

    if ((NULL == p_hub) || (NULL == p_record))
    {
        return SENSOR_HUB_ERR_PARAM;
    }

    if (!p_hub->initialized)
    {
        return SENSOR_HUB_ERR_NOT_INIT;
    }

    SensorHub_ClearRecord(p_record);

    p_hub->sequence++;
    p_record->sequence = p_hub->sequence;

    /*
     * 对应硬件功能：
     * - 读取一帧 MQ 数据，并交给状态机处理
     *
     * 依赖资料：
     * - MQAcquire_ReadFrame()
     * - MQStateMachine_ProcessFrame()
     *
     * 前提假设：
     * - 即使本轮采样失败，也仍然把结果打包进 record
     */
    acquire_err = MQAcquire_ReadFrame(&p_hub->mq_frame);
    process_err = MQStateMachine_ProcessFrame(&p_hub->mq_sm,
                                              &p_hub->mq_frame,
                                              &p_hub->mq_metrics,
                                              &p_hub->mq_metrics_cfg);

    /*
     * 对应硬件功能：
     * - 先读 SHT40，再执行带补偿的 SGP40 VOC 采样
     *
     * 依赖资料：
     * - SHT40SGP40_ServiceSample()
     *
     * 前提假设：
     * - 外层维持 1 秒调用周期
     */
    env_err = SHT40SGP40_ServiceSample(&p_hub->env_service);

    sensor_hub_snapshot_mq(p_hub, p_record, acquire_err, process_err);
    sensor_hub_snapshot_env(p_hub, p_record, env_err);
    sensor_hub_snapshot_i2c(p_record);

    p_hub->last_hub_error = SENSOR_HUB_OK;
    return SENSOR_HUB_OK;
}
