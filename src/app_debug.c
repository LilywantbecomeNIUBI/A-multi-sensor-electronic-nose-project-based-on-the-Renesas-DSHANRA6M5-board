/*
 * app_debug.c
 *
 *  Created on: 2026年3月16日
 *      Author: leo
 */

#include "app_debug.h"

static volatile int g_app_uart7_tx_complete = 0;
static bool g_app_uart7_opened = false;

/*
 * 对应硬件功能：
 * - SCI7 发送完成中断回调
 *
 * 依赖资料：
 * - hal_data.h 中 uart7_callback 声明
 * - UART_EVENT_TX_COMPLETE
 *
 * 前提假设：
 * - 工程里只保留这一份 uart7_callback 定义
 */
void uart7_callback(uart_callback_args_t * p_args)
{
    if ((NULL != p_args) && (UART_EVENT_TX_COMPLETE == p_args->event))
    {
        g_app_uart7_tx_complete = 1;
    }
}

static void app_debug_uart_wait_for_tx(void)
{
    while (!g_app_uart7_tx_complete)
    {
        ;
    }

    g_app_uart7_tx_complete = 0;
}

fsp_err_t AppDebug_UartOpen(void)
{
    fsp_err_t err;

    if (g_app_uart7_opened)
    {
        return FSP_SUCCESS;
    }

    err = g_uart7.p_api->open(g_uart7.p_ctrl, g_uart7.p_cfg);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    g_app_uart7_opened = true;
    g_app_uart7_tx_complete = 0;
    return FSP_SUCCESS;
}

fsp_err_t AppDebug_WriteBytes(uint8_t const * p_data, uint32_t len)
{
    fsp_err_t err;

    if ((NULL == p_data) || (0U == len))
    {
        return FSP_ERR_ASSERTION;
    }

    err = g_uart7.p_api->write(g_uart7.p_ctrl, p_data, len);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    app_debug_uart_wait_for_tx();
    return FSP_SUCCESS;
}

fsp_err_t AppDebug_WriteString(char const * p_str)
{
    uint32_t len = 0U;

    if (NULL == p_str)
    {
        return FSP_ERR_ASSERTION;
    }

    while ('\0' != p_str[len])
    {
        len++;
    }

    return AppDebug_WriteBytes((uint8_t const *) p_str, len);
}

uint32_t AppDebug_BufAppendChar(char * p_buf, uint32_t buf_size, uint32_t pos, char ch)
{
    if ((NULL == p_buf) || (0U == buf_size))
    {
        return pos;
    }

    if ((pos + 1U) < buf_size)
    {
        p_buf[pos] = ch;
        pos++;
        p_buf[pos] = '\0';
    }

    return pos;
}

uint32_t AppDebug_BufAppendString(char * p_buf, uint32_t buf_size, uint32_t pos, char const * p_str)
{
    uint32_t i = 0U;

    if (NULL == p_str)
    {
        return pos;
    }

    while ('\0' != p_str[i])
    {
        pos = AppDebug_BufAppendChar(p_buf, buf_size, pos, p_str[i]);
        i++;
    }

    return pos;
}

uint32_t AppDebug_BufAppendUInt32(char * p_buf, uint32_t buf_size, uint32_t pos, uint32_t value)
{
    char temp[10];
    uint32_t digit_num = 0U;

    if (0U == value)
    {
        return AppDebug_BufAppendChar(p_buf, buf_size, pos, '0');
    }

    while ((value > 0U) && (digit_num < (uint32_t) sizeof(temp)))
    {
        temp[digit_num] = (char) ('0' + (value % 10U));
        value /= 10U;
        digit_num++;
    }

    while (digit_num > 0U)
    {
        digit_num--;
        pos = AppDebug_BufAppendChar(p_buf, buf_size, pos, temp[digit_num]);
    }

    return pos;
}

uint32_t AppDebug_BufAppendInt32(char * p_buf, uint32_t buf_size, uint32_t pos, int32_t value)
{
    int64_t signed_value = (int64_t) value;
    uint32_t abs_value;

    if (signed_value < 0)
    {
        pos = AppDebug_BufAppendChar(p_buf, buf_size, pos, '-');
        signed_value = -signed_value;
    }

    abs_value = (uint32_t) signed_value;
    return AppDebug_BufAppendUInt32(p_buf, buf_size, pos, abs_value);
}

uint32_t AppDebug_BufAppendFloat1(char * p_buf, uint32_t buf_size, uint32_t pos, float value)
{
    int32_t scaled;
    uint32_t integer_part;
    uint32_t frac_part;

    if (value >= 0.0f)
    {
        scaled = (int32_t) ((value * 10.0f) + 0.5f);
    }
    else
    {
        scaled = (int32_t) ((value * 10.0f) - 0.5f);
    }

    if (scaled < 0)
    {
        pos = AppDebug_BufAppendChar(p_buf, buf_size, pos, '-');
        scaled = -scaled;
    }

    integer_part = (uint32_t) (scaled / 10);
    frac_part    = (uint32_t) (scaled % 10);

    pos = AppDebug_BufAppendUInt32(p_buf, buf_size, pos, integer_part);
    pos = AppDebug_BufAppendChar(p_buf, buf_size, pos, '.');
    pos = AppDebug_BufAppendUInt32(p_buf, buf_size, pos, frac_part);

    return pos;
}

uint32_t AppDebug_BufAppendHex8(char * p_buf, uint32_t buf_size, uint32_t pos, uint8_t value)
{
    static char const hex_tab[] = "0123456789ABCDEF";

    pos = AppDebug_BufAppendChar(p_buf, buf_size, pos, hex_tab[(value >> 4) & 0x0F]);
    pos = AppDebug_BufAppendChar(p_buf, buf_size, pos, hex_tab[value & 0x0F]);

    return pos;
}

fsp_err_t AppDebug_PrintError(char const * prefix, int32_t err_code)
{
    char line[128];
    uint32_t pos = 0U;

    if (NULL == prefix)
    {
        return FSP_ERR_ASSERTION;
    }

    line[0] = '\0';
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, prefix);
    pos = AppDebug_BufAppendInt32(line, sizeof(line), pos, err_code);
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, "\r\n");
    FSP_PARAMETER_NOT_USED(pos);

    return AppDebug_WriteString(line);
}

fsp_err_t AppDebug_PrintI2CDiag(void)
{
    char line[160];
    uint32_t pos = 0U;

    line[0] = '\0';
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, "[I2C] cb=");
    pos = AppDebug_BufAppendUInt32(line, sizeof(line), pos, g_app_i2c_callback_count);
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, ", timeout=");
    pos = AppDebug_BufAppendUInt32(line, sizeof(line), pos, g_app_i2c_timeout_count);
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, ", recover=");
    pos = AppDebug_BufAppendUInt32(line, sizeof(line), pos, g_app_i2c_recover_count);
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, ", aborted=");
    pos = AppDebug_BufAppendUInt32(line, sizeof(line), pos, g_app_i2c_aborted ? 1U : 0U);
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, ", last_event=");
    pos = AppDebug_BufAppendUInt32(line, sizeof(line), pos, (uint32_t) g_app_i2c_last_event);
    pos = AppDebug_BufAppendString(line, sizeof(line), pos, "\r\n");
    FSP_PARAMETER_NOT_USED(pos);

    return AppDebug_WriteString(line);
}
