/*
 * App/Adapters/STM32/LWIPSNMPAdapter.c
 *
 * ISnmpNotifierPort implementation — LWIP SNMPv2c trap sender.
 *
 * The sendTrap() implementation records the trap locally (always) and then
 * calls the LWIP snmp_send_trap() API (TODO stub — requires the OID tree
 * and varbind list to be populated once the NTCIP MIB is registered).
 */
#include "LWIPSNMPAdapter.h"
#include <string.h>

/* --------------------------------------------------------------------------
 * Forward declarations
 * --------------------------------------------------------------------------*/
static void SNMP_SendTrap(void *ctx, SnmpTrapType_t trapType, uint32_t payload);

/* --------------------------------------------------------------------------
 * Public API
 * --------------------------------------------------------------------------*/

void LWIPSNMPAdapter_Init(LWIPSNMPAdapterCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
}

ISnmpNotifierPort_t LWIPSNMPAdapter_CreatePort(LWIPSNMPAdapterCtx_t *ctx)
{
  ISnmpNotifierPort_t port;

  port.ctx = ctx;
  port.sendTrap = SNMP_SendTrap;

  return port;
}

/* --------------------------------------------------------------------------
 * Port callback
 * --------------------------------------------------------------------------*/

static void SNMP_SendTrap(void *vctx, SnmpTrapType_t trapType, uint32_t payload)
{
  LWIPSNMPAdapterCtx_t *ctx = (LWIPSNMPAdapterCtx_t *) vctx;

  /* Always record the last trap for diagnostics. */
  ctx->lastTrap = trapType;
  ctx->lastPayload = payload;
  ctx->trapCount++;

  #ifdef STM32H743xx

  /* TODO: HAL impl — build and dispatch LWIP SNMPv2c trap PDU.
   *
   * Steps needed:
   *   1. Look up the enterprise OID for this trapType from the NTCIP MIB
   *      registration (App/Domain/NTCIP/).
   *   2. Build a struct snmp_varbind list:
   *        - sysUpTime (OID 1.3.6.1.2.1.1.3.0)
   *        - snmpTrapOID (OID 1.3.6.1.6.3.1.1.4.1.0)
   *        - payload varbind (enterprise-specific OID, value = payload)
   *   3. Call snmp_send_trap() or snmp_trap_dst_enable() as appropriate
   *      for the LWIP SNMP agent version in use.
   *
   * Example (pseudo-code, adjust to actual LWIP SNMP API):
   *   struct snmp_obj_id trapOid = { ... };
   *   snmp_send_trap(&trapOid, SNMP_GENTRAP_ENTERPRISESPC, trapType);
   */
  (void) trapType;
  (void) payload;
  #endif
}
