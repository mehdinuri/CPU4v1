/* App/Adapters/STM32/SnmpSecurityAdapter.h
 *
 * STM32-backed SNMP security adapter that persists credential changes through
 * MCS and exposes the Domain-facing ISnmpSecurityPort surface.
 */
#ifndef SNMP_SECURITY_ADAPTER_H
#define SNMP_SECURITY_ADAPTER_H

#include "Ports/ISnmpSecurityPort.h"

typedef struct
{
  uint8_t reserved;
} SnmpSecurityAdapterCtx_t;

void SnmpSecurityAdapterInit(SnmpSecurityAdapterCtx_t *ctx);
ISnmpSecurityPort_t SnmpSecurityAdapterCreatePort(
  SnmpSecurityAdapterCtx_t *ctx);

#endif /* SNMP_SECURITY_ADAPTER_H */
