#pragma once

/*
 * App/Adapters/STM32/LCDAdapter.h
 *
 * IDisplayPort concrete implementation for STM32H743.
 * Wraps the 122×32 px parallel LCD driver (lcdDrv.c).
 * Rendering is double-buffered: draw calls write to a shadow buffer; flush()
 * transfers the buffer to the display hardware (TODO HAL stub).
 */
#include <stdbool.h>
#include "Ports/IDisplayPort.h"

/* Character grid dimensions (8×8 px font assumed). */
#define LCD_COLS_MAX  15U   /* 122 px / 8 px per char ≈ 15 columns */
#define LCD_ROWS_MAX   4U   /* 32 px / 8 px per char = 4 rows       */
#define LCD_BUF_SIZE  (LCD_COLS_MAX * LCD_ROWS_MAX)

typedef struct
{
  char shadowBuf[LCD_ROWS_MAX][LCD_COLS_MAX + 1U];    /* +1 for null terminator */
  bool dirty;     /* True if any draw call has been made since last flush() */
} LCDAdapterCtx_t;

/** Initialise the adapter context and (TODO) the LCD peripheral. */
void LCDAdapter_Init(LCDAdapterCtx_t *ctx);

/** Build an IDisplayPort_t wired to ctx. */
IDisplayPort_t LCDAdapter_CreatePort(LCDAdapterCtx_t *ctx);
