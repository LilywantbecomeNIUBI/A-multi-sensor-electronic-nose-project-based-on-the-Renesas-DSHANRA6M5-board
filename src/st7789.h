#ifndef ST7789_H
#define ST7789_H

#include <stdbool.h>
#include <stdint.h>
#include "lcd_port.h"

/*
 * ST7789 driver interface for the current 240x280 LCD module.
 *
 * Driver layering:
 * - This driver sends ST7789 commands/data through lcd_port.c.
 * - lcd_port.c has already been adapted to the current new hardware scheme:
 *     SPI0: P202 / P203 / P204
 *     RES : P205
 *     CS  : P208
 *     DC  : P210
 *     BLK : P602
 *
 * Current display assumptions:
 * - Portrait  resolution : 240 x 280
 * - Landscape resolution : 280 x 240
 * - Pixel format         : RGB565
 */

#ifdef __cplusplus
extern "C" {
#endif

#define ST7789_LCD_WIDTH_PORTRAIT   (240U)
#define ST7789_LCD_HEIGHT_PORTRAIT  (280U)
#define ST7789_LCD_WIDTH_LANDSCAPE  (280U)
#define ST7789_LCD_HEIGHT_LANDSCAPE (240U)

/**
 * @brief 将一帧图像数据绘制到屏幕指定区域
 * @param x       区域左上角 X 坐标
 * @param y       区域左上角 Y 坐标
 * @param width   区域宽度（像素数）
 * @param height  区域高度（像素数）
 * @param pixels  像素数据指针，每个像素为 16 位 RGB565 格式，按行优先排列
 * @return FSP_SUCCESS 成功，否则错误码
 *
 * 说明：
 * - 本接口只负责设置窗口并发送像素缓冲区。
 * - 当前实现不在本函数内逐像素做字节交换，调用者提供的数据缓冲区
 *   需要与当前发送实现保持一致。
 */
fsp_err_t ST7789_DrawFrame(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint16_t *pixels);

/**
 * @brief 播放多帧动画
 * @param frames         帧数据首地址，按 frame0、frame1、frame2... 顺序连续存放
 * @param frame_count    总帧数
 * @param width          单帧宽度（应与屏幕当前方向匹配）
 * @param height         单帧高度
 * @param interval_ms    帧间隔（毫秒）
 * @param loop           是否循环播放（true=循环，false=播放一次）
 */
void ST7789_PlayFrames(const uint16_t *frames, uint32_t frame_count,
                       uint16_t width, uint16_t height, uint32_t interval_ms, bool loop);

typedef enum e_st7789_rotation
{
    ST7789_ROTATION_0   = 0,
    ST7789_ROTATION_180 = 1,
    ST7789_ROTATION_90  = 2,
    ST7789_ROTATION_270 = 3,
} st7789_rotation_t;

typedef struct st_st7789_cfg
{
    st7789_rotation_t rotation;
    bool              backlight_on_after_init;
} st7789_cfg_t;

/* RGB565 颜色宏 */
#define RGB565(r,g,b) ((((r)&0x1F)<<11) | (((g)&0x3F)<<5) | ((b)&0x1F))
#define COLOR_WHITE RGB565(31,63,31)
#define COLOR_BLACK RGB565(0,0,0)

fsp_err_t ST7789_Init(st7789_cfg_t const * p_cfg);
fsp_err_t ST7789_DisplayOn(void);
fsp_err_t ST7789_DisplayOff(void);
fsp_err_t ST7789_SleepOut(void);
fsp_err_t ST7789_SetRotation(st7789_rotation_t rotation);
fsp_err_t ST7789_SetAddressWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
fsp_err_t ST7789_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
fsp_err_t ST7789_FillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
fsp_err_t ST7789_Clear(uint16_t color);
uint16_t  ST7789_GetWidth(void);
uint16_t  ST7789_GetHeight(void);

#ifdef __cplusplus
}
#endif

#endif /* ST7789_H */
