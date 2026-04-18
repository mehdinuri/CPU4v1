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
 * g_AdcCurrentCtx is non-static: the ADC ISR in Core/Src/adc.c publishes
 * into it directly (see HardwarePorts.h).
 */
static tSGpioOutputAdapterCtx SGpioOutputCtx;
static tSGpioInputAdapterCtx SGpioInputCtx;
static tSIwdgWatchdogAdapterCtx SIwdgWatchdogCtx;
static tSHalTickAdapterCtx SHalTickCtx;
static tSFlashPersistenceAdapterCtx SFlashPersistenceCtx;
static tSCanBusAdapterCtx SCanBusCtx;
static tSTimerAdapterCtx STimerCtx;
tSAdcCurrentAdapterCtx g_AdcCurrentCtx;

/* Flash-sync deadman instance — plain domain struct, no adapter wrapper. */
tSFlashSyncWatchdog g_FlashSyncWatchdog;

/* Global port instances consumed by Tasks/Services. */
ISignalOutputPort_t g_SignalOutputPort;
ISignalInputPort_t g_SignalInputPort;
IWatchdogPort_t g_WatchdogPort;
ITickPort_t g_TickPort;
IPersistencePort_t g_PersistencePort;
ICurrentMeasurementPort_t g_CurrentMeasurementPort;
ICanBusPort_t g_CanBusPort;
ITimerPort_t g_TimerPort;
uint8_t g_bCardId;

void MainApplication_Init(void)
{
  tSSignalInputSnapshot SInputSnapshot;

  GpioOutputAdapter_Init(&SGpioOutputCtx);
  g_SignalOutputPort = GpioOutputAdapter_CreatePort(&SGpioOutputCtx);

  GpioInputAdapter_Init(&SGpioInputCtx);
  g_SignalInputPort = GpioInputAdapter_CreatePort(&SGpioInputCtx);
  SignalInput_Sample(&g_SignalInputPort, &SInputSnapshot);
  if (SignalCardIdentity_IsValid(SInputSnapshot.bCardId) == 0U)
  {
    Error_Handler();
  }

  g_bCardId = SInputSnapshot.bCardId;

  IwdgWatchdogAdapter_Init(&SIwdgWatchdogCtx);
  g_WatchdogPort = IwdgWatchdogAdapter_CreatePort(&SIwdgWatchdogCtx);

  HalTickAdapter_Init(&SHalTickCtx);
  g_TickPort = HalTickAdapter_CreatePort(&SHalTickCtx);

  FlashPersistenceAdapter_Init(&SFlashPersistenceCtx);
  g_PersistencePort = FlashPersistenceAdapter_CreatePort(&SFlashPersistenceCtx);

  AdcCurrentAdapter_Init(&g_AdcCurrentCtx);
  g_CurrentMeasurementPort = AdcCurrentAdapter_CreatePort(&g_AdcCurrentCtx);

  CanBusAdapter_Init(&SCanBusCtx);
  g_CanBusPort = CanBusAdapter_CreatePort(&SCanBusCtx);

  TimerAdapter_Init(&STimerCtx);
  g_TimerPort = TimerAdapter_CreatePort(&STimerCtx);

  FlashSyncWatchdog_Reset(&g_FlashSyncWatchdog);
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
