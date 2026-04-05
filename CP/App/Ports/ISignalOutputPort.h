#pragma once

/*
 * App/Ports/ISignalOutputPort.h
 *
 * Drives SSM (Signal State Module) signal heads via CAN.
 * Implementations batch lamp-state changes and flush to CAN at end of tick.
 */
#include <stdint.h>
#include <stdbool.h>

/* Physical lamp colors. SIGNAL_COLOR_OFF turns the lamp off entirely. */
typedef enum
{
  SIGNAL_COLOR_OFF    = 0,
  SIGNAL_COLOR_RED    = 1,
  SIGNAL_COLOR_YELLOW = 2,
  SIGNAL_COLOR_GREEN  = 3,
  SIGNAL_COLOR_FLASH  = 4,    /* Flashing yellow (hardware-timed on SSM) */
} SignalColor_t;

typedef struct ISignalOutputPort
{
  void *ctx;

  /* Set a single lamp output (0-based outputId) to a color.
   * Calls are batched until flush(). */
  void (*setLampState)(void *ctx, uint8_t outputId, SignalColor_t color);

  /* Commit all batched lamp states to the hardware (send CAN frames). */
  void (*flush)(void *ctx);

  /* True if the SSM has acknowledged the last flush. */
  bool (*isReady)(void *ctx);
} ISignalOutputPort_t;

/* Inline dispatch helpers — zero overhead at -O2. */
static inline void SignalOutput_SetLamp(ISignalOutputPort_t *p,
                                        uint8_t id,
                                        SignalColor_t c)
{
  p->setLampState(p->ctx, id, c);
}

static inline void SignalOutput_Flush(ISignalOutputPort_t *p)
{
  p->flush(p->ctx);
}

static inline bool SignalOutput_IsReady(ISignalOutputPort_t *p)
{
  return p->isReady(p->ctx);
}
