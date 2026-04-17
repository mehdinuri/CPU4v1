/* App/Platform/STM32/Services/Time/TimeTypes.h
 *
 * Time-service owned date/time types. These stay HAL-free so time
 * consumers do not depend on STM32 RTC register structures.
 */
#ifndef TIME_TYPES_H
#define TIME_TYPES_H

#include <stdint.h>

typedef struct
{
  uint8_t WeekDay;
  uint8_t Month;
  uint8_t Date;
  uint8_t Year;
} tSTimeDate;

typedef struct
{
  uint8_t Hours;
  uint8_t Minutes;
  uint8_t Seconds;
} tSTimeClock;

typedef struct _STime
{
  tSTimeDate SCurrentDate;
  tSTimeClock SCurrentTime;

  uint8_t fDST;
  uint8_t bCentury;
  uint16_t sFullYear;
  uint32_t lSecondOfDay;
  uint16_t sMinuteOfDay;
  uint16_t sDayOfYear;
} tSTime, *tpSTime;

#endif /* TIME_TYPES_H */
