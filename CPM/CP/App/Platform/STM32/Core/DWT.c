/*
 * App/Platform/STM32/Core/DWT.c
 *
 * DWT cycle-counter busy-wait and D2 SRAM clock enable.
 * Moved from Core/Src/main.c USER CODE blocks.
 */
#include "DWT.h"

void DWTInit(void)
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  DWT->CYCCNT = 0U;
}

void DWTDelayuSeconds(volatile uint32_t uSeconds)
{
  uint32_t lStart;
  uint32_t lTarget;
  const uint32_t lCyclesPeruSec = (HAL_RCC_GetHCLKFreq() / 1000000UL);

  if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk))
  {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  }

  if (!(DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk))
  {
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    DWT->CYCCNT = 0U;
  }

  lStart = DWT->CYCCNT;
  lTarget = lStart + (uSeconds * lCyclesPeruSec);

  if (lTarget > lStart)
  {
    while (DWT->CYCCNT < lTarget)
    {
      ;
    }
  }
  else
  {
    while ((DWT->CYCCNT > lStart) || (DWT->CYCCNT < lTarget))
    {
      ;
    }
  }
}
