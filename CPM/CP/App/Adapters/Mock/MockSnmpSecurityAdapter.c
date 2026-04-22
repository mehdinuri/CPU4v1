/* App/Adapters/Mock/MockSnmpSecurityAdapter.c */
#include "MockSnmpSecurityAdapter.h"

#include <string.h>

static uint8_t MockGetStrictReleasePolicy(void *ctx)
{
  const MockSnmpSecurityAdapterCtx_t *self =
    (const MockSnmpSecurityAdapterCtx_t *) ctx;

  return (self == NULL) ? 0U : self->strictReleasePolicy;
}

static uint8_t MockGetSnmpV3Username(void *ctx, char *dst, size_t dstSize)
{
  const MockSnmpSecurityAdapterCtx_t *self =
    (const MockSnmpSecurityAdapterCtx_t *) ctx;

  if ((self == NULL) || (dst == NULL) || (dstSize == 0U))
  {
    return 0U;
  }

  (void) memset(dst, 0, dstSize);
  strncpy(dst, &self->username[0], dstSize - 1U);
  dst[dstSize - 1U] = '\0';
  return 1U;
}

static uint8_t MockSetSnmpV2cCommunities(void *ctx,
                                         const char *readCommunity,
                                         const char *writeCommunity,
                                         const char *trapCommunity)
{
  MockSnmpSecurityAdapterCtx_t *self = (MockSnmpSecurityAdapterCtx_t *) ctx;

  if ((self == NULL) || (readCommunity == NULL) || (writeCommunity == NULL)
      || (trapCommunity == NULL))
  {
    return 0U;
  }

  (void) memset(&self->readCommunity[0], 0, sizeof(self->readCommunity));
  (void) memset(&self->writeCommunity[0], 0, sizeof(self->writeCommunity));
  (void) memset(&self->trapCommunity[0], 0, sizeof(self->trapCommunity));
  strncpy(&self->readCommunity[0],
          readCommunity,
          sizeof(self->readCommunity) - 1U);
  strncpy(&self->writeCommunity[0],
          writeCommunity,
          sizeof(self->writeCommunity) - 1U);
  strncpy(&self->trapCommunity[0],
          trapCommunity,
          sizeof(self->trapCommunity) - 1U);
  self->setV2cCount++;
  return 1U;
}

static uint8_t MockSetSnmpV3Username(void *ctx, const char *username)
{
  MockSnmpSecurityAdapterCtx_t *self = (MockSnmpSecurityAdapterCtx_t *) ctx;

  if ((self == NULL) || (username == NULL))
  {
    return 0U;
  }

  (void) memset(&self->username[0], 0, sizeof(self->username));
  strncpy(&self->username[0], username, sizeof(self->username) - 1U);
  self->setV3UsernameCount++;
  return 1U;
}

static uint8_t MockSetSnmpV3Credentials(void *ctx,
                                        const char *username,
                                        const char *authPassphrase,
                                        const char *privPassphrase)
{
  MockSnmpSecurityAdapterCtx_t *self = (MockSnmpSecurityAdapterCtx_t *) ctx;

  if ((self == NULL) || (authPassphrase == NULL) || (privPassphrase == NULL))
  {
    return 0U;
  }

  if (username != NULL)
  {
    (void) memset(&self->username[0], 0, sizeof(self->username));
    strncpy(&self->username[0], username, sizeof(self->username) - 1U);
  }

  (void) memset(&self->authPassphrase[0], 0, sizeof(self->authPassphrase));
  (void) memset(&self->privPassphrase[0], 0, sizeof(self->privPassphrase));
  strncpy(&self->authPassphrase[0],
          authPassphrase,
          sizeof(self->authPassphrase) - 1U);
  strncpy(&self->privPassphrase[0],
          privPassphrase,
          sizeof(self->privPassphrase) - 1U);
  self->setV3CredentialsCount++;
  return 1U;
}

void MockSnmpSecurityAdapterInit(MockSnmpSecurityAdapterCtx_t *ctx)
{
  if (ctx == NULL)
  {
    return;
  }

  (void) memset(ctx, 0, sizeof(*ctx));
  strncpy(&ctx->username[0], "maester", sizeof(ctx->username) - 1U);
}

ISnmpSecurityPort_t MockSnmpSecurityAdapterCreatePort(
  MockSnmpSecurityAdapterCtx_t *ctx)
{
  ISnmpSecurityPort_t port;

  port.ctx = ctx;
  port.GetStrictReleasePolicy = MockGetStrictReleasePolicy;
  port.GetSnmpV3Username = MockGetSnmpV3Username;
  port.SetSnmpV2cCommunities = MockSetSnmpV2cCommunities;
  port.SetSnmpV3Username = MockSetSnmpV3Username;
  port.SetSnmpV3Credentials = MockSetSnmpV3Credentials;

  return port;
}
