/**
 ******************************************************************************
 * @file    Adapters/STM32/FlashPersistenceAdapter.h
 * @brief   STM32 adapter for IPersistencePort.
 *          Backed by internal flash via the existing StorageTask/StorageRequest
 *          queue flow. Keys map to (address, max-size) entries.
 ******************************************************************************
 */

#ifndef ADAPTERS_STM32_FLASH_PERSISTENCE_ADAPTER_H
#define ADAPTERS_STM32_FLASH_PERSISTENCE_ADAPTER_H

#include <stdint.h>
#include "Ports/IPersistencePort.h"

typedef struct
{
  uint8_t reserved;
} FlashPersistenceAdapterCtx_t;

void FlashPersistenceAdapter_Init(FlashPersistenceAdapterCtx_t *ctx);
IPersistencePort_t FlashPersistenceAdapter_CreatePort(
  FlashPersistenceAdapterCtx_t *ctx);

#endif /* ADAPTERS_STM32_FLASH_PERSISTENCE_ADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
