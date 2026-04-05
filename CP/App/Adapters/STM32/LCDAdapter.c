/*
 * App/Adapters/STM32/LCDAdapter.c
 *
 * IDisplayPort implementation — parallel LCD (122×32 px, 4×15 char grid).
 * All hardware calls are TODO stubs; the shadow buffer is kept functional
 * so the UITask can run on host builds without modification.
 */
#include "LCDAdapter.h"
#include <string.h>

/* --------------------------------------------------------------------------
 * Forward declarations
 * --------------------------------------------------------------------------*/
static void LCD_Clear(void *ctx);
static void LCD_PrintAt(void *ctx, uint8_t col, uint8_t row, const char *text);
static void LCD_Flush(void *ctx);

/* --------------------------------------------------------------------------
 * Public API
 * --------------------------------------------------------------------------*/

void LCDAdapter_Init(LCDAdapterCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));

  /* Fill the shadow buffer with spaces so partial updates look clean. */
  for (uint8_t r = 0U; r < LCD_ROWS_MAX; r++)
  {
    memset(ctx->shadowBuf[r], ' ', LCD_COLS_MAX);
    ctx->shadowBuf[r][LCD_COLS_MAX] = '\0';
  }

  ctx->dirty = false;

  #ifdef STM32H743xx

  /* TODO: HAL impl — initialise GPIO pins for parallel LCD interface,
   * send initialisation command Sequence (KS0108 / compatible controller).
   * lcdDrv_Init();
   */
  #endif
}

IDisplayPort_t LCDAdapter_CreatePort(LCDAdapterCtx_t *ctx)
{
  IDisplayPort_t port;

  port.ctx = ctx;
  port.clear = LCD_Clear;
  port.printAt = LCD_PrintAt;
  port.flush = LCD_Flush;

  return port;
}

/* --------------------------------------------------------------------------
 * Port callbacks
 * --------------------------------------------------------------------------*/

static void LCD_Clear(void *vctx)
{
  LCDAdapterCtx_t *ctx = (LCDAdapterCtx_t *) vctx;

  for (uint8_t r = 0U; r < LCD_ROWS_MAX; r++)
  {
    memset(ctx->shadowBuf[r], ' ', LCD_COLS_MAX);
    ctx->shadowBuf[r][LCD_COLS_MAX] = '\0';
  }

  ctx->dirty = true;
}

static void LCD_PrintAt(void *vctx, uint8_t col, uint8_t row, const char *text)
{
  LCDAdapterCtx_t *ctx = (LCDAdapterCtx_t *) vctx;

  if ((row >= LCD_ROWS_MAX) || (col >= LCD_COLS_MAX) || (text == NULL) )
  {
    return;
  }

  uint8_t pos = col;

  while (*text != '\0' && pos < LCD_COLS_MAX)
  {
    ctx->shadowBuf[row][pos] = *text;
    pos++;
    text++;
  }

  ctx->dirty = true;
}

static void LCD_Flush(void *vctx)
{
  LCDAdapterCtx_t *ctx = (LCDAdapterCtx_t *) vctx;

  if (!ctx->dirty)
  {
    return;
  }

  #ifdef STM32H743xx

  /* TODO: HAL impl — transfer shadowBuf to LCD controller.
   * Iterate rows and columns, call lcdDrv_SetCursor(col, row) and
   * lcdDrv_WriteChar(ch) for each character.
   *
   * for (uint8_t r = 0; r < LCD_ROWS_MAX; r++) {
   *     lcdDrv_SetCursor(0, r);
   *     for (uint8_t c = 0; c < LCD_COLS_MAX; c++) {
   *         lcdDrv_WriteChar(ctx->shadowBuf[r][c]);
   *     }
   * }
   */
  #endif

  ctx->dirty = false;
}
