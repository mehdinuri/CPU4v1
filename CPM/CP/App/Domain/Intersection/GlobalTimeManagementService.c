/* App/Domain/Intersection/GlobalTimeManagementService.c */
#include "GlobalTimeManagementService.h"

#include <limits.h>
#include <stddef.h>
#include <string.h>

typedef struct
{
  int32_t year;
  uint8_t month;
  uint8_t date;
  uint8_t weekDay;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  uint32_t secondsOfDay;
  int64_t localEpochSeconds;
  int64_t midnightEpochSeconds;
} LocalClock_t;

typedef struct
{
  uint8_t scheduleNumber;
  uint8_t dayPlanNumber;
} ScheduleSelection_t;

typedef struct
{
  uint8_t valid;
  uint8_t eventNumber;
  uint8_t actionNumber;
  int64_t eventEpochSeconds;
} DayPlanEventSelection_t;

static const IntersectionConfig_t *GetConfig(
  const GlobalTimeManagementService_t *service)
{
  if ((service == NULL) || (service->engine == NULL))
  {
    return NULL;
  }

  return IntersectionEngineGetConfig(service->engine);
}

static int64_t FloorDiv64(int64_t numerator, int64_t denominator)
{
  int64_t quotient = numerator / denominator;
  int64_t remainder = numerator % denominator;

  if ((remainder != 0LL) && (((remainder < 0LL) && (denominator > 0LL))
                             || ((remainder > 0LL) && (denominator < 0LL))))
  {
    quotient--;
  }

  return quotient;
}

static uint32_t PositiveModulo32(int64_t value, uint32_t modulus)
{
  int64_t remainder;

  if (modulus == 0U)
  {
    return 0U;
  }

  remainder = value % (int64_t) modulus;
  if (remainder < 0LL)
  {
    remainder += (int64_t) modulus;
  }

  return (uint32_t) remainder;
}

static int64_t CivilToDays(int32_t year, uint8_t month, uint8_t date)
{
  int32_t adjustedYear = year - (month <= 2U);
  int32_t era = (adjustedYear >= 0) ? (adjustedYear / 400)
                : ((adjustedYear - 399) / 400);
  uint32_t yearOfEra = (uint32_t) (adjustedYear - (era * 400));
  uint32_t monthPrime = (uint32_t) month + ((month > 2U) ? 9U : 21U);
  uint32_t dayOfYear = ((153U * (monthPrime % 12U)) + 2U) / 5U
                       + (uint32_t) date - 1U;
  uint32_t dayOfEra = (yearOfEra * 365U) + (yearOfEra / 4U)
                      - (yearOfEra / 100U) + dayOfYear;

  return ((int64_t) era * 146097LL) + (int64_t) dayOfEra - 719468LL;
}

static void DaysToCivil(int64_t days, int32_t *year, uint8_t *month,
                        uint8_t *date)
{
  int64_t z = days + 719468LL;
  int64_t era = (z >= 0LL) ? (z / 146097LL) : ((z - 146096LL) / 146097LL);
  uint32_t dayOfEra = (uint32_t) (z - (era * 146097LL));
  uint32_t yearOfEra = (dayOfEra - (dayOfEra / 1460U)
                        + (dayOfEra / 36524U) - (dayOfEra / 146096U))
                       / 365U;
  int32_t fullYear = (int32_t) yearOfEra + (int32_t) (era * 400LL);
  uint32_t dayOfYear = dayOfEra - ((365U * yearOfEra) + (yearOfEra / 4U)
                                   - (yearOfEra / 100U));
  uint32_t mp = ((5U * dayOfYear) + 2U) / 153U;
  uint32_t day = dayOfYear - ((153U * mp) + 2U) / 5U + 1U;
  uint32_t monthValue = (mp < 10U) ? (mp + 3U) : (mp - 9U);
  fullYear += (monthValue <= 2U) ? 1 : 0;

  if (year != NULL)
  {
    *year = fullYear;
  }

  if (month != NULL)
  {
    *month = (uint8_t) monthValue;
  }

  if (date != NULL)
  {
    *date = (uint8_t) day;
  }
}

static uint8_t WeekDayFromDays(int64_t days)
{
  int64_t weekDay = (days + 4LL) % 7LL;

  if (weekDay < 0LL)
  {
    weekDay += 7LL;
  }

  return (uint8_t) (weekDay + 1LL);
}

static uint8_t DaysInMonth(int32_t year, uint8_t month)
{
  static const uint8_t kMonthDays[12] = {
    31U, 28U, 31U, 30U, 31U, 30U, 31U, 31U, 30U, 31U, 30U, 31U
  };
  uint8_t monthDays;
  uint8_t leapYear;

  if ((month == 0U) || (month > 12U))
  {
    return 31U;
  }

  monthDays = kMonthDays[month - 1U];
  leapYear = (uint8_t) (((year % 4) == 0) && (((year % 100) != 0)
                                              || ((year % 400) == 0)));

  if ((month == 2U) && (leapYear != 0U))
  {
    monthDays = 29U;
  }

  return monthDays;
}

static uint8_t PopCount16(uint16_t value)
{
  uint8_t count = 0U;

  while (value != 0U)
  {
    count = (uint8_t) (count + (uint8_t) (value & 1U));
    value >>= 1U;
  }

  return count;
}

static uint8_t PopCount8(uint8_t value)
{
  uint8_t count = 0U;

  while (value != 0U)
  {
    count = (uint8_t) (count + (uint8_t) (value & 1U));
    value >>= 1U;
  }

  return count;
}

static uint8_t PopCount32(uint32_t value)
{
  uint8_t count = 0U;

  while (value != 0U)
  {
    count = (uint8_t) (count + (uint8_t) (value & 1UL));
    value >>= 1U;
  }

  return count;
}

static uint8_t ReadLocalClock(GlobalTimeManagementService_t *service,
                              LocalClock_t *clock)
{
  RtcSnapshot_t snapshot;
  int32_t year;
  int64_t days;

  if ((service == NULL) || (clock == NULL) || (service->rtcPort == NULL)
      || (RealtimeClockReadSnapshot(service->rtcPort, &snapshot) == 0U)
      || (snapshot.Century == 0U) || (snapshot.Month == 0U)
      || (snapshot.Month > 12U) || (snapshot.Date == 0U)
      || (snapshot.Date > 31U) || (snapshot.Hours > 23U)
      || (snapshot.Minutes > 59U) || (snapshot.Seconds > 59U))
  {
    return 0U;
  }

  year = (((int32_t) snapshot.Century - 1) * 100) + (int32_t) snapshot.Year;
  days = CivilToDays(year, snapshot.Month, snapshot.Date);

  memset(clock, 0, sizeof(*clock));
  clock->year = year;
  clock->month = snapshot.Month;
  clock->date = snapshot.Date;
  clock->weekDay = WeekDayFromDays(days);
  clock->hour = snapshot.Hours;
  clock->minute = snapshot.Minutes;
  clock->second = snapshot.Seconds;
  clock->secondsOfDay = ((uint32_t) snapshot.Hours * 3600U)
                        + ((uint32_t) snapshot.Minutes * 60U)
                        + (uint32_t) snapshot.Seconds;
  clock->midnightEpochSeconds = days * 86400LL;
  clock->localEpochSeconds = clock->midnightEpochSeconds
                             + (int64_t) clock->secondsOfDay;

  return 1U;
}

static uint8_t WriteLocalClock(GlobalTimeManagementService_t *service,
                               int64_t localEpochSeconds)
{
  RtcSnapshot_t snapshot;
  int64_t days;
  uint32_t secondsOfDay;
  int32_t year;
  uint8_t month;
  uint8_t date;

  if ((service == NULL) || (service->rtcPort == NULL)
      || (localEpochSeconds < 0LL))
  {
    return 0U;
  }

  days = FloorDiv64(localEpochSeconds, 86400LL);
  secondsOfDay = (uint32_t) (localEpochSeconds - (days * 86400LL));
  DaysToCivil(days, &year, &month, &date);

  if ((year < 1) || (year > 9999))
  {
    return 0U;
  }

  memset(&snapshot, 0, sizeof(snapshot));
  snapshot.Century = (uint8_t) ((year / 100) + 1);
  snapshot.Year = (uint8_t) (year % 100);
  snapshot.Month = month;
  snapshot.Date = date;
  snapshot.WeekDay = WeekDayFromDays(days);
  snapshot.Hours = (uint8_t) (secondsOfDay / 3600U);
  snapshot.Minutes = (uint8_t) ((secondsOfDay % 3600U) / 60U);
  snapshot.Seconds = (uint8_t) (secondsOfDay % 60U);

  return RealtimeClockWriteSnapshot(service->rtcPort, &snapshot);
}

static uint8_t BitMaskMatches16(uint16_t mask, uint8_t value)
{
  return (uint8_t) ((value < 16U) && ((mask & (uint16_t) (1U << value)) != 0U));
}

static uint8_t BitMaskMatches32(uint32_t mask, uint8_t value)
{
  return (uint8_t) ((value < 32U) && ((mask & (1UL << value)) != 0UL));
}

static uint8_t ComputeTransitionDay(int32_t year, uint8_t month,
                                    uint8_t occurrences, uint8_t dayOfWeek,
                                    uint8_t dayOfMonth)
{
  uint8_t day;
  uint8_t count;
  uint8_t limit = DaysInMonth(year, month);

  if (dayOfMonth > limit)
  {
    dayOfMonth = limit;
  }

  if (occurrences == 9U)
  {
    return dayOfMonth;
  }

  if ((occurrences >= 1U) && (occurrences <= 4U))
  {
    day = dayOfMonth;
    count = occurrences;

    while (day <= limit)
    {
      if (WeekDayFromDays(CivilToDays(year, month, day)) == dayOfWeek)
      {
        count--;
        if (count == 0U)
        {
          return day;
        }
      }

      day++;
    }

    return limit;
  }

  day = dayOfMonth;
  count = (uint8_t) (occurrences - 4U);

  while (day >= 1U)
  {
    if (WeekDayFromDays(CivilToDays(year, month, day)) == dayOfWeek)
    {
      count--;
      if (count == 0U)
      {
        return day;
      }
    }

    if (day == 1U)
    {
      break;
    }

    day--;
  }

  return 1U;
}

static int64_t ComputeLocalTransitionEpoch(
  int32_t year,
  const IntersectionDaylightSavingEntryConfig_t *entry,
  uint8_t beginTransition)
{
  uint8_t month = beginTransition ? entry->beginMonth : entry->endMonth;
  uint8_t occurrences = beginTransition ? entry->beginOccurrences
                        : entry->endOccurrences;
  uint8_t dayOfWeek = beginTransition ? entry->beginDayOfWeek
                      : entry->endDayOfWeek;
  uint8_t dayOfMonth = beginTransition ? entry->beginDayOfMonth
                       : entry->endDayOfMonth;
  uint32_t secondsToTransition = beginTransition
                                 ? entry->beginSecondsToTransition
                                 : entry->endSecondsToTransition;
  uint8_t date = ComputeTransitionDay(year,
                                      month,
                                      occurrences,
                                      dayOfWeek,
                                      dayOfMonth);

  return (CivilToDays(year, month, date) * 86400LL)
         + (int64_t) secondsToTransition;
}

static int32_t ResolveDstAdjustmentForUtc(
  const IntersectionGlobalTimeManagementConfig_t *globalTimeManagement,
  int64_t utcEpochSeconds)
{
  int32_t bestAdjustment = 0;
  int64_t bestBeginUtc = LLONG_MIN;
  uint8_t entryIndex;

  if ((globalTimeManagement == NULL)
      || (globalTimeManagement->globalDaylightSaving != 20U))
  {
    return 0;
  }

  for (entryIndex = 0U;
       entryIndex < INTERSECTION_DAYLIGHT_SAVING_ENTRY_COUNT_MAX;
       entryIndex++)
  {
    const IntersectionDaylightSavingEntryConfig_t *entry =
      &globalTimeManagement->daylightSavingEntries[entryIndex];

    if (entry->beginMonth == 14U)
    {
      continue;
    }

    if (entry->beginMonth == 13U)
    {
      int64_t beginUtc = (int64_t) entry->beginSecondsToTransition;
      int64_t endUtc = (int64_t) entry->endSecondsToTransition;
      uint8_t active = 0U;

      if (endUtc > beginUtc)
      {
        active = (uint8_t) ((utcEpochSeconds >= beginUtc)
                            && (utcEpochSeconds < endUtc));
      }
      else
      {
        active = (uint8_t) ((utcEpochSeconds >= beginUtc)
                            || (utcEpochSeconds < endUtc));
      }

      if ((active != 0U) && (beginUtc >= bestBeginUtc))
      {
        bestBeginUtc = beginUtc;
        bestAdjustment = (int32_t) entry->secondsToAdjust;
      }

      continue;
    }

    {
      int64_t standardLocalEpoch = utcEpochSeconds
                                   + (int64_t)
                                   globalTimeManagement->controllerStandardTimeZoneSeconds;
      int64_t localDays = FloorDiv64(standardLocalEpoch, 86400LL);
      int32_t localYear = 0;
      uint8_t month = 0U;
      uint8_t date = 0U;
      int32_t yearOffset;

      DaysToCivil(localDays, &localYear, &month, &date);
      (void) month;
      (void) date;

      for (yearOffset = -1; yearOffset <= 1; yearOffset++)
      {
        int32_t beginYear = localYear + yearOffset;
        int32_t endYear = beginYear;
        int64_t beginLocal = ComputeLocalTransitionEpoch(beginYear, entry, 1U);
        int64_t endLocal = ComputeLocalTransitionEpoch(endYear, entry, 0U);
        int64_t beginUtc;
        int64_t endUtc;

        if (endLocal <= beginLocal)
        {
          endYear++;
          endLocal = ComputeLocalTransitionEpoch(endYear, entry, 0U);
        }

        beginUtc = beginLocal
                   - (int64_t)
                   globalTimeManagement->controllerStandardTimeZoneSeconds;
        endUtc = endLocal
                 - (int64_t)
                 globalTimeManagement->controllerStandardTimeZoneSeconds
                 - (int64_t) entry->secondsToAdjust;

        if ((utcEpochSeconds >= beginUtc) && (utcEpochSeconds < endUtc)
            && (beginUtc >= bestBeginUtc))
        {
          bestBeginUtc = beginUtc;
          bestAdjustment = (int32_t) entry->secondsToAdjust;
        }
      }
    }
  }

  return bestAdjustment;
}

static uint8_t GetGlobalTimeForConfig(
  GlobalTimeManagementService_t *service,
  const IntersectionGlobalTimeManagementConfig_t *globalTimeManagement,
  uint32_t *globalTimeSeconds)
{
  LocalClock_t clock;
  int64_t utcEpoch;
  int64_t nextUtcEpoch;
  int32_t adjustment;
  uint8_t iteration;

  if ((globalTimeManagement == NULL) || (globalTimeSeconds == NULL)
      || (ReadLocalClock(service, &clock) == 0U))
  {
    return 0U;
  }

  utcEpoch = clock.localEpochSeconds
             - (int64_t)
             globalTimeManagement->controllerStandardTimeZoneSeconds;

  for (iteration = 0U; iteration < 3U; iteration++)
  {
    adjustment = ResolveDstAdjustmentForUtc(globalTimeManagement, utcEpoch);
    nextUtcEpoch = clock.localEpochSeconds
                   - (int64_t)
                   globalTimeManagement->controllerStandardTimeZoneSeconds
                   - (int64_t) adjustment;

    if (nextUtcEpoch == utcEpoch)
    {
      break;
    }

    utcEpoch = nextUtcEpoch;
  }

  if ((utcEpoch < 0LL) || (utcEpoch > (int64_t) UINT32_MAX))
  {
    return 0U;
  }

  *globalTimeSeconds = (uint32_t) utcEpoch;

  return 1U;
}

static uint8_t WriteGlobalTimeForConfig(
  GlobalTimeManagementService_t *service,
  const IntersectionGlobalTimeManagementConfig_t *globalTimeManagement,
  uint32_t globalTimeSeconds)
{
  int64_t localEpochSeconds;
  int32_t adjustment;

  if (globalTimeManagement == NULL)
  {
    return 0U;
  }

  adjustment = ResolveDstAdjustmentForUtc(globalTimeManagement,
                                          (int64_t) globalTimeSeconds);
  localEpochSeconds = (int64_t) globalTimeSeconds
                      + (int64_t)
                      globalTimeManagement->controllerStandardTimeZoneSeconds
                      + (int64_t) adjustment;

  return WriteLocalClock(service, localEpochSeconds);
}

static void SelectScheduleForClock(const IntersectionConfig_t *config,
                                   const LocalClock_t *clock,
                                   ScheduleSelection_t *selection)
{
  uint8_t scheduleIndex;
  uint8_t bestMonthBits = 0xFFU;
  uint8_t bestDateBits = 0xFFU;
  uint8_t bestDayBits = 0xFFU;

  if (selection == NULL)
  {
    return;
  }

  memset(selection, 0, sizeof(*selection));

  if ((config == NULL) || (clock == NULL))
  {
    return;
  }

  for (scheduleIndex = 0U;
       scheduleIndex < INTERSECTION_TIMEBASE_SCHEDULE_COUNT_MAX;
       scheduleIndex++)
  {
    const IntersectionTimebaseScheduleEntryConfig_t *schedule =
      &config->globalTimeManagement.schedules[scheduleIndex];
    uint8_t monthBits;
    uint8_t dateBits;
    uint8_t dayBits;

    if ((schedule->monthMask == 0U) || (schedule->dayMask == 0U)
        || (schedule->dateMask == 0U) || (schedule->dayPlanNumber == 0U)
        || (BitMaskMatches16(schedule->monthMask, clock->month) == 0U)
        || (BitMaskMatches32(schedule->dateMask, clock->date) == 0U)
        || (BitMaskMatches16((uint16_t) schedule->dayMask, clock->weekDay) == 0U))
    {
      continue;
    }

    monthBits = PopCount16(schedule->monthMask);
    dateBits = PopCount32(schedule->dateMask);
    dayBits = PopCount8(schedule->dayMask);

    if ((selection->scheduleNumber == 0U)
        || (monthBits < bestMonthBits)
        || ((monthBits == bestMonthBits) && (dateBits < bestDateBits))
        || ((monthBits == bestMonthBits) && (dateBits == bestDateBits)
            && (dayBits < bestDayBits)))
    {
      selection->scheduleNumber = (uint8_t) (scheduleIndex + 1U);
      selection->dayPlanNumber = schedule->dayPlanNumber;
      bestMonthBits = monthBits;
      bestDateBits = dateBits;
      bestDayBits = dayBits;
    }
  }
}

static void ConsiderDayPlanEvents(const IntersectionConfig_t *config,
                                  uint8_t dayPlanNumber,
                                  int64_t midnightEpochSeconds,
                                  int64_t currentLocalEpochSeconds,
                                  DayPlanEventSelection_t *selection)
{
  uint8_t eventIndex;

  if ((config == NULL) || (selection == NULL) || (dayPlanNumber == 0U)
      || (dayPlanNumber > INTERSECTION_DAY_PLAN_COUNT_MAX))
  {
    return;
  }

  for (eventIndex = 0U; eventIndex < INTERSECTION_DAY_PLAN_EVENT_COUNT_MAX;
       eventIndex++)
  {
    const IntersectionDayPlanEventConfig_t *event =
      &config->globalTimeManagement.dayPlans[dayPlanNumber - 1U][eventIndex];
    int64_t eventEpochSeconds = midnightEpochSeconds
                                + ((int64_t) event->hour * 3600LL)
                                + ((int64_t) event->minute * 60LL);

    if ((eventEpochSeconds > currentLocalEpochSeconds)
        || (eventEpochSeconds < (currentLocalEpochSeconds - 86400LL)))
    {
      continue;
    }

    if ((selection->valid == 0U)
        || (eventEpochSeconds > selection->eventEpochSeconds)
        || ((eventEpochSeconds == selection->eventEpochSeconds)
            && ((uint8_t) (eventIndex + 1U) >= selection->eventNumber)))
    {
      selection->valid = 1U;
      selection->eventNumber = (uint8_t) (eventIndex + 1U);
      selection->actionNumber = event->actionNumber;
      selection->eventEpochSeconds = eventEpochSeconds;
    }
  }
}

static uint8_t ResolveSyncPosition(const IntersectionConfig_t *config,
                                   const LocalClock_t *clock,
                                   const DayPlanEventSelection_t *event,
                                   uint8_t actionNumber,
                                   uint8_t *syncPositionSeconds)
{
  const IntersectionTimebaseActionConfig_t *action;
  uint8_t patternNumber;
  uint32_t cycleSeconds;
  int64_t referenceEpochSeconds;

  if ((config == NULL) || (clock == NULL) || (syncPositionSeconds == NULL)
      || (actionNumber == 0U)
      || (actionNumber > INTERSECTION_TIMEBASE_ACTION_COUNT_MAX))
  {
    return 0U;
  }

  action = &config->timebase.actions[actionNumber - 1U];
  patternNumber = action->pattern;

  if ((patternNumber == 0U) || (patternNumber > INTERSECTION_PATTERN_COUNT_MAX))
  {
    return 0U;
  }

  cycleSeconds = config->coordination.patterns[patternNumber - 1U].cycleTimeSeconds;
  if (cycleSeconds == 0U)
  {
    return 0U;
  }

  if (config->timebase.patternSyncMinutes == 65535U)
  {
    if ((event == NULL) || (event->valid == 0U))
    {
      return 0U;
    }

    referenceEpochSeconds = event->eventEpochSeconds;
  }
  else
  {
    referenceEpochSeconds = clock->midnightEpochSeconds
                            + ((int64_t) config->timebase.patternSyncMinutes
                               * 60LL);

    if (clock->localEpochSeconds < referenceEpochSeconds)
    {
      referenceEpochSeconds -= 86400LL;
    }
  }

  *syncPositionSeconds = (uint8_t) PositiveModulo32(
    clock->localEpochSeconds - referenceEpochSeconds,
    cycleSeconds);

  return 1U;
}

static uint8_t RefreshStatuses(GlobalTimeManagementService_t *service,
                               LocalClock_t *clockOut,
                               DayPlanEventSelection_t *eventOut)
{
  const IntersectionConfig_t *config = GetConfig(service);
  LocalClock_t clock;
  LocalClock_t previousClock;
  ScheduleSelection_t currentSelection;
  ScheduleSelection_t previousSelection;
  DayPlanEventSelection_t eventSelection;

  if ((config == NULL) || (ReadLocalClock(service, &clock) == 0U))
  {
    return 0U;
  }

  SelectScheduleForClock(config, &clock, &currentSelection);

  previousClock = clock;
  previousClock.localEpochSeconds -= 86400LL;
  previousClock.midnightEpochSeconds -= 86400LL;
  DaysToCivil(FloorDiv64(previousClock.localEpochSeconds, 86400LL),
              &previousClock.year,
              &previousClock.month,
              &previousClock.date);
  previousClock.weekDay =
    WeekDayFromDays(FloorDiv64(previousClock.localEpochSeconds, 86400LL));
  previousClock.secondsOfDay = clock.secondsOfDay;

  SelectScheduleForClock(config, &previousClock, &previousSelection);

  memset(&eventSelection, 0, sizeof(eventSelection));
  ConsiderDayPlanEvents(config,
                        previousSelection.dayPlanNumber,
                        previousClock.midnightEpochSeconds,
                        clock.localEpochSeconds,
                        &eventSelection);
  ConsiderDayPlanEvents(config,
                        currentSelection.dayPlanNumber,
                        clock.midnightEpochSeconds,
                        clock.localEpochSeconds,
                        &eventSelection);

  service->scheduleStatus = currentSelection.scheduleNumber;
  service->dayPlanStatus = currentSelection.dayPlanNumber;
  service->dayPlanEventStatus = eventSelection.valid ? eventSelection.eventNumber
                                : 0U;
  service->actionStatus = eventSelection.valid ? eventSelection.actionNumber : 0U;

  if (clockOut != NULL)
  {
    *clockOut = clock;
  }

  if (eventOut != NULL)
  {
    *eventOut = eventSelection;
  }

  return 1U;
}

void GlobalTimeManagementServiceInit(GlobalTimeManagementService_t *service)
{
  if (service == NULL)
  {
    return;
  }

  memset(service, 0, sizeof(*service));
}

void GlobalTimeManagementServiceBind(GlobalTimeManagementService_t *service,
                                     IntersectionEngine_t *engine,
                                     IRealtimeClockPort_t *rtcPort)
{
  if (service == NULL)
  {
    return;
  }

  service->engine = engine;
  service->rtcPort = rtcPort;
  GlobalTimeManagementServiceReset(service);
}

void GlobalTimeManagementServiceReset(GlobalTimeManagementService_t *service)
{
  if (service == NULL)
  {
    return;
  }

  service->lastObservedLocalTimeSeconds = 0U;
  service->lastObservedLocalTimeValid = 0U;
  service->scheduleStatus = 0U;
  service->dayPlanStatus = 0U;
  service->dayPlanEventStatus = 0U;
  service->actionStatus = 0U;
  service->lastAppliedActionNumber = 0U;
}

void GlobalTimeManagementServiceStep(GlobalTimeManagementService_t *service)
{
  const IntersectionConfig_t *config;
  LocalClock_t clock;
  DayPlanEventSelection_t event;
  uint8_t currentActionPlan = 0U;
  uint8_t syncPositionSeconds = 0U;
  uint8_t actionChanged = 0U;
  uint8_t syncValid = 0U;
  uint8_t realignSync = 0U;

  if ((service == NULL) || (service->engine == NULL))
  {
    return;
  }

  config = GetConfig(service);
  if ((config == NULL) || (RefreshStatuses(service, &clock, &event) == 0U))
  {
    return;
  }

  if (IntersectionEngineGetActionPlanControl(service->engine,
                                             &currentActionPlan) == 0U)
  {
    currentActionPlan = 0U;
  }

  if ((service->actionStatus != 0U) && (currentActionPlan != service->actionStatus))
  {
    if (IntersectionEngineSetActionPlanControl(service->engine,
                                               service->actionStatus) != 0U)
    {
      service->lastAppliedActionNumber = service->actionStatus;
      actionChanged = 1U;
    }
  }
  else if ((service->actionStatus != 0U) && (currentActionPlan == service->actionStatus))
  {
    service->lastAppliedActionNumber = service->actionStatus;
  }
  else if ((service->actionStatus == 0U) && (service->lastAppliedActionNumber != 0U)
           && (currentActionPlan == service->lastAppliedActionNumber))
  {
    if (IntersectionEngineSetActionPlanControl(service->engine, 0U) != 0U)
    {
      service->lastAppliedActionNumber = 0U;
      actionChanged = 1U;
    }
  }

  if ((service->lastObservedLocalTimeValid == 0U)
      || (clock.localEpochSeconds < 0LL)
      || ((uint32_t) clock.localEpochSeconds < service->lastObservedLocalTimeSeconds)
      || (((uint32_t) clock.localEpochSeconds - service->lastObservedLocalTimeSeconds)
          > 1U))
  {
    realignSync = 1U;
  }

  service->lastObservedLocalTimeSeconds = (uint32_t) clock.localEpochSeconds;
  service->lastObservedLocalTimeValid = 1U;

  syncValid = ResolveSyncPosition(config,
                                  &clock,
                                  &event,
                                  service->actionStatus,
                                  &syncPositionSeconds);

  if ((syncValid != 0U) && ((actionChanged != 0U) || (realignSync != 0U)))
  {
    (void) IntersectionEngineSetSystemSyncControl(service->engine,
                                                  syncPositionSeconds);
  }
}

void GlobalTimeManagementServiceHandleCommittedConfig(
  GlobalTimeManagementService_t *service,
  const IntersectionGlobalTimeManagementConfig_t *globalTimeManagementConfig)
{
  const IntersectionConfig_t *currentConfig = GetConfig(service);
  uint32_t globalTimeSeconds = 0U;

  if ((service == NULL) || (globalTimeManagementConfig == NULL))
  {
    return;
  }

  if ((currentConfig != NULL)
      && (memcmp(&currentConfig->globalTimeManagement,
                 globalTimeManagementConfig,
                 sizeof(*globalTimeManagementConfig)) == 0))
  {
    return;
  }

  if (GetGlobalTimeForConfig(service,
                             (currentConfig != NULL)
                             ? &currentConfig->globalTimeManagement
                             : globalTimeManagementConfig,
                             &globalTimeSeconds) == 0U)
  {
    return;
  }

  if (WriteGlobalTimeForConfig(service,
                               globalTimeManagementConfig,
                               globalTimeSeconds) != 0U)
  {
    service->lastObservedLocalTimeValid = 0U;
  }
}

uint8_t GlobalTimeManagementServiceGetScheduleStatus(
  GlobalTimeManagementService_t *service,
  uint8_t *scheduleStatus)
{
  if ((scheduleStatus == NULL) || (RefreshStatuses(service, NULL, NULL) == 0U))
  {
    return 0U;
  }

  *scheduleStatus = service->scheduleStatus;

  return 1U;
}

uint8_t GlobalTimeManagementServiceGetDayPlanStatus(
  GlobalTimeManagementService_t *service,
  uint8_t *dayPlanStatus)
{
  if ((dayPlanStatus == NULL) || (RefreshStatuses(service, NULL, NULL) == 0U))
  {
    return 0U;
  }

  *dayPlanStatus = service->dayPlanStatus;

  return 1U;
}

uint8_t GlobalTimeManagementServiceGetControllerLocalTime(
  GlobalTimeManagementService_t *service,
  uint32_t *controllerLocalTimeSeconds)
{
  LocalClock_t clock;

  if ((controllerLocalTimeSeconds == NULL)
      || (ReadLocalClock(service, &clock) == 0U)
      || (clock.localEpochSeconds < 0LL)
      || (clock.localEpochSeconds > (int64_t) UINT32_MAX))
  {
    return 0U;
  }

  *controllerLocalTimeSeconds = (uint32_t) clock.localEpochSeconds;

  return 1U;
}

uint8_t GlobalTimeManagementServiceGetGlobalTime(
  GlobalTimeManagementService_t *service,
  uint32_t *globalTimeSeconds)
{
  const IntersectionConfig_t *config = GetConfig(service);

  if ((config == NULL) || (globalTimeSeconds == NULL))
  {
    return 0U;
  }

  return GetGlobalTimeForConfig(service,
                                &config->globalTimeManagement,
                                globalTimeSeconds);
}

uint8_t GlobalTimeManagementServiceGetGlobalLocalTimeDifferential(
  GlobalTimeManagementService_t *service,
  int32_t *globalLocalTimeDifferentialSeconds)
{
  const IntersectionConfig_t *config = GetConfig(service);
  uint32_t globalTimeSeconds = 0U;
  int32_t adjustment;

  if ((config == NULL) || (globalLocalTimeDifferentialSeconds == NULL)
      || (GetGlobalTimeForConfig(service,
                                 &config->globalTimeManagement,
                                 &globalTimeSeconds) == 0U))
  {
    return 0U;
  }

  adjustment = ResolveDstAdjustmentForUtc(&config->globalTimeManagement,
                                          (int64_t) globalTimeSeconds);
  *globalLocalTimeDifferentialSeconds =
    config->globalTimeManagement.controllerStandardTimeZoneSeconds + adjustment;

  return 1U;
}

uint8_t GlobalTimeManagementServiceSetGlobalTime(
  GlobalTimeManagementService_t *service,
  uint32_t globalTimeSeconds)
{
  const IntersectionConfig_t *config = GetConfig(service);

  if ((config == NULL)
      || (WriteGlobalTimeForConfig(service,
                                   &config->globalTimeManagement,
                                   globalTimeSeconds) == 0U))
  {
    return 0U;
  }

  service->lastObservedLocalTimeValid = 0U;

  return 1U;
}

uint8_t GlobalTimeManagementServiceComputeStandardTimeZone(
  const IntersectionGlobalTimeManagementConfig_t *globalTimeManagementConfig,
  uint32_t globalTimeSeconds,
  int32_t desiredDifferentialSeconds,
  int32_t *standardTimeZoneSeconds)
{
  int32_t adjustment;

  if ((globalTimeManagementConfig == NULL) || (standardTimeZoneSeconds == NULL))
  {
    return 0U;
  }

  adjustment = ResolveDstAdjustmentForUtc(globalTimeManagementConfig,
                                          (int64_t) globalTimeSeconds);
  *standardTimeZoneSeconds = desiredDifferentialSeconds - adjustment;

  return 1U;
}
