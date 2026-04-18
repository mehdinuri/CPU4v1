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
#include "Config/EepromMap.h"
#include "utilities.h"
#include "cmsis_os.h"
#include "adc.h"
#include "tim.h"

/* Private variables ---------------------------------------------------------*/
static MeasurementServiceCtx_t s_svc;

typedef struct
{
  volatile uint8_t  bFlashStatePending;
  volatile uint8_t  bFlashStateDirty;
  volatile uint8_t  bPeriodPending;
  volatile uint8_t  bPeriodDirty;
  volatile uint8_t  bOffsetOpPending;
  volatile uint8_t  bOffsetValPending;
  volatile uint8_t  bOffsetDirty;
  volatile uint8_t  bCommResetPending;
  volatile uint16_t sCommChecksPending;
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
  uint8_t bFlashStateDirty;
  uint8_t bFlashStatePending;
  uint8_t bPeriodDirty;
  uint8_t bPeriodPending;
  uint8_t bOffsetDirty;
  uint8_t bOffsetOpPending;
  uint8_t bOffsetValPending;
  uint8_t bCommResetPending;
  uint16_t sCommChecksPending;

  uint32_t primask = MeasurementIrqLock();

  bFlashStateDirty   = s_pendingOps.bFlashStateDirty;
  bFlashStatePending = s_pendingOps.bFlashStatePending;
  bPeriodDirty       = s_pendingOps.bPeriodDirty;
  bPeriodPending     = s_pendingOps.bPeriodPending;
  bOffsetDirty       = s_pendingOps.bOffsetDirty;
  bOffsetOpPending   = s_pendingOps.bOffsetOpPending;
  bOffsetValPending  = s_pendingOps.bOffsetValPending;
  bCommResetPending  = s_pendingOps.bCommResetPending;
  sCommChecksPending = s_pendingOps.sCommChecksPending;

  s_pendingOps.bFlashStateDirty  = 0U;
  s_pendingOps.bPeriodDirty      = 0U;
  s_pendingOps.bOffsetDirty      = 0U;
  s_pendingOps.bCommResetPending = 0U;
  s_pendingOps.sCommChecksPending = 0U;

  MeasurementIrqUnlock(primask);

  if (bFlashStateDirty != 0U)
  {
    MeasurementService_FlashStateSet(&s_svc, bFlashStatePending);
  }

  if (bPeriodDirty != 0U)
  {
    MeasurementService_PeriodApply(&s_svc, bPeriodPending);
  }

  if (bOffsetDirty != 0U)
  {
    MeasurementService_OffsetApply(&s_svc, bOffsetOpPending, bOffsetValPending);
  }

  if (bCommResetPending != 0U)
  {
    MeasurementService_CommCntrReset(&s_svc);
  }

  while (sCommChecksPending > 0U)
  {
    MeasurementService_CommCheck(&s_svc);
    sCommChecksPending--;
  }
}

/* Public application code --------------------------------------------------*/

/* Forwarders called from other tasks/ISRs.
 * The MeasurementTask remains the sole owner of s_svc; external callers only
 * persist settings if needed, then queue pending runtime changes here. */
void MeasurementFlashStateSet(uint8_t fState)
{
  uint32_t primask = MeasurementIrqLock();
  s_pendingOps.bFlashStatePending = fState;
  s_pendingOps.bFlashStateDirty   = 1U;
  MeasurementIrqUnlock(primask);
}

void MeasurementPeriodSet(uint8_t bPeriod)
{
  if (MeasurementService_PeriodIsValid(bPeriod) != 0U)
  {
    uint32_t lRaw = (uint32_t) bPeriod;

    if (Eeprom_Write(&g_eepromPort,
                     EEPROM_ADDR_PERIOD,
                     &lRaw,
                     sizeof(lRaw)) != 0U)
    {
      uint32_t primask = MeasurementIrqLock();
      s_pendingOps.bPeriodPending = bPeriod;
      s_pendingOps.bPeriodDirty   = 1U;
      MeasurementIrqUnlock(primask);
    }
  }
}

void MeasurementOffsetSet(uint8_t bOp, uint8_t bVal)
{
  if (MeasurementService_OffsetOpIsValid(bOp) != 0U)
  {
    tSMeasurementOffset SOffset;
    SOffset.eOperation = bOp;
    SOffset.bValue     = bVal;

    if (Eeprom_Write(&g_eepromPort,
                     EEPROM_ADDR_OFFSET,
                     &SOffset,
                     sizeof(SOffset)) != 0U)
    {
      uint32_t primask = MeasurementIrqLock();
      s_pendingOps.bOffsetOpPending  = bOp;
      s_pendingOps.bOffsetValPending = bVal;
      s_pendingOps.bOffsetDirty      = 1U;
      MeasurementIrqUnlock(primask);
    }
  }
}

void MeasurementCommCheck(void)
{
  uint32_t primask = MeasurementIrqLock();
  if (s_pendingOps.sCommChecksPending < 0xFFFFU)
  {
    s_pendingOps.sCommChecksPending++;
  }
  MeasurementIrqUnlock(primask);
}

void MeasurementCommCntrReset(void)
{
  uint32_t primask = MeasurementIrqLock();
  s_pendingOps.bCommResetPending = 1U;
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
