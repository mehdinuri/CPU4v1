/**
 ******************************************************************************
 * @file    Adapters/Mock/MockPersistenceAdapter.h
 * @brief   Test double for IPersistencePort.
 *          Backed by a fixed-size in-memory blob per key.
 ******************************************************************************
 */

#ifndef ADAPTERS_MOCK_PERSISTENCE_ADAPTER_H
#define ADAPTERS_MOCK_PERSISTENCE_ADAPTER_H

#include <stdint.h>
#include "Ports/IPersistencePort.h"

#define MOCK_PERSIST_MAX_BLOB_SIZE 32U

typedef struct
{
  uint8_t abBlob[MOCK_PERSIST_MAX_BLOB_SIZE];
  uint16_t size;         /* 0 = empty/unset */
  uint8_t forceReadFail;
  uint8_t forceWriteFail;
} MockPersistenceSlot_t;

typedef struct
{
  MockPersistenceSlot_t slots[PERSIST_KEY__COUNT];
  uint32_t readCount;
  uint32_t writeCount;
} MockPersistenceAdapterCtx_t;

void MockPersistenceAdapter_Init(MockPersistenceAdapterCtx_t *ctx);
IPersistencePort_t MockPersistenceAdapter_CreatePort(
  MockPersistenceAdapterCtx_t *ctx);

#endif /* ADAPTERS_MOCK_PERSISTENCE_ADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
