/* App/Platform/STM32/Services/Time/TimeSourceState.h */
#ifndef TIME_SOURCE_STATE_H
#define TIME_SOURCE_STATE_H

#include <stdint.h>

#define TIME_SOURCE_NONE 0U
#define TIME_SOURCE_NET 1U
#define TIME_SOURCE_RTC 2U
#define TIME_SOURCE_GPS 3U
#define TIME_SOURCE_CENTER 4U

void TimeSourceSet(uint8_t bNewTimeSource);
uint8_t TimeSourceGet(void);

#endif /* TIME_SOURCE_STATE_H */
