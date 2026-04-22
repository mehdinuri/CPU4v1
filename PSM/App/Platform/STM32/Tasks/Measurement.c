/**
 ******************************************************************************
 * @file    App/Platform/STM32/Tasks/Measurement.c
 * @brief   Measurement task — uses globally wired ports from HardwarePorts.h.
 *          All business logic lives in App/Domain/MeasurementService.c.
 *          Adapter contexts and ISR setters live in
 *          App/Platform/STM32/Bootstrap/main_stm32.c.
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "Measurement.h"
#include "HardwarePorts.h"
#include "Domain/MeasurementService.h"
#include "utilities.h"
#include "cmsis_os.h"
#include "adc.h"
#include "tim.h"

/* Private variables ---------------------------------------------------------*/
static MeasurementServiceCtx_t s_svc;

typedef struct
{
  volatile uint8_t flashStatePending;
  volatile uint8_t flashStateDirty;
  volatile uint8_t periodPending;
  volatile uint8_t periodDirty;
  volatile uint8_t offsetOpPending;
  volatile uint8_t offsetValPending;
  volatile uint8_t offsetDirty;
  volatile uint8_t commResetPending;
  volatile uint16_t commChecksPending;
} MeasurementPendingOps_t;

static MeasurementPendingOps_t s_pendingOps;

/* Private function prototypes -----------------------------------------------*/
static void GridVoltMeasurementStart(void);
static void GridFreqMeasurementStart(void);
static void RegulatorMeasurementsStart(void);
static uint32_t MeasurementIrqLock(void);
static void MeasurementIrqUnlock(uint32_t primask);
static void MeasurementPendingApply(void);

/* Private application code --------------------------------------------------*/
static void GridVoltMeasurementStart(void)
{
  ADC1StartDMA();
  Tim3Start();
}

static void GridFreqMeasurementStart(void)
{
  Tim2ICStartIT();
  Tim2OCStartIT();
}

static void RegulatorMeasurementsStart(void)
{
  ADC2StartDMA();
  ADC3StartDMA();
}

static uint32_t MeasurementIrqLock(void)
{
  uint32_t primask = __get_PRIMASK();

  __disable_irq();

  return primask;
}

static void MeasurementIrqUnlock(uint32_t primask)
{
  if (primask == 0U)
  {
    __enable_irq();
  }
}

static void MeasurementPendingApply(void)
{
  uint8_t flashStateDirty;
  uint8_t flashStatePending;
  uint8_t periodDirty;
  uint8_t periodPending;
  uint8_t offsetDirty;
  uint8_t offsetOpPending;
  uint8_t offsetValPending;
  uint8_t commResetPending;
  uint16_t commChecksPending;

  uint32_t primask = MeasurementIrqLock();

  flashStateDirty = s_pendingOps.flashStateDirty;
  flashStatePending = s_pendingOps.flashStatePending;
  periodDirty = s_pendingOps.periodDirty;
  periodPending = s_pendingOps.periodPending;
  offsetDirty = s_pendingOps.offsetDirty;
  offsetOpPending = s_pendingOps.offsetOpPending;
  offsetValPending = s_pendingOps.offsetValPending;
  commResetPending = s_pendingOps.commResetPending;
  commChecksPending = s_pendingOps.commChecksPending;

  s_pendingOps.flashStateDirty = 0U;
  s_pendingOps.periodDirty = 0U;
  s_pendingOps.offsetDirty = 0U;
  s_pendingOps.commResetPending = 0U;
  s_pendingOps.commChecksPending = 0U;

  MeasurementIrqUnlock(primask);

  if (flashStateDirty != 0U)
  {
    MeasurementService_FlashStateSet(&s_svc, flashStatePending);
  }

  if (periodDirty != 0U)
  {
    MeasurementService_PeriodApply(&s_svc, periodPending);
  }

  if (offsetDirty != 0U)
  {
    MeasurementService_OffsetApply(&s_svc, offsetOpPending, offsetValPending);
  }

  if (commResetPending != 0U)
  {
    MeasurementService_CommCntrReset(&s_svc);
  }

  while (commChecksPending > 0U)
  {
    MeasurementService_CommCheck(&s_svc);
    commChecksPending--;
  }
} /* MeasurementPendingApply */

/* Public application code --------------------------------------------------*/

/* Forwarders called from other tasks/ISRs.
 * The MeasurementTask remains the sole owner of s_svc; external callers only
 * persist settings if needed, then queue pending runtime changes here. */
void MeasurementFlashStateSet(uint8_t state)
{
  uint32_t primask = MeasurementIrqLock();

  s_pendingOps.flashStatePending = state;
  s_pendingOps.flashStateDirty = 1U;
  MeasurementIrqUnlock(primask);
}

void MeasurementPeriodSet(uint8_t period)
{
  /* Delegate validation + EEPROM write to the shared persistence helper,
   * then queue the runtime apply for MeasurementTask (the sole owner of
   * s_svc).  This keeps the task layer free of duplicate validate/write
   * logic — there is one persist implementation, shared with the domain. */
  if (MeasurementService_PeriodPersist(&g_eepromPort, period) != 0U)
  {
    uint32_t primask = MeasurementIrqLock();

    s_pendingOps.periodPending = period;
    s_pendingOps.periodDirty = 1U;
    MeasurementIrqUnlock(primask);
  }
}

void MeasurementOffsetSet(uint8_t op, uint8_t val)
{
  if (MeasurementService_OffsetPersist(&g_eepromPort, op, val) != 0U)
  {
    uint32_t primask = MeasurementIrqLock();

    s_pendingOps.offsetOpPending = op;
    s_pendingOps.offsetValPending = val;
    s_pendingOps.offsetDirty = 1U;
    MeasurementIrqUnlock(primask);
  }
}

void MeasurementCommCheck(void)
{
  uint32_t primask = MeasurementIrqLock();

  if (s_pendingOps.commChecksPending < 0xFFFFU)
  {
    s_pendingOps.commChecksPending++;
  }

  MeasurementIrqUnlock(primask);
}

void MeasurementCommCntrReset(void)
{
  uint32_t primask = MeasurementIrqLock();

  s_pendingOps.commResetPending = 1U;
  MeasurementIrqUnlock(primask);
}

void MeasurementThreadFlagSet(void)
{
  osThreadFlagsSet(MeasurementTaskHandle, THREAD_FLAGS_MEASUREMENT_DONE);
}

/* USER CODE BEGIN Header_vMeasurementTask */

/**
 * @brief Function implementing the MeasurementTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_vMeasurementTask */
void MeasurementTaskFunc(void *argument)
{
  /* USER CODE BEGIN vMeasurementTask */
  UNUSED(argument);

  /* Initialise service using globally wired ports from MainApplication_Init().
   * Adapter contexts and port instances are owned by main_stm32.c. */
  MeasurementService_Init(&s_svc,
                          &g_indicatorLEDPort,
                          &g_signalInputPort,
                          &g_canTxPort,
                          &g_voltageSensorPort,
                          &g_frequencySensorPort,
                          &g_eepromPort);

  /* Start hardware measurement pipelines */
  GridFreqMeasurementStart();
  GridVoltMeasurementStart();
  RegulatorMeasurementsStart();

  /* Infinite loop */
  while (pdTRUE)
  {
    osThreadFlagsWait(THREAD_FLAGS_MEASUREMENT_DONE,
                      osFlagsWaitAll,
                      osWaitForever);

    MeasurementPendingApply();
    MeasurementService_UpdateSensorData(&s_svc);
    MeasurementService_UpdateLEDs(&s_svc);

    if (MeasurementService_IsFlash(&s_svc) != 0U)
    {
      MeasurementService_FlashSync(&s_svc, HAL_GetTick());
    }
    else
    {
      MeasurementService_SendMeasurements(&s_svc);
    }

    MaintenanceTaskSignal(EVENT_FLAGS_MAINTENANCE_MEASUREMENT_TASK_ACTIVE);
  }

  /* USER CODE END vMeasurementTask */
} /* MeasurementTaskFunc */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
