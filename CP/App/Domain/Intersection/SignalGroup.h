#pragma once

/*
 * App/Domain/Intersection/SignalGroup.h
 *
 * Signal group (individual traffic movement) state machine.
 *
 * State transitions:
 *   CLOSED → OPENING → OPEN → GREEN_FLASH → CLOSING → CLOSED
 *
 * The state machine drives lamp output calls through ISignalOutputPort.
 */
#include "Types.h"
#include "Ports/ISignalOutputPort.h"
#include "Ports/ISNMPNotifierPort.h"

/**
 * Reset a signal group runtime to the CLOSED (red) state.
 */
void SG_Reset(SignalGroupRuntime_t *rt);

/**
 * Tick one 100 ms period. Updates state timers and emits lamp state
 * changes via signalOut when transitions occur.
 *
 * @param sgIdx        Signal group index (for output ID calculation)
 * @param rt           Mutable runtime state
 * @param cfg          Immutable configuration
 * @param signalOut    Port to emit lamp state changes
 */
void SG_Tick(uint8_t sgIdx,
             SignalGroupRuntime_t      *rt,
             const SignalGroupConfig_t *cfg,
             ISignalOutputPort_t       *signalOut);

/**
 * Command the signal group to begin opening (transition toward green).
 * This starts the OPENING sub-state timer.
 */
void SG_Open(uint8_t sgIdx,
             SignalGroupRuntime_t      *rt,
             const SignalGroupConfig_t *cfg,
             ISignalOutputPort_t       *signalOut);

/**
 * Command the signal group to begin closing (transition toward red).
 * This starts GREEN_FLASH (if configured), then CLOSING.
 */
void SG_Close(uint8_t sgIdx,
              SignalGroupRuntime_t      *rt,
              const SignalGroupConfig_t *cfg,
              ISignalOutputPort_t       *signalOut);

/**
 * Return true if the signal group has fully closed (red, no clearance pending).
 */
bool SG_IsClosed(const SignalGroupRuntime_t *rt);

/**
 * Return true if the signal group is fully open (green).
 */
bool SG_IsOpen(const SignalGroupRuntime_t *rt);

/**
 * Update lamp fault status from SSM measurement data.
 * Fires SNMP trap if a critical fault is newly detected.
 */
void SG_UpdateFaults(uint8_t sgIdx,
                     SignalGroupRuntime_t      *rt,
                     const SignalGroupConfig_t *cfg,
                     bool redBroken,
                     bool yellowBroken,
                     bool greenBroken,
                     ISnmpNotifierPort_t        *snmpNotifier);
