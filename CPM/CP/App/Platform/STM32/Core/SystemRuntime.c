/* App/Platform/STM32/Core/SystemRuntime.c */
#include "SystemRuntime.h"

#include "MLM.h"
#include "cmsis_os2.h"
#include "gpio.h"
#include "iwdg.h"
#include "main.h"
#include "defs.h"

static uint8_t s_resetEvent;
static uint8_t s_resetSource;
static uint8_t s_standby;

static void ClearResetFlags(void)
{
  __HAL_RCC_CLEAR_RESET_FLAGS();
}

static void ClearStandbyFlag(void)
{
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
}

static void ClearWakeupFlag(void)
{
  HAL_PWREx_ClearWakeupFlag(PWR_WAKEUP_FLAG1);
}

void SetDeviceResetEvent(void)
{
  s_resetEvent = EVENT_RESET_POWER_ON_CLEAR_CIRCUIT;

  if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDG1RST))
  {
    s_resetEvent = EVENT_RESET_WINDOW_WATCHDOG;
  }
  else if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDG1RST))
  {
    s_resetEvent = EVENT_RESET_INDEPENDENT_WATCHDOG;
  }
  else if (__HAL_RCC_GET_FLAG(RCC_FLAG_LPWR1RST)
           || __HAL_RCC_GET_FLAG(RCC_FLAG_LPWR2RST))
  {
    s_resetEvent = EVENT_RESET_LOW_POWER;
  }
  else if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST))
  {
    s_resetEvent = EVENT_RESET_SOFTWARE;
  }
  else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST))
  {
    s_resetEvent = EVENT_RESET_PIN;
  }
  else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST))
  {
    s_resetEvent = EVENT_RESET_PORRST;
  }
}

uint8_t GetDeviceResetEvent(void)
{
  return s_resetEvent;
}

void SetDeviceResetSource(void)
{
  s_resetSource = RESET_SOURCE_NONE;

  if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDG1RST))
  {
    s_resetSource |= (uint8_t) (RESET_SOURCE_WWDG << 1U);
  }

  if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDG1RST))
  {
    s_resetSource |= (uint8_t) (RESET_SOURCE_IWDG << 1U);
  }

  if (__HAL_RCC_GET_FLAG(RCC_FLAG_LPWR1RST)
      || __HAL_RCC_GET_FLAG(RCC_FLAG_LPWR2RST))
  {
    s_resetSource |= (uint8_t) (RESET_SOURCE_LOW_POWER << 1U);
  }

  if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST))
  {
    s_resetSource |= (uint8_t) (RESET_SOURCE_SOFTWARE << 1U);
  }

  if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST))
  {
    s_resetSource |= (uint8_t) (RESET_SOURCE_PIN << 1U);
  }

  if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST))
  {
    s_resetSource |= (uint8_t) (RESET_SOURCE_POR << 1U);
  }
}

uint8_t GetDeviceResetSource(void)
{
  return s_resetSource;
}

void ClearAllFlags(void)
{
  ClearResetFlags();
  ClearWakeupFlag();
  ClearStandbyFlag();
}

uint8_t GetStandbyState(void)
{
  return s_standby;
}

void SetStandbyState(uint8_t fState)
{
  s_standby = fState;
}

void NotifyStandbyState(void)
{
  if (GetStandbyState() == 0U)
  {
    return;
  }

  if (StandbyEventHandle != NULL)
  {
    (void) osEventFlagsSet(StandbyEventHandle, EVENT_FLAGS_STANDBY_100HZ_MISSING);
    return;
  }

  (void) osKernelLock();
  GPIOChargerShutdownDisable();
}

void CheckWakeupState(void)
{
  if (__HAL_PWR_GET_FLAG(PWR_FLAG_SB) == 0U)
  {
    return;
  }

  IWDGSetMaxTimeout();
  ClearStandbyFlag();

  if (!HAL_PWREx_GetWakeupFlag(PWR_WAKEUP_FLAG1))
  {
    ClearWakeupFlag();
    ClearResetFlags();
    GPIOChargerShutdownDisable();
  }
}

void ExecStandbyInfoOps(void)
{
  LogRequest(LOG_REQ_APPEND, NULL, EVENT_POWER_NORMAL_TO_STAND_BY, 0U, 0U, 0U, 0U);

  osDelay(100U);
  GPIOChargerShutdownDisable();
}

void SecureSystemReset(void)
{
  if (osKernelGetState() == osKernelRunning)
  {
    osDelay(1000U);
  }

  SystemReset();
}
