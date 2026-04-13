/*
 * sensor_hub.h
 *
 *  Created on: 2026年3月16日
 *      Author: leo
 */

#ifndef SENSOR_HUB_H_
#define SENSOR_HUB_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "ads1115.h"
#include "mq_acquire.h"
#include "mq_metrics.h"
#include "mq_state_machine.h"
#include "sht40_sgp40_service.h"
#include "i2c_master.h"

/*
 * 这一开关的目的：
 * - 不强行改变你当前已经跑通的 I2C 打开语义
 *
 * 默认值 0 的含义：
 * - SensorHub_Init() 不额外显式调用 I2CMaster_Open()
 * - 继续沿用你当前 ADS1115 / SHT40 / SGP40 各自已验证过的打开路径
 *
 * 若你当前联合服务版本仍要求“上层先 Open I2C”，可改成 1
 */
#ifndef SENSOR_HUB_ENABLE_EXPLICIT_I2C_OPEN
#define SENSOR_HUB_ENABLE_EXPLICIT_I2C_OPEN    (0U)
#endif

#define SENSOR_HUB_OK                       (0)
#define SENSOR_HUB_ERR_PARAM                (-3000)
#define SENSOR_HUB_ERR_NOT_INIT             (-3001)
#define SENSOR_HUB_ERR_I2C_OPEN             (-3002)
#define SENSOR_HUB_ERR_MQ_SM_INIT           (-3003)
#define SENSOR_HUB_ERR_MQ_OPEN              (-3004)
#define SENSOR_HUB_ERR_MQ_SET_ADDRESS       (-3005)
#define SENSOR_HUB_ERR_MQ_SET_GAIN          (-3006)
#define SENSOR_HUB_ERR_MQ_SET_DR            (-3007)
#define SENSOR_HUB_ERR_ENV_INIT             (-3008)

/*
 * 单轮 MQ 快照：
 * - 只保留统一输出需要的字段
 * - 不把整块状态机稳定窗口数组搬进 record，避免记录体过大
 */
typedef struct st_sensor_hub_mq_snapshot
{
    bool frame_complete;
    bool metrics_valid;
    bool baseline_ready;
    bool responding;
    bool recovered;

    int32_t acquire_err;
    int32_t process_err;
    int32_t last_metrics_err;

    mq_state_machine_state_t   state;
    mq_state_machine_quality_t quality;
    mq_state_machine_event_t   event;
    uint32_t                   flags;

    uint32_t warmup_elapsed_frames;
    uint32_t stable_window_count;
    uint32_t consecutive_suspect_count;
    uint32_t consecutive_error_count;
    uint32_t update_count;

    int16_t raw[ADS1115_CHANNEL_MAX];
    int32_t mv[ADS1115_CHANNEL_MAX];

    int32_t baseline_mv[ADS1115_CHANNEL_MAX];
    int32_t delta_mv[ADS1115_CHANNEL_MAX];
    int32_t ratio_permille[ADS1115_CHANNEL_MAX];
    int32_t peak_delta_mv[ADS1115_CHANNEL_MAX];
} sensor_hub_mq_snapshot_t;

/*
 * 单轮环境链路快照：
 * - 温湿度、补偿后的 SGP40 原始值、VOC 指数、blackout、最近阶段和错误码
 */
typedef struct st_sensor_hub_env_snapshot
{
    bool valid;

    int32_t service_err;
    sht40_sgp40_service_stage_t last_stage;

    float    temperature_c;
    float    humidity_rh;
    int32_t  temperature_milli_c;
    int32_t  humidity_milli_rh;
    uint16_t sraw;
    int32_t  voc_index;
    uint32_t sample_count;
    bool     blackout;
} sensor_hub_env_snapshot_t;

/*
 * 统一单轮记录：
 * - 供 sensor_uart 直接打印
 * - 也适合作为以后 CSV / 上位机导出的源数据结构
 */
typedef struct st_sensor_hub_record
{
    uint32_t sequence;

    /*
     * 对应硬件功能：
     * - 记录当前这一帧采样时，P612 雾化片驱动是否处于喷雾导通态
     *
     * 依赖资料：
     * - spray_ctrl.h/.c 中的状态机定义
     * - P612 仅在 SPRAY 态输出 HIGH
     *
     * 前提假设：
     * - 由 hal_entry.c 在调用 SensorUart_PrintRecord() 前填充
     * - 仅 SPRAY_CTRL_STATE_SPRAY 记为 true
     */
    bool atomizer_on;

    sensor_hub_mq_snapshot_t  mq;
    sensor_hub_env_snapshot_t env;

    uint32_t i2c_callback_count;
    uint32_t i2c_timeout_count;
    uint32_t i2c_recover_count;
    bool     i2c_aborted;
    app_i2c_event_t i2c_last_event;
} sensor_hub_record_t;

/*
 * 运行态对象：
 * - 保存 MQ 状态机与联合环境服务对象
 * - 这部分由 sensor_hub.c 管理，不由 sensor_uart 直接修改
 */
typedef struct st_sensor_hub
{
    mq_frame_t                mq_frame;
    mq_metrics_t              mq_metrics;
    mq_metrics_config_t       mq_metrics_cfg;
    mq_state_machine_t        mq_sm;
    mq_state_machine_config_t mq_sm_cfg;

    sht40_sgp40_service_t     env_service;

    uint32_t sequence;

    int32_t last_hub_error;
    int32_t last_mq_init_error;
    int32_t last_env_init_error;

    bool initialized;
} sensor_hub_t;

void SensorHub_Clear(sensor_hub_t * p_hub);
void SensorHub_ClearRecord(sensor_hub_record_t * p_record);

/*
 * 对应硬件功能：
 * - 初始化 MQ 采样链 + MQ 状态机 + SHT40/SGP40 联合服务
 *
 * 依赖资料：
 * - ADS1115_Open/SetAddress/SetGain/SetDataRate
 * - MQStateMachine_Init
 * - SHT40SGP40_ServiceInit
 *
 * 前提假设：
 * - 当前 MQ 仍使用 ADS1115#1 = 0x48
 * - 当前仍保持 PGA = 6.144V、DR = 128SPS
 */
int32_t SensorHub_Init(sensor_hub_t * p_hub);

/*
 * 对应硬件功能：
 * - 完成一次统一采样
 * - 无论某一路是否失败，都尽量生成完整 record
 *
 * 依赖资料：
 * - MQAcquire_ReadFrame()
 * - MQStateMachine_ProcessFrame()
 * - SHT40SGP40_ServiceSample()
 *
 * 前提假设：
 * - 外层以 1 秒周期调用，满足 VOC 算法 1Hz 采样要求
 */
int32_t SensorHub_SampleOnce(sensor_hub_t * p_hub, sensor_hub_record_t * p_record);

#endif /* SENSOR_HUB_H_ */
