/* App/Adapters/STM32/FlashStorageAdapter.h
 *
 * STM32 internal flash storage adapter.
 */
#ifndef FLASH_STORAGE_ADAPTER_H
#define FLASH_STORAGE_ADAPTER_H

#include "Ports/IFlashStoragePort.h"

typedef struct
{
  uint8_t reserved;
} FlashStorageAdapterCtx_t;

void FlashStorageAdapterInit(FlashStorageAdapterCtx_t *ctx);
IFlashStoragePort_t FlashStorageAdapterCreatePort(
  FlashStorageAdapterCtx_t *ctx);

#endif /* FLASH_STORAGE_ADAPTER_H */
