/*
 * sgp40_voc_simple.c
 *
 *  Created on: 2026年3月15日
 *      Author: leo
 */

#include <string.h>
#include "sgp40_voc_simple.h"
#include "sensirion_i2c.h"

#define SGP40_VOC_SIMPLE_BLACKOUT_SECONDS    (45U)

static void sgp40_voc_simple_clear(sgp40_voc_simple_t * p_ctx)
{
    if (NULL != p_ctx)
    {
        memset(p_ctx, 0, sizeof(sgp40_voc_simple_t));
    }
}

int32_t SGP40VOC_SimpleInit(sgp40_voc_simple_t * p_ctx)
{
    int16_t ret;

    if (NULL == p_ctx)
    {
        return SGP40_VOC_SIMPLE_ERR_PARAM;
    }

    sgp40_voc_simple_clear(p_ctx);

    /*
     * 对应硬件功能：
     * - 初始化 Sensirion 平台 I2C，并验证 SGP40 是否在线
     *
     * 依赖资料：
     * - sensirion_i2c_init()
     * - sgp40_probe()
     * - sgp40_get_serial_id()
     *
     * 前提假设：
     * - 当前工程 IIC2 B组 / g_i2c_master0 / i2c2_callback 已由 FSP 配好
     */
    sensirion_i2c_init();

    ret = sgp40_probe();
    if (0 != ret)
    {
        return (int32_t) ret;
    }

    ret = sgp40_get_serial_id(p_ctx->serial_id);
    if (0 != ret)
    {
        return (int32_t) ret;
    }

    /*
     * 对应硬件功能：
     * - 初始化 VOC 算法状态机
     *
     * 依赖资料：
     * - VocAlgorithm_init()
     *
     * 前提假设：
     * - 采样周期后续严格保持 1 秒
     */
    VocAlgorithm_init(&p_ctx->voc_params);

    p_ctx->initialized = true;
    p_ctx->sample_count = 0U;
    p_ctx->sraw = 0U;
    p_ctx->voc_index = 0;

    return SGP40_VOC_SIMPLE_OK;
}

int32_t SGP40VOC_SimpleSample(sgp40_voc_simple_t * p_ctx)
{
    int16_t ret;

    if (NULL == p_ctx)
    {
        return SGP40_VOC_SIMPLE_ERR_PARAM;
    }

    if (!p_ctx->initialized)
    {
        return SGP40_VOC_SIMPLE_ERR_NOT_INIT;
    }

    /*
     * 对应硬件功能：
     * - 使用 SGP40 默认补偿参数执行一次阻塞测量
     *
     * 依赖资料：
     * - sgp40_measure_raw_blocking_read()
     *
     * 前提假设：
     * - 当前先不接 SHT40，因此使用默认 RH/T 补偿常量
     */
    ret = sgp40_measure_raw_blocking_read(&p_ctx->sraw);
    if (0 != ret)
    {
        return (int32_t) ret;
    }

    /*
     * 对应硬件功能：
     * - 把本次 SGP40 原始值送入 VOC 算法
     *
     * 依赖资料：
     * - VocAlgorithm_process()
     *
     * 前提假设：
     * - 外部调用周期为 1 秒
     */
    VocAlgorithm_process(&p_ctx->voc_params, (int32_t) p_ctx->sraw, &p_ctx->voc_index);
    p_ctx->sample_count++;

    return SGP40_VOC_SIMPLE_OK;
}

int32_t SGP40VOC_SimpleSampleWithRht(sgp40_voc_simple_t * p_ctx,
                                     int32_t humidity_milli_rh,
                                     int32_t temperature_milli_c)
{
    int16_t ret;

    if (NULL == p_ctx)
    {
        return SGP40_VOC_SIMPLE_ERR_PARAM;
    }

    if (!p_ctx->initialized)
    {
        return SGP40_VOC_SIMPLE_ERR_NOT_INIT;
    }

    /*
     * 对应硬件功能：
     * - 后续接入 SHT40 后，使用真实温湿度补偿执行一次阻塞测量
     *
     * 依赖资料：
     * - sgp40_measure_raw_with_rht_blocking_read()
     *
     * 前提假设：
     * - humidity_milli_rh / temperature_milli_c 的量纲与 Sensirion API 一致
     */
    ret = sgp40_measure_raw_with_rht_blocking_read(humidity_milli_rh,
                                                   temperature_milli_c,
                                                   &p_ctx->sraw);
    if (0 != ret)
    {
        return (int32_t) ret;
    }

    VocAlgorithm_process(&p_ctx->voc_params, (int32_t) p_ctx->sraw, &p_ctx->voc_index);
    p_ctx->sample_count++;

    return SGP40_VOC_SIMPLE_OK;
}

bool SGP40VOC_SimpleInBlackout(sgp40_voc_simple_t const * p_ctx)
{
    if (NULL == p_ctx)
    {
        return true;
    }

    return (p_ctx->sample_count <= SGP40_VOC_SIMPLE_BLACKOUT_SECONDS);
}
