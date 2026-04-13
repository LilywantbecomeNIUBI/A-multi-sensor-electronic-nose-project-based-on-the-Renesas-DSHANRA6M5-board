/*
 * ads1115.h
 *
 *  Created on: 2026年3月11日
 *      Author: leo
 */

#ifndef ADS1115_H_
#define ADS1115_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "i2c_master.h"

/* -------------------- 设备地址 -------------------- */
#define ADS1115_I2C_ADDR_GND          (0x48U)
#define ADS1115_I2C_ADDR_VDD          (0x49U)
#define ADS1115_I2C_ADDR_SDA          (0x4AU)
#define ADS1115_I2C_ADDR_SCL          (0x4BU)

/* 当前项目默认地址：ADDR 接 GND */
#define ADS1115_I2C_ADDR_DEFAULT      (ADS1115_I2C_ADDR_GND)

/* -------------------- Pointer Register -------------------- */
#define ADS1115_REG_CONVERSION        (0x00U)
#define ADS1115_REG_CONFIG            (0x01U)
#define ADS1115_REG_LO_THRESH         (0x02U)
#define ADS1115_REG_HI_THRESH         (0x03U)

/* -------------------- Config Register Bits -------------------- */
/* OS[15] */
#define ADS1115_CONFIG_OS_MASK        (0x8000U)
#define ADS1115_CONFIG_OS_NO_EFFECT   (0x0000U)
#define ADS1115_CONFIG_OS_SINGLE      (0x8000U)

/* MUX[14:12] */
#define ADS1115_CONFIG_MUX_MASK       (0x7000U)
#define ADS1115_CONFIG_MUX_DIFF_0_1   (0x0000U)
#define ADS1115_CONFIG_MUX_DIFF_0_3   (0x1000U)
#define ADS1115_CONFIG_MUX_DIFF_1_3   (0x2000U)
#define ADS1115_CONFIG_MUX_DIFF_2_3   (0x3000U)
#define ADS1115_CONFIG_MUX_AIN0_GND   (0x4000U)
#define ADS1115_CONFIG_MUX_AIN1_GND   (0x5000U)
#define ADS1115_CONFIG_MUX_AIN2_GND   (0x6000U)
#define ADS1115_CONFIG_MUX_AIN3_GND   (0x7000U)

/* PGA[11:9] */
#define ADS1115_CONFIG_PGA_MASK       (0x0E00U)
#define ADS1115_CONFIG_PGA_6_144V     (0x0000U)
#define ADS1115_CONFIG_PGA_4_096V     (0x0200U)
#define ADS1115_CONFIG_PGA_2_048V     (0x0400U)
#define ADS1115_CONFIG_PGA_1_024V     (0x0600U)
#define ADS1115_CONFIG_PGA_0_512V     (0x0800U)
#define ADS1115_CONFIG_PGA_0_256V     (0x0A00U)

/* MODE[8] */
#define ADS1115_CONFIG_MODE_MASK      (0x0100U)
#define ADS1115_CONFIG_MODE_CONTINUOUS (0x0000U)
#define ADS1115_CONFIG_MODE_SINGLE    (0x0100U)

/* DR[7:5] */
#define ADS1115_CONFIG_DR_MASK        (0x00E0U)
#define ADS1115_CONFIG_DR_8SPS        (0x0000U)
#define ADS1115_CONFIG_DR_16SPS       (0x0020U)
#define ADS1115_CONFIG_DR_32SPS       (0x0040U)
#define ADS1115_CONFIG_DR_64SPS       (0x0060U)
#define ADS1115_CONFIG_DR_128SPS      (0x0080U)
#define ADS1115_CONFIG_DR_250SPS      (0x00A0U)
#define ADS1115_CONFIG_DR_475SPS      (0x00C0U)
#define ADS1115_CONFIG_DR_860SPS      (0x00E0U)

/* COMP_MODE[4] */
#define ADS1115_CONFIG_COMP_MODE_TRAD (0x0000U)
#define ADS1115_CONFIG_COMP_MODE_WINDOW (0x0010U)

/* COMP_POL[3] */
#define ADS1115_CONFIG_COMP_POL_LOW   (0x0000U)
#define ADS1115_CONFIG_COMP_POL_HIGH  (0x0008U)

/* COMP_LAT[2] */
#define ADS1115_CONFIG_COMP_LAT_NON   (0x0000U)
#define ADS1115_CONFIG_COMP_LAT_LATCH (0x0004U)

/* COMP_QUE[1:0] */
#define ADS1115_CONFIG_COMP_QUE_1     (0x0000U)
#define ADS1115_CONFIG_COMP_QUE_2     (0x0001U)
#define ADS1115_CONFIG_COMP_QUE_4     (0x0002U)
#define ADS1115_CONFIG_COMP_DISABLE   (0x0003U)

/* -------------------- 项目默认配置 -------------------- */
#define ADS1115_PROJECT_DEFAULT_PGA   ADS1115_CONFIG_PGA_6_144V
#define ADS1115_PROJECT_DEFAULT_DR    ADS1115_CONFIG_DR_128SPS

typedef enum e_ads1115_channel
{
    ADS1115_CHANNEL_AIN0 = 0,
    ADS1115_CHANNEL_AIN1,
    ADS1115_CHANNEL_AIN2,
    ADS1115_CHANNEL_AIN3,
    ADS1115_CHANNEL_MAX
} ads1115_channel_t;

typedef enum e_ads1115_pga
{
    ADS1115_PGA_6_144V = ADS1115_CONFIG_PGA_6_144V,
    ADS1115_PGA_4_096V = ADS1115_CONFIG_PGA_4_096V,
    ADS1115_PGA_2_048V = ADS1115_CONFIG_PGA_2_048V,
    ADS1115_PGA_1_024V = ADS1115_CONFIG_PGA_1_024V,
    ADS1115_PGA_0_512V = ADS1115_CONFIG_PGA_0_512V,
    ADS1115_PGA_0_256V = ADS1115_CONFIG_PGA_0_256V
} ads1115_pga_t;

typedef enum e_ads1115_data_rate
{
    ADS1115_DR_8SPS   = ADS1115_CONFIG_DR_8SPS,
    ADS1115_DR_16SPS  = ADS1115_CONFIG_DR_16SPS,
    ADS1115_DR_32SPS  = ADS1115_CONFIG_DR_32SPS,
    ADS1115_DR_64SPS  = ADS1115_CONFIG_DR_64SPS,
    ADS1115_DR_128SPS = ADS1115_CONFIG_DR_128SPS,
    ADS1115_DR_250SPS = ADS1115_CONFIG_DR_250SPS,
    ADS1115_DR_475SPS = ADS1115_CONFIG_DR_475SPS,
    ADS1115_DR_860SPS = ADS1115_CONFIG_DR_860SPS
} ads1115_data_rate_t;

typedef struct st_ads1115_scan_result
{
    int16_t raw[ADS1115_CHANNEL_MAX];
} ads1115_scan_result_t;

fsp_err_t ADS1115_Open(void);
fsp_err_t ADS1115_Close(void);

fsp_err_t ADS1115_SetAddress(uint8_t slave_addr);
uint8_t   ADS1115_GetAddress(void);

fsp_err_t ADS1115_SetGain(ads1115_pga_t pga);
ads1115_pga_t ADS1115_GetGain(void);

fsp_err_t ADS1115_SetDataRate(ads1115_data_rate_t dr);
ads1115_data_rate_t ADS1115_GetDataRate(void);

fsp_err_t ADS1115_ReadRegister(uint8_t reg, uint16_t * p_value);
fsp_err_t ADS1115_WriteRegister(uint8_t reg, uint16_t value);

fsp_err_t ADS1115_ReadSingleEnded(ads1115_channel_t channel, int16_t * p_raw);
fsp_err_t ADS1115_ReadSingleEndedMilliVolts(ads1115_channel_t channel, int32_t * p_mv);
fsp_err_t ADS1115_ReadAllChannels(ads1115_scan_result_t * p_result);

int32_t ADS1115_RawToMilliVolts(int16_t raw);
uint16_t ADS1115_GetFullScaleMilliVolts(void);

#endif /* ADS1115_H_ */
