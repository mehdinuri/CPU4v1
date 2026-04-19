/* App/Adapters/STM32/BrokenInputSettingsAdapter.h */
#ifndef BROKEN_INPUT_SETTINGS_ADAPTER_H
#define BROKEN_INPUT_SETTINGS_ADAPTER_H

#include "Ports/IBrokenInputSettingsPort.h"

typedef struct
{
  uint8_t reserved;
} BrokenInputSettingsAdapterCtx_t;

void BrokenInputSettingsAdapterInit(BrokenInputSettingsAdapterCtx_t *ctx);
IBrokenInputSettingsPort_t BrokenInputSettingsAdapterCreatePort(
  BrokenInputSettingsAdapterCtx_t *ctx);

#endif /* BROKEN_INPUT_SETTINGS_ADAPTER_H */
