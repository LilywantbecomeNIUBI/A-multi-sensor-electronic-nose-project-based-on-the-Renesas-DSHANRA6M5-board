#ifndef LCD_PORT_H
#define LCD_PORT_H

#include <stdbool.h>
#include <stdint.h>
#include "hal_data.h"

/*
 * LCD port layer for the current DShanRA6M5 screen scheme.
 *
 * Current verified hardware mapping:
 *   SPI instance : g_spi0 (channel 0)
 *   SPI0_MISO    : P202 -> LCD SDO / DO / MISO
 *   SPI0_MOSI    : P203 -> LCD SDA / SDI / DIN / MOSI
 *   SPI0_RSPCK   : P204 -> LCD SCL / SCK / CLK
 *   LCD_RES      : P205 -> LCD RES / RST (GPIO, low active)
 *   LCD_CS       : P208 -> LCD CS        (GPIO, low active)
 *   LCD_DC       : P210 -> LCD DC / RS   (GPIO, low = command, high = data)
 *   LCD_BLK      : P602 -> LCD BLK       (GPIO, high = on)
 *
 * Notes:
 * 1. This file matches the current new wiring scheme and current FSP configuration.
 * 2. LCD slave select is controlled by GPIO(P208), so the SPI hardware SSL pin is not used.
 * 3. MISO(P202) is configured in FSP as part of SPI0, but the current port layer only writes
 *    to the LCD and does not use SPI read-back.
 *
 * Active levels confirmed by the current driver design:
 *   CS  : low enable
 *   RES : low reset
 *   BLK : low off, high on
 *   DC  : low command, high data
 */

#ifdef __cplusplus
extern "C" {
#endif

#define LCD_PORT_PIN_CS      BSP_IO_PORT_02_PIN_08
#define LCD_PORT_PIN_DC      BSP_IO_PORT_02_PIN_10
#define LCD_PORT_PIN_RES     BSP_IO_PORT_02_PIN_05
#define LCD_PORT_PIN_BLK     BSP_IO_PORT_06_PIN_02

#define LCD_PORT_SPI_TIMEOUT_MS      (1000U)
#define LCD_PORT_RESET_LOW_DELAY_MS  (1U)
#define LCD_PORT_RESET_HIGH_DELAY_MS (120U)
#define LCD_PORT_POST_RESET_DELAY_MS (5U)
#define LCD_PORT_MAX_XFER_ONCE       (65535U)

fsp_err_t LCD_PortOpen(void);
fsp_err_t LCD_PortClose(void);
void      LCD_PortClearFlags(void);
bool      LCD_PortWaitTxComplete(uint32_t timeout_ms);
bool      LCD_PortIsOpen(void);

fsp_err_t LCD_PortSetCS(bool selected);
fsp_err_t LCD_PortSetDC(bool data_mode);
fsp_err_t LCD_PortSetRES(bool reset_active);
fsp_err_t LCD_PortSetBLK(bool on);

fsp_err_t LCD_PortHardwareReset(void);

fsp_err_t LCD_PortWriteCommand(uint8_t cmd);
fsp_err_t LCD_PortWriteData8(uint8_t data);
fsp_err_t LCD_PortWriteData16(uint16_t data);
fsp_err_t LCD_PortWriteData(const uint8_t * p_data, uint32_t length);

#ifdef __cplusplus
}
#endif

#endif /* LCD_PORT_H */
