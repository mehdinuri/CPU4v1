#include "SystemPower.h"

#include "fdcan.h"
#include "gpio.h"
#include "iwdg.h"
#include "main.h"
#include "tim.h"

static void DisableDebugLocal(void)
{
#ifndef DEBUG
  DBGMCU->CR = 0x00000000;
#endif
}

static void DisableInterruptRequestsLocal(void)
{
  __disable_irq();
}

void ClearResetFlags(void)
{
  __HAL_RCC_CLEAR_RESET_FLAGS();
}

void ClearStandbyFlag(void)
{
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
}

void ClearWakeupFlag(void)
{
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
}

void ClearAllFlags(void)
{
  ClearStandbyFlag();
  ClearWakeupFlag();
  ClearResetFlags();
}

void PrepareForStandbyMode(void)
{
  DisableDebugLocal();

  CANDeInit(&hfdcan1);
  Tim2DeInit();
  GPIODeInit();
  Tim1DeInit();
  HAL_RCC_DeInit();
}

void EnterStandbyMode(void)
{
  __HAL_RCC_PWR_CLK_ENABLE();
  HAL_PWR_EnableBkUpAccess();
  HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
  HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);
  HAL_PWR_EnterSTANDBYMode();
}

void EnterStandbyModeWithPreparation(uint8_t fPrep)
{
  IWDGSetMaxTimeout();
  DisableInterruptRequestsLocal();
  ClearAllFlags();

  if (fPrep != 0U)
  {
    PrepareForStandbyMode();
  }

  EnterStandbyMode();
  SystemReset();
}

void CheckWakeupOnReset(void)
{
  if (__HAL_PWR_GET_FLAG(PWR_FLAG_SB))
  {
    IWDGSetMaxTimeout();
    ClearStandbyFlag();

    if (__HAL_PWR_GET_FLAG(PWR_WAKEUP_PIN1) == 0U)
    {
      ClearWakeupFlag();
      ClearResetFlags();
      EnterStandbyModeWithPreparation(0U);
    }
  }
}
