/* App/Platform/STM32/Tasks/Tasks.c */

#include "Tasks.h"

#include <stddef.h>

#include "cmsis_os.h"

#include "Adapters/STM32/ControlBusAdapter.h"
#include "Adapters/STM32/FieldBusAdapter.h"
#include "Adapters/STM32/StatusLEDAdapter.h"
#include "Bootstrap/DomainServices.h"
#include "Bootstrap/HardwarePorts.h"

#include "stm32g4xx_hal.h"

/* 10 ms canonical tick for the malfunction engine. */
#define MALFUNCTION_TICK_PERIOD_MS 10U
#define MAINTENANCE_TICK_PERIOD_MS 100U

extern FieldBusAdapterCtx_t *MainApplicationGetFieldBusAdapter(void);

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
    FieldBusAdapterCtx_t *fieldBus = MainApplicationGetFieldBusAdapter();

    FieldBusAdapterCommit(fieldBus, HAL_GetTick());
    MalfunctionEngineTick(&g_malfunctionEngine);
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
    EventLogRecord_t record;

    /* Drain the event log and forward to CP via FDCAN1 events. */
    while (EventLogReadNext(&g_eventLogPort, &record) != 0U)
    {
      ControlBusFrame_t frame;

      frame.extendedId = 0x4300U;
      frame.length = 8U;
      frame.data[0] = (uint8_t) (record.eventCode & 0xFFU);
      frame.data[1] = (uint8_t) ((record.eventCode >> 8U) & 0xFFU);
      frame.data[2] = (uint8_t) (record.source & 0xFFU);
      frame.data[3] = (uint8_t) ((record.source >> 8U) & 0xFFU);
      frame.data[4] = record.params[0];
      frame.data[5] = record.params[1];
      frame.data[6] = record.params[2];
      frame.data[7] = record.params[3];
      (void) ControlBusSendFrame(&g_controlBusPort, &frame);
    }

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
