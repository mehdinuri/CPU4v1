/*
 * Platform/STM32/main_stm32.c
 *
 * Dependency injection root for the STM32H743 target.
 *
 * Responsibilities:
 *   1. Declare static adapter context instances (no malloc).
 *   2. Initialise each adapter.
 *   3. Build IPort_t value-type wrappers and inject them into Domain init.
 *   4. Create FreeRTOS Tasks via osThreadNew().
 *   5. Expose MainApplication_Init() — called from Core/Src/main.c
 *      inside the USER CODE BEGIN 2 guard after MX_*_Init() calls.
 *
 * CubeMX-generated HAL handles (hfdcan1, hrtc, huart5, hiwdg1, …) are
 * declared extern here; they are defined in Core/Src/main.c.
 */
#include "Tasks/Tasks.h"

/* --- Adapter headers --- */
#include "Adapters/STM32/SignalCardAdapter.h"
#include "Adapters/STM32/DetectorInputAdapter.h"
#include "Adapters/STM32/RTCAdapter.h"
#include "Adapters/STM32/LWIPSNMPAdapter.h"
#include "Adapters/STM32/FlashStorageAdapter.h"
#include "Adapters/STM32/LCDAdapter.h"
#include "Adapters/STM32/KeypadAdapter.h"
#include "Adapters/STM32/ModemAdapter.h"

/* --- Domain headers --- */
#include "Domain/Intersection/Program.h"

/* --- Port headers --- */
#include "Ports/ISignalOutputPort.h"
#include "Ports/IDetectorInputPort.h"
#include "Ports/ISystemClockPort.h"
#include "Ports/ISNMPNotifierPort.h"
#include "Ports/IPersistentStoragePort.h"
#include "Ports/IDisplayPort.h"
#include "Ports/IUserInputPort.h"
#include "Ports/IModemPort.h"

#ifdef STM32H743xx
#include "stm32h7xx_hal.h"

/* CubeMX-generated HAL peripheral handles. */
extern FDCAN_HandleTypeDef hfdcan1;
extern RTC_HandleTypeDef hrtc;
extern UART_HandleTypeDef huart5;    /* GPS UART */
extern UART_HandleTypeDef huart4;    /* Modem UART */
extern IWDG_HandleTypeDef hiwdg1;
#endif

/* ---------------------------------------------------------------------------
 * Flash sector base address for persistent storage.
 * Adjust to match the linker script — this is Bank 2, Sector 7 on STM32H743.
 * ---------------------------------------------------------------------------*/
#define FLASH_STORAGE_BASE_ADDR  0x081C0000U

/* ---------------------------------------------------------------------------
 * Static adapter context instances — no heap allocation.
 * ---------------------------------------------------------------------------*/
static SignalCardAdapterCtx_t s_sigCtx;
static DetectorInputAdapterCtx_t s_detCtx;
static RTCAdapterCtx_t s_rtcCtx;
static LWIPSNMPAdapterCtx_t s_snmpCtx;
static FlashStorageAdapterCtx_t s_flashCtx;
static LCDAdapterCtx_t s_lcdCtx;
static KeypadAdapterCtx_t s_keypadCtx;
static ModemAdapterCtx_t s_modemCtx;

/* ---------------------------------------------------------------------------
 * Domain context — single static instance.
 * ---------------------------------------------------------------------------*/
static ProgramCtx_t s_progCtx;

/* ---------------------------------------------------------------------------
 * Port value instances (stored statically so pointers remain valid forever).
 * ---------------------------------------------------------------------------*/
static ISignalOutputPort_t s_sigPort;
static IDetectorInputPort_t s_detPort;
static ISystemClockPort_t s_clkPort;
static ISnmpNotifierPort_t s_snmpPort;
static IPersistentStoragePort_t s_flashPort;
static IDisplayPort_t s_lcdPort;
static IUserInputPort_t s_keypadPort;
static IModemPort_t s_modemPort;

/* ---------------------------------------------------------------------------
 * FreeRTOS message queues (static allocation).
 * ---------------------------------------------------------------------------*/
#define CAN_TX_QUEUE_DEPTH      32U
#define STORAGE_QUEUE_DEPTH      8U

/* Forward declarations of queue element types (defined in task .c files).
 * Sizes must match the structs defined in the respective task files. */
#define CAN_TX_FRAME_SIZE   13U   /* sizeof(CANTxFrame_t)      */
#define STORAGE_REQ_SIZE    72U   /* sizeof(StorageRequest_t)  */

static uint8_t s_canTxQueueBuf[CAN_TX_QUEUE_DEPTH * CAN_TX_FRAME_SIZE];
static osMessageQueueAttr_t s_canTxQueueAttr = {
  .name = "CANTxQ",
  .cb_mem = NULL,
  .cb_size = 0U,
  .mq_mem = s_canTxQueueBuf,
  .mq_size = sizeof(s_canTxQueueBuf),
};

static uint8_t s_storageQueueBuf[STORAGE_QUEUE_DEPTH * STORAGE_REQ_SIZE];
static osMessageQueueAttr_t s_storageQueueAttr = {
  .name = "StorageQ",
  .cb_mem = NULL,
  .cb_size = 0U,
  .mq_mem = s_storageQueueBuf,
  .mq_size = sizeof(s_storageQueueBuf),
};

static osMessageQueueId_t s_canTxQueue;
static osMessageQueueId_t s_storageQueue;

/* ---------------------------------------------------------------------------
 * Task argument structs (stored statically; task files define the types).
 * For simplicity, Tasks that need multiple pointers receive a small struct.
 * ---------------------------------------------------------------------------*/

/* CANRxTask arg — mirrors CANTaskArgs_t in CANRxTask.c */
typedef struct
{
  DetectorInputAdapterCtx_t *detCtx;
  SignalCardAdapterCtx_t    *sigCtx;
} CANTaskArgs_t;
static CANTaskArgs_t s_canTaskArgs;

/* StorageTask arg — mirrors StorageTaskArgs_t in StorageTask.c */
typedef struct
{
  IPersistentStoragePort_t *storage;
  osMessageQueueId_t queue;
} StorageTaskArgs_t;
static StorageTaskArgs_t s_storageTaskArgs;

/* UITask arg — mirrors UITaskArgs_t in UITask.c */
typedef struct
{
  IDisplayPort_t       *display;
  IUserInputPort_t     *input;
  const ProgramCtx_t   *Program;
} UITaskArgs_t;
static UITaskArgs_t s_uiTaskArgs;

/* ---------------------------------------------------------------------------
 * FreeRTOS thread attribute tables.
 * Stack sizes in bytes — tune based on actual HWM measurements.
 * ---------------------------------------------------------------------------*/
static const osThreadAttr_t s_ProgramTaskAttr = {
  .name = "ProgramTask",
  .priority = osPriorityHigh,
  .stack_size = 1024U,
};
static const osThreadAttr_t s_canRxTaskAttr = {
  .name = "CANRxTask",
  .priority = osPriorityHigh,
  .stack_size = 512U,
};
static const osThreadAttr_t s_canTxTaskAttr = {
  .name = "CANTxTask",
  .priority = osPriorityAboveNormal,
  .stack_size = 512U,
};
static const osThreadAttr_t s_networkTaskAttr = {
  .name = "NetworkTask",
  .priority = osPriorityNormal,
  .stack_size = 2048U,
};
static const osThreadAttr_t s_gpsTaskAttr = {
  .name = "GPSTask",
  .priority = osPriorityBelowNormal,
  .stack_size = 512U,
};
static const osThreadAttr_t s_uiTaskAttr = {
  .name = "UITask",
  .priority = osPriorityLow,
  .stack_size = 512U,
};
static const osThreadAttr_t s_storageTaskAttr = {
  .name = "StorageTask",
  .priority = osPriorityLow,
  .stack_size = 512U,
};
static const osThreadAttr_t s_timeTaskAttr = {
  .name = "TimeTask",
  .priority = osPriorityLow,
  .stack_size = 256U,
};
static const osThreadAttr_t s_maintenanceTaskAttr = {
  .name = "MaintenanceTask",
  .priority = osPriorityIdle,
  .stack_size = 256U,
};

/* ---------------------------------------------------------------------------
 * MainApplication_Init
 *
 * Call this from Core/Src/main.c inside the USER CODE BEGIN 2 guard,
 * after all MX_*_Init() calls have completed.
 * ---------------------------------------------------------------------------*/
void MainApplication_Init(void)
{
  /* ------------------------------------------------------------------
   * 1. Initialise all adapters.
   * ------------------------------------------------------------------ */
  #ifdef STM32H743xx
  SignalCardAdapter_Init(&s_sigCtx,    &hfdcan1);
  RTCAdapter_Init(&s_rtcCtx,    &hrtc);
  ModemAdapter_Init(&s_modemCtx,  &huart4);
  #else
  SignalCardAdapter_Init(&s_sigCtx,    NULL);
  RTCAdapter_Init(&s_rtcCtx,    NULL);
  ModemAdapter_Init(&s_modemCtx,  NULL);
  #endif

  DetectorInputAdapter_Init(&s_detCtx);
  LWIPSNMPAdapter_Init(&s_snmpCtx);
  FlashStorageAdapter_Init(&s_flashCtx, FLASH_STORAGE_BASE_ADDR);
  LCDAdapter_Init(&s_lcdCtx);
  KeypadAdapter_Init(&s_keypadCtx);

  /* ------------------------------------------------------------------
   * 2. Build port interface value types from adapter contexts.
   * ------------------------------------------------------------------ */
  s_sigPort = SignalCardAdapter_CreatePort(&s_sigCtx);
  s_detPort = DetectorInputAdapter_CreatePort(&s_detCtx);
  s_clkPort = RTCAdapter_CreatePort(&s_rtcCtx);
  s_snmpPort = LWIPSNMPAdapter_CreatePort(&s_snmpCtx);
  s_flashPort = FlashStorageAdapter_CreatePort(&s_flashCtx);
  s_lcdPort = LCDAdapter_CreatePort(&s_lcdCtx);
  s_keypadPort = KeypadAdapter_CreatePort(&s_keypadCtx);
  s_modemPort = ModemAdapter_CreatePort(&s_modemCtx);

  /* ------------------------------------------------------------------
   * 3. Inject Ports into Domain and initialise the Intersection Program.
   * ------------------------------------------------------------------ */
  ProgramInit(&s_progCtx,
              &s_sigPort,
              &s_detPort,
              &s_clkPort,
              &s_snmpPort);

  /* Load configuration from flash storage.
   * TODO: Read ProgramConfig_t from flash via s_flashPort and call
   * ProgramLoadConfig(&s_progCtx, &storedConfig); */

  /* ------------------------------------------------------------------
   * 4. Create FreeRTOS message queues.
   * ------------------------------------------------------------------ */
  s_canTxQueue = osMessageQueueNew(CAN_TX_QUEUE_DEPTH,
                                   CAN_TX_FRAME_SIZE,
                                   &s_canTxQueueAttr);

  s_storageQueue = osMessageQueueNew(STORAGE_QUEUE_DEPTH,
                                     STORAGE_REQ_SIZE,
                                     &s_storageQueueAttr);

  /* ------------------------------------------------------------------
   * 5. Populate task argument structs.
   * ------------------------------------------------------------------ */
  s_canTaskArgs.detCtx = &s_detCtx;
  s_canTaskArgs.sigCtx = &s_sigCtx;

  s_storageTaskArgs.storage = &s_flashPort;
  s_storageTaskArgs.queue = s_storageQueue;

  s_uiTaskArgs.display = &s_lcdPort;
  s_uiTaskArgs.input = &s_keypadPort;
  s_uiTaskArgs.Program = &s_progCtx;

  /* ------------------------------------------------------------------
   * 6. Create FreeRTOS Tasks.
   * ------------------------------------------------------------------ */
  osThreadNew(ProgramTask,     &s_progCtx,         &s_ProgramTaskAttr);
  osThreadNew(CANRxTask,       &s_canTaskArgs,      &s_canRxTaskAttr);
  osThreadNew(CANTxTask,       s_canTxQueue,        &s_canTxTaskAttr);
  osThreadNew(NetworkTask,     NULL,                &s_networkTaskAttr);
  osThreadNew(GPSTask,         &s_clkPort,          &s_gpsTaskAttr);
  osThreadNew(UITask,          &s_uiTaskArgs,       &s_uiTaskAttr);
  osThreadNew(StorageTask,     &s_storageTaskArgs,  &s_storageTaskAttr);
  osThreadNew(TimeTask,        &s_clkPort,          &s_timeTaskAttr);
  osThreadNew(MaintenanceTask, NULL,                &s_maintenanceTaskAttr);

  /* FreeRTOS scheduler is started by the caller (Core/Src/main.c). */
} /* MainApplication_Init */
