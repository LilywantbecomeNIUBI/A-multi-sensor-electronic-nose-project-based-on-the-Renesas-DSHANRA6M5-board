/**
 * @file lv_port_disp.c
 *
 */

/*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/
#if 1

/*********************
 * INCLUDES
 *********************/
#include "lv_port_disp.h"
#include <stdbool.h>
extern void uart_printf(const char *format, ...);
/* 引入你的底层驱动 */
#include "st7789.h"
#include "lcd_port.h"

/*********************
 * DEFINES
 *********************/
#ifndef MY_DISP_HOR_RES
    #warning Please define or replace the macro MY_DISP_HOR_RES with the actual screen width, default value 320 is used for now.
    #define MY_DISP_HOR_RES    240   /* 你的屏幕宽度 240 */
#endif

#ifndef MY_DISP_VER_RES
    #warning Please define or replace the macro MY_DISP_HOR_RES with the actual screen height, default value 240 is used for now.
    #define MY_DISP_VER_RES    280   /* 你的屏幕高度 280 */
#endif

/**********************
 * TYPEDEFS
 **********************/

/**********************
 * STATIC PROTOTYPES
 **********************/
static void disp_init(void);
static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);

/**********************
 * STATIC VARIABLES
 **********************/

/**********************
 * MACROS
 **********************/

/**********************
 * GLOBAL FUNCTIONS
 **********************/

void lv_port_disp_init(void)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    disp_init();

    /*-----------------------------
     * Create a buffer for drawing
     *----------------------------*/

    /* Example for 2) */
    static lv_disp_draw_buf_t draw_buf_dsc_2;
    static lv_color_t buf_2_1[MY_DISP_HOR_RES * 10];                        /*A buffer for 10 rows*/
    static lv_color_t buf_2_2[MY_DISP_HOR_RES * 10];                        /*An other buffer for 10 rows*/
    lv_disp_draw_buf_init(&draw_buf_dsc_2, buf_2_1, buf_2_2, MY_DISP_HOR_RES * 10);   /*Initialize the display buffer*/

    /*-----------------------------------
     * Register the display in LVGL
     *----------------------------------*/

    static lv_disp_drv_t disp_drv;                         /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);                    /*Basic initialization*/

    /*Set up the functions to access to your display*/

    /*Set the resolution of the display*/
    disp_drv.hor_res = MY_DISP_HOR_RES;
    disp_drv.ver_res = MY_DISP_VER_RES;

    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = disp_flush;

    /*Set a display buffer*/
    disp_drv.draw_buf = &draw_buf_dsc_2;

    /*Finally register the driver*/
    lv_disp_drv_register(&disp_drv);
}

/**********************
 * STATIC FUNCTIONS
 **********************/

/*Initialize your display and the required peripherals.*/
/* 记得在文件顶部*/
static void disp_init(void)
{
    st7789_cfg_t st_cfg = {
        .rotation = ST7789_ROTATION_0,
        .backlight_on_after_init = true,
    };

    fsp_err_t err = ST7789_Init(&st_cfg);
    if (FSP_SUCCESS != err)
    {
        uart_printf("[LVGL ERROR] ST7789 初始化失败! 错误码: %d\r\n", err);
        while(1); // 初始化失败直接卡死，不要让它跑到后面的死循环骗你
    }
    uart_printf("[LVGL] ST7789 屏幕初始化成功！\r\n");
}

volatile bool disp_flush_enabled = true;

/* Enable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_enable_update(void)
{
    disp_flush_enabled = true;
}

/* Disable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_disable_update(void)
{
    disp_flush_enabled = false;
}

/*Flush the content of the internal buffer the specific area on the display*/
static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    if(disp_flush_enabled) {
        /* 计算当前需要刷新的区域有多少个像素 */
        uint32_t width = area->x2 - area->x1 + 1;
        uint32_t height = area->y2 - area->y1 + 1;
        uint32_t size = width * height;

        /* 1. 设置 ST7789 的绘制窗口 */
        ST7789_SetAddressWindow(area->x1, area->y1, area->x2, area->y2);

        /* 2. 把 LVGL 准备好的颜色数据一块塞给 SPI 接口。
         * 因为是 16位色 (RGB565)，所以字节数是像素数 * 2
         */
        LCD_PortWriteData((const uint8_t *)color_p, size * 2);
    }

    /*IMPORTANT!!!
     *Inform the graphics library that you are ready with the flushing*/
    lv_disp_flush_ready(disp_drv);
}

#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
