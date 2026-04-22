/* App/Adapters/STM32/UserAuthStoreAdapter.c */
#include "UserAuthStoreAdapter.h"

#include <string.h>

static uint8_t LoadRecord(void *ctx, UserAuthStoreRecord_t *record)
{
  UserAuthStoreAdapterCtx_t *adapter = (UserAuthStoreAdapterCtx_t *) ctx;

  if ((adapter == NULL) || (adapter->persistencePort == NULL)
      || (record == NULL))
  {
    return 0U;
  }

  return PersistenceRead(adapter->persistencePort,
                         PERSIST_OBJECT_AUTH_STATE,
                         0U,
                         record,
                         sizeof(*record));
}

static uint8_t SaveRecord(void *ctx, const UserAuthStoreRecord_t *record)
{
  UserAuthStoreAdapterCtx_t *adapter = (UserAuthStoreAdapterCtx_t *) ctx;

  if ((adapter == NULL) || (adapter->persistencePort == NULL)
      || (record == NULL))
  {
    return 0U;
  }

  return PersistenceWrite(adapter->persistencePort,
                          PERSIST_OBJECT_AUTH_STATE,
                          0U,
                          record,
                          sizeof(*record));
}

void UserAuthStoreAdapterInit(UserAuthStoreAdapterCtx_t *ctx,
                              IPersistencePort_t *persistencePort)
{
  if (ctx != NULL)
  {
    (void) memset(ctx, 0, sizeof(*ctx));
    ctx->persistencePort = persistencePort;
  }
}

IUserAuthStorePort_t UserAuthStoreAdapterCreatePort(
  UserAuthStoreAdapterCtx_t *ctx)
{
  IUserAuthStorePort_t port;

  port.ctx = ctx;
  port.Load = LoadRecord;
  port.Save = SaveRecord;
  return port;
}
