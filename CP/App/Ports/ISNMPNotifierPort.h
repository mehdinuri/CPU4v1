#pragma once

/*
 * App/Ports/ISNMPNotifierPort.h
 *
 * Emit SNMP trap notifications. The Domain calls this when events occur
 * (lamp failure, Detector fault, phase change). The concrete adapter
 * forwards them to the LWIP SNMP agent; the mock silently records them.
 */
#include <stdint.h>
#include <stdbool.h>

/* Trap type codes (NTCIP 1202 alarm types + local extensions). */
typedef enum
{
  SNMP_TRAP_LAMP_FAILURE          = 1,    /* NTCIP 1202 lampFailure        */
  SNMP_TRAP_DETECTOR_FAILURE      = 2,    /* NTCIP 1202 DetectorFailure    */
  SNMP_TRAP_POWER_FAILURE         = 3,    /* NTCIP 1202 powerFailure       */
  SNMP_TRAP_CONFLICT_FAULT        = 4,    /* Green-green or yellow-green   */
  SNMP_TRAP_COMM_FAILURE          = 5,    /* Lost comms with MCS server    */
  SNMP_TRAP_CONTROLLER_STARTUP    = 6,    /* NTCIP 1201 startup notify     */
  SNMP_TRAP_TIMING_PLAN_CHANGE    = 7,    /* Active timing plan changed    */
  SNMP_TRAP_SPECIAL_FUNCTION      = 8,    /* Police/preempt event          */
} SnmpTrapType_t;

typedef struct ISnmpNotifierPort
{
  void *ctx;

  /* Send a trap with an optional 4-byte payload (e.g. SG/Detector index). */
  void (*sendTrap)(void *ctx, SnmpTrapType_t trapType, uint32_t payload);
} ISnmpNotifierPort_t;

static inline void SnmpNotifier_SendTrap(ISnmpNotifierPort_t *p,
                                         SnmpTrapType_t type,
                                         uint32_t payload)
{
  p->sendTrap(p->ctx, type, payload);
}
