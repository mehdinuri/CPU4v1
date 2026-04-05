/*
 * Platform/STM32/Tasks/ProgramTask.c
 *
 * FreeRTOS wrapper for the Domain Intersection controller tick.
 *
 * Priority : osPriorityHigh
 * Period   : 100 ms (via osDelay — soft real-time)
 * Argument : ProgramCtx_t*  (injected from main_stm32.c)
 */
#include "Tasks.h"
#include "Domain/Intersection/Program.h"

void ProgramTask(void *argument)
{
  ProgramCtx_t *ctx = (ProgramCtx_t *) argument;

  for (;;)
  {
    osDelay(100U);               /* 100 ms period */
    ProgramTick(ctx);
  }
}
