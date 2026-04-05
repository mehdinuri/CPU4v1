#pragma once

/*
 * App/Ports/ISystemClockPort.h
 *
 * Abstract wall-clock access. Concrete implementations may use the STM32
 * RTC, a GPS-disciplined clock, or — for tests — an integer counter that
 * the test can advance manually.
 */
#include <stdint.h>
#include <stdbool.h>

typedef struct ISystemClockPort
{
  void *ctx;

  /* Current UTC epoch (seconds since 1970-01-01 00:00:00 UTC). */
  uint32_t (*getEpoch)(void *ctx);

  /* Set the current UTC epoch. Returns true if the set succeeded. */
  bool (*setEpoch)(void *ctx, uint32_t epoch);

  /* DST offset currently applied (signed seconds, e.g. +3600 for UTC+1). */
  int32_t (*getDstOffsetSeconds)(void *ctx);
} ISystemClockPort_t;

static inline uint32_t SystemClock_GetEpoch(ISystemClockPort_t *p)
{
  return p->getEpoch(p->ctx);
}

static inline bool SystemClock_SetEpoch(ISystemClockPort_t *p, uint32_t epoch)
{
  return p->setEpoch(p->ctx, epoch);
}

static inline int32_t SystemClock_GetDstOffset(ISystemClockPort_t *p)
{
  return p->getDstOffsetSeconds(p->ctx);
}
