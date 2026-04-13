/*
 * sht40_sgp40_service.h
 *
 *  Created on: 2026年3月16日
 *      Author: OpenAI
 */

#ifndef SHT40_SGP40_SERVICE_H_
#define SHT40_SGP40_SERVICE_H_

#include <stdint.h>
#include <stdbool.h>
#include "sht40.h"
#include "sgp40_voc_simple.h"

#define SHT40_SGP40_SERVICE_OK            (0)
#define SHT40_SGP40_SERVICE_ERR_PARAM     (-2000)
#define SHT40_SGP40_SERVICE_ERR_NOT_INIT  (-2001)
#define SHT40_SGP40_SERVICE_ERR_NO_DEV    (-2002)

typedef enum e_sht40_sgp40_service_stage
{
    SHT40_SGP40_STAGE_NONE = 0,
    SHT40_SGP40_STAGE_INIT_GET_DEVICE,
    SHT40_SGP40_STAGE_INIT_SHT40,
    SHT40_SGP40_STAGE_INIT_SGP40_VOC,
    SHT40_SGP40_STAGE_SAMPLE_SHT40,
    SHT40_SGP40_STAGE_SAMPLE_SGP40_VOC
} sht40_sgp40_service_stage_t;

typedef struct st_sht40_sgp40_service_data
{
    float    temperature_c;
    float    humidity_rh;
    int32_t  temperature_milli_c;
    int32_t  humidity_milli_rh;
    uint16_t sraw;
    int32_t  voc_index;
    uint32_t sample_count;
    bool     blackout;
} sht40_sgp40_service_data_t;

typedef struct st_sht40_sgp40_service
{
    SHT40Device * p_sht40;
    sgp40_voc_simple_t sgp40_voc;
    sht40_sgp40_service_data_t latest;
    sht40_sgp40_service_stage_t last_stage;
    int32_t  last_error;
    bool     initialized;
} sht40_sgp40_service_t;

/*
 * 对应硬件功能：
 * - 初始化 SHT40 + SGP40 + VOC 联合服务层
 *
 * 依赖资料：
 * - sht40.h/.c 中的 SHT40GetDevice()/Init()
 * - sgp40_voc_simple.h/.c 中的 SGP40VOC_SimpleInit()
 *
 * 前提假设：
 * - I2C 平台层已由上层完成 Open，或允许 Sensirion 层按现有逻辑重复 Open
 */
int32_t SHT40SGP40_ServiceInit(sht40_sgp40_service_t * p_service);

/*
 * 对应硬件功能：
 * - 先读取 SHT40 温湿度，再执行一次带补偿的 SGP40 VOC 采样
 *
 * 依赖资料：
 * - SHT40Device->Read()
 * - SGP40VOC_SimpleSampleWithRht()
 *
 * 前提假设：
 * - 外部以 1 秒周期调用，满足 VOC 算法采样周期要求
 */
int32_t SHT40SGP40_ServiceSample(sht40_sgp40_service_t * p_service);

const sht40_sgp40_service_data_t * SHT40SGP40_ServiceGetLatest(sht40_sgp40_service_t const * p_service);
sht40_sgp40_service_stage_t SHT40SGP40_ServiceGetLastStage(sht40_sgp40_service_t const * p_service);
int32_t SHT40SGP40_ServiceGetLastError(sht40_sgp40_service_t const * p_service);

#endif /* SHT40_SGP40_SERVICE_H_ */
