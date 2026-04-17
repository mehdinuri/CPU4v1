/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  include files */
#include "time.h"

#include <string.h>

#include "HardwarePorts.h"
#include "MLM.h"
#include "Platform/STM32/Core/Tim2CaptureView.h"
#include "data.h"
#include "gps.h"
#include "main.h"

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Definitions */
/* month days */
#define MONTH_DAYS { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
#define TIME_RTC_CFG_VAL 0x2014U

/* days */
typedef enum _tETimeWeekdays
{
  WEEKDAY_NONE = 0,
  WEEKDAY_MONDAY,
  WEEKDAY_TUESDAY,
  WEEKDAY_WEDNESDAY,
  WEEKDAY_THURSDAY,
  WEEKDAY_FRIDAY,
  WEEKDAY_SATURDAY,
  WEEKDAY_SUNDAY
} tETimeWeekdays;

/*  months */
typedef enum _tETimeMonths
{
  MONTH_NONE = 0,
  MONTH_JANUARY,
  MONTH_FEBRUARY,
  MONTH_MARCH,
  MONTH_APRIL,
  MONTH_MAY,
  MONTH_JUNE,
  MONTH_JULY,
  MONTH_AUGUST,
  MONTH_SEPTEMBER,
  MONTH_OCTOBER,
  MONTH_NOVEMBER,
  MONTH_DECEMBER
} tETimeMonths;
/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  constants */

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Members */
/*  private members */
static tSTime STime;

static uint8_t TimeLineSyncSignalValid(void)
{
  uint32_t frequency = Tim2CapturedFreqHzGet();

  return (uint8_t) ((frequency >= 95U) && (frequency <= 105U));
}

static void TimeApplyCommandedSource(void)
{
  uint8_t commandedSource = (uint8_t) UNIT_CLOCK_SOURCE_RTC_SQWR;

  if (UnitClockPortGetCommandedSource(&g_unitClockPort, &commandedSource)
      == FALSE)
  {
    TimeSourceSet(TIME_SOURCE_RTC);

    return;
  }

  switch (commandedSource)
  {
      case UNIT_CLOCK_SOURCE_GNSS:
      {
        if (GpsModemAliveGet() != FALSE)
        {
          TimeSourceSet(TIME_SOURCE_GPS);
        }
        else
        {
          TimeSourceSet(TIME_SOURCE_RTC);
        }

        break;
      }

      case UNIT_CLOCK_SOURCE_LINE_SYNC:
      {
        if (TimeLineSyncSignalValid() != FALSE)
        {
          TimeSourceSet(TIME_SOURCE_NET);
        }
        else
        {
          TimeSourceSet(TIME_SOURCE_RTC);
        }

        break;
      }

      case UNIT_CLOCK_SOURCE_RTC_SQWR:
      default:
      {
        TimeSourceSet(TIME_SOURCE_RTC);
        break;
      }
  }
}

/*  os members */

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Methods */
static void TimeRtcSnapshotToTime(const RtcSnapshot_t *snapshot, tpSTime pTime)
{
  pTime->SCurrentDate.WeekDay = snapshot->WeekDay;
  pTime->SCurrentDate.Month = snapshot->Month;
  pTime->SCurrentDate.Date = snapshot->Date;
  pTime->SCurrentDate.Year = snapshot->Year;
  pTime->SCurrentTime.Hours = snapshot->Hours;
  pTime->SCurrentTime.Minutes = snapshot->Minutes;
  pTime->SCurrentTime.Seconds = snapshot->Seconds;
  pTime->bCentury = snapshot->Century;
}

static void TimeRtcSnapshotFromTime(const tpSTime pTime,
                                    RtcSnapshot_t *snapshot)
{
  snapshot->WeekDay = pTime->SCurrentDate.WeekDay;
  snapshot->Month = pTime->SCurrentDate.Month;
  snapshot->Date = pTime->SCurrentDate.Date;
  snapshot->Year = pTime->SCurrentDate.Year;
  snapshot->Hours = pTime->SCurrentTime.Hours;
  snapshot->Minutes = pTime->SCurrentTime.Minutes;
  snapshot->Seconds = pTime->SCurrentTime.Seconds;
  snapshot->Century = pTime->bCentury;
}

static void TimeRtcEnsureInitialized(void)
{
  uint32_t signature = 0U;
  RtcSnapshot_t snapshot = { 0 };

  if (RealtimeClockReadMetadata(&g_rtcPort,
                                RTC_META_INIT_SIGNATURE,
                                &signature) == FALSE)
  {
    Error_Handler();
  }

  if (signature == TIME_RTC_CFG_VAL)
  {
    return;
  }

  snapshot.Century = TIME_CURRENT_CENTURY - 1U;
  snapshot.Month = MONTH_JANUARY;
  snapshot.Year = 24U;
  snapshot.Date = 1U;
  snapshot.WeekDay = WEEKDAY_MONDAY;
  snapshot.Hours = 8U;
  snapshot.Minutes = 0U;
  snapshot.Seconds = 0U;

  if (RealtimeClockWriteSnapshot(&g_rtcPort, &snapshot) == FALSE)
  {
    Error_Handler();
  }

  if (RealtimeClockWriteMetadata(&g_rtcPort,
                                 RTC_META_INIT_SIGNATURE,
                                 TIME_RTC_CFG_VAL) == FALSE)
  {
    Error_Handler();
  }
}

uint8_t TimeSecondGet(void)
{
  return STime.SCurrentTime.Seconds;
}

uint8_t TimeMinuteGet(void)
{
  return STime.SCurrentTime.Minutes;
}

uint8_t TimeHourGet(void)
{
  return STime.SCurrentTime.Hours;
}

uint8_t TimeDateGet(void)
{
  return STime.SCurrentDate.Date;
}

uint8_t TimeMonthGet(void)
{
  return STime.SCurrentDate.Month;
}

uint8_t TimeYearGet(void)
{
  return STime.SCurrentDate.Year;
}

uint8_t TimeCenturyGet(void)
{
  return STime.bCentury;
}

void TimeCenturySet(uint8_t bCent)
{
  STime.bCentury = bCent;
}

uint16_t TimeFullYearGet(void)
{
  return STime.sFullYear;
}

void TimeFullYearSet(void)
{
  STime.sFullYear = (TimeYearGet() + (TimeCenturyGet() * 100));
}

uint8_t TimeWeekdayGet(void)
{
  return STime.SCurrentDate.WeekDay;
}

uint8_t TimeDstGet(void)
{
  return STime.fDST;
}

void TimeDSTSet(uint8_t fDSTVal)
{
  STime.fDST = fDSTVal;
}

uint32_t TimeSecondOfDayGet(void)
{
  return STime.lSecondOfDay;
}

void TimeSecondOfDaySet(uint32_t lSecOfDay)
{
  STime.lSecondOfDay = lSecOfDay;
}

uint16_t TimeMinuteOfDayGet(void)
{
  return STime.sMinuteOfDay;
}

void TimeMinuteOfDaySet(uint16_t sMinOfDay)
{
  STime.sMinuteOfDay = sMinOfDay;
}

uint16_t TimeDayOfYearGet(void)
{
  return STime.sDayOfYear;
}

void TimeDayOfYearSet(uint16_t sDayOfYear)
{
  STime.sDayOfYear = sDayOfYear;
}

uint8_t TimeIsLeapYear(uint16_t sYear)
{
  return ((sYear % 4 == 0) && (sYear % 100 != 0)) || (sYear % 400 == 0);
}

uint8_t TimeMonthDayFromWeekDayCalc(uint8_t bWeekDay, uint8_t bOrder)
{
  uint8_t bMonthDay;

  if (((bWeekDay >= WEEKDAY_MONDAY) && (bWeekDay <= WEEKDAY_SUNDAY))
      && ((bOrder >= 1) && (bOrder <= 4)))                                                                /* if values are valid */
  {
    bMonthDay = TimeDateGet();
    /* rewind back to first "today"day of the month */
    while (bMonthDay > MAX_DAYS_IN_A_WEEK)
    {
      bMonthDay -= MAX_DAYS_IN_A_WEEK;
    }

    /* now bMonthDay is the first "today"day of the month */
    uint8_t bCurWeekDay = TimeWeekdayGet();

    if (bCurWeekDay <= bWeekDay)
    {
      bMonthDay += bWeekDay - bCurWeekDay;
    }
    else
    {
      bMonthDay -= bCurWeekDay - bWeekDay;
    }

    /* now bMonthDay is the first "bWeekDay"day of the month */
    bMonthDay += ((bOrder - 1)
                  * MAX_DAYS_IN_A_WEEK); /* bWeekDayOrder - 1 is used since bMonthDay */
    /* already was the 1st "bWeekDay"day of the month */
    /* now bMonthDay is the "bOrder"th "bWeekDay"day of the month */

    return bMonthDay;
  }

  return 0;
}

uint16_t TimeMinuteOfDayCalc(uint8_t bHours, uint8_t bMinutes)
{
  uint16_t sMinOfDay = bHours * MAX_MINUTES_IN_A_HOUR;

  sMinOfDay += bMinutes;

  return sMinOfDay;
}

uint32_t TimeSecondOfDayCalc(uint16_t bMinutesOfDay, uint8_t bSeconds)
{
  uint32_t lSecOfDay = bMinutesOfDay * MAX_SECONDS_IN_A_MINUTE;

  lSecOfDay += bSeconds;

  return lSecOfDay;
}

uint16_t TimeDayOfYearCalc(uint8_t bMonth, uint8_t bMonthDay, uint16_t sYear)
{
  uint8_t baDaysInMonth[] = MONTH_DAYS;

  if (TimeIsLeapYear(sYear))
  {
    baDaysInMonth[MONTH_FEBRUARY] = MAX_DAYS_IN_A_LEAP_MONTH;
  }

  uint16_t sDayOfYear = bMonthDay;

  for (uint8_t bIdx = MONTH_JANUARY; bIdx < bMonth; bIdx++)
  {
    sDayOfYear += baDaysInMonth[bIdx];
  }

  return sDayOfYear;
}

uint8_t TimeWeekDayOfYearCalc(uint8_t bMonth, uint8_t bMonthDay, uint16_t sYear)
{
  /* Tomohiko Sakamoto's Algorithm */
  const uint8_t baMonths1stDayOffsets[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2,
                                            4 };

  sYear -= bMonth < 3;
  uint8_t bWeekDay = (sYear + sYear / 4 - sYear / 100 + sYear / 400
                      + baMonths1stDayOffsets[bMonth - 1] + bMonthDay)
                     % MAX_DAYS_IN_A_WEEK;

  if (bWeekDay)
  {
    return bWeekDay;
  }

  return WEEKDAY_SUNDAY;
}

uint16_t TimeFullYearCalc(tpSTime pSTime)
{
  return pSTime->SCurrentDate.Year + (pSTime->bCentury * 100);
}

void TimeDSTAdjustmentInit(void)
{
  /* when program loading is in progress, time is sent from MCT. Assumed that, */
  /* this time information is adjusted. find dst state for this time and save it */
  uint8_t bCurMonth = TimeMonthGet();

  if ((bCurMonth > MONTH_MARCH) && (bCurMonth < MONTH_OCTOBER))
  {
    TimeDSTSet(TRUE);
  }
  else if ((bCurMonth == MONTH_MARCH) || (bCurMonth == MONTH_OCTOBER))
  {
    uint8_t bDSTChangeDate = TimeMonthDayFromWeekDayCalc(WEEKDAY_SUNDAY, 4); /* Last Sunday of October */
    uint8_t bCurDate = TimeDateGet();
    uint8_t bCurHour = TimeHourGet();

    if ((bCurMonth == MONTH_MARCH) && (bCurDate > bDSTChangeDate))
    {
      TimeDSTSet(TRUE);
    }
    else if ((bCurMonth == MONTH_MARCH) && (bCurDate == bDSTChangeDate)
             && (bCurHour >= 1))
    {
      TimeDSTSet(TRUE);
    }
    else if ((bCurMonth == MONTH_OCTOBER) && (bCurDate < bDSTChangeDate))
    {
      TimeDSTSet(TRUE);
    }
    else if ((bCurMonth == MONTH_OCTOBER) && (bCurDate == bDSTChangeDate)
             && (bCurHour < 2))
    {
      TimeDSTSet(TRUE);
    }
    else if ((bCurMonth == MONTH_OCTOBER) && (bCurDate == bDSTChangeDate)
             && (bCurHour >= 2))
    {
      TimeDSTSet(FALSE);
    }
    else
    {
      TimeDSTSet(FALSE);
    }
  }
  else
  {
    TimeDSTSet(FALSE);
  }
} /* TimeDSTAdjustmentInit */

void TimeDSTAdjustmentRun(void)
{
  /* check for DST adjustment */
  uint8_t bCurMonth = TimeMonthGet();
  uint8_t bCurDate = TimeDateGet();

  if (TimeDstGet() == FALSE) /* check for DST start */
  {
    if (bCurMonth == MONTH_MARCH)
    {
      uint8_t bDSTChangeDate = TimeMonthDayFromWeekDayCalc(WEEKDAY_SUNDAY, 4); /* Last Sunday of March */

      if (bCurDate == bDSTChangeDate)
      {
        if (TimeHourGet() == 1)
        {
          if (RealtimeClockApplyDstAdjustment(&g_rtcPort,
                                              RTC_DST_ADD_ONE_HOUR) == FALSE)
          {
            Error_Handler();
          }
        }
      }
    }
  }
  else /* check for DST end */
  {
    if (bCurMonth == MONTH_OCTOBER)
    {
      uint8_t bDSTChangeDate = TimeMonthDayFromWeekDayCalc(WEEKDAY_SUNDAY, 4);

      if (bCurDate == bDSTChangeDate)
      {
        if (TimeHourGet() == 2)
        {
          if (RealtimeClockApplyDstAdjustment(&g_rtcPort,
                                              RTC_DST_SUBTRACT_ONE_HOUR)
              == FALSE)
          {
            Error_Handler();
          }
        }
      }
    }
  }
} /* TimeDSTAdjustmentRun */

void TimeRTCRead(void)
{
  RtcSnapshot_t snapshot = { 0 };

  if (RealtimeClockReadSnapshot(&g_rtcPort, &snapshot) == FALSE)
  {
    Error_Handler();
  }

  TimeRtcSnapshotToTime(&snapshot, &STime);

  TimeFullYearSet();
  TimeMinuteOfDaySet(TimeMinuteOfDayCalc(TimeHourGet(), TimeMinuteGet()));
  TimeSecondOfDaySet(TimeSecondOfDayCalc(TimeMinuteOfDayGet(),
                                         TimeSecondGet()));
  TimeDayOfYearSet(TimeDayOfYearCalc(TimeMonthGet(),
                                     TimeDateGet(),
                                     TimeFullYearGet()));
}

void TimeGet(tpSTime pSTimeBuffer)
{
  memset(pSTimeBuffer, 0, sizeof(tSTime));

  osMutexAcquire(TimeMutexHandle, osWaitForever);
  memcpy(pSTimeBuffer, &STime, sizeof(tSTime));
  osMutexRelease(TimeMutexHandle);
}

void TimeSet(tpSTime pSNewTime)
{
  RtcSnapshot_t snapshot = { 0 };

  /*  capture STime to be able to modify correctly time data */
  if (TimeMutexHandle != NULL)
  {
    osMutexAcquire(TimeMutexHandle, osWaitForever);
  }

  /*  set time of rtc hardware */
  TimeRtcSnapshotFromTime(pSNewTime, &snapshot);
  if (RealtimeClockWriteSnapshot(&g_rtcPort, &snapshot) == FALSE)
  {
    Error_Handler();
  }

  TimeRTCRead();

  /*  release STime */
  if (TimeMutexHandle != NULL)
  {
    osMutexRelease(TimeMutexHandle);
  }
}

uint8_t TimeIsValid(tpSTime pSTime)
{
  if (pSTime == NULL)
  {
    return FALSE;
  }

  uint16_t sFullYear = TimeFullYearCalc(pSTime);

  if (sFullYear < EPOCH_REFERENCE_YEAR)
  {
    return FALSE;
  }

  if ((pSTime->SCurrentDate.Month < MONTH_JANUARY)
      || (pSTime->SCurrentDate.Month > MONTH_DECEMBER) )
  {
    return FALSE;
  }

  uint8_t baDaysInMonth[] = MONTH_DAYS;

  if (TimeIsLeapYear(sFullYear))
  {
    baDaysInMonth[MONTH_FEBRUARY] = MAX_DAYS_IN_A_LEAP_MONTH;
  }

  if ((pSTime->SCurrentDate.Date < 1)
      || (pSTime->SCurrentDate.Date
          > baDaysInMonth[pSTime->SCurrentDate.Month]))
  {
    return FALSE;
  }

  if ((pSTime->SCurrentDate.WeekDay < WEEKDAY_MONDAY)
      || (pSTime->SCurrentDate.WeekDay > WEEKDAY_SUNDAY) )
  {
    return FALSE;
  }

  if (pSTime->SCurrentTime.Hours >= MAX_HOURS_IN_A_DAY)
  {
    return FALSE;
  }

  if (pSTime->SCurrentTime.Minutes >= MAX_MINUTES_IN_A_HOUR)
  {
    return FALSE;
  }

  if (pSTime->SCurrentTime.Seconds >= MAX_SECONDS_IN_A_MINUTE)
  {
    return FALSE;
  }

  return TRUE;
} /* TimeIsValid */

void TimeHourInc(tpSTime pSTime)
{
  pSTime->SCurrentTime.Hours++;
  if (pSTime->SCurrentTime.Hours == MAX_HOURS_IN_A_DAY)
  {
    pSTime->lSecondOfDay = 0;
    pSTime->sMinuteOfDay = 0;
    pSTime->SCurrentTime.Hours = 0;

    pSTime->sDayOfYear++;
    pSTime->SCurrentDate.Date++;

    uint8_t baDaysInMonth[] = MONTH_DAYS;

    if (TimeIsLeapYear(TimeFullYearCalc(pSTime)))
    {
      baDaysInMonth[MONTH_FEBRUARY] = MAX_DAYS_IN_A_LEAP_MONTH;
    }

    if (pSTime->SCurrentDate.Date > baDaysInMonth[pSTime->SCurrentDate.Month])
    {
      pSTime->SCurrentDate.Date = 1;
      pSTime->SCurrentDate.Month++;
      if (pSTime->SCurrentDate.Month > MONTH_DECEMBER)
      {
        pSTime->SCurrentDate.Month = MONTH_JANUARY;
        pSTime->SCurrentDate.Year++;
        pSTime->sDayOfYear = 1;
      }
    }

    pSTime->SCurrentDate.WeekDay++;
    if (pSTime->SCurrentDate.WeekDay > WEEKDAY_SUNDAY)
    {
      pSTime->SCurrentDate.WeekDay = WEEKDAY_MONDAY;
    }
  }
}

void TimeHourDec(tpSTime pSTime)
{
  if (pSTime->SCurrentTime.Hours == 0)
  {
    pSTime->SCurrentTime.Hours = MAX_HOURS_IN_A_DAY - 1;
    pSTime->sMinuteOfDay = TimeMinuteOfDayCalc(pSTime->SCurrentTime.Hours,
                                               pSTime->SCurrentTime.Minutes);
    pSTime->lSecondOfDay = TimeSecondOfDayCalc(pSTime->sMinuteOfDay,
                                               pSTime->SCurrentTime.Seconds);
    pSTime->sDayOfYear--;
    pSTime->SCurrentDate.Date--;

    if (pSTime->SCurrentDate.Date < 1)
    {
      pSTime->SCurrentDate.Month--;
      if (pSTime->SCurrentDate.Month < MONTH_JANUARY)
      {
        pSTime->SCurrentDate.Month = MONTH_DECEMBER;
        pSTime->SCurrentDate.Year--;
        if (TimeIsLeapYear(TimeFullYearCalc(pSTime)))
        {
          pSTime->sDayOfYear = MAX_DAYS_IN_A_LEAP_YEAR;
        }
        else
        {
          pSTime->sDayOfYear = MAX_DAYS_IN_A_YEAR;
        }
      }

      uint8_t baDaysInMonth[] = MONTH_DAYS;

      if (TimeIsLeapYear(TimeFullYearCalc(pSTime)))
      {
        baDaysInMonth[MONTH_FEBRUARY] = MAX_DAYS_IN_A_LEAP_MONTH;
      }

      pSTime->SCurrentDate.Date = baDaysInMonth[pSTime->SCurrentDate.Month];
    }

    pSTime->SCurrentDate.WeekDay--;
    if (pSTime->SCurrentDate.WeekDay < WEEKDAY_MONDAY)
    {
      pSTime->SCurrentDate.WeekDay = WEEKDAY_SUNDAY;
    }
  }
  else
  {
    pSTime->SCurrentTime.Hours--;
    pSTime->sMinuteOfDay = TimeMinuteOfDayCalc(pSTime->SCurrentTime.Hours,
                                               pSTime->SCurrentTime.Minutes);
    pSTime->lSecondOfDay = TimeSecondOfDayCalc(pSTime->sMinuteOfDay,
                                               pSTime->SCurrentTime.Seconds);
  }
} /* TimeHourDec */

void TimeUTCCalculate(tpSTime pSTime)
{
  uint8_t ucIdx = 0;

  if (GetDeviceTimeZone() > 0)
  {
    for (ucIdx = 0; ucIdx < GetDeviceTimeZone(); ucIdx++)
    {
      TimeHourDec(pSTime);
    }
  }
  else
  {
    for (ucIdx = 0; ucIdx < (-1) * GetDeviceTimeZone(); ucIdx++)
    {
      TimeHourInc(pSTime);
    }
  }
}

uint8_t TimeEpochCalculate(tpSTime pSTime, uint32_t *ulEpoch)
{
  if (!TimeIsValid(pSTime))
  {
    return FALSE;
  }

  uint16_t sFullYear = TimeFullYearCalc(pSTime);

  /* Count seconds for all complete years since 1970 */
  for (uint16_t sYear = EPOCH_REFERENCE_YEAR; sYear < sFullYear; sYear++)
  {
    if (TimeIsLeapYear(sYear))
    {
      *ulEpoch += MAX_DAYS_IN_A_LEAP_YEAR * MAX_SECONDS_IN_A_DAY;
    }
    else
    {
      *ulEpoch += MAX_DAYS_IN_A_YEAR * MAX_SECONDS_IN_A_DAY;
    }
  }

  uint8_t baDaysInMonth[] = MONTH_DAYS;

  /* Update February days for leap year if applicable */
  if (TimeIsLeapYear(sFullYear))
  {
    baDaysInMonth[MONTH_FEBRUARY] = MAX_DAYS_IN_A_LEAP_MONTH;
  }

  /* Count seconds for all complete months in the current year */
  for (uint8_t bMonth = MONTH_JANUARY;
       bMonth < pSTime->SCurrentDate.Month;
       bMonth++)
  {
    *ulEpoch += baDaysInMonth[bMonth] * MAX_SECONDS_IN_A_DAY;
  }

  /* Count seconds for all complete days in the current month */
  *ulEpoch += (pSTime->SCurrentDate.Date - 1) * MAX_SECONDS_IN_A_DAY;

  /* Add hours, minutes, and seconds */
  *ulEpoch += pSTime->SCurrentTime.Hours * MAX_SECONDS_IN_A_HOUR;
  *ulEpoch += pSTime->SCurrentTime.Minutes * MAX_SECONDS_IN_A_MINUTE;
  *ulEpoch += pSTime->SCurrentTime.Seconds;

  return TRUE;
} /* TimeEpochCalculate */

uint8_t TimeCalculate(uint32_t lEpoch, tpSTime pSTime)
{
  if (pSTime == NULL)
  {
    return FALSE;
  }

  uint8_t baDaysInMonth[] = MONTH_DAYS;

  /* Set DST */
  pSTime->fDST = FALSE;

  /* Calculate total days and seconds of the day */
  uint32_t ulTotalDays = lEpoch / MAX_SECONDS_IN_A_DAY;

  pSTime->lSecondOfDay = lEpoch % MAX_SECONDS_IN_A_DAY;
  uint32_t ulRemainingSeconds = pSTime->lSecondOfDay;

  /* Determine the current year */
  uint16_t sYear = EPOCH_REFERENCE_YEAR;

  while (TRUE)
  {
    uint16_t sDaysInYear = MAX_DAYS_IN_A_YEAR;

    if (TimeIsLeapYear(sYear))
    {
      sDaysInYear++;
    }

    if (ulTotalDays < sDaysInYear)
    {
      break;
    }

    ulTotalDays -= sDaysInYear;
    sYear++;
  }

  /* Set century and year */
  pSTime->bCentury = sYear / 100;
  pSTime->SCurrentDate.Year = sYear % 100;

  /* Update February days for leap year if applicable */
  if (TimeIsLeapYear(sYear))
  {
    baDaysInMonth[MONTH_FEBRUARY] = MAX_DAYS_IN_A_LEAP_MONTH;
  }

  uint8_t bMonth = MONTH_JANUARY;

  /* Determine the current month and day */
  while (ulTotalDays >= baDaysInMonth[bMonth])
  {
    ulTotalDays -= baDaysInMonth[bMonth];
    bMonth++;
  }

  pSTime->SCurrentDate.Month = bMonth;
  pSTime->SCurrentDate.Date = ulTotalDays + 1;

  /* Determine hours, minutes, and seconds */
  pSTime->SCurrentTime.Hours = ulRemainingSeconds / MAX_SECONDS_IN_A_HOUR;
  ulRemainingSeconds %= MAX_SECONDS_IN_A_HOUR;
  pSTime->SCurrentTime.Minutes = ulRemainingSeconds / MAX_SECONDS_IN_A_MINUTE;
  pSTime->SCurrentTime.Seconds = ulRemainingSeconds % MAX_SECONDS_IN_A_MINUTE;

  /* Calculate additional fields */
  pSTime->sMinuteOfDay = TimeMinuteOfDayCalc(pSTime->SCurrentTime.Hours,
                                             pSTime->SCurrentTime.Minutes);

  /* Calculate day of the year */
  pSTime->sDayOfYear = TimeDayOfYearCalc(pSTime->SCurrentDate.Month,
                                         pSTime->SCurrentDate.Date,
                                         sYear);

  /* Calculate week day */
  pSTime->SCurrentDate.WeekDay =
    TimeWeekDayOfYearCalc(pSTime->SCurrentDate.Month,
                          pSTime->SCurrentDate.Date,
                          sYear);

  return TRUE;
} /* TimeCalculate */

void TimeInit(void)
{
  TimeRtcEnsureInitialized();
  TimeRTCRead();
  TimeSourceSet(TIME_SOURCE_RTC);
  TimeApplyCommandedSource();
}

/* /////////////////////////////////////////// */
/* Task code */
void TimeTaskFunc(void *argument)
{
  UNUSED(argument);

  TimeInit();

  SRuntimes.SaSignalStateRuntimes[SignalStateRuntimeCurNoGet()].bExecutionMode =
    SIGNAL_STATE_EXEC_MODE_INIT;

  /* power on logs */
  LogRequest(LOG_REQ_APPEND_ASYNCH, NULL, EVENT_POWER_ON, 0, 0, 0, 0);
  LogRequest(LOG_REQ_APPEND_ASYNCH, NULL, GetDeviceResetEvent(), 0, 0, 0, 0);

  while (FOREVER)
  {
    /* capture STime to be able to modify correctly time data */
    osMutexAcquire(TimeMutexHandle, osWaitForever);

    TimeRTCRead();

    if (IsDaylightSavingTimeFlagSet())
    {
      TimeDSTAdjustmentInit();
      TimeDSTAdjustmentRun();
    }

    /* release STime */
    osMutexRelease(TimeMutexHandle);

    TimeApplyCommandedSource();

    if (!IsSystemStartTimeWritten())
    {
      if (TimeSecondOfDayGet() % 3600 == 0)
      {
        SystemStartTimeIncUpHours();
        if (SystemStartTimeGetUpHours() >= SystemStartTimeGetMinUpHours())
        {
          SystemStartTimeStart();
        }
      }
    }

    SignalMaintenanceTask(EVENT_FLAGS_MAINTENANCE_TIME_TASK_ACTIVE);

    osDelay(1000);

    osThreadFlagsSet(ProgramTaskHandle, THREAD_FLAGS_PROGRAM_SEC_ELAPSED);

    if (TimeSourceGet() == TIME_SOURCE_GPS)
    {
      GpsSynchro();
    }
  }
} /* TimeTaskFunc */
