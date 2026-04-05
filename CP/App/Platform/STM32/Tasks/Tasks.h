#pragma once

/*
 * Platform/STM32/Tasks/Tasks.h
 *
 * FreeRTOS task function declarations for the STM32H743 platform layer.
 * Each task is a thin wrapper around Domain or adapter functions — no
 * business logic lives here.
 *
 * Tasks are created in main_stm32.c via osThreadNew() after all adapters
 * have been initialised.
 */
#include "cmsis_os2.h"

/* ---------------------------------------------------------------------------
 * Task entry point declarations
 * All follow the CMSIS-RTOS v2 signature: void f(void *argument)
 * ---------------------------------------------------------------------------*/

/**
 * ProgramTask — 100 ms periodic tick.
 * argument: pointer to ProgramCtx_t (cast internally).
 */
void ProgramTask(void *argument);

/**
 * CANRxTask — event-driven, blocks on FDCAN RxFIFO0 notification.
 * Dispatches received frames to DetectorInputAdapter_UpdateFromCAN() and
 * SignalCardAdapter_NotifyAck().
 * argument: unused (adapters accessed through file-scope pointers in
 *           main_stm32.c via an injected CANTaskArgs_t struct).
 */
void CANRxTask(void *argument);

/**
 * CANTxTask — drains a FreeRTOS message queue of CAN frames.
 * argument: osMessageQueueId_t for the CAN Tx queue.
 */
void CANTxTask(void *argument);

/**
 * NetworkTask — LWIP event loop, DHCP, TCP reconnect to MCS server.
 * argument: unused.
 */
void NetworkTask(void *argument);

/**
 * GPSTask — UART5 NMEA sentence parser, feeds RTCAdapter_SetEpoch().
 * argument: unused.
 */
void GPSTask(void *argument);

/**
 * UITask — LCD render + keypad scan, 100 ms period.
 * argument: unused (accesses LCDAdapter and KeypadAdapter through
 *           file-scope pointers in main_stm32.c).
 */
void UITask(void *argument);

/**
 * StorageTask — processes IPersistentStoragePort async write queue.
 * argument: osMessageQueueId_t for the storage request queue.
 */
void StorageTask(void *argument);

/**
 * TimeTask — 1-second loop, syncs RTC from GPS / SNMP.
 * argument: pointer to ISystemClockPort_t.
 */
void TimeTask(void *argument);

/**
 * MaintenanceTask — IWDG watchdog feed + stack HWM logging.
 * argument: unused.
 */
void MaintenanceTask(void *argument);
