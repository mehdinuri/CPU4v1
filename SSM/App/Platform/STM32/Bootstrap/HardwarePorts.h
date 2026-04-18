/**
 ******************************************************************************
 * @file    Platform/STM32/Bootstrap/HardwarePorts.h
 * @brief   Global port instances wired by the composition root (main_ssm.c).
 *          Tasks and services read through these. Do not instantiate adapters
 *          anywhere else.
 ******************************************************************************
 */

#ifndef PLATFORM_STM32_BOOTSTRAP_HARDWARE_PORTS_H
#define PLATFORM_STM32_BOOTSTRAP_HARDWARE_PORTS_H

#include "Ports/ISignalOutputPort.h"
#include "Ports/ISignalInputPort.h"
#include "Ports/IWatchdogPort.h"
#include "Ports/ITickPort.h"
#include "Ports/IPersistencePort.h"
#include "Ports/ICurrentMeasurementPort.h"
#include "Ports/ICanBusPort.h"
#include "Ports/ITimerPort.h"
#include "Adapters/STM32/AdcCurrentAdapter.h"
#include "Domain/FlashSyncWatchdog.h"

extern ISignalOutputPort_t g_SignalOutputPort;
extern ISignalInputPort_t g_SignalInputPort;
extern IWatchdogPort_t g_WatchdogPort;
extern ITickPort_t g_TickPort;
extern IPersistencePort_t g_PersistencePort;
extern ICurrentMeasurementPort_t g_CurrentMeasurementPort;
extern ICanBusPort_t g_CanBusPort;
extern ITimerPort_t g_TimerPort;
extern uint8_t g_bCardId;

/* ISR-side handle needed by adc.c to publish new samples directly. */
extern tSAdcCurrentAdapterCtx g_AdcCurrentCtx;

/* Flash-sync deadman. Fed by the CAN parser on every PSM flash-sync frame;
 * queried by the measurement task each cycle. See Domain/FlashSyncWatchdog.h.
 */
extern tSFlashSyncWatchdog g_FlashSyncWatchdog;

#endif /* PLATFORM_STM32_BOOTSTRAP_HARDWARE_PORTS_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
