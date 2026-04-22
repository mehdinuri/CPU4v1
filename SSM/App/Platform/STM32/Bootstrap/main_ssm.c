/**
 ******************************************************************************
 * @file    Platform/STM32/Bootstrap/main_ssm.c
 * @brief   Composition root: instantiate adapters, wire g_*Port globals.
 *          This file is the only place that sees every adapter type.
 ******************************************************************************
 */

#include "Platform/STM32/Bootstrap/main_ssm.h"
#include "main.h"
#include "Platform/STM32/Bootstrap/HardwarePorts.h"
#include "Adapters/STM32/GpioOutputAdapter.h"
#include "Adapters/STM32/GpioInputAdapter.h"
#include "Adapters/STM32/IwdgWatchdogAdapter.h"
#include "Adapters/STM32/HalTickAdapter.h"
#include "Adapters/STM32/FlashPersistenceAdapter.h"
#include "Adapters/STM32/AdcCurrentAdapter.h"
#include "Adapters/STM32/CanBusAdapter.h"
#include "Adapters/STM32/TimerAdapter.h"
#include "Domain/FlashSyncWatchdog.h"
#include "Domain/SignalCardIdentity.h"

/* Adapter contexts — static storage duration, one instance per port.
 * g_adcCurrentCtx is non-static: the ADC ISR in Core/Src/adc.c publishes
 * into it directly (see HardwarePorts.h).
 */
static GpioOutputAdapterCtx_t gpioOutputCtx;
static GpioInputAdapterCtx_t gpioInputCtx;
static IwdgWatchdogAdapterCtx_t iwdgWatchdogCtx;
static HalTickAdapterCtx_t halTickCtx;
static FlashPersistenceAdapterCtx_t flashPersistenceCtx;
static CanBusAdapterCtx_t canBusCtx;
static TimerAdapterCtx_t timerCtx;
AdcCurrentAdapterCtx_t g_adcCurrentCtx;

/* Flash-sync deadman instance — plain domain struct, no adapter wrapper. */
FlashSyncWatchdog_t g_flashSyncWatchdog;

/* Global port instances consumed by Tasks/Services. */
ISignalOutputPort_t g_signalOutputPort;
ISignalInputPort_t g_signalInputPort;
IWatchdogPort_t g_watchdogPort;
ITickPort_t g_tickPort;
IPersistencePort_t g_persistencePort;
ICurrentMeasurementPort_t g_currentMeasurementPort;
ICanBusPort_t g_canBusPort;
ITimerPort_t g_timerPort;
uint8_t g_cardId;

void MainApplication_Init(void)
{
  SignalInputSnapshot_t inputSnapshot;

  GpioOutputAdapter_Init(&gpioOutputCtx);
  g_signalOutputPort = GpioOutputAdapter_CreatePort(&gpioOutputCtx);

  GpioInputAdapter_Init(&gpioInputCtx);
  g_signalInputPort = GpioInputAdapter_CreatePort(&gpioInputCtx);
  SignalInput_Sample(&g_signalInputPort, &inputSnapshot);
  if (SignalCardIdentity_IsValid(inputSnapshot.cardId) == 0U)
  {
    Error_Handler();
  }

  g_cardId = inputSnapshot.cardId;

  IwdgWatchdogAdapter_Init(&iwdgWatchdogCtx);
  g_watchdogPort = IwdgWatchdogAdapter_CreatePort(&iwdgWatchdogCtx);

  HalTickAdapter_Init(&halTickCtx);
  g_tickPort = HalTickAdapter_CreatePort(&halTickCtx);

  FlashPersistenceAdapter_Init(&flashPersistenceCtx);
  g_persistencePort =
    FlashPersistenceAdapter_CreatePort(&flashPersistenceCtx);

  AdcCurrentAdapter_Init(&g_adcCurrentCtx);
  g_currentMeasurementPort = AdcCurrentAdapter_CreatePort(&g_adcCurrentCtx);

  CanBusAdapter_Init(&canBusCtx);
  g_canBusPort = CanBusAdapter_CreatePort(&canBusCtx);

  TimerAdapter_Init(&timerCtx);
  g_timerPort = TimerAdapter_CreatePort(&timerCtx);

  FlashSyncWatchdog_Reset(&g_flashSyncWatchdog);
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
