/**
 ******************************************************************************
 * File Name          : signal_monitor.c
 * Description        : Code for monitoring signals from CAN
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "measurement.h"
#include "fdcan.h"
#include "can_msg_sender.h"
#include "can_msg_parser.h"
#include "adc.h"
#include "storage.h"
#include "tim.h"
#include "utilities.h"
#include "gpio.h"
#include "Domain/SignalOutput.h"
#include "Domain/SignalOutputImageBuilder.h"
#include "Domain/CurrentMeasurement.h"
#include "Domain/FlashSyncWatchdog.h"
#include "Domain/OutputVerify.h"
#include "Domain/SignalCardIdentity.h"
#include "Domain/VoltageCurrentFrame.h"
#include "Platform/STM32/Bootstrap/HardwarePorts.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define COMM_LED_PERIOD 5

/* Bounded wait for the ADC-driven measurement cycle. Nominal period is
 * ~20 ms (50 Hz mains, one DMA half per edge). A missing wake is most
 * likely the ADC/DMA path being wedged; after N consecutive timeouts the
 * outputs are forced to the safe (off) state instead of latching stale
 * commands. A single missed wake is tolerated — jitter happens.
 */
#define MEASUREMENT_CYCLE_TIMEOUT_MS    100U
#define MEASUREMENT_MAX_CONSECUTIVE_TIMEOUTS 3U

/* Flash-sync deadman threshold. PSM's sync cadence is 1 Hz (one frame per
 * half-cycle of the 0.5 Hz flash waveform), so a typical gap is ~500 ms.
 * 1500 ms allows up to three missed frames before we assume PSM has gone
 * silent and collapse to the safe (all-off) state.
 */
#define FLASH_SYNC_STALE_TIMEOUT_MS     1500U

/* Private variables ---------------------------------------------------------*/
static uint8_t bCommLedToggleCntr = 0;

static tSSignalOutputStateCntr SaSignalOutputStateCntrs[SIGNAL_OUTPUTS_PER_SSM];

/* OutputVerify plumbing. Each cycle commands the 12 outputs via
 * SignalGroupSwitchStatesSet (captured into SPendingCommandedImage), then
 * observes what actually happened via SignalOutputStatesCheck (into
 * SObservedImage). Because the ISR-accumulated counters reflect the period
 * *before* the most recent command took effect, the verify compares the
 * *previous* command (SLastCommandedImage) against the current observation,
 * then promotes pending → last.
 *
 * SObservedImage is also the voltage-side input to the outgoing CAN frame
 * (VoltageCurrentFrame_Encode), so it serves double duty.
 */
static tSSignalOutputImage SPendingCommandedImage;
static tSSignalOutputImage SLastCommandedImage;
static tSSignalOutputImage SObservedImage;
static tSOutputVerifyState SOutputVerify;
static uint8_t bVerifyArmed = 0U;   /* first cycle has no "last" yet */

/* Private function prototypes -----------------------------------------------*/

/* Public typedef -----------------------------------------------------------*/

/* Public define ------------------------------------------------------------*/

/* Public variables ---------------------------------------------------------*/

/* Public function prototypes -----------------------------------------------*/

/* Private application code --------------------------------------------------*/
static void SignalOutputStateCntrsInit(void)
{
  memset(SaSignalOutputStateCntrs, 0, sizeof(SaSignalOutputStateCntrs));
}

static void TaskInit(void)
{
  bCommLedToggleCntr = 0;

  memset(&SPendingCommandedImage, 0, sizeof(SPendingCommandedImage));
  memset(&SLastCommandedImage, 0, sizeof(SLastCommandedImage));
  memset(&SObservedImage, 0, sizeof(SObservedImage));
  OutputVerify_Reset(&SOutputVerify);
  bVerifyArmed = 0U;

  SignalOutputStateCntrsInit();
}

static void SignalOutputStatesTimerStart(void)
{
  Tim4OCStartIT();
}

static void SGCurrentMeasurementStart(void)
{
  ADCSGCurrentMeasurementStart();
  Tim3Start();
}

static void GridFreqMeasurementStart(void)
{
  Tim2ICStartIT();
  Tim2OCStartIT();
}

static void SignalOutputStatesCheck(void)
{
  tSSignalOutputStateCntr SSnapshot[SIGNAL_OUTPUTS_PER_SSM];
  uint8_t i;

  /* Snapshot the counters under a brief critical section. The TIM4 OC ISR
   * (MeasurementSGStatesCntrsSet) writes to SaSignalOutputStateCntrs every
   * ~1 ms; we must stop it from racing us between the read and the zero.
   * taskENTER_CRITICAL masks the ISR via BASEPRI for the few μs this takes —
   * correct primitive for task-vs-ISR shared counters (osMutex would have
   * been UB from the ISR side).
   */
  taskENTER_CRITICAL();
  memcpy(SSnapshot, SaSignalOutputStateCntrs, sizeof(SSnapshot));
  SignalOutputStateCntrsInit();
  taskEXIT_CRITICAL();

  for (i = 0U; i < SIGNAL_OUTPUTS_PER_SSM; i++)
  {
    SObservedImage.aChannels[i] =
      SignalOutput_IsActive(SSnapshot[i].bOnCntr, SSnapshot[i].bOffCntr);
  }
} /* SignalOutputStatesCheck */

static void OutputVerifyStep(void)
{
  /* The counters we just folded into SObservedImage were accumulated while
   * SLastCommandedImage was the active command (the *previous* cycle's).
   * Verify against that; then promote SPendingCommandedImage → "last" so
   * the next cycle's verify uses the right reference.
   *
   * Skip the very first cycle: SLastCommandedImage is all-zeros then, but
   * so is SObservedImage (no samples yet), so technically it would match —
   * the bVerifyArmed guard keeps the semantics explicit regardless.
   */
  if (bVerifyArmed != 0U)
  {
    OutputVerify_Step(&SOutputVerify, &SLastCommandedImage, &SObservedImage);
    if (SOutputVerify.bFaultActive != 0U)
    {
      MaintenanceTaskSignal(EVENT_FLAGS_MAINTENANCE_OUTPUT_VERIFY_FAULT);
    }
  }

  SLastCommandedImage = SPendingCommandedImage;
  bVerifyArmed = 1U;
}

static void SignalGroupSwitchStatesSet(void)
{
  tSSignalOutputBuildInputs SInputs;
  uint8_t bGroup;
  uint8_t bOutput;
  uint8_t bIdx;

  SInputs.bFlashActive = CANFlashStatusGet();
  SInputs.bFlashSyncActive = CANFlashSyncStatusGet();

  /* Flash-sync deadman: if we're in flash mode but the PSM sync source has
   * gone silent, collapse to the safe (all-off) state instead of trusting a
   * stale fFlashSyncStatus. Raise a maintenance fault bit so the condition
   * is recorded before the hardware watchdog eventually resets the board.
   *
   * In non-flash mode the deadman doesn't apply — the sync frames aren't
   * expected to arrive at all in that mode.
   */
  if ((SInputs.bFlashActive != 0U)
      && FlashSyncWatchdog_IsStale(&g_FlashSyncWatchdog,
                                   Tick_Now_ms(&g_TickPort),
                                   FLASH_SYNC_STALE_TIMEOUT_MS))
  {
    SignalOutput_AllOff(&g_SignalOutputPort);
    MaintenanceTaskSignal(EVENT_FLAGS_MAINTENANCE_FLASHSYNC_STALE);
    memset(&SPendingCommandedImage, 0, sizeof(SPendingCommandedImage));

    return;
  }

  for (bGroup = 0U; bGroup < SIGNAL_GROUPS_PER_SSM; bGroup++)
  {
    for (bOutput = 0U; bOutput < SIGNAL_OUTPUTS_PER_SIGNAL_GROUP; bOutput++)
    {
      bIdx = (uint8_t) ((bGroup * SIGNAL_OUTPUTS_PER_SIGNAL_GROUP) + bOutput);
      SInputs.aRunChannels[bIdx] = (uint8_t) CANSignalOutputStateGet(bGroup,
                                                                     bOutput);
      SInputs.aFlashChannels[bIdx] =
        (uint8_t) CANSignalOutputFlashStateGet(bGroup,
                                               bOutput);
    }
  }

  if (SignalOutputImageBuilder_BuildSafe(&SInputs, &SPendingCommandedImage)
      != 0U)
  {
    MaintenanceTaskSignal(EVENT_FLAGS_MAINTENANCE_INVALID_COMMAND);
  }

  SignalOutput_Apply(&g_SignalOutputPort, &SPendingCommandedImage);
}

static uint8_t VoltageCurrentStatusBuild(
  const tSCurrentMeasurementSnapshot *pSCurSnap)
{
  uint8_t bStatus = 0U;
  uint32_t lFaults = MaintenanceTaskFaultsGet();

  if (CANRxFaultLatched() != 0U)
  {
    MaintenanceTaskSignal(EVENT_FLAGS_MAINTENANCE_CAN_RX_FAULT);
    lFaults = (uint32_t) (lFaults | EVENT_FLAGS_MAINTENANCE_CAN_RX_FAULT);
  }

  if (CANTxFaultLatched() != 0U)
  {
    lFaults = (uint32_t) (lFaults | EVENT_FLAGS_MAINTENANCE_CAN_TX_FAULT);
  }

  if (StorageFaultLatched() != 0U)
  {
    lFaults = (uint32_t) (lFaults | EVENT_FLAGS_MAINTENANCE_STORAGE_FAULT);
  }

  if ((lFaults & EVENT_FLAGS_MAINTENANCE_MEASUREMENT_FAULT) != 0U)
  {
    bStatus = (uint8_t) (bStatus | VOLTAGE_CURRENT_STATUS_MEASUREMENT_FAULT);
  }

  if ((lFaults & EVENT_FLAGS_MAINTENANCE_FLASHSYNC_STALE) != 0U)
  {
    bStatus = (uint8_t) (bStatus | VOLTAGE_CURRENT_STATUS_FLASHSYNC_STALE);
  }

  if ((lFaults & EVENT_FLAGS_MAINTENANCE_OUTPUT_VERIFY_FAULT) != 0U)
  {
    bStatus = (uint8_t) (bStatus | VOLTAGE_CURRENT_STATUS_OUTPUT_VERIFY_FAULT);
  }

  if ((lFaults & EVENT_FLAGS_MAINTENANCE_CAN_RX_FAULT) != 0U)
  {
    bStatus = (uint8_t) (bStatus | VOLTAGE_CURRENT_STATUS_CAN_RX_FAULT);
  }

  if ((lFaults & EVENT_FLAGS_MAINTENANCE_CAN_TX_FAULT) != 0U)
  {
    bStatus = (uint8_t) (bStatus | VOLTAGE_CURRENT_STATUS_CAN_TX_FAULT);
  }

  if ((lFaults & EVENT_FLAGS_MAINTENANCE_STORAGE_FAULT) != 0U)
  {
    bStatus = (uint8_t) (bStatus | VOLTAGE_CURRENT_STATUS_STORAGE_FAULT);
  }

  if ((pSCurSnap->bStatus & CURRENT_MEASUREMENT_STATUS_SATURATED) != 0U)
  {
    bStatus = (uint8_t) (bStatus | VOLTAGE_CURRENT_STATUS_CURRENT_SATURATED);
  }

  if ((lFaults & EVENT_FLAGS_MAINTENANCE_INVALID_COMMAND) != 0U)
  {
    bStatus = (uint8_t) (bStatus | VOLTAGE_CURRENT_STATUS_INVALID_COMMAND);
  }

  return bStatus;
}

static void VoltageCurrentSend(void)
{
  tSCurrentMeasurementSnapshot SCurSnap;
  tSVoltageCurrentFrameInputs SFrameIn;
  tSCanFrame SFrame;

  /* Pull the latest RMS snapshot from the ADC adapter and pack the 10-bit
   * currents via the domain service. SObservedImage already holds the
   * 12 voltage-channel states (populated by SignalOutputStatesCheck).
   */
  CurrentMeasurement_GetLatest(&g_CurrentMeasurementPort, &SCurSnap);
  CurrentMeasurement_Pack(&SCurSnap, &SFrameIn.SCurrentWire);
  SFrameIn.SVoltageImage = SObservedImage;
  SFrameIn.bStatus = VoltageCurrentStatusBuild(&SCurSnap);

  SFrame.sStdId = (uint16_t) (FDCAN_SSM_VOLTAGE_CURRENT_1_STD_ID
                              + g_bCardId);
  SFrame.bLen = VOLTAGE_CURRENT_FRAME_BYTES;
  VoltageCurrentFrame_Encode(&SFrameIn, SFrame.abData);

  if (CanBus_SendStd(&g_CanBusPort, CAN_BUS_FDCAN1, &SFrame) == 0U)
  {
    MaintenanceTaskSignal(EVENT_FLAGS_MAINTENANCE_CAN_TX_FAULT);
  }
}

/* Public application code --------------------------------------------------*/

/**
 * @note Called from TIM4 OC ISR (see HAL_TIM_OC_DelayElapsedCallback).
 *       Runs in interrupt context: do NOT use FreeRTOS blocking primitives
 *       (osMutex, osEventFlags without the FromISR variant, etc.). The ISR
 *       is already atomic with respect to task-context readers; the
 *       matching consumer (SignalOutputStatesCheck) masks this ISR via
 *       taskENTER_CRITICAL while it snapshots+zeros the counters.
 */
void MeasurementSGStatesCntrsSet(void)
{
  if (!GPIOGetR1_V())
  {
    SaSignalOutputStateCntrs[0].bOnCntr++;
  }
  else
  {
    SaSignalOutputStateCntrs[0].bOffCntr++;
  }

  if (!GPIOGetY1_V())
  {
    SaSignalOutputStateCntrs[1].bOnCntr++;
  }
  else
  {
    SaSignalOutputStateCntrs[1].bOffCntr++;
  }

  if (!GPIOGetG1_V())
  {
    SaSignalOutputStateCntrs[2].bOnCntr++;
  }
  else
  {
    SaSignalOutputStateCntrs[2].bOffCntr++;
  }

  if (!GPIOGetR2_V())
  {
    SaSignalOutputStateCntrs[3].bOnCntr++;
  }
  else
  {
    SaSignalOutputStateCntrs[3].bOffCntr++;
  }

  if (!GPIOGetY2_V())
  {
    SaSignalOutputStateCntrs[4].bOnCntr++;
  }
  else
  {
    SaSignalOutputStateCntrs[4].bOffCntr++;
  }

  if (!GPIOGetG2_V())
  {
    SaSignalOutputStateCntrs[5].bOnCntr++;
  }
  else
  {
    SaSignalOutputStateCntrs[5].bOffCntr++;
  }

  if (!GPIOGetR3_V())
  {
    SaSignalOutputStateCntrs[6].bOnCntr++;
  }
  else
  {
    SaSignalOutputStateCntrs[6].bOffCntr++;
  }

  if (!GPIOGetY3_V())
  {
    SaSignalOutputStateCntrs[7].bOnCntr++;
  }
  else
  {
    SaSignalOutputStateCntrs[7].bOffCntr++;
  }

  if (!GPIOGetG3_V())
  {
    SaSignalOutputStateCntrs[8].bOnCntr++;
  }
  else
  {
    SaSignalOutputStateCntrs[8].bOffCntr++;
  }

  if (!GPIOGetR4_V())
  {
    SaSignalOutputStateCntrs[9].bOnCntr++;
  }
  else
  {
    SaSignalOutputStateCntrs[9].bOffCntr++;
  }

  if (!GPIOGetY4_V())
  {
    SaSignalOutputStateCntrs[10].bOnCntr++;
  }
  else
  {
    SaSignalOutputStateCntrs[10].bOffCntr++;
  }

  if (!GPIOGetG4_V())
  {
    SaSignalOutputStateCntrs[11].bOnCntr++;
  }
  else
  {
    SaSignalOutputStateCntrs[11].bOffCntr++;
  }
} /* MeasurementSGStatesCntrsSet */

void MeasurementThreadFlagSet(void)
{
  osThreadFlagsSet(MeasurementTaskHandle, THREAD_FLAGS_MEASUREMENT_DONE);
}

/* Public application code --------------------------------------------------*/

/* USER CODE BEGIN Header_vMeasurementTask */
uint32_t lMeasurementStart = 0;
uint32_t lMeasurementFinish = 0;
uint32_t lMeasurementDuration = 0;

/**
 * @brief Function implementing the xSignalMonitorTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_vMeasurementTask */
void MeasurementTaskFunc(void *argument)
{
  /* USER CODE BEGIN vMeasurementTask */
  uint8_t bConsecutiveTimeouts = 0U;

  TaskInit();

  CANSignalOutputFlashConfigCheck();

  GridFreqMeasurementStart();
  lMeasurementStart = HAL_GetTick();

  SignalOutputStatesTimerStart();
  SGCurrentMeasurementStart();

  /* Infinite loop */
  while (pdTRUE)
  {
    uint32_t lFlags = osThreadFlagsWait(THREAD_FLAGS_MEASUREMENT_DONE,
                                        osFlagsWaitAll,
                                        MEASUREMENT_CYCLE_TIMEOUT_MS);

    /* osThreadFlagsWait returns the flags on success or an error bit
     * (>= 0x80000000) on timeout / parameter / resource error. Timeout is
     * the only expected non-success path here.
     */
    if ((lFlags & 0x80000000U) != 0U)
    {
      bConsecutiveTimeouts++;
      if (bConsecutiveTimeouts >= MEASUREMENT_MAX_CONSECUTIVE_TIMEOUTS)
      {
        /* ADC path is wedged. Collapse to a known-safe state and tell the
         * maintenance task. Keep the counter saturated so repeated misses
         * don't wrap; we'll reset on the next good wake.
         */
        SignalOutput_AllOff(&g_SignalOutputPort);
        MaintenanceTaskSignal(EVENT_FLAGS_MAINTENANCE_MEASUREMENT_FAULT);
        bConsecutiveTimeouts = MEASUREMENT_MAX_CONSECUTIVE_TIMEOUTS;
      }

      continue;
    }

    bConsecutiveTimeouts = 0U;

    /* Compute RMS + auto-range + publish the latest ADC half in task
     * context. Used to run inside the DMA ISR (~2 kcycles of float math
     * plus a sqrtf per channel); moving it here keeps the ISR well under
     * 1 μs and stops it blocking FDCAN RX.
     */
    ADCSGCurrentProcessLatestHalf();

    lMeasurementFinish = HAL_GetTick();
    lMeasurementDuration = lMeasurementFinish - lMeasurementStart;
    lMeasurementStart = lMeasurementFinish;

    SignalGroupSwitchStatesSet();
    SignalOutputStatesCheck();
    OutputVerifyStep();

    VoltageCurrentSend();

    if (bCommLedToggleCntr++ == COMM_LED_PERIOD)
    {
      bCommLedToggleCntr = 0;
      GPIOComLEDToggle();
    }

    MaintenanceTaskSignal(EVENT_FLAGS_MAINTENANCE_MEASUREMENT_TASK_ACTIVE);
  }

  /* USER CODE END vMeasurementTask */
} /* MeasurementTaskFunc */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
