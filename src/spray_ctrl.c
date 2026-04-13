#include "spray_ctrl.h"

#define ATOMIZER_CTRL_PIN          BSP_IO_PORT_06_PIN_12
#define SPRAY_CTRL_SPRAY_MS        (3500U)
#define SPRAY_CTRL_RECOVERY_MS     (240000U)

static spray_ctrl_state_t g_spray_ctrl_state          = SPRAY_CTRL_STATE_IDLE;
static uint32_t           g_spray_ctrl_state_enter_ms = 0U;

/* --- 新增：发送一个 100ms 的脉冲，模拟按键按下并松开 --- */
static void Atomizer_SendPulse(void)
{
    // 1. 拉高引脚（相当于按下开关）
    R_IOPORT_PinWrite(&g_ioport_ctrl, ATOMIZER_CTRL_PIN, BSP_IO_LEVEL_HIGH);

    // 2. 维持 100 毫秒（给硬件芯片足够的反应时间去识别脉冲）
    R_BSP_SoftwareDelay(100, BSP_DELAY_UNITS_MILLISECONDS);

    // 3. 拉低引脚（相当于松开开关）
    R_IOPORT_PinWrite(&g_ioport_ctrl, ATOMIZER_CTRL_PIN, BSP_IO_LEVEL_LOW);
}

static fsp_err_t spray_ctrl_enter_idle(uint32_t now_ms)
{
    fsp_err_t err;

    err = Atomizer_Off();
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = SensorUart_EnableHumanMode();
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    g_spray_ctrl_state = SPRAY_CTRL_STATE_IDLE;
    g_spray_ctrl_state_enter_ms = now_ms;
    return FSP_SUCCESS;
}

static fsp_err_t spray_ctrl_enter_spray(uint32_t now_ms)
{
    fsp_err_t err;

    err = SensorUart_EnableCsvMode();
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = Atomizer_On();
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    g_spray_ctrl_state = SPRAY_CTRL_STATE_SPRAY;
    g_spray_ctrl_state_enter_ms = now_ms;
    return FSP_SUCCESS;
}

static fsp_err_t spray_ctrl_enter_recovery(uint32_t now_ms)
{
    fsp_err_t err;

    err = Atomizer_Off();
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    g_spray_ctrl_state = SPRAY_CTRL_STATE_RECOVERY;
    g_spray_ctrl_state_enter_ms = now_ms;
    return FSP_SUCCESS;
}

/* --- 修改：使用脉冲触发 --- */
fsp_err_t Atomizer_On(void)
{
    Atomizer_SendPulse();
    return FSP_SUCCESS;
}

/* --- 修改：使用脉冲触发 --- */
fsp_err_t Atomizer_Off(void)
{
    Atomizer_SendPulse();
    return FSP_SUCCESS;
}

fsp_err_t SprayCtrl_Init(void)
{
    return spray_ctrl_enter_idle(0U);
}

fsp_err_t SprayCtrl_Update(uint32_t now_ms, app_button_event_t button_event)
{
    switch (g_spray_ctrl_state)
    {
        case SPRAY_CTRL_STATE_IDLE:
        {
            if (APP_BUTTON_EVENT_PRESSED == button_event)
            {
                return spray_ctrl_enter_spray(now_ms);
            }
            break;
        }

        case SPRAY_CTRL_STATE_SPRAY:
        {
            if ((uint32_t) (now_ms - g_spray_ctrl_state_enter_ms) >= SPRAY_CTRL_SPRAY_MS)
            {
                return spray_ctrl_enter_recovery(now_ms);
            }
            break;
        }

        case SPRAY_CTRL_STATE_RECOVERY:
        {
            if ((uint32_t) (now_ms - g_spray_ctrl_state_enter_ms) >= SPRAY_CTRL_RECOVERY_MS)
            {
                return spray_ctrl_enter_spray(now_ms);
            }
            break;
        }

        default:
        {
            return spray_ctrl_enter_idle(now_ms);
        }
    }

    return FSP_SUCCESS;
}

spray_ctrl_state_t SprayCtrl_GetState(void)
{
    return g_spray_ctrl_state;
}

bool SprayCtrl_IsIdle(void)
{
    return (SPRAY_CTRL_STATE_IDLE == g_spray_ctrl_state);
}
