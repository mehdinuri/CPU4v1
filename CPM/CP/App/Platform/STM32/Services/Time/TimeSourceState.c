/* App/Platform/STM32/Services/Time/TimeSourceState.c */
#include "TimeSourceState.h"

static uint8_t s_timeSource = TIME_SOURCE_NONE;

void TimeSourceSet(uint8_t bNewTimeSource)
{
  s_timeSource = bNewTimeSource;
}

uint8_t TimeSourceGet(void)
{
  return s_timeSource;
}
