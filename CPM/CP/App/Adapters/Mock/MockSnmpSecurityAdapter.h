/* App/Adapters/Mock/MockSnmpSecurityAdapter.h */
#ifndef MOCK_SNMP_SECURITY_ADAPTER_H
#define MOCK_SNMP_SECURITY_ADAPTER_H

#include "Ports/ISnmpSecurityPort.h"

typedef struct
{
  uint8_t strictReleasePolicy;
  uint32_t setV2cCount;
  uint32_t setV3UsernameCount;
  uint32_t setV3CredentialsCount;
  char readCommunity[33];
  char writeCommunity[33];
  char trapCommunity[33];
  char username[33];
  char authPassphrase[21];
  char privPassphrase[21];
} MockSnmpSecurityAdapterCtx_t;

void MockSnmpSecurityAdapterInit(MockSnmpSecurityAdapterCtx_t *ctx);
ISnmpSecurityPort_t MockSnmpSecurityAdapterCreatePort(
  MockSnmpSecurityAdapterCtx_t *ctx);

#endif /* MOCK_SNMP_SECURITY_ADAPTER_H */
