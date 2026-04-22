/**
 ******************************************************************************
 * File Name          : can_msg_parser.c
 * Description        : Code for freertos applications
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include <string.h>
#include "task.h"
#include "utilities.h"
#include "gpio.h"
#include "can_msg_parser.h"
#include "measurement.h"
#include "fdcan.h"
#include "storage.h"
#include "Domain/SignalFlashConfig.h"
#include "Domain/FlashSyncWatchdog.h"
#include "Domain/SignalCardIdentity.h"
#include "Domain/CanRxBackpressure.h"
#include "Platform/STM32/Bootstrap/HardwarePorts.h"
/* Private typedef -----------------------------------------------------------*/
typedef enum
{
  OUTPUT_SET_ACTIVE = 0,
  OUTPUT_SET_FLASH
} OutputSetType_e;

/* Private variables ---------------------------------------------------------*/
static uint8_t flashStatus = FALSE;
static uint8_t flashSyncStatus = FALSE;
static OutputGroup_t signalOutputGroups[SIGNAL_GROUPS_PER_SSM];
static volatile uint32_t canRxFaultCount = 0U;
/* Last flash-config we successfully saved; used to skip redundant writes. */
static SignalFlashConfig_t lastSavedFlashCfg;

/* TRUE once we have either successfully loaded the persisted flash config
 * or successfully saved one. While FALSE, SignalOutputFlashConfigSet must
 * not write — a transient read error should not cause us to clobber the
 * valid persisted config with the default (all-steady) runtime state.
 */
static uint8_t flashCfgKnown = FALSE;

/* CAN Rx backpressure state. Reset on any successful enqueue; incremented
 * on alloc/queue-put failure. All access is from the FDCAN1 RX FIFO0 ISR,
 * which is non-reentrant, so no atomic or lock is needed inside.
 */
static CanRxBackpressure_t canRxBackpressure;

/* Threshold above which we escalate silent CAN Rx loss to the sticky
 * EVENT_FLAGS_MAINTENANCE_CAN_RX_OVERRUN flag. At 1 kHz tick and typical
 * CP→SSM command rates, 8 back-to-back drops = well outside normal
 * jitter; anything above this indicates real backpressure.
 */
#define CAN_RX_OVERRUN_THRESHOLD 8U

/* Private function prototypes -----------------------------------------------*/
void vCanMsgParserInit(void);

/* Public typedef -----------------------------------------------------------*/

/* Public define ------------------------------------------------------------*/

/* Public variables ---------------------------------------------------------*/

/* Public function prototypes -----------------------------------------------*/

/* Private application code --------------------------------------------------*/
void CANMsgParserInit(void)
{
  memset(signalOutputGroups, 0, sizeof(signalOutputGroups));
  memset(&lastSavedFlashCfg, 0, sizeof(lastSavedFlashCfg));
  flashCfgKnown = FALSE;
  canRxFaultCount = 0U;
  CanRxBackpressure_Reset(&canRxBackpressure);
}

void CANRxFaultRecord(void)
{
  /* RMW on a plain volatile is not atomic — multiple tasks / ISR paths
   * may increment concurrently and lose counts. Use a single atomic
   * increment so the latched-fault flag stays accurate.
   */
  (void) __atomic_fetch_add(&canRxFaultCount, 1U, __ATOMIC_RELAXED);
}

static void SignalOutputFlashConfigSet(void)
{
  SignalFlashConfig_t newCfg;
  uint8_t groupIdx, outputIdx;

  memset(&newCfg, 0, sizeof(newCfg));

  /* Snapshot runtime isFlashing under a brief critical section so we get
   * a consistent view. The subsequent memcmp / Save work must stay outside
   * the critical section — Save goes through the storage task and can
   * block for up to STORAGE_SYNC_REQ_TIMEOUT_MS.
   */
  taskENTER_CRITICAL();
  for (groupIdx = 0; groupIdx < SIGNAL_GROUPS_PER_SSM; groupIdx++)
  {
    for (outputIdx = 0;
         outputIdx < SIGNAL_OUTPUTS_PER_SIGNAL_GROUP;
         outputIdx++)
    {
      uint8_t idx = (groupIdx * SIGNAL_OUTPUTS_PER_SIGNAL_GROUP) + outputIdx;

      newCfg.isFlashing[idx] =
        signalOutputGroups[groupIdx].signalOutputs[outputIdx].isFlashing
        ? 1U : 0U;
    }
  }

  taskEXIT_CRITICAL();

  /* Do not write while the persisted config is of unknown state — a
   * transient read error earlier would otherwise let the first CP command
   * overwrite a perfectly valid stored config with the runtime default.
   */
  if (flashCfgKnown == FALSE)
  {
    return;
  }

  if (memcmp(&newCfg, &lastSavedFlashCfg, sizeof(newCfg)) != 0)
  {
    if (SignalFlashConfig_Save(&g_persistencePort, &newCfg))
    {
      lastSavedFlashCfg = newCfg;
    }
  }
}

static void FlashStatusSet(uint8_t status)
{
  flashStatus = status;
}

static void FlashSyncStatusSet(uint8_t status)
{
  /* Receipt of ANY PSM FLASH_SYNC frame means the system is currently in
   * flash mode — so raise flashStatus unconditionally. The status
   * argument carries the half-cycle phase bit (on vs off), not the
   * mode flag; it's stored separately in flashSyncStatus and consumed
   * downstream by the flash-output image builder.
   */
  FlashStatusSet(TRUE);
  flashSyncStatus = status;
}

static void SignalOutputSet(uint8_t groupIdx,
                            uint8_t outputIdx,
                            OutputSetType_e eType,
                            uint8_t status)
{
  if (groupIdx < SIGNAL_GROUPS_PER_SSM)
  {
    if (outputIdx < SIGNAL_OUTPUTS_PER_SIGNAL_GROUP)
    {
      switch (eType)
      {
          case OUTPUT_SET_ACTIVE:
          {
            FlashStatusSet(FALSE);
            signalOutputGroups[groupIdx].signalOutputs[outputIdx].
            isActive = status;
            break;
          }

          case OUTPUT_SET_FLASH:
          {
            signalOutputGroups[groupIdx].signalOutputs[outputIdx].
            isFlashing = status;
            break;
          }
      }
    }
  }
}

static void SignalOutputsUpdate(uint8_t *data, OutputSetType_e eSetType)
{
  uint8_t groupIdx, outputIdx;
  uint8_t outputBitIdx = SignalCardIdentity_OutputBitBase(g_cardId);

  /* Writes to signalOutputGroups race with reads from MeasurementTask
   * (CANSignalOutputStateGet / CANSignalOutputFlashStateGet). The work is
   * bounded (12 byte writes) so a single critical section around the whole
   * batch keeps ISR latency low and readers see a consistent commanded
   * image rather than a partially-updated one.
   */
  taskENTER_CRITICAL();
  for (groupIdx = 0; groupIdx < SIGNAL_GROUPS_PER_SSM; groupIdx++)
  {
    for (outputIdx = 0;
         outputIdx < SIGNAL_OUTPUTS_PER_SIGNAL_GROUP;
         outputIdx++)
    {
      uint8_t status = GET_BIT_VALUE(data[outputBitIdx / NUM_BITS_IN_BYTE],
                                     (outputBitIdx % NUM_BITS_IN_BYTE));

      SignalOutputSet(groupIdx, outputIdx, eSetType, status);

      outputBitIdx++;
    }
  }

  taskEXIT_CRITICAL();

  SignalOutputFlashConfigSet();
}

static void CANMsgParse(FdcanRxMsg_t *rxMsg)
{
  if ((rxMsg == NULL)
      || (rxMsg->rxHeader.IdType != FDCAN_STANDARD_ID)
      || (rxMsg->rxHeader.RxFrameType != FDCAN_DATA_FRAME)
      || (rxMsg->rxHeader.DataLength != FDCAN_MAX_DATA_LEN))
  {
    CANRxFaultRecord();
    MaintenanceTaskSignal(EVENT_FLAGS_MAINTENANCE_CAN_RX_FAULT);

    return;
  }

  /* Up-front reject above already guarantees IdType == FDCAN_STANDARD_ID,
   * so dispatch directly on the identifier.
   */
  switch (rxMsg->rxHeader.Identifier)
  {
      case FDCAN_CP_SIGNAL_OUTPUTS_1_STD_ID:
      {
        if (SignalCardIdentity_CommandBank(g_cardId) == 0U)
        {
          SignalOutputsUpdate(rxMsg->data, OUTPUT_SET_ACTIVE);
        }

        break;
      }

      case FDCAN_CP_SIGNAL_OUTPUTS_2_STD_ID:
      {
        if (SignalCardIdentity_CommandBank(g_cardId) == 1U)
        {
          SignalOutputsUpdate(rxMsg->data, OUTPUT_SET_ACTIVE);
        }

        break;
      }

      case FDCAN_CP_FLASH_SIGNALS_1_STD_ID:
      {
        if (SignalCardIdentity_CommandBank(g_cardId) == 0U)
        {
          SignalOutputsUpdate(rxMsg->data, OUTPUT_SET_FLASH);
        }

        break;
      }

      case FDCAN_CP_FLASH_SIGNALS_2_STD_ID:
      {
        if (SignalCardIdentity_CommandBank(g_cardId) == 1U)
        {
          SignalOutputsUpdate(rxMsg->data, OUTPUT_SET_FLASH);
        }

        break;
      }

      case FDCAN_PSM_FLASH_SYNC_1_STD_ID:
      case FDCAN_PSM_FLASH_SYNC_2_STD_ID:
      {
        FlashSyncStatusSet(GET_BIT_VALUE(rxMsg->data[0], 0));

        /* Pet the deadman. If PSM goes silent the measurement
         * task will notice via IsStale() and force outputs off.
         */
        FlashSyncWatchdog_Feed(&g_flashSyncWatchdog,
                               Tick_Now_ms(&g_tickPort));
        break;
      }

      default:
      {
        CANRxFaultRecord();
        MaintenanceTaskSignal(EVENT_FLAGS_MAINTENANCE_CAN_RX_FAULT);
        break;
      }
  } /* switch */
} /* CANMsgParse */

/* Public application code --------------------------------------------------*/
uint8_t CANFlashStatusGet(void)
{
  return flashStatus;
}

uint8_t CANFlashSyncStatusGet(void)
{
  return flashSyncStatus;
}

GPIO_PinState CANSignalOutputStateGet(uint8_t groupIdx, uint8_t outputIdx)
{
  uint8_t value;

  if ((groupIdx >= SIGNAL_GROUPS_PER_SSM)
      || (outputIdx >= SIGNAL_OUTPUTS_PER_SIGNAL_GROUP))
  {
    return GPIO_PIN_RESET;
  }

  taskENTER_CRITICAL();
  value =
    signalOutputGroups[groupIdx].signalOutputs[outputIdx].isActive;
  taskEXIT_CRITICAL();

  return value ? GPIO_PIN_SET : GPIO_PIN_RESET;
}

GPIO_PinState CANSignalOutputFlashStateGet(uint8_t groupIdx,
                                           uint8_t outputIdx)
{
  uint8_t value;

  if ((groupIdx >= SIGNAL_GROUPS_PER_SSM)
      || (outputIdx >= SIGNAL_OUTPUTS_PER_SIGNAL_GROUP))
  {
    return GPIO_PIN_RESET;
  }

  taskENTER_CRITICAL();
  value =
    signalOutputGroups[groupIdx].signalOutputs[outputIdx].isFlashing;
  taskEXIT_CRITICAL();

  return value ? GPIO_PIN_SET : GPIO_PIN_RESET;
}

void CANSignalOutputFlashConfigCheck(void)
{
  SignalFlashConfig_t loaded;
  uint8_t groupIdx, outputIdx;
  uint8_t loadOk;

  /* Load is unbounded (can block on the storage queue) — keep it out of
   * any critical section. The subsequent writes to signalOutputGroups
   * must be under taskENTER_CRITICAL because CANMsgParser/MeasurementTask
   * also read/write those same fields.
   */
  loadOk = SignalFlashConfig_Load(&g_persistencePort, &loaded);

  if (loadOk)
  {
    taskENTER_CRITICAL();
    for (groupIdx = 0; groupIdx < SIGNAL_GROUPS_PER_SSM; groupIdx++)
    {
      for (outputIdx = 0;
           outputIdx < SIGNAL_OUTPUTS_PER_SIGNAL_GROUP;
           outputIdx++)
      {
        uint8_t idx = (groupIdx * SIGNAL_OUTPUTS_PER_SIGNAL_GROUP)
                      + outputIdx;

        signalOutputGroups[groupIdx].signalOutputs[outputIdx].isFlashing
          =
            loaded.isFlashing[idx];
      }
    }

    taskEXIT_CRITICAL();

    lastSavedFlashCfg = loaded;
    flashCfgKnown = TRUE;

    return;
  }

  /* Load failed. Leave the runtime state at the safe default (all steady)
   * and leave the persisted-state cache untouched so SignalOutputFlashConfigSet
   * stays in "unknown" mode and refuses to write. A transient read/backend
   * failure must not erase the last valid persisted program by overwriting
   * it with defaults.
   */
  taskENTER_CRITICAL();
  for (groupIdx = 0; groupIdx < SIGNAL_GROUPS_PER_SSM; groupIdx++)
  {
    for (outputIdx = 0;
         outputIdx < SIGNAL_OUTPUTS_PER_SIGNAL_GROUP;
         outputIdx++)
    {
      signalOutputGroups[groupIdx].signalOutputs[outputIdx].isFlashing =
        0U;
    }
  }

  taskEXIT_CRITICAL();
} /* CANSignalOutputFlashConfigCheck */

static void CANRxRequestDropRecord(void)
{
  CANRxFaultRecord();

  if (CanRxBackpressure_RecordDrop(&canRxBackpressure,
                                   CAN_RX_OVERRUN_THRESHOLD) != 0U)
  {
    /* osEventFlagsSet is idempotent, so repeated calls while still in
     * overrun are cheap. The flag is sticky — cleared only via the
     * maintenance/operator path.
     */
    MaintenanceTaskSignal(EVENT_FLAGS_MAINTENANCE_CAN_RX_OVERRUN);
  }
}

void CANRxRequest(FdcanRxMsg_t *rxMsg)
{
  if (rxMsg == NULL)
  {
    CANRxRequestDropRecord();

    return;
  }

  FdcanRxMsg_t *req =
    (FdcanRxMsg_t *) osMemoryPoolAlloc(CANRxReqsMemPoolHandle,
                                       0);

  if (req != NULL)
  {
    memcpy(req, rxMsg, sizeof(FdcanRxMsg_t));
    if (osMessageQueuePut(CANRxReqsQueueHandle, &req, 0, 0) != osOK)
    {
      osMemoryPoolFree(CANRxReqsMemPoolHandle, req);
      CANRxRequestDropRecord();
    }
    else
    {
      /* Successful enqueue — reset the consecutive-drop streak. */
      CanRxBackpressure_RecordSuccess(&canRxBackpressure);
    }
  }
  else
  {
    CANRxRequestDropRecord();
  }
}

uint8_t CANRxFaultLatched(void)
{
  uint32_t count = __atomic_load_n(&canRxFaultCount, __ATOMIC_RELAXED);

  return (count != 0U) ? 1U : 0U;
}

/* USER CODE BEGIN Header_CANMsgParserTaskFunc */

/**
 * @brief Function implementing the CANMsgParserTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_CANMsgParserTaskFunc */
void CANMsgParserTaskFunc(void *argument)
{
  /* USER CODE BEGIN CANMsgParserTaskFunc */
  UNUSED(argument);

  FdcanRxMsg_t *rxMsg = NULL;

  CANMsgParserInit();

  CANStart(&hfdcan1);

  /* Infinite loop */
  while (pdTRUE)
  {
    if (osMessageQueueGet(CANRxReqsQueueHandle, &rxMsg, NULL,
                          MAINTENANCE_TASK_HEARTBEAT_PERIOD_MS) == osOK)
    {
      CANMsgParse(rxMsg);
      osMemoryPoolFree(CANRxReqsMemPoolHandle, rxMsg);
    }

    MaintenanceTaskSignal(EVENT_FLAGS_MAINTENANCE_CAN_PARSER_TASK_ACTIVE);
  }

  /* USER CODE END CANMsgParserTaskFunc */
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
