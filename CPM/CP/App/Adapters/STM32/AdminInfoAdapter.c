/* App/Adapters/STM32/AdminInfoAdapter.c */
#include "AdminInfoAdapter.h"

#include <string.h>

#include "data.h"

static uint8_t ReadInfo(void *ctx)
{
  uint8_t ok;

  (void) ctx;
  ok = ReadAdminUsername();
  ok = (uint8_t) (ok && ReadAdminValidity());
  return ok;
}

static void GetInfo(void *ctx, AdminInfo_t *info)
{
  (void) ctx;
  if (info == NULL)
  {
    return;
  }

  info->username = GetAdminUsername();
  info->validity = GetAdminValidity();
}

void AdminInfoAdapterInit(AdminInfoAdapterCtx_t *ctx)
{
  if (ctx != NULL)
  {
    (void) memset(ctx, 0, sizeof(*ctx));
  }
}

IAdminInfoPort_t AdminInfoAdapterCreatePort(AdminInfoAdapterCtx_t *ctx)
{
  IAdminInfoPort_t port;

  port.ctx = ctx;
  port.Read = ReadInfo;
  port.Get = GetInfo;
  return port;
}
