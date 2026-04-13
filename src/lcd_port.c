#include "lcd_port.h"
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>

/* 声明外部 AI 工程自带的打印接口，直接借用它的通道 */
extern int AppDebug_WriteString(const char * str);


/*
 * lcd_port.c
 *
 * Hardware function:
 * - Provides the low-level LCD bus/control layer for the current ST7789 screen scheme.
 * - SPI data transfer uses FSP Common SPI: g_spi0.
 * - LCD control signals are driven by GPIO:
 *     CS  -> P208
 *     DC  -> P210
 *     RES -> P205
 *     BLK -> P602
 *
 * Dependency:
 * - Depends on FSP-generated g_spi0 / g_ioport objects from hal_data.h.
 * - Depends on current pin mux configuration:
 *     P202 = SPI0_MISO
 *     P203 = SPI0_MOSI
 *     P204 = SPI0_RSPCK
 *
 * Assumption:
 * - The actual board/flying-wire connection has been changed to this new scheme.
 */

static volatile bool        g_lcd_spi_tx_done  = false;
static volatile spi_event_t g_lcd_spi_last_evt = SPI_EVENT_TRANSFER_ABORTED;
static bool                 g_lcd_port_opened  = false;

static fsp_err_t lcd_port_pin_write(bsp_io_port_pin_t pin, bsp_io_level_t level)
{
    return g_ioport.p_api->pinWrite(&g_ioport_ctrl, pin, level);
}

static fsp_err_t lcd_port_write_bytes(const uint8_t * p_data, uint32_t length)
{
    fsp_err_t err;
    uint32_t  offset = 0U;

    if ((NULL == p_data) || (0U == length))
    {
        return FSP_ERR_INVALID_ARGUMENT;
    }

    if (!g_lcd_port_opened)
    {
        return FSP_ERR_NOT_OPEN;
    }

    while (offset < length)
    {
        uint32_t chunk = length - offset;
        if (chunk > LCD_PORT_MAX_XFER_ONCE)
        {
            chunk = LCD_PORT_MAX_XFER_ONCE;
        }

        LCD_PortClearFlags();
        err = g_spi0.p_api->write(g_spi0.p_ctrl, &p_data[offset], chunk, SPI_BIT_WIDTH_8_BITS);
        if (FSP_SUCCESS != err)
        {
            return err;
        }

        if (!LCD_PortWaitTxComplete(LCD_PORT_SPI_TIMEOUT_MS))
        {
            return FSP_ERR_TIMEOUT;
        }

        offset += chunk;
    }

    return FSP_SUCCESS;
}

/* 补回全局的 uart_printf 函数 */
void uart_printf(const char *format, ...)
{
    char buffer[128];
    va_list args;

    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (len > 0)
    {
        /* 底层直接调用 AI 框架的打印功能发送出去 */
        AppDebug_WriteString(buffer);
    }
}

/* SPI0 callback used by the LCD port layer to wait for each write transaction to finish. */
void spi_callback(spi_callback_args_t * p_args)
{
    if (NULL == p_args)
    {
        return;
    }

    if (0U != p_args->channel)
    {
        return;
    }

    g_lcd_spi_last_evt = p_args->event;

    if ((SPI_EVENT_TRANSFER_COMPLETE == p_args->event) ||
        (SPI_EVENT_TRANSFER_ABORTED  == p_args->event) ||
        (SPI_EVENT_ERR_MODE_FAULT    == p_args->event) ||
        (SPI_EVENT_ERR_READ_OVERFLOW == p_args->event) ||
        (SPI_EVENT_ERR_PARITY        == p_args->event) ||
        (SPI_EVENT_ERR_OVERRUN       == p_args->event) ||
        (SPI_EVENT_ERR_FRAMING       == p_args->event) ||
        (SPI_EVENT_ERR_MODE_UNDERRUN == p_args->event))
    {
        g_lcd_spi_tx_done = true;
    }
}

fsp_err_t LCD_PortOpen(void)
{
    fsp_err_t err;

    if (g_lcd_port_opened)
    {
        return FSP_SUCCESS;
    }

    LCD_PortClearFlags();
    err = g_spi0.p_api->open(g_spi0.p_ctrl, g_spi0.p_cfg);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    g_lcd_port_opened = true;

    /* Power-on default after SPI open:
     * CS  inactive
     * DC  data mode
     * RES released
     */
    err = LCD_PortSetCS(false);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = LCD_PortSetDC(true);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = LCD_PortSetRES(false);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    return FSP_SUCCESS;
}

fsp_err_t LCD_PortClose(void)
{
    fsp_err_t err;

    if (!g_lcd_port_opened)
    {
        return FSP_SUCCESS;
    }

    err = g_spi0.p_api->close(g_spi0.p_ctrl);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    g_lcd_port_opened = false;
    LCD_PortClearFlags();

    return FSP_SUCCESS;
}

void LCD_PortClearFlags(void)
{
    g_lcd_spi_tx_done  = false;
    g_lcd_spi_last_evt = SPI_EVENT_TRANSFER_ABORTED;
}

bool LCD_PortWaitTxComplete(uint32_t timeout_ms)
{
    while ((false == g_lcd_spi_tx_done) && (timeout_ms > 0U))
    {
        R_BSP_SoftwareDelay(1U, BSP_DELAY_UNITS_MILLISECONDS);
        timeout_ms--;
    }

    if (false == g_lcd_spi_tx_done)
    {
        return false;
    }

    return (SPI_EVENT_TRANSFER_COMPLETE == g_lcd_spi_last_evt);
}

bool LCD_PortIsOpen(void)
{
    return g_lcd_port_opened;
}

/* selected = true  -> CS low  -> LCD selected
 * selected = false -> CS high -> LCD not selected
 */
fsp_err_t LCD_PortSetCS(bool selected)
{
    return lcd_port_pin_write(LCD_PORT_PIN_CS, selected ? BSP_IO_LEVEL_LOW : BSP_IO_LEVEL_HIGH);
}

/* data_mode = true  -> DC high -> data phase
 * data_mode = false -> DC low  -> command phase
 */
fsp_err_t LCD_PortSetDC(bool data_mode)
{
    return lcd_port_pin_write(LCD_PORT_PIN_DC, data_mode ? BSP_IO_LEVEL_HIGH : BSP_IO_LEVEL_LOW);
}

/* reset_active = true  -> RES low  -> hardware reset active
 * reset_active = false -> RES high -> normal run
 */
fsp_err_t LCD_PortSetRES(bool reset_active)
{
    return lcd_port_pin_write(LCD_PORT_PIN_RES, reset_active ? BSP_IO_LEVEL_LOW : BSP_IO_LEVEL_HIGH);
}

/* on = true  -> BLK high -> backlight on
 * on = false -> BLK low  -> backlight off
 */
fsp_err_t LCD_PortSetBLK(bool on)
{
    return lcd_port_pin_write(LCD_PORT_PIN_BLK, on ? BSP_IO_LEVEL_HIGH : BSP_IO_LEVEL_LOW);
}

fsp_err_t LCD_PortHardwareReset(void)
{
    fsp_err_t err;

    err = LCD_PortSetRES(true);
    if (FSP_SUCCESS != err)
    {
        return err;
    }
    R_BSP_SoftwareDelay(LCD_PORT_RESET_LOW_DELAY_MS, BSP_DELAY_UNITS_MILLISECONDS);

    err = LCD_PortSetRES(false);
    if (FSP_SUCCESS != err)
    {
        return err;
    }
    R_BSP_SoftwareDelay(LCD_PORT_RESET_HIGH_DELAY_MS, BSP_DELAY_UNITS_MILLISECONDS);

    R_BSP_SoftwareDelay(LCD_PORT_POST_RESET_DELAY_MS, BSP_DELAY_UNITS_MILLISECONDS);

    return FSP_SUCCESS;
}

fsp_err_t LCD_PortWriteCommand(uint8_t cmd)
{
    fsp_err_t err;

    /* Command transfer sequence:
     * 1. Select LCD
     * 2. DC = command
     * 3. Send 1-byte command
     * 4. Restore DC/data state and release CS
     */
    err = LCD_PortSetCS(true);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = LCD_PortSetDC(false);
    if (FSP_SUCCESS != err)
    {
        (void) LCD_PortSetCS(false);
        return err;
    }

    err = lcd_port_write_bytes(&cmd, 1U);

    (void) LCD_PortSetDC(true);
    (void) LCD_PortSetCS(false);

    return err;
}

fsp_err_t LCD_PortWriteData8(uint8_t data)
{
    return LCD_PortWriteData(&data, 1U);
}

fsp_err_t LCD_PortWriteData16(uint16_t data)
{
    uint8_t bytes[2];

    bytes[0] = (uint8_t) (data >> 8);
    bytes[1] = (uint8_t) (data & 0xFFU);

    return LCD_PortWriteData(bytes, 2U);
}

fsp_err_t LCD_PortWriteData(const uint8_t * p_data, uint32_t length)
{
    fsp_err_t err;

    /* Data transfer sequence:
     * 1. Select LCD
     * 2. DC = data
     * 3. Send data bytes through SPI0
     * 4. Release CS
     */
    err = LCD_PortSetCS(true);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = LCD_PortSetDC(true);
    if (FSP_SUCCESS != err)
    {
        (void) LCD_PortSetCS(false);
        return err;
    }

    err = lcd_port_write_bytes(p_data, length);

    (void) LCD_PortSetCS(false);

    return err;
}
