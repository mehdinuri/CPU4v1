#ifndef _TIME
#define _TIME

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  include files */
#include "TimeTypes.h"

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  definitions */
#define TIME_CURRENT_CENTURY 21
#define TIME_WEEKDAYS_MAX 7

/*  time struct */

/*  Epoch References and some time defines */
#define MAX_MINUTES_IN_A_HOUR 60
#define MAX_SECONDS_IN_A_MINUTE 60
#define MAX_SECONDS_IN_A_HOUR (MAX_MINUTES_IN_A_HOUR * MAX_SECONDS_IN_A_MINUTE)
#define MAX_HOURS_IN_A_DAY 24
#define MAX_SECONDS_IN_A_DAY (MAX_SECONDS_IN_A_HOUR * MAX_HOURS_IN_A_DAY)
#define MAX_MONTHS_IN_A_YEAR 12
#define MAX_DAYS_IN_A_WEEK 7
#define MIN_DAYS_IN_A_MONTH 28
#define MAX_DAYS_IN_A_MONTH 31
#define MAX_DAYS_IN_A_LEAP_MONTH 29
#define MAX_DAYS_IN_A_YEAR 365
#define MAX_DAYS_IN_A_LEAP_YEAR 366

#define EPOCH_REFERENCE_YEAR 1970
/*  public members */

/* public functions */
extern uint8_t TimeWeekdayGet(void);
extern uint8_t TimeDstGet(void);
extern uint32_t TimeSecondOfDayGet(void);
extern uint8_t TimeYearGet(void);
extern uint16_t TimeFullYearGet(void);
extern uint8_t TimeCenturyGet(void);
extern uint16_t TimeMinuteOfDayGet(void);
extern uint16_t TimeDayOfYearGet(void);
extern uint16_t TimeMinuteOfDayCalc(uint8_t bHours, uint8_t bMinutes);
extern uint32_t TimeSecondOfDayCalc(uint16_t bMinutesOfDay, uint8_t bSeconds);
extern uint16_t TimeDayOfYearCalc(uint8_t bMonth,
                                  uint8_t bMonthDay,
                                  uint16_t sYear);
extern uint8_t TimeWeekDayOfYearCalc(uint8_t bMonth,
                                     uint8_t bMonthDay,
                                     uint16_t sYear);
extern void TimeGet(tpSTime pSTimeBuffer);
extern void TimeSet(tpSTime pSNewTime);
extern uint16_t TimeFullYearCalc(tpSTime pSTime);
extern uint8_t TimeIsValid(tpSTime pSNewTime);
extern void TimeHourInc(tpSTime pTime);
extern void TimeHourDec(tpSTime pTime);
extern uint8_t TimeIsLeapYear(uint16_t sYear);
extern void TimeUTCCalculate(tpSTime pSTime);
extern uint8_t TimeEpochCalculate(tpSTime pSTime, uint32_t *ulEpoch);
extern uint8_t TimeCalculate(uint32_t lEpoch, tpSTime pSTime);

#endif /* ifndef _TIME */
