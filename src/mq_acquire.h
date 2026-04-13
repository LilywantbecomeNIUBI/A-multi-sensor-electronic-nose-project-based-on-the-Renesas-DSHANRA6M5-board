/*
 * mq_acquire.h
 *
 *  Created on: 2026年3月12日
 *      Author: leo
 */

/*  模块定位：
 *  - MQ 采集组织层（第一批）
 *
 *  当前只负责：
 *  - 调用 ADS1115 四通道扫描接口
 *  - 生成一帧 4 路采样结果
 *  - 保存 raw / mV
 *  - 返回本轮是否采样完整
 *
 *  当前不负责：
 *  - ADS1115 初始化/配置（Open/SetGain/SetDataRate）
 *  - 数据合理性判断
 *  - baseline / delta / ratio / peak / recovery
 *  - ppm 计算
 */

#ifndef MQ_ACQUIRE_H_
#define MQ_ACQUIRE_H_

#include <stdint.h>
#include <stdbool.h>
#include "ads1115.h"

typedef struct st_mq_frame
{
    bool    complete;                         /* 本轮是否采样完整 */
    int16_t raw[ADS1115_CHANNEL_MAX];         /* CH0~CH3 原始值 */
    int32_t mv[ADS1115_CHANNEL_MAX];          /* CH0~CH3 毫伏值 */
} mq_frame_t;

/*
 * 对应功能：
 * - 清空一帧采样结果
 *
 * 依赖资料：
 * - 无额外硬件资料；仅依赖本模块数据结构定义
 *
 * 前提假设：
 * - p_frame 指向有效对象
 */
void MQAcquire_ClearFrame(mq_frame_t * p_frame);

/*
 * 对应功能：
 * - 读取一帧 4 路 MQ 采样结果
 *
 * 依赖资料：
 * - ads1115.h/.c 已提供：
 *   ADS1115_ReadAllChannels()
 *   ADS1115_RawToMilliVolts()
 *
 * 前提假设：
 * - ADS1115 已由上层完成 Open/地址/Gain/DataRate 配置
 * - 当前扫描顺序固定按 CH0~CH3 对应 AIN0~AIN3
 */
fsp_err_t MQAcquire_ReadFrame(mq_frame_t * p_frame);

/*
 * 对应功能：
 * - 查询当前帧是否完整
 *
 * 依赖资料：
 * - 无额外硬件资料；仅依赖本模块 complete 标志
 *
 * 前提假设：
 * - p_frame 可以为 NULL；NULL 时返回 false
 */
bool MQAcquire_IsFrameComplete(mq_frame_t const * p_frame);

#endif /* MQ_ACQUIRE_H_ */
