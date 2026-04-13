#include "app_button.h"
#include <stddef.h>

#define APP_BUTTON_PIN            BSP_IO_PORT_00_PIN_00
#define APP_BUTTON_ACTIVE_LEVEL   BSP_IO_LEVEL_LOW
#define APP_BUTTON_DEBOUNCE_MS    (20U)

static bsp_io_level_t g_app_button_last_raw_level = BSP_IO_LEVEL_HIGH;
static bsp_io_level_t g_app_button_stable_level   = BSP_IO_LEVEL_HIGH;
static uint32_t       g_app_button_last_change_ms = 0U;
static bool           g_app_button_inited         = false;

fsp_err_t AppButton_Init(void)
{
    fsp_err_t err;
    bsp_io_level_t level = BSP_IO_LEVEL_HIGH;

    err = R_IOPORT_PinRead(&g_ioport_ctrl, APP_BUTTON_PIN, &level);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    g_app_button_last_raw_level = level;
    g_app_button_stable_level   = level;
    g_app_button_last_change_ms = 0U;
    g_app_button_inited         = true;
    return FSP_SUCCESS;
}

fsp_err_t AppButton_Update(uint32_t now_ms, app_button_event_t * p_event)
{
    fsp_err_t err;
    bsp_io_level_t level = BSP_IO_LEVEL_HIGH;

    if (NULL == p_event)
    {
        return FSP_ERR_ASSERTION;
    }

    *p_event = APP_BUTTON_EVENT_NONE;

    if (!g_app_button_inited)
    {
        return FSP_ERR_NOT_OPEN;
    }

    err = R_IOPORT_PinRead(&g_ioport_ctrl, APP_BUTTON_PIN, &level);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    if (level != g_app_button_last_raw_level)
    {
        g_app_button_last_raw_level = level;
        g_app_button_last_change_ms = now_ms;
        return FSP_SUCCESS;
    }

    if (((uint32_t) (now_ms - g_app_button_last_change_ms) >= APP_BUTTON_DEBOUNCE_MS) &&
        (g_app_button_stable_level != level))
    {
        g_app_button_stable_level = level;
        if (APP_BUTTON_ACTIVE_LEVEL == g_app_button_stable_level)
        {
            *p_event = APP_BUTTON_EVENT_PRESSED;
        }
    }

    return FSP_SUCCESS;
}
