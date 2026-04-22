/* Core/Src/FreeRTOSRuntimeStats.c
 *
 * The active MP runtime still enables FreeRTOS run-time stats, but the old
 * app_freertos.c task tree is gone. Keep the required hooks in a tiny
 * dedicated module instead of pulling the legacy task graph back in.
 */

#include "main.h"

void configureTimerForRunTimeStats(void)
{
}

unsigned long getRunTimeCounterValue(void)
{
  return (unsigned long) HAL_GetTick();
}
