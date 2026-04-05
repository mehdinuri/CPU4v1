#pragma once

/*
 * App/Ports/IDisplayPort.h
 *
 * LCD display output. The concrete adapter wraps lcdDrv.c (122×32 px
 * parallel LCD); the mock records draw calls for assertion in tests.
 */
#include <stdint.h>

typedef struct IDisplayPort
{
  void *ctx;

  /* Clear the entire display. */
  void (*clear)(void *ctx);

  /* Print a null-terminated string at (col, row).
   * col and row are character-grid coordinates (not pixels). */
  void (*printAt)(void *ctx, uint8_t col, uint8_t row, const char *text);

  /* Force all pending draw calls to be flushed to the display hardware. */
  void (*flush)(void *ctx);
} IDisplayPort_t;

static inline void Display_Clear(IDisplayPort_t *p)
{
  p->clear(p->ctx);
}

static inline void Display_PrintAt(IDisplayPort_t *p,
                                   uint8_t col,
                                   uint8_t row,
                                   const char *text)
{
  p->printAt(p->ctx, col, row, text);
}

static inline void Display_Flush(IDisplayPort_t *p)
{
  p->flush(p->ctx);
}
