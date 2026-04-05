#pragma once

/*
 * App/Adapters/STM32/LWIPSNMPAdapter.h
 *
 * ISnmpNotifierPort concrete implementation for STM32H743.
 * Builds minimal SNMPv2c trap PDUs and forwards them via the LWIP SNMP
 * agent.  Until the LWIP SNMP trap API is fully wired, the last trap type
 * and payload are stored in the context so they can be inspected from a
 * debugger or maintenance task.
 */
#include "Ports/ISNMPNotifierPort.h"

typedef struct
{
  SnmpTrapType_t lastTrap;        /* Most recent trap type sent (debug aid)  */
  uint32_t lastPayload;           /* Most recent trap payload                */
  uint32_t trapCount;             /* Total traps sent since startup          */
} LWIPSNMPAdapterCtx_t;

/** Initialise the adapter context (zero all fields). */
void LWIPSNMPAdapter_Init(LWIPSNMPAdapterCtx_t *ctx);

/** Build an ISnmpNotifierPort_t wired to ctx. */
ISnmpNotifierPort_t LWIPSNMPAdapter_CreatePort(LWIPSNMPAdapterCtx_t *ctx);
