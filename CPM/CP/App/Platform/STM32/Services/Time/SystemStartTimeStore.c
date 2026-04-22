/* App/Platform/STM32/Services/Time/SystemStartTimeStore.c */
#include "SystemStartTimeStore.h"

#include <string.h>

#include "PersistencePorts.h"
#include "time.h"

static tSSystemStartTime s_systemStartTime;
static uint8_t s_loaded;
static uint8_t s_minSystemUpHours = SYSTEM_START_TIME_DEF_MIN_SYSTEM_UP_HOURS;
static uint8_t s_systemUpHours;

static void ApplyDefaults(void)
{
  (void) memset(&s_systemStartTime, 0, sizeof(s_systemStartTime));
}

static void EnsureLoaded(void)
{
  if (s_loaded != 0U)
  {
    return;
  }

  ApplyDefaults();
  (void) PersistenceRead(&g_persistencePort,
                         PERSIST_OBJECT_SYSTEM_START_TIME,
                         0U,
                         &s_systemStartTime,
                         sizeof(s_systemStartTime));
  s_loaded = 1U;
}

void SystemStartTimeInit(void)
{
  ApplyDefaults();
  s_loaded = 1U;
}

void SystemStartTimeSet(tpSSystemStartTime pSSystemStartTime)
{
  if (pSSystemStartTime == NULL)
  {
    return;
  }

  EnsureLoaded();
  s_systemStartTime = *pSSystemStartTime;
}

uint8_t SystemStartTimeSave(void)
{
  EnsureLoaded();
  return PersistenceWrite(&g_persistencePort,
                          PERSIST_OBJECT_SYSTEM_START_TIME,
                          0U,
                          &s_systemStartTime,
                          sizeof(s_systemStartTime));
}

uint8_t SystemStartTimeRead(void)
{
  EnsureLoaded();
  return 1U;
}

void SystemStartTimeGet(tpSSystemStartTime pSSystemStartTime)
{
  if (pSSystemStartTime == NULL)
  {
    return;
  }

  EnsureLoaded();
  *pSSystemStartTime = s_systemStartTime;
}

void SystemStartTimeStart(void)
{
  tSSystemStartTime stored = { 0 };
  tSTime now = { 0 };

  TimeGet(&now);
  stored.bAlreadyWritten = SYSTEM_START_TIME_ALREADY_WRITTEN;
  stored.bMonthDay = now.SCurrentDate.Date;
  stored.bMonth = now.SCurrentDate.Month;
  stored.sYear = TimeFullYearCalc(&now);
  SystemStartTimeSet(&stored);
  (void) SystemStartTimeSave();
}

uint8_t IsSystemStartTimeWritten(void)
{
  EnsureLoaded();
  return (uint8_t) (s_systemStartTime.bAlreadyWritten
                    == SYSTEM_START_TIME_ALREADY_WRITTEN);
}

void SystemStartTimeSetMinUpHours(uint8_t bMinUpHours)
{
  s_minSystemUpHours = bMinUpHours;
}

uint8_t SystemStartTimeGetMinUpHours(void)
{
  return s_minSystemUpHours;
}

void SystemStartTimeSetUpHours(uint8_t bUpHours)
{
  s_systemUpHours = bUpHours;
}

void SystemStartTimeIncUpHours(void)
{
  s_systemUpHours++;
}

uint8_t SystemStartTimeGetUpHours(void)
{
  return s_systemUpHours;
}
