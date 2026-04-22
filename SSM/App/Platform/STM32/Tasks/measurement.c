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
#include "Domain/SignalDiagnostics.h"
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
static uint8_t commLedToggleCntr = 0;

static SignalOutputStateCntr_t signalOutputStateCntrs[SIGNAL_OUTPUTS_PER_SSM];

/* OutputVerify plumbing. Each cycle commands the 12 outputs via
 * SignalGroupSwitchStatesSet (captured into pendingCommandedImage), then
 * observes what actually happened via SignalOutputStatesCheck (into
 * observedImage). Because the ISR-accumulated counters reflect the period
 * *before* the most recent command took effect, the verify compares the
 * *previous* command (lastCommandedImage) against the current observation,
 * then promotes pending → last.
 *
 * observedImage is also the voltage-side input to the outgoing CAN frame
 * (VoltageCurrentFrame_Encode), so it serves double duty.
 */
static SignalOutputImage_t pendingCommandedImage;
static SignalOutputImage_t lastCommandedImage;
static SignalOutputImage_t observedImage;
static OutputVerifyState_t outputVerify;
static SignalDiagnosticsState_t diagnostics;
static uint8_t verifyArmed = 0U;   /* first cycle has no "last" yet */

/* Private function prototypes -----------------------------------------------*/

/* Public typedef -----------------------------------------------------------*/

/* Public define ------------------------------------------------------------*/

/* Public variables ---------------------------------------------------------*/

/* Public function prototypes -----------------------------------------------*/

/* Private application code --------------------------------------------------*/
static void SignalOutputStateCntrsInit(void)
{
  memset(signalOutputStateCntrs, 0, sizeof(signalOutputStateCntrs));
}

static void TaskInit(void)
{
  commLedToggleCntr = 0;

  memset(&pendingCommandedImage, 0, sizeof(pendingCommandedImage));
  memset(&lastCommandedImage, 0, sizeof(lastCommandedImage));
  memset(&observedImage, 0, sizeof(observedImage));
  OutputVerify_Reset(&outputVerify);
  SignalDiagnostics_Reset(&diagnostics);
  verifyArmed = 0U;

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
  SignalOutputStateCntr_t snapshot[SIGNAL_OUTPUTS_PER_SSM];
  uint8_t i;

  /* Snapshot the counters under a brief critical section. The TIM4 OC ISR
   * (MeasurementSGStatesCntrsSet) writes to signalOutputStateCntrs every
   * ~1 ms; we must stop it from racing us between the read and the zero.
   * taskENTER_CRITICAL masks the ISR via BASEPRI for the few μs this takes —
   * correct primitive for task-vs-ISR shared counters (osMutex would have
   * been UB from the ISR side).
   */
  taskENTER_CRITICAL();
  memcpy(snapshot, signalOutputStateCntrs, sizeof(snapshot));
  SignalOutputStateCntrsInit();
  taskEXIT_CRITICAL();

  for (i = 0U; i < SIGNAL_OUTPUTS_PER_SSM; i++)
  {
    observedImage.channels[i] =
      SignalOutput_IsActive(snapshot[i].onCntr, snapshot[i].offCntr);
  }
} /* SignalOutputStatesCheck */

static void OutputVerifyStep(void)
{
  /* The counters we just folded into observedImage were accumulated while
   * lastCommandedImage was the active command (the *previous* cycle's).
   * Verify against that; then promote pendingCommandedImage → "last" so
   * the next cycle's verify uses the right reference.
   *
   * Skip the very first cycle: lastCommandedImage is all-zeros then, but
   * so is observedImage (no samples yet), so technically it would match —
   * the verifyArmed guard keeps the semantics explicit regardless.
   */
  if (verifyArmed != 0U)
  {
    OutputVerify_Step(&outputVerify, &lastCommandedImage, &observedImage);
    if (outputVerify.faultActive != 0U)
    {
      /* Hardened Fault Handling: Bypass task latency and immediately drop
       * all signals to Dark. External MMU will trip on Red Fail/Loss of Signal.
       */
      SignalOutput_AllOff(&g_signalOutputPort);
      MaintenanceTaskSignal(EVENT_FLAGS_MAINTENANCE_OUTPUT_VERIFY_FAULT);
    }
  }

  lastCommandedImage = pendingCommandedImage;
  verifyArmed = 1U;
}

static void SignalGroupSwitchStatesSet(void)
{
  SignalOutputBuildInputs_t inputs;
  uint8_t group;
  uint8_t output;
  uint8_t idx;

  /* Refuse to command signals if a hardware fault has been latched. */
  if (outputVerify.faultActive != 0U)
  {
    SignalOutput_AllOff(&g_signalOutputPort);
    memset(&pendingCommandedImage, 0, sizeof(pendingCommandedImage));

    return;
  }

  inputs.flashActive = CANFlashStatusGet();
  inputs.flashSyncActive = CANFlashSyncStatusGet();

  /* Flash-sync deadman: if we're in flash mode but the PSM sync source has
   * gone silent, collapse to the safe (all-off) state instead of trusting a
   * stale flashSyncStatus. Raise a maintenance fault bit so the condition
   * is recorded before the hardware watchdog eventually resets the board.
   *
   * In non-flash mode the deadman doesn't apply — the sync frames aren't
   * expected to arrive at all in that mode.
   */
  if ((inputs.flashActive != 0U)
      && FlashSyncWatchdog_IsStale(&g_flashSyncWatchdog,
                                   Tick_Now_ms(&g_tickPort),
                                   FLASH_SYNC_STALE_TIMEOUT_MS))
  {
    SignalOutput_AllOff(&g_signalOutputPort);
    MaintenanceTaskSignal(EVENT_FLAGS_MAINTENANCE_FLASHSYNC_STALE);
    memset(&pendingCommandedImage, 0, sizeof(pendingCommandedImage));

    return;
  }

  for (group = 0U; group < SIGNAL_GROUPS_PER_SSM; group++)
  {
    for (output = 0U; output < SIGNAL_OUTPUTS_PER_SIGNAL_GROUP; output++)
    {
      idx = (uint8_t) ((group * SIGNAL_OUTPUTS_PER_SIGNAL_GROUP) + output);
      inputs.runChannels[idx] = (uint8_t) CANSignalOutputStateGet(group,
                                                                  output);
      inputs.flashChannels[idx] =
        (uint8_t) CANSignalOutputFlashStateGet(group,
                                               output);
    }
  }

  SignalOutputImageBuilder_Build(&inputs, &pendingCommandedImage);

  SignalOutput_Apply(&g_signalOutputPort, &pendingCommandedImage);
} /* SignalGroupSwitchStatesSet */

static uint8_t VoltageCurrentStatusBuild(
  const CurrentMeasurementSnapshot_t *curSnap)
{
  uint8_t status = 0U;
  uint32_t faults = MaintenanceTaskFaultsGet();

  if (CANRxFaultLatched() != 0U)
  {
    MaintenanceTaskSignal(EVENT_FLAGS_MAINTENANCE_CAN_RX_FAULT);
    faults = (uint32_t) (faults | EVENT_FLAGS_MAINTENANCE_CAN_RX_FAULT);
  }

  if (CANTxFaultLatched() != 0U)
  {
    faults = (uint32_t) (faults | EVENT_FLAGS_MAINTENANCE_CAN_TX_FAULT);
  }

  if (StorageFaultLatched() != 0U)
  {
    faults = (uint32_t) (faults | EVENT_FLAGS_MAINTENANCE_STORAGE_FAULT);
  }

  if ((faults & EVENT_FLAGS_MAINTENANCE_MEASUREMENT_FAULT) != 0U)
  {
    status = (uint8_t) (status | VOLTAGE_CURRENT_STATUS_MEASUREMENT_FAULT);
  }

  if ((faults & EVENT_FLAGS_MAINTENANCE_FLASHSYNC_STALE) != 0U)
  {
    status = (uint8_t) (status | VOLTAGE_CURRENT_STATUS_FLASHSYNC_STALE);
  }

  if ((faults & EVENT_FLAGS_MAINTENANCE_OUTPUT_VERIFY_FAULT) != 0U)
  {
    status = (uint8_t) (status | VOLTAGE_CURRENT_STATUS_OUTPUT_VERIFY_FAULT);
  }

  if ((faults & EVENT_FLAGS_MAINTENANCE_CAN_RX_FAULT) != 0U)
  {
    status = (uint8_t) (status | VOLTAGE_CURRENT_STATUS_CAN_RX_FAULT);
  }

  if ((faults & EVENT_FLAGS_MAINTENANCE_CAN_TX_FAULT) != 0U)
  {
    status = (uint8_t) (status | VOLTAGE_CURRENT_STATUS_CAN_TX_FAULT);
  }

  if ((faults & EVENT_FLAGS_MAINTENANCE_STORAGE_FAULT) != 0U)
  {
    status = (uint8_t) (status | VOLTAGE_CURRENT_STATUS_STORAGE_FAULT);
  }

  if ((curSnap->status & CURRENT_MEASUREMENT_STATUS_SATURATED) != 0U)
  {
    status = (uint8_t) (status | VOLTAGE_CURRENT_STATUS_CURRENT_SATURATED);
  }

  if ((faults & EVENT_FLAGS_MAINTENANCE_LAMP_OUT_FAULT) != 0U)
  {
    status = (uint8_t) (status | VOLTAGE_CURRENT_STATUS_LAMP_OUT_FAULT);
  }

  return status;
} /* VoltageCurrentStatusBuild */

static void VoltageCurrentSend(const CurrentMeasurementSnapshot_t *curSnap)
{
  VoltageCurrentFrameInputs_t frameIn;
  CanFrame_t frame;

  /* pack the 10-bit currents via the domain service. observedImage
   * already holds the 12 voltage-channel states.
   */
  CurrentMeasurement_Pack(curSnap, &frameIn.currentWire);
  frameIn.voltageImage = observedImage;
  frameIn.status = VoltageCurrentStatusBuild(curSnap);

  frame.stdId = (uint16_t) (FDCAN_SSM_VOLTAGE_CURRENT_1_STD_ID
                            + g_cardId);
  frame.len = VOLTAGE_CURRENT_FRAME_BYTES;
  VoltageCurrentFrame_Encode(&frameIn, frame.abData);

  if (CanBus_SendStd(&g_canBusPort, CAN_BUS_FDCAN1, &frame) == 0U)
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
    signalOutputStateCntrs[0].onCntr++;
  }
  else
  {
    signalOutputStateCntrs[0].offCntr++;
  }

  if (!GPIOGetY1_V())
  {
    signalOutputStateCntrs[1].onCntr++;
  }
  else
  {
    signalOutputStateCntrs[1].offCntr++;
  }

  if (!GPIOGetG1_V())
  {
    signalOutputStateCntrs[2].onCntr++;
  }
  else
  {
    signalOutputStateCntrs[2].offCntr++;
  }

  if (!GPIOGetR2_V())
  {
    signalOutputStateCntrs[3].onCntr++;
  }
  else
  {
    signalOutputStateCntrs[3].offCntr++;
  }

  if (!GPIOGetY2_V())
  {
    signalOutputStateCntrs[4].onCntr++;
  }
  else
  {
    signalOutputStateCntrs[4].offCntr++;
  }

  if (!GPIOGetG2_V())
  {
    signalOutputStateCntrs[5].onCntr++;
  }
  else
  {
    signalOutputStateCntrs[5].offCntr++;
  }

  if (!GPIOGetR3_V())
  {
    signalOutputStateCntrs[6].onCntr++;
  }
  else
  {
    signalOutputStateCntrs[6].offCntr++;
  }

  if (!GPIOGetY3_V())
  {
    signalOutputStateCntrs[7].onCntr++;
  }
  else
  {
    signalOutputStateCntrs[7].offCntr++;
  }

  if (!GPIOGetG3_V())
  {
    signalOutputStateCntrs[8].onCntr++;
  }
  else
  {
    signalOutputStateCntrs[8].offCntr++;
  }

  if (!GPIOGetR4_V())
  {
    signalOutputStateCntrs[9].onCntr++;
  }
  else
  {
    signalOutputStateCntrs[9].offCntr++;
  }

  if (!GPIOGetY4_V())
  {
    signalOutputStateCntrs[10].onCntr++;
  }
  else
  {
    signalOutputStateCntrs[10].offCntr++;
  }

  if (!GPIOGetG4_V())
  {
    signalOutputStateCntrs[11].onCntr++;
  }
  else
  {
    signalOutputStateCntrs[11].offCntr++;
  }
} /* MeasurementSGStatesCntrsSet */

void MeasurementThreadFlagSet(void)
{
  osThreadFlagsSet(MeasurementTaskHandle, THREAD_FLAGS_MEASUREMENT_DONE);
}

/* Public application code --------------------------------------------------*/

/* USER CODE BEGIN Header_vMeasurementTask */
uint32_t measurementStart = 0;
uint32_t measurementFinish = 0;
uint32_t measurementDuration = 0;

/**
 * @brief Function implementing the xSignalMonitorTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_vMeasurementTask */
void MeasurementTaskFunc(void *argument)
{
  /* USER CODE BEGIN vMeasurementTask */
  uint8_t consecutiveTimeouts = 0U;

  TaskInit();

  CANSignalOutputFlashConfigCheck();

  GridFreqMeasurementStart();
  measurementStart = HAL_GetTick();

  SignalOutputStatesTimerStart();
  SGCurrentMeasurementStart();

  /* Infinite loop */
  while (pdTRUE)
  {
    uint32_t flags = osThreadFlagsWait(THREAD_FLAGS_MEASUREMENT_DONE,
                                       osFlagsWaitAll,
                                       MEASUREMENT_CYCLE_TIMEOUT_MS);

    /* osThreadFlagsWait returns the flags on success or an error bit
     * (>= 0x80000000) on timeout / parameter / resource error. Timeout is
     * the only expected non-success path here.
     */
    if ((flags & 0x80000000U) != 0U)
    {
      consecutiveTimeouts++;
      if (consecutiveTimeouts >= MEASUREMENT_MAX_CONSECUTIVE_TIMEOUTS)
      {
        /* ADC path is wedged. Collapse to a known-safe state and tell the
         * maintenance task. Keep the counter saturated so repeated misses
         * don't wrap; we'll reset on the next good wake.
         */
        SignalOutput_AllOff(&g_signalOutputPort);
        MaintenanceTaskSignal(EVENT_FLAGS_MAINTENANCE_MEASUREMENT_FAULT);
        consecutiveTimeouts = MEASUREMENT_MAX_CONSECUTIVE_TIMEOUTS;
      }

      continue;
    }

    consecutiveTimeouts = 0U;

    /* Compute RMS + auto-range + publish the latest ADC half in task
     * context. Used to run inside the DMA ISR (~2 kcycles of float math
     * plus a sqrtf per channel); moving it here keeps the ISR well under
     * 1 μs and stops it blocking FDCAN RX.
     */
    ADCSGCurrentProcessLatestHalf();

    measurementFinish = HAL_GetTick();
    measurementDuration = measurementFinish - measurementStart;
    measurementStart = measurementFinish;

    SignalGroupSwitchStatesSet();
    SignalOutputStatesCheck();
    OutputVerifyStep();

    CurrentMeasurementSnapshot_t curSnap;

    CurrentMeasurement_GetLatest(&g_currentMeasurementPort, &curSnap);

    if (SignalDiagnostics_Step(&diagnostics, &observedImage,
                               &curSnap) != 0U)
    {
      MaintenanceTaskSignal(EVENT_FLAGS_MAINTENANCE_LAMP_OUT_FAULT);
    }

    VoltageCurrentSend(&curSnap);

    if (commLedToggleCntr++ == COMM_LED_PERIOD)
    {
      commLedToggleCntr = 0;
      GPIOComLEDToggle();
    }

    MaintenanceTaskSignal(EVENT_FLAGS_MAINTENANCE_MEASUREMENT_TASK_ACTIVE);
  }

  /* USER CODE END vMeasurementTask */
} /* MeasurementTaskFunc */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
