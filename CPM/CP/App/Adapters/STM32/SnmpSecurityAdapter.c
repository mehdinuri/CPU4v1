/* App/Adapters/STM32/SnmpSecurityAdapter.c */
#include "SnmpSecurityAdapter.h"

#include <string.h>

#include "MCS.h"

static uint8_t AdapterGetStrictReleasePolicy(void *ctx)
{
  (void) ctx;

#if CP_SNMP_STRICT_RELEASE
  return 1U;
#else
  return 0U;
#endif
}

static uint8_t AdapterGetSnmpV3Username(void *ctx, char *dst, size_t dstSize)
{
  (void) ctx;

  if ((dst == NULL) || (dstSize == 0U))
  {
    return 0U;
  }

  (void) memset(dst, 0, dstSize);
  if (MCSGetSNMPv3Username() == NULL)
  {
    return 0U;
  }

  strncpy(dst, MCSGetSNMPv3Username(), dstSize - 1U);
  dst[dstSize - 1U] = '\0';
  return 1U;
}

static uint8_t AdapterSetSnmpV2cCommunities(void *ctx,
                                            const char *readCommunity,
                                            const char *writeCommunity,
                                            const char *trapCommunity)
{
  (void) ctx;

  return MCSSetSNMPCommunities(readCommunity, writeCommunity, trapCommunity);
}

static uint8_t AdapterSetSnmpV3Username(void *ctx, const char *username)
{
  (void) ctx;

  return MCSSetSNMPv3Username(username);
}

static uint8_t AdapterSetSnmpV3Credentials(void *ctx,
                                           const char *username,
                                           const char *authPassphrase,
                                           const char *privPassphrase)
{
  (void) ctx;

  return MCSSetSNMPv3Credentials(username, authPassphrase, privPassphrase);
}

void SnmpSecurityAdapterInit(SnmpSecurityAdapterCtx_t *ctx)
{
  if (ctx != NULL)
  {
    ctx->reserved = 0U;
  }
}

ISnmpSecurityPort_t SnmpSecurityAdapterCreatePort(
  SnmpSecurityAdapterCtx_t *ctx)
{
  ISnmpSecurityPort_t port;

  port.ctx = ctx;
  port.GetStrictReleasePolicy = AdapterGetStrictReleasePolicy;
  port.GetSnmpV3Username = AdapterGetSnmpV3Username;
  port.SetSnmpV2cCommunities = AdapterSetSnmpV2cCommunities;
  port.SetSnmpV3Username = AdapterSetSnmpV3Username;
  port.SetSnmpV3Credentials = AdapterSetSnmpV3Credentials;

  return port;
}
