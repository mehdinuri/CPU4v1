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

extern ISignalOutputPort_t g_signalOutputPort;
extern ISignalInputPort_t g_signalInputPort;
extern IWatchdogPort_t g_watchdogPort;
extern ITickPort_t g_tickPort;
extern IPersistencePort_t g_persistencePort;
extern ICurrentMeasurementPort_t g_currentMeasurementPort;
extern ICanBusPort_t g_canBusPort;
extern ITimerPort_t g_timerPort;
extern uint8_t g_cardId;

/* ISR-side handle needed by adc.c to publish new samples directly. */
extern AdcCurrentAdapterCtx_t g_adcCurrentCtx;

/* Flash-sync deadman. Fed by the CAN parser on every PSM flash-sync frame;
 * queried by the measurement task each cycle. See Domain/FlashSyncWatchdog.h.
 */
extern FlashSyncWatchdog_t g_flashSyncWatchdog;

#endif /* PLATFORM_STM32_BOOTSTRAP_HARDWARE_PORTS_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
