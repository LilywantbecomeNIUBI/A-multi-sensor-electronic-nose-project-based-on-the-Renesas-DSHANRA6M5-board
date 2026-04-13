/*
 * sht40_sgp40_service.c
 *
 *  Created on: 2026年3月16日
 *      Author: OpenAI
 */

#include <string.h>
#include "sht40_sgp40_service.h"

static void sht40_sgp40_service_clear(sht40_sgp40_service_t * p_service)
{
    if (NULL != p_service)
    {
        memset(p_service, 0, sizeof(sht40_sgp40_service_t));
        p_service->last_stage = SHT40_SGP40_STAGE_NONE;
        p_service->last_error = SHT40_SGP40_SERVICE_OK;
    }
}

/*
 * 对应硬件功能：
 * - 把 SHT40 驱动输出的 float 单位（℃/%RH）转换为 SGP40 补偿接口要求的 milli 单位
 *
 * 依赖资料：
 * - sht40.h：temperature/humidity 当前缓存为 float
 * - sgp40.h：sgp40_measure_raw_with_rht_blocking_read() 要求 milli °C / milli %RH
 *
 * 前提假设：
 * - 当前工程允许使用浮点到整型的就地换算，不额外引入数学库
 */
static int32_t sht40_sgp40_float_to_milli(float value)
{
    if (value >= 0.0f)
    {
        return (int32_t) ((value * 1000.0f) + 0.5f);
    }

    return (int32_t) ((value * 1000.0f) - 0.5f);
}

int32_t SHT40SGP40_ServiceInit(sht40_sgp40_service_t * p_service)
{
    int32_t ret;

    if (NULL == p_service)
    {
        return SHT40_SGP40_SERVICE_ERR_PARAM;
    }

    sht40_sgp40_service_clear(p_service);

    /*
     * 对应硬件功能：
     * - 获取当前工程中已经跑通的 SHT40 设备对象
     *
     * 依赖资料：
     * - SHT40GetDevice()
     *
     * 前提假设：
     * - 仍沿用教材中的“设备对象 + 函数指针”封装风格
     */
    p_service->last_stage = SHT40_SGP40_STAGE_INIT_GET_DEVICE;
    p_service->p_sht40 = SHT40GetDevice();
    if ((NULL == p_service->p_sht40) ||
        (NULL == p_service->p_sht40->Init) ||
        (NULL == p_service->p_sht40->Read))
    {
        p_service->last_error = SHT40_SGP40_SERVICE_ERR_NO_DEV;
        return p_service->last_error;
    }

    /*
     * 对应硬件功能：
     * - 复用当前已验证通过的 SHT40 soft reset 初始化流程
     *
     * 依赖资料：
     * - sht40.c 中的 SHT40DrvInit()，通过 p_sht40->Init() 暴露
     *
     * 前提假设：
     * - I2C 平台层已经可用
     */
    p_service->last_stage = SHT40_SGP40_STAGE_INIT_SHT40;
    ret = (int32_t) p_service->p_sht40->Init(p_service->p_sht40);
    if (SHT40_SGP40_SERVICE_OK != ret)
    {
        p_service->last_error = ret;
        return ret;
    }

    /*
     * 对应硬件功能：
     * - 初始化 SGP40 在线检测、序列号读取以及 VOC 算法状态机
     *
     * 依赖资料：
     * - SGP40VOC_SimpleInit()
     *
     * 前提假设：
     * - sensirion_i2c_ra6m5.c 已实现地址切换与延时适配
     */
    p_service->last_stage = SHT40_SGP40_STAGE_INIT_SGP40_VOC;
    ret = SGP40VOC_SimpleInit(&p_service->sgp40_voc);
    if (SHT40_SGP40_SERVICE_OK != ret)
    {
        p_service->last_error = ret;
        return ret;
    }

    p_service->initialized = true;
    p_service->last_stage = SHT40_SGP40_STAGE_NONE;
    p_service->last_error = SHT40_SGP40_SERVICE_OK;

    return SHT40_SGP40_SERVICE_OK;
}

int32_t SHT40SGP40_ServiceSample(sht40_sgp40_service_t * p_service)
{
    int32_t ret;
    float temperature_c;
    float humidity_rh;
    int32_t temperature_milli_c;
    int32_t humidity_milli_rh;

    if (NULL == p_service)
    {
        return SHT40_SGP40_SERVICE_ERR_PARAM;
    }

    if (!p_service->initialized)
    {
        p_service->last_stage = SHT40_SGP40_STAGE_NONE;
        p_service->last_error = SHT40_SGP40_SERVICE_ERR_NOT_INIT;
        return p_service->last_error;
    }

    /*
     * 对应硬件功能：
     * - 先执行一次 SHT40 温湿度测量，拿到真实环境补偿值
     *
     * 依赖资料：
     * - SHT40DrvRead()，通过 p_sht40->Read() 暴露
     *
     * 前提假设：
     * - 当前采用阻塞式 1 次命令 + 延时 + 1 次读的最小调试方案
     */
    p_service->last_stage = SHT40_SGP40_STAGE_SAMPLE_SHT40;
    ret = (int32_t) p_service->p_sht40->Read(p_service->p_sht40);
    if (SHT40_SGP40_SERVICE_OK != ret)
    {
        p_service->last_error = ret;
        return ret;
    }

    temperature_c = p_service->p_sht40->temperature;
    humidity_rh   = p_service->p_sht40->humidity;

    /*
     * 对应硬件功能：
     * - 将 SHT40 的 float 单位转换成 SGP40 补偿 API 所需的 milli 单位
     *
     * 依赖资料：
     * - sgp40.h 中温湿度补偿接口原型与单位说明
     *
     * 前提假设：
     * - 当前 SHT40 驱动缓存值单位分别是 ℃ 和 %RH
     */
    temperature_milli_c = sht40_sgp40_float_to_milli(temperature_c);
    humidity_milli_rh   = sht40_sgp40_float_to_milli(humidity_rh);

    /*
     * 对应硬件功能：
     * - 用 SHT40 的真实温湿度执行一次 SGP40 补偿测量，并把原始值送入 VOC 算法
     *
     * 依赖资料：
     * - SGP40VOC_SimpleSampleWithRht()
     *
     * 前提假设：
     * - 外层以固定 1 秒周期调用本函数
     */
    p_service->last_stage = SHT40_SGP40_STAGE_SAMPLE_SGP40_VOC;
    ret = SGP40VOC_SimpleSampleWithRht(&p_service->sgp40_voc,
                                       humidity_milli_rh,
                                       temperature_milli_c);
    if (SHT40_SGP40_SERVICE_OK != ret)
    {
        p_service->last_error = ret;
        return ret;
    }

    p_service->latest.temperature_c       = temperature_c;
    p_service->latest.humidity_rh         = humidity_rh;
    p_service->latest.temperature_milli_c = temperature_milli_c;
    p_service->latest.humidity_milli_rh   = humidity_milli_rh;
    p_service->latest.sraw                = p_service->sgp40_voc.sraw;
    p_service->latest.voc_index           = p_service->sgp40_voc.voc_index;
    p_service->latest.sample_count        = p_service->sgp40_voc.sample_count;
    p_service->latest.blackout            = SGP40VOC_SimpleInBlackout(&p_service->sgp40_voc);

    p_service->last_stage = SHT40_SGP40_STAGE_NONE;
    p_service->last_error = SHT40_SGP40_SERVICE_OK;

    return SHT40_SGP40_SERVICE_OK;
}

const sht40_sgp40_service_data_t * SHT40SGP40_ServiceGetLatest(sht40_sgp40_service_t const * p_service)
{
    if (NULL == p_service)
    {
        return NULL;
    }

    return &p_service->latest;
}

sht40_sgp40_service_stage_t SHT40SGP40_ServiceGetLastStage(sht40_sgp40_service_t const * p_service)
{
    if (NULL == p_service)
    {
        return SHT40_SGP40_STAGE_NONE;
    }

    return p_service->last_stage;
}

int32_t SHT40SGP40_ServiceGetLastError(sht40_sgp40_service_t const * p_service)
{
    if (NULL == p_service)
    {
        return SHT40_SGP40_SERVICE_ERR_PARAM;
    }

    return p_service->last_error;
}
