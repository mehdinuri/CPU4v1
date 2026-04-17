/* App/Adapters/STM32/LCDAdapter.c
 *
 * IDisplayPort concrete implementation wrapping lcdDrv.c.
 *
 * The 20x4 character display is driven as a 122x32 pixel graphical
 * LCD.  Write() maps row/column character coordinates to page/pixel-
 * column addresses using the 6-pixel-wide font from lcdDrv.c.
 *
 * Row 0-3 map to pages 0-3.  Column n maps to pixel column n*6.
 */
#include "LCDAdapter.h"
#include "lcdDrv.h"

/* Each character is LCD_CHAR_WIDTH (6) pixels wide. */
#define CHAR_PIXEL_WIDTH LCD_CHAR_WIDTH

/* ------------------------------------------------------------------
 * Static helpers
 * ------------------------------------------------------------------ */
static void AdapterClear(void *ctx)
{
  (void) ctx;
  ClearLine(0U);
  ClearLine(1U);
  ClearLine(2U);
  ClearLine(3U);
}

static void AdapterWrite(void *ctx, uint8_t row, uint8_t col,
                         const char *text, uint8_t len)
{
  uint8_t i;
  uint8_t pixelCol;

  (void) ctx;

  if ((text == NULL) || (row > (uint8_t) LCD_LAST_PAGE_NO))
  {
    return;
  }

  pixelCol = (uint8_t) (col * CHAR_PIXEL_WIDTH);

  for (i = 0U; i < len; i++)
  {
    LCDWriteChar(row, &pixelCol, text[i]);
  }
}

static void AdapterPowerOn(void *ctx)
{
  (void) ctx;
  OpenLCD();
}

static void AdapterPowerOff(void *ctx)
{
  (void) ctx;
  LCDClose();
}

/* ------------------------------------------------------------------
 * Public API
 * ------------------------------------------------------------------ */
void LCDAdapterInit(LCDAdapterCtx_t *ctx)
{
  ctx->initialised = 0U;
  HardwareSetupLCD();
  ctx->initialised = 1U;
}

IDisplayPort_t LCDAdapterCreatePort(LCDAdapterCtx_t *ctx)
{
  IDisplayPort_t port;

  port.ctx = ctx;
  port.Clear = AdapterClear;
  port.Write = AdapterWrite;
  port.PowerOn = AdapterPowerOn;
  port.PowerOff = AdapterPowerOff;

  return port;
}
