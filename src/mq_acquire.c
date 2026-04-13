/*
 * mq_acquire.c
 *
 *  Created on: 2026年3月12日
 *      Author: leo
 */
/*  模块定位：
 *  - MQ 采集组织层（第一批）
 *
 *  当前只负责：
 *  - 调用 ADS1115_ReadAllChannels() 获取 4 路 raw
 *  - 调用 ADS1115_RawToMilliVolts() 生成 4 路 mV
 *  - 组织成一帧 mq_frame_t
 *  - 给出本轮是否采样完整
 *
 *  当前不负责：
 *  - ADS1115 初始化/配置
 *  - 采样阶段管理
 *  - 数据质量判断
 *  - 任何算法特征提取
 */

#include "mq_acquire.h"

/*
 * 对应功能：
 * - 根据一帧 raw 数据，填充对应的 mV 数据
 *
 * 依赖资料：
 * - ads1115.h/.c 中 ADS1115_RawToMilliVolts()
 *
 * 前提假设：
 * - p_frame 非 NULL
 * - p_frame->raw[] 已经填好
 */
static void MQAcquire_FillMilliVolts(mq_frame_t * p_frame)
{
    uint32_t ch;

    for (ch = 0U; ch < (uint32_t) ADS1115_CHANNEL_MAX; ch++)
    {
        p_frame->mv[ch] = ADS1115_RawToMilliVolts(p_frame->raw[ch]);
    }
}

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
void MQAcquire_ClearFrame(mq_frame_t * p_frame)
{
    uint32_t ch;

    if (NULL == p_frame)
    {
        return;
    }

    p_frame->complete = false;

    for (ch = 0U; ch < (uint32_t) ADS1115_CHANNEL_MAX; ch++)
    {
        p_frame->raw[ch] = 0;
        p_frame->mv[ch]  = 0;
    }
}

/*
 * 对应功能：
 * - 读取一帧 4 路 MQ 采样结果
 *
 * 依赖资料：
 * - ads1115.h/.c 中：
 *   ADS1115_ReadAllChannels()
 *   ADS1115_RawToMilliVolts()
 *
 * 前提假设：
 * - ADS1115 已由上层完成 Open/地址/Gain/DataRate 配置
 * - 当前扫描顺序固定按 CH0~CH3 对应 AIN0~AIN3
 */
fsp_err_t MQAcquire_ReadFrame(mq_frame_t * p_frame)
{
    ads1115_scan_result_t scan_result;
    fsp_err_t err;
    uint32_t ch;

    if (NULL == p_frame)
    {
        return FSP_ERR_ASSERTION;
    }

    MQAcquire_ClearFrame(p_frame);

    err = ADS1115_ReadAllChannels(&scan_result);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    for (ch = 0U; ch < (uint32_t) ADS1115_CHANNEL_MAX; ch++)
    {
        p_frame->raw[ch] = scan_result.raw[ch];
    }

    MQAcquire_FillMilliVolts(p_frame);

    p_frame->complete = true;

    return FSP_SUCCESS;
}

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
bool MQAcquire_IsFrameComplete(mq_frame_t const * p_frame)
{
    if (NULL == p_frame)
    {
        return false;
    }

    return p_frame->complete;
}

