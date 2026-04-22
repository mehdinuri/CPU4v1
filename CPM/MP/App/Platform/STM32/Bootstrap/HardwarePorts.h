/* App/Platform/STM32/Bootstrap/HardwarePorts.h
 *
 * Global port instances wired by MainApplication_Init() before the
 * RTOS scheduler is started. All hardware access from application or
 * task code goes through these port handles - never through HAL
 * directly from Domain or Tasks.
 */
#ifndef HARDWARE_PORTS_H
#define HARDWARE_PORTS_H

#include "Ports/IControlBusPort.h"
#include "Ports/IFieldBusPort.h"
#include "Ports/IPersistencePort.h"
#include "Ports/IRealtimeClockPort.h"
#include "Ports/ISafetyRelayPort.h"
#include "Ports/IStatusLEDPort.h"
#include "Ports/ISystemMonitorPort.h"
#include "Ports/IUnitAlarmPort.h"
#include "Ports/IWatchdogPort.h"

extern IControlBusPort_t g_controlBusPort;
extern IFieldBusPort_t g_fieldBusPort;
extern ISafetyRelayPort_t g_safetyRelayPort;
extern IStatusLEDPort_t g_statusLEDPort;
extern IPersistencePort_t g_persistencePort;
extern IRealtimeClockPort_t g_rtcPort;
extern ISystemMonitorPort_t g_systemMonitorPort;
extern IWatchdogPort_t g_watchdogPort;
extern IUnitAlarmPort_t g_unitAlarmPort;

#endif /* HARDWARE_PORTS_H */
