/* App/Platform/STM32/Tasks/IntersectionControlTask.c
 *
 * Thin RTOS wrapper around the new controller-core engine. The domain owns
 * timing semantics; the task only provides the 10 ms scheduler cadence and
 * output dispatch boundary.
 */
#include "IntersectionControlTask.h"

#include "cmsis_os2.h"

#include "DomainServices.h"

#define INTERSECTION_CONTROL_TASK_PERIOD_MS 10U

void IntersectionControlTaskFunc(void *argument)
{
  uint32_t wakeTick;

  (void) argument;

  wakeTick = osKernelGetTickCount();

  for (;;)
  {
    (void) IntersectionControllerStep(&g_intersectionController);
    DetectorReportServiceStep(&g_detectorReportService);
    GlobalTimeManagementServiceStep(&g_globalTimeManagementService);

    wakeTick += INTERSECTION_CONTROL_TASK_PERIOD_MS;
    (void) osDelayUntil(wakeTick);
  }
}
