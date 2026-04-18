/* App/Adapters/STM32/LWIPSNMPBridge.h
 *
 * Central hook from lwIP's SNMP walker into the canonical domain-side
 * NTCIP object directory.
 */
#ifndef LWIP_SNMP_BRIDGE_H
#define LWIP_SNMP_BRIDGE_H

#include "LWIPSNMPAdapter.h"

void LWIPSNMPBridgeBindAdapter(LWIPSNMPAdapterCtx_t *ctx);

#endif /* LWIP_SNMP_BRIDGE_H */
