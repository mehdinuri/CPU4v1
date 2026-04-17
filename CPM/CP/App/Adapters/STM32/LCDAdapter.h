/* App/Adapters/STM32/LCDAdapter.h
 *
 * IDisplayPort concrete implementation for the 20x4 graphical LCD
 * (122x32 pixels, dual SED1520 controller, 8-bit parallel bus).
 * Wraps lcdDrv.c functions; GPIO is driven through the macros in
 * lcdDrv.h which call gpio.c.
 */
#ifndef LCD_ADAPTER_H
#define LCD_ADAPTER_H

#include "Ports/IDisplayPort.h"

typedef struct
{
  uint8_t initialised;
} LCDAdapterCtx_t;

/* Initialise the adapter and bring up the LCD hardware. */
void LCDAdapterInit(LCDAdapterCtx_t *ctx);

/* Build an IDisplayPort_t wired to ctx. */
IDisplayPort_t LCDAdapterCreatePort(LCDAdapterCtx_t *ctx);

#endif /* LCD_ADAPTER_H */
