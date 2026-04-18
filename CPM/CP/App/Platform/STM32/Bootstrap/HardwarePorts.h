/* App/Platform/STM32/Core/HardwarePorts.h
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

#include "Ports/IHeaterPort.h"
#include "Ports/IPowerMonitorPort.h"
#include "Ports/IRelayPort.h"
#include "Ports/IDoorSensorPort.h"
#include "Ports/IStatusLEDPort.h"
#include "Ports/IUserInputPort.h"
#include "Ports/IDisplayPort.h"
#include "Ports/IRealtimeClockPort.h"
#include "Ports/ISerialPort.h"
#include "Ports/IModemPort.h"
#include "Ports/IOutputDriverPort.h"
#include "Ports/IMmuPort.h"
#include "Ports/IModuleBusPort.h"
#include "Ports/IUnitAlarmPort.h"
#include "Ports/IUnitInputPort.h"
#include "Ports/IUnitClockPort.h"

/* One instance per physical hardware resource — initialised in
 * MainApplication_Init() before the scheduler starts. */
extern IHeaterPort_t g_heaterPort;
extern IPowerMonitorPort_t g_powerMonitorPort;
extern IRelayPort_t g_relayPort;
extern IDoorSensorPort_t g_doorPort;
extern IStatusLEDPort_t g_commLEDPort;
extern IUserInputPort_t g_keypadPort;
extern IDisplayPort_t g_lcdPort;
extern IRealtimeClockPort_t g_rtcPort;
extern IOutputDriverPort_t g_outputDriverPort;
extern IMmuPort_t g_mmuPort;
extern IModuleBusPort_t g_moduleBusPort;
extern IUnitAlarmPort_t g_unitAlarmPort;
extern IUnitInputPort_t g_unitInputPort;
extern IUnitClockPort_t g_unitClockPort;

/* Serial port instances — one per physical UART. */
extern ISerialPort_t g_modemPort;       /* UART4  — GPRS modem           */
extern ISerialPort_t g_internalGpsPort; /* UART8  — internal GPS (on PCB) */
extern ISerialPort_t g_auxSerialPort;   /* USART2 — external GPS or UI    */
extern ISerialPort_t g_wifiPort;        /* UART5  — WiFi placeholder      */

/* Active modem driver — selected at boot by WireModemDriver() in
 * main_stm32.c from the module type stored in EEPROM. */
extern IModemPort_t g_modemDriverPort;

#endif /* HARDWARE_PORTS_H */
