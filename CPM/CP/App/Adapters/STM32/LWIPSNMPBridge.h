/* App/Adapters/STM32/LWIPSNMPBridge.h
 *
 * Central hook from lwIP's SNMP walker into the canonical domain-side
 * NTCIP object directory.
 */
#ifndef LWIP_SNMP_BRIDGE_H
#define LWIP_SNMP_BRIDGE_H

#include "LWIPSNMPAdapter.h"
#include "lwip/apps/snmp_core.h"

void LWIPSNMPBridgeBindAdapter(LWIPSNMPAdapterCtx_t *ctx);
u8_t LWIPSNMPBridgeGetManagedState(const u32_t *oid, u8_t oidLen);
u8_t LWIPSNMPBridgeFindNextManagedOid(const u32_t *startOid,
                                      u8_t startOidLen,
                                      const u32_t *subtreeOid,
                                      u8_t subtreeOidLen,
                                      struct snmp_obj_id *nextOid);
void LWIPSNMPBridgeBindNodeInstance(const u32_t *oid,
                                    u8_t oidLen,
                                    struct snmp_node_instance *nodeInstance);

#endif /* LWIP_SNMP_BRIDGE_H */
