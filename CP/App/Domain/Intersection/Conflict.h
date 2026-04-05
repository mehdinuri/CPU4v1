#pragma once

/*
 * App/Domain/Intersection/Conflict.h
 *
 * Conflict detection and clearance enforcement between signal groups.
 *
 * Safety rule: two signal groups that Conflict in the configuration
 * MUST NEVER both be in a non-red state simultaneously. If a violation
 * is detected, the controller transitions to CTRL_STATE_ALL_RED and
 * emits an SNMP trap.
 */
#include "Types.h"
#include "Ports/ISNMPNotifierPort.h"

/**
 * Check all active signal groups for simultaneous green/yellow Conflicts.
 *
 * @param sgConfigs     Signal group configuration array
 * @param sgRuntimes    Signal group runtime array
 * @param sgCount       Number of signal groups configured
 * @param snmpNotifier  Port to emit Conflict trap on detection
 * @return              CONFLICT_NONE if safe; non-zero Conflict type if fault
 */
ConflictType_t Conflict_Check(const SignalGroupConfig_t  *sgConfigs,
                              const SignalGroupRuntime_t *sgRuntimes,
                              uint8_t sgCount,
                              ISnmpNotifierPort_t        *snmpNotifier);

/**
 * Return the clearance interval (seconds) required between closing SG A
 * and opening SG B. Returns 0 if A and B have no Conflict.
 */
uint8_t Conflict_GetClearanceSeconds(const SignalGroupConfig_t *sgA,
                                     uint8_t sgBIndex);

/**
 * Return true if SG A and SG B have a configured Conflict.
 */
bool Conflict_Exists(const SignalGroupConfig_t *sgA, uint8_t sgBIndex);
