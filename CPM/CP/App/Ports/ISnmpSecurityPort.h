/* App/Ports/ISnmpSecurityPort.h
 *
 * Port interface for CP SNMP security state and credential rotation.
 */
#ifndef I_SNMP_SECURITY_PORT_H
#define I_SNMP_SECURITY_PORT_H

#include <stddef.h>
#include <stdint.h>

typedef struct
{
  void *ctx;

  uint8_t (*GetStrictReleasePolicy)(void *ctx);
  uint8_t (*GetSnmpV3Username)(void *ctx, char *dst, size_t dstSize);
  uint8_t (*SetSnmpV2cCommunities)(void *ctx,
                                   const char *readCommunity,
                                   const char *writeCommunity,
                                   const char *trapCommunity);
  uint8_t (*SetSnmpV3Username)(void *ctx, const char *username);
  uint8_t (*SetSnmpV3Credentials)(void *ctx,
                                  const char *username,
                                  const char *authPassphrase,
                                  const char *privPassphrase);
} ISnmpSecurityPort_t;

static inline uint8_t SnmpSecurityPortGetStrictReleasePolicy(
  const ISnmpSecurityPort_t *port)
{
  if ((port == NULL) || (port->GetStrictReleasePolicy == NULL))
  {
    return 0U;
  }

  return port->GetStrictReleasePolicy(port->ctx);
}

static inline uint8_t SnmpSecurityPortGetSnmpV3Username(
  const ISnmpSecurityPort_t *port,
  char *dst,
  size_t dstSize)
{
  if ((port == NULL) || (port->GetSnmpV3Username == NULL))
  {
    return 0U;
  }

  return port->GetSnmpV3Username(port->ctx, dst, dstSize);
}

static inline uint8_t SnmpSecurityPortSetSnmpV2cCommunities(
  ISnmpSecurityPort_t *port,
  const char *readCommunity,
  const char *writeCommunity,
  const char *trapCommunity)
{
  if ((port == NULL) || (port->SetSnmpV2cCommunities == NULL))
  {
    return 0U;
  }

  return port->SetSnmpV2cCommunities(port->ctx,
                                     readCommunity,
                                     writeCommunity,
                                     trapCommunity);
}

static inline uint8_t SnmpSecurityPortSetSnmpV3Username(
  ISnmpSecurityPort_t *port,
  const char *username)
{
  if ((port == NULL) || (port->SetSnmpV3Username == NULL))
  {
    return 0U;
  }

  return port->SetSnmpV3Username(port->ctx, username);
}

static inline uint8_t SnmpSecurityPortSetSnmpV3Credentials(
  ISnmpSecurityPort_t *port,
  const char *username,
  const char *authPassphrase,
  const char *privPassphrase)
{
  if ((port == NULL) || (port->SetSnmpV3Credentials == NULL))
  {
    return 0U;
  }

  return port->SetSnmpV3Credentials(port->ctx,
                                    username,
                                    authPassphrase,
                                    privPassphrase);
}

#endif /* I_SNMP_SECURITY_PORT_H */
