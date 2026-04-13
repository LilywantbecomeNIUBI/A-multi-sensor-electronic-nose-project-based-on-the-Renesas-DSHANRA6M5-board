/*
 * sgp40_voc_simple.h
 *
 *  Created on: 2026年3月15日
 *      Author: leo
 */

#ifndef SGP40_VOC_SIMPLE_H_
#define SGP40_VOC_SIMPLE_H_

#include <stdint.h>
#include <stdbool.h>
#include "sgp40.h"
#include "sensirion_voc_algorithm.h"

#define SGP40_VOC_SIMPLE_OK            (0)
#define SGP40_VOC_SIMPLE_ERR_PARAM     (-1000)
#define SGP40_VOC_SIMPLE_ERR_NOT_INIT  (-1001)

typedef struct st_sgp40_voc_simple
{
    VocAlgorithmParams voc_params;
    uint8_t  serial_id[SGP40_SERIAL_ID_NUM_BYTES];
    uint16_t sraw;
    int32_t  voc_index;
    uint32_t sample_count;
    bool     initialized;
} sgp40_voc_simple_t;

int32_t SGP40VOC_SimpleInit(sgp40_voc_simple_t * p_ctx);
int32_t SGP40VOC_SimpleSample(sgp40_voc_simple_t * p_ctx);
int32_t SGP40VOC_SimpleSampleWithRht(sgp40_voc_simple_t * p_ctx,
                                     int32_t humidity_milli_rh,
                                     int32_t temperature_milli_c);
bool SGP40VOC_SimpleInBlackout(sgp40_voc_simple_t const * p_ctx);

#endif /* SGP40_VOC_SIMPLE_H_ */
