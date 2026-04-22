/* App/Platform/STM32/Tasks/IntersectionControlTask.c
 *
 * Thin RTOS wrapper around the new controller-core engine. The domain owns
 * timing semantics; the task only provides the 10 ms scheduler cadence and
 * output dispatch boundary.
 */
#include "IntersectionControlTask.h"

#include "cmsis_os2.h"

#include "Adapters/STM32/ControlBusAdapter.h"
#include "Adapters/STM32/FieldBusTxAdapter.h"
#include "Adapters/STM32/FieldBusRxAdapter.h"
#include "Adapters/STM32/MmiCanAdapter.h"
#include "DomainServices.h"
#include "HardwarePorts.h"

#define INTERSECTION_CONTROL_TASK_PERIOD_MS 10U

extern ControlBusAdapterCtx_t *MainApplicationGetControlBusAdapter(void);

void IntersectionControlTaskFunc(void *argument)
{
  uint32_t wakeTick;

  (void) argument;

  wakeTick = osKernelGetTickCount();

  for (;;)
  {
    FieldBusRxAdapterStep();
    ControlBusAdapterStep(MainApplicationGetControlBusAdapter());
    CpMpLinkServiceStep(&g_cpMpLinkService);
    (void) MmuSetConfigReady(&g_mmuPort,
                             ConfigurationServiceIsLoadedFromRepository(
                               &g_configurationService));
    (void) IntersectionControllerStep(&g_intersectionController);
    FieldBusTxAdapterStep();
    DetectorReportServiceStep(&g_detectorReportService);
    GlobalTimeManagementServiceStep(&g_globalTimeManagementService);
    (void) UiCommsIdentityServiceRefresh(&g_uiCommsIdentityService);
    (void) UiDoorServiceStep(&g_uiDoorService, wakeTick);
    EventReportServiceUpdateCpMpLinkHealthy(
      &g_eventReportService,
      CpMpLinkServicePeerHealthy(&g_cpMpLinkService));
    EventReportServiceStep(&g_eventReportService, wakeTick);
    (void) MmiSnapshotCacheRefresh(&g_mmiSnapshotCache);
    if (UiDoorServiceConsumeChanged(&g_uiDoorService) != 0U)
    {
      MmiCanAdapterNotifyRuntimeTopic(MMI_PROTOCOL_V2_RUNTIME_TOPIC_DOOR, 0U);
    }
    MmiCanAdapterStep();

    wakeTick += INTERSECTION_CONTROL_TASK_PERIOD_MS;
    (void) osDelayUntil(wakeTick);
  }
}
