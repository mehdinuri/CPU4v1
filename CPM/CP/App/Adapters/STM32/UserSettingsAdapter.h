/* App/Adapters/STM32/UserSettingsAdapter.h */
#ifndef USER_SETTINGS_ADAPTER_H
#define USER_SETTINGS_ADAPTER_H

#include "Ports/IUserSettingsPort.h"

typedef struct
{
  uint8_t reserved;
} UserSettingsAdapterCtx_t;

void UserSettingsAdapterInit(UserSettingsAdapterCtx_t *ctx);
IUserSettingsPort_t UserSettingsAdapterCreatePort(UserSettingsAdapterCtx_t *ctx);

#endif /* USER_SETTINGS_ADAPTER_H */
