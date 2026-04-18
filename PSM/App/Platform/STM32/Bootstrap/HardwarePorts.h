/* App/Platform/STM32/Bootstrap/HardwarePorts.h
 *
 * Global port instances created by MainApplication_Init().
 * Each port wraps one concrete STM32 adapter; all hardware access goes
 * through these port handles — never through gpio.c or HAL directly from
 * application or task code.
 *
 * Include this header in any task or module that needs hardware access.
 * Do NOT include the adapter headers directly outside of main_stm32.c.
 */
#ifndef HARDWARE_PORTS_H
#define HARDWARE_PORTS_H

#include "Ports/IIndicatorLEDPort.h"
#include "Ports/ISignalInputPort.h"
#include "Ports/IEepromPort.h"
#include "Ports/ICANTxPort.h"
#include "Ports/ICANRxPort.h"
#include "Ports/IVoltageSensorPort.h"
#include "Ports/IFrequencySensorPort.h"
#include "Ports/IFrequencyCapturePort.h"
#include "Ports/IWatchdogPort.h"

/* One instance per physical hardware resource — initialised in
 * MainApplication_Init() before the scheduler starts. */
extern IIndicatorLEDPort_t       g_indicatorLEDPort;
extern ISignalInputPort_t        g_signalInputPort;
extern IEepromPort_t             g_eepromPort;
extern ICANTxPort_t              g_canTxPort;
extern ICANRxPort_t              g_canRxPort;
extern IVoltageSensorPort_t      g_voltageSensorPort;
extern IFrequencySensorPort_t    g_frequencySensorPort;
extern IFrequencyCapturePort_t   g_frequencyCapturePort;
extern IWatchdogPort_t           g_watchdogPort;

/* Called once from MX_FREERTOS_Init() before the scheduler starts,
 * after all HAL peripherals have been initialised by MX_xxx_Init(). */
void MainApplication_Init(void);

#endif /* HARDWARE_PORTS_H */
