/*
 * sht40.h
 *
 *  Created on: 2026年3月12日
 *      Author: leo
 */
#ifndef SHT40_H
#define SHT40_H

#include <stdint.h>
#include <stdbool.h>

/*
 * 硬件功能：SHT40 I2C 寻址与命令宏定义
 * 依赖手册：SHT4x Datasheet -> 4.1 I2C communication & 4.4 Command Overview
 */
#define SHT40_I2C_ADDR              0x44    // SHT40-AD1B 默认 7-bit 地址
#define SHT40_CMD_MEASURE_HIGH_RES  0xFD    // 触发高精度温湿度测量命令
#define SHT40_CMD_SOFT_RESET        0x94    // 软复位命令

/*
 * 硬件功能：SHT40 设备对象结构体
 * 依赖手册：参考教材面向对象的设备封装风格，屏蔽底层细节
 */
typedef struct SHT40Dev {
    char  *name;
    float temperature;   // 缓存的真实温度，单位：℃
    float humidity;      // 缓存的真实湿度，单位：%RH

    // 操作接口
    int (*Init)(struct SHT40Dev *ptdev);
    int (*Read)(struct SHT40Dev *ptdev);
} SHT40Device;

/* 暴露给外部应用层的设备获取接口 */
SHT40Device* SHT40GetDevice(void);

#endif /* SHT40_H */

