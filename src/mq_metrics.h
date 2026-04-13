/*
 * mq_metrics.h
 *
 *  Created on: 2026年3月15日
 *      Author: leo
 */

/*
 * mq_metrics.h
 *
 *  模块定位：
 *  - MQ 指标整理层（第二批）
 *
 *  当前负责：
 *  - 基于 mq_frame_t 维护 baseline
 *  - 生成 delta / ratio / peak
 *  - 给出当前 metrics 是否有效
 *  - 给出当前是否处于响应中、是否接近恢复
 *
 *  当前不负责：
 *  - ADS1115 初始化/配置/读写
 *  - I2C 总线恢复
 *  - ppm 计算
 *  - SHT40/SGP40 补偿
 *  - 气体种类判定
 */

#ifndef MQ_METRICS_H_
#define MQ_METRICS_H_

#include <stdint.h>
#include <stdbool.h>
#include "mq_acquire.h"
#include "hal_data.h"

typedef struct st_mq_metrics_config
{
    uint16_t response_threshold_mv;   /* 判定“有响应”的最小 delta 阈值 */
    uint16_t recovery_band_mv;        /* 判定“已接近恢复”的基线回归带 */
} mq_metrics_config_t;

typedef struct st_mq_metrics
{
    bool     valid;                                   /* 当前 metrics 是否有效 */
    bool     baseline_ready;                          /* baseline 是否已建立 */
    bool     responding;                              /* 当前是否处于响应中 */
    bool     recovered;                               /* 当前是否已回到接近基线 */

    int32_t  baseline_mv[ADS1115_CHANNEL_MAX];        /* 基线 mV */
    int32_t  current_mv[ADS1115_CHANNEL_MAX];         /* 当前 mV */
    int32_t  delta_mv[ADS1115_CHANNEL_MAX];           /* 当前相对基线增量 */
    int32_t  peak_delta_mv[ADS1115_CHANNEL_MAX];      /* 历史最大增量 */
    int32_t  ratio_permille[ADS1115_CHANNEL_MAX];     /* 相对变化率，千分比 */

    uint32_t update_count;                            /* 已更新次数 */
} mq_metrics_t;

/*
 * 对应功能：
 * - 清空 metrics 结构体
 *
 * 依赖资料：
 * - 无额外硬件资料；仅依赖本模块数据结构定义
 *
 * 前提假设：
 * - p_metrics 指向有效对象
 */
void MQMetrics_Clear(mq_metrics_t * p_metrics);

/*
 * 对应功能：
 * - 获取默认参数
 *
 * 依赖资料：
 * - 无额外硬件资料；仅依赖本模块默认阈值设计
 *
 * 前提假设：
 * - p_cfg 指向有效对象
 */
void MQMetrics_GetDefaultConfig(mq_metrics_config_t * p_cfg);

/*
 * 对应功能：
 * - 用当前一帧建立 baseline
 *
 * 依赖资料：
 * - mq_acquire.h 中 mq_frame_t 定义
 *
 * 前提假设：
 * - p_frame 必须完整
 * - baseline 直接采用当前帧 mV
 */
fsp_err_t MQMetrics_SetBaselineFromFrame(mq_metrics_t * p_metrics,
                                         mq_frame_t const * p_frame);

/*
 * 对应功能：
 * - 用 4 路 baseline mV 数组建立 baseline
 *
 * 依赖资料：
 * - ADS1115_CHANNEL_MAX 通道数定义
 * - mq_metrics_t 中 baseline/current/delta/peak/ratio 字段
 *
 * 前提假设：
 * - p_baseline_mv 指向长度为 ADS1115_CHANNEL_MAX 的有效数组
 * - baseline 已由上层多帧稳定窗口计算完成
 */
fsp_err_t MQMetrics_SetBaselineFromMilliVolts(mq_metrics_t * p_metrics,
                                              int32_t const p_baseline_mv[ADS1115_CHANNEL_MAX]);

/*
 * 对应功能：
 * - 清除历史 peak 记录
 *
 * 依赖资料：
 * - 无额外硬件资料；仅依赖本模块数据结构定义
 *
 * 前提假设：
 * - baseline 保持不变
 */
void MQMetrics_ResetPeaks(mq_metrics_t * p_metrics);

/*
 * 对应功能：
 * - 基于当前一帧更新 metrics
 *
 * 依赖资料：
 * - mq_acquire.h 中 mq_frame_t 定义
 *
 * 前提假设：
 * - p_frame 必须完整
 * - 必须先建立 baseline
 */
fsp_err_t MQMetrics_Update(mq_metrics_t * p_metrics,
                           mq_frame_t const * p_frame,
                           mq_metrics_config_t const * p_cfg);

#endif /* MQ_METRICS_H_ */
