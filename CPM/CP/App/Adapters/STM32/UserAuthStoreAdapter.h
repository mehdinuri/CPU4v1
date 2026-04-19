/* App/Adapters/STM32/UserAuthStoreAdapter.h */
#ifndef USER_AUTH_STORE_ADAPTER_H
#define USER_AUTH_STORE_ADAPTER_H

#include "Ports/IPersistencePort.h"
#include "Ports/IUserAuthStorePort.h"

typedef struct
{
  IPersistencePort_t *persistencePort;
} UserAuthStoreAdapterCtx_t;

void UserAuthStoreAdapterInit(UserAuthStoreAdapterCtx_t *ctx,
                              IPersistencePort_t *persistencePort);
IUserAuthStorePort_t UserAuthStoreAdapterCreatePort(
  UserAuthStoreAdapterCtx_t *ctx);

#endif /* USER_AUTH_STORE_ADAPTER_H */
