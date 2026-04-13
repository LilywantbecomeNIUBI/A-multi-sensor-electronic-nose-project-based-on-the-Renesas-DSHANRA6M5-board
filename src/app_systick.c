#include "app_systick.h"
#include <stdbool.h>

static volatile uint32_t g_app_systick_ms = 0U;
static bool g_app_systick_inited = false;

fsp_err_t AppSystick_Init(void)
{
    uint32_t sysclk_hz;

    if (g_app_systick_inited)
    {
        return FSP_SUCCESS;
    }

    sysclk_hz = R_BSP_SourceClockHzGet(FSP_PRIV_CLOCK_PLL);
    if (0U == sysclk_hz)
    {
        return FSP_ERR_ASSERTION;
    }

    if (0UL != SysTick_Config(sysclk_hz / 1000U))
    {
        return FSP_ERR_ASSERTION;
    }

    g_app_systick_ms = 0U;
    g_app_systick_inited = true;
    return FSP_SUCCESS;
}

uint32_t AppSystick_GetMs(void)
{
    return g_app_systick_ms;
}

void SysTick_Handler(void)
{
    g_app_systick_ms++;
}
