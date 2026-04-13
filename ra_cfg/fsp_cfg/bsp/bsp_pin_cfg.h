/* generated configuration header file - do not edit */
#ifndef BSP_PIN_CFG_H_
#define BSP_PIN_CFG_H_
#include "r_ioport.h"

/* Common macro for FSP header files. There is also a corresponding FSP_FOOTER macro at the end of this file. */
FSP_HEADER

#define LCD_RES (BSP_IO_PORT_02_PIN_05)
#define LCD_CS (BSP_IO_PORT_02_PIN_08)
#define LCD_DC (BSP_IO_PORT_02_PIN_10)
#define LCD_BLK (BSP_IO_PORT_06_PIN_08)

extern const ioport_cfg_t g_bsp_pin_cfg; /* R7FA6M5BF2CBG.pincfg */

void BSP_PinConfigSecurityInit();

/* Common macro for FSP header files. There is also a corresponding FSP_HEADER macro at the top of this file. */
FSP_FOOTER
#endif /* BSP_PIN_CFG_H_ */
