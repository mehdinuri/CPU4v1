/* App/Platform/STM32/Tasks/Tasks.c */

#include "Tasks.h"

#include <stddef.h>

#include "cmsis_os.h"

#include "Adapters/STM32/ControlBusAdapter.h"
#include "Adapters/STM32/FieldBusRxAdapter.h"
#include "Adapters/STM32/StatusLEDAdapter.h"
#include "Bootstrap/DomainServices.h"
#include "Bootstrap/HardwarePorts.h"

#include "stm32g4xx_hal.h"

/* 10 ms canonical tick for the malfunction engine. */
#define MALFUNCTION_TICK_PERIOD_MS 10U
#define MAINTENANCE_TICK_PERIOD_MS 100U

extern FieldBusRxAdapterCtx_t *MainApplicationGetFieldBusRxAdapter(void);
extern ControlBusAdapterCtx_t *MainApplicationGetControlBusAdapter(void);

static osThreadId_t s_malfunctionTask;
static osThreadId_t s_maintenanceTask;

static const osThreadAttr_t s_malfunctionAttr = {
  .name = "MP_Malfunction",
  .priority = osPriorityAboveNormal,
  .stack_size = 1024U
};

static const osThreadAttr_t s_maintenanceAttr = {
  .name = "MP_Maintenance",
  .priority = osPriorityNormal,
  .stack_size = 512U
};

static void MalfunctionTaskFunc(void *argument)
{
  (void) argument;

  uint32_t wakeTick = osKernelGetTickCount();

  for (;;)
  {
    FieldBusRxAdapterCtx_t *fieldBus = MainApplicationGetFieldBusRxAdapter();
    ControlBusAdapterCtx_t *controlBus = MainApplicationGetControlBusAdapter();

    ControlBusAdapterStep(controlBus);
    FieldBusRxAdapterStep(fieldBus);
    FieldBusRxAdapterCommit(fieldBus, HAL_GetTick());
    MalfunctionEngineTick(&g_malfunctionEngine);
    CpMpLinkServiceStep(&g_cpMpLinkService);
    (void) WatchdogFeed(&g_watchdogPort);

    wakeTick += MALFUNCTION_TICK_PERIOD_MS;
    osDelayUntil(wakeTick);
  }
}

static void MaintenanceTaskFunc(void *argument)
{
  (void) argument;

  uint32_t wakeTick = osKernelGetTickCount();

  for (;;)
  {
    wakeTick += MAINTENANCE_TICK_PERIOD_MS;
    osDelayUntil(wakeTick);
  }
}

void Tasks_Start(void)
{
  s_malfunctionTask = osThreadNew(MalfunctionTaskFunc,
                                  NULL,
                                  &s_malfunctionAttr);
  s_maintenanceTask = osThreadNew(MaintenanceTaskFunc,
                                  NULL,
                                  &s_maintenanceAttr);

  (void) s_malfunctionTask;
  (void) s_maintenanceTask;
}
