#include "st7789.h"
#include <stddef.h>

typedef struct st_st7789_runtime
{
    uint16_t           width;
    uint16_t           height;
    uint16_t           x_offset;
    uint16_t           y_offset;
    uint8_t            madctl;
    st7789_rotation_t  rotation;
    bool               initialized;
} st7789_runtime_t;

typedef struct st_st7789_rotation_cfg
{
    uint16_t width;
    uint16_t height;
    uint16_t x_offset;
    uint16_t y_offset;
    uint8_t  madctl;
} st7789_rotation_cfg_t;

#define ST7789_CMD_SLPOUT   (0x11U)
#define ST7789_CMD_NORON    (0x13U)
#define ST7789_CMD_INVOFF   (0x20U)
#define ST7789_CMD_INVON    (0x21U)
#define ST7789_CMD_DISPOFF  (0x28U)
#define ST7789_CMD_DISPON   (0x29U)
#define ST7789_CMD_CASET    (0x2AU)
#define ST7789_CMD_RASET    (0x2BU)
#define ST7789_CMD_RAMWR    (0x2CU)
#define ST7789_CMD_MADCTL   (0x36U)
#define ST7789_CMD_COLMOD   (0x3AU)

#define ST7789_COLMOD_RGB565       (0x05U)
#define ST7789_SLEEP_OUT_DELAY_MS  (120U)
#define ST7789_FILL_CHUNK_PIXELS   (64U)

/*
 * Rotation table source:
 * - MADCTL values and +20 offset rule come from the user-provided ST7789 sample
 *   lcd_init.c for this 240x280 module.
 * - Width/height come from the ZJY169S0800TG01 module document.
 */
static const st7789_rotation_cfg_t g_st7789_rotation_cfg[] =
{
    [ST7789_ROTATION_0]   = {240U, 280U,  0U, 20U, 0x00U},
    [ST7789_ROTATION_180] = {240U, 280U,  0U, 20U, 0xC0U},
    [ST7789_ROTATION_90]  = {280U, 240U, 20U,  0U, 0x70U},
    [ST7789_ROTATION_270] = {280U, 240U, 20U,  0U, 0xA0U},
};

static st7789_runtime_t g_st7789 =
{
    .width       = ST7789_LCD_WIDTH_PORTRAIT,
    .height      = ST7789_LCD_HEIGHT_PORTRAIT,
    .x_offset    = 0U,
    .y_offset    = 20U,
    .madctl      = 0x00U,
    .rotation    = ST7789_ROTATION_0,
    .initialized = false,
};

static fsp_err_t st7789_write_u8(uint8_t cmd, uint8_t data)
{
    fsp_err_t err;

    err = LCD_PortWriteCommand(cmd);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    return LCD_PortWriteData8(data);
}

static fsp_err_t st7789_write_u8_multi(uint8_t cmd, uint8_t const * p_data, uint32_t length)
{
    fsp_err_t err;

    err = LCD_PortWriteCommand(cmd);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    return LCD_PortWriteData(p_data, length);
}

static fsp_err_t st7789_write_color_repeat(uint16_t color, uint32_t pixel_count)
{
    uint8_t bytes[ST7789_FILL_CHUNK_PIXELS * 2U];
    uint32_t i;
    uint32_t remaining = pixel_count;
    fsp_err_t err;

    for (i = 0U; i < ST7789_FILL_CHUNK_PIXELS; i++)
    {
        bytes[(2U * i)]     = (uint8_t) (color >> 8);
        bytes[(2U * i) + 1] = (uint8_t) (color & 0xFFU);
    }

    while (remaining > 0U)
    {
        uint32_t chunk_pixels = remaining;
        if (chunk_pixels > ST7789_FILL_CHUNK_PIXELS)
        {
            chunk_pixels = ST7789_FILL_CHUNK_PIXELS;
        }

        err = LCD_PortWriteData(bytes, chunk_pixels * 2U);
        if (FSP_SUCCESS != err)
        {
            return err;
        }

        remaining -= chunk_pixels;
    }

    return FSP_SUCCESS;
}

fsp_err_t ST7789_SleepOut(void)
{
    fsp_err_t err = LCD_PortWriteCommand(ST7789_CMD_SLPOUT);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    R_BSP_SoftwareDelay(ST7789_SLEEP_OUT_DELAY_MS, BSP_DELAY_UNITS_MILLISECONDS);
    return FSP_SUCCESS;
}

fsp_err_t ST7789_DisplayOn(void)
{
    return LCD_PortWriteCommand(ST7789_CMD_DISPON);
}

fsp_err_t ST7789_DisplayOff(void)
{
    return LCD_PortWriteCommand(ST7789_CMD_DISPOFF);
}

fsp_err_t ST7789_SetRotation(st7789_rotation_t rotation)
{
    st7789_rotation_cfg_t const * p_rot;
    fsp_err_t err;

    if ((uint32_t) rotation >= (sizeof(g_st7789_rotation_cfg) / sizeof(g_st7789_rotation_cfg[0])))
    {
        return FSP_ERR_INVALID_ARGUMENT;
    }

    p_rot = &g_st7789_rotation_cfg[rotation];

    err = st7789_write_u8(ST7789_CMD_MADCTL, p_rot->madctl);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    g_st7789.width    = p_rot->width;
    g_st7789.height   = p_rot->height;
    g_st7789.x_offset = p_rot->x_offset;
    g_st7789.y_offset = p_rot->y_offset;
    g_st7789.madctl   = p_rot->madctl;
    g_st7789.rotation = rotation;

    return FSP_SUCCESS;
}

fsp_err_t ST7789_SetAddressWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    uint8_t buf[4];
    fsp_err_t err;
    uint16_t xs;
    uint16_t xe;
    uint16_t ys;
    uint16_t ye;

    if ((x1 > x2) || (y1 > y2))
    {
        return FSP_ERR_INVALID_ARGUMENT;
    }

    xs = (uint16_t) (x1 + g_st7789.x_offset);
    xe = (uint16_t) (x2 + g_st7789.x_offset);
    ys = (uint16_t) (y1 + g_st7789.y_offset);
    ye = (uint16_t) (y2 + g_st7789.y_offset);

    buf[0] = (uint8_t) (xs >> 8);
    buf[1] = (uint8_t) (xs & 0xFFU);
    buf[2] = (uint8_t) (xe >> 8);
    buf[3] = (uint8_t) (xe & 0xFFU);
    err = st7789_write_u8_multi(ST7789_CMD_CASET, buf, 4U);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    buf[0] = (uint8_t) (ys >> 8);
    buf[1] = (uint8_t) (ys & 0xFFU);
    buf[2] = (uint8_t) (ye >> 8);
    buf[3] = (uint8_t) (ye & 0xFFU);
    err = st7789_write_u8_multi(ST7789_CMD_RASET, buf, 4U);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    return LCD_PortWriteCommand(ST7789_CMD_RAMWR);
}

fsp_err_t ST7789_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    fsp_err_t err;

    if ((x >= g_st7789.width) || (y >= g_st7789.height))
    {
        return FSP_ERR_INVALID_ARGUMENT;
    }

    err = ST7789_SetAddressWindow(x, y, x, y);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    return LCD_PortWriteData16(color);
}

fsp_err_t ST7789_FillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color)
{
    uint32_t pixel_count;
    fsp_err_t err;

    if ((0U == width) || (0U == height))
    {
        return FSP_ERR_INVALID_ARGUMENT;
    }

    if ((x >= g_st7789.width) || (y >= g_st7789.height))
    {
        return FSP_ERR_INVALID_ARGUMENT;
    }

    if (((uint32_t) x + width) > g_st7789.width)
    {
        width = (uint16_t) (g_st7789.width - x);
    }

    if (((uint32_t) y + height) > g_st7789.height)
    {
        height = (uint16_t) (g_st7789.height - y);
    }

    err = ST7789_SetAddressWindow(x, y, (uint16_t) (x + width - 1U), (uint16_t) (y + height - 1U));
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    pixel_count = (uint32_t) width * (uint32_t) height;
    return st7789_write_color_repeat(color, pixel_count);
}

fsp_err_t ST7789_Clear(uint16_t color)
{
    return ST7789_FillRect(0U, 0U, g_st7789.width, g_st7789.height, color);
}

fsp_err_t ST7789_Init(st7789_cfg_t const * p_cfg)
{
    st7789_cfg_t local_cfg =
    {
        .rotation = ST7789_ROTATION_0,
        .backlight_on_after_init = true,
    };
    uint8_t data_b2[5] = {0x0CU, 0x0CU, 0x00U, 0x33U, 0x33U};
    uint8_t data_d0[2] = {0xA4U, 0xA1U};
    uint8_t data_e0[14] = {0xD0U, 0x08U, 0x0EU, 0x09U, 0x09U, 0x05U, 0x31U, 0x33U, 0x48U, 0x17U, 0x14U, 0x15U, 0x31U, 0x34U};
    uint8_t data_e1[14] = {0xD0U, 0x08U, 0x0EU, 0x09U, 0x09U, 0x15U, 0x31U, 0x33U, 0x48U, 0x17U, 0x14U, 0x15U, 0x31U, 0x34U};
    fsp_err_t err;

    if (NULL != p_cfg)
    {
        local_cfg = *p_cfg;
    }

    err = LCD_PortOpen();
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = LCD_PortHardwareReset();
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = ST7789_SleepOut();
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = ST7789_SetRotation(local_cfg.rotation);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = st7789_write_u8(ST7789_CMD_COLMOD, ST7789_COLMOD_RGB565);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = st7789_write_u8_multi(0xB2U, data_b2, sizeof(data_b2));
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = st7789_write_u8(0xB7U, 0x35U);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = st7789_write_u8(0xBBU, 0x32U);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = st7789_write_u8(0xC2U, 0x01U);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = st7789_write_u8(0xC3U, 0x15U);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = st7789_write_u8(0xC4U, 0x20U);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = st7789_write_u8(0xC6U, 0x0FU);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = st7789_write_u8_multi(0xD0U, data_d0, sizeof(data_d0));
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = st7789_write_u8_multi(0xE0U, data_e0, sizeof(data_e0));
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = st7789_write_u8_multi(0xE1U, data_e1, sizeof(data_e1));
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = LCD_PortWriteCommand(ST7789_CMD_INVON);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = LCD_PortWriteCommand(ST7789_CMD_NORON);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = ST7789_DisplayOn();
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    if (local_cfg.backlight_on_after_init)
    {
        err = LCD_PortSetBLK(true);
        if (FSP_SUCCESS != err)
        {
            return err;
        }
    }

    g_st7789.initialized = true;
    return FSP_SUCCESS;
}

uint16_t ST7789_GetWidth(void)
{
    return g_st7789.width;
}

uint16_t ST7789_GetHeight(void)
{
    return g_st7789.height;
}
