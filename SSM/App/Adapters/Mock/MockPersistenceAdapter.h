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
  uint16_t sSize;         /* 0 = empty/unset */
  uint8_t bForceReadFail;
  uint8_t bForceWriteFail;
} tSMockPersistenceSlot;

typedef struct
{
  tSMockPersistenceSlot SaSlots[PERSIST_KEY__COUNT];
  uint32_t lReadCount;
  uint32_t lWriteCount;
} tSMockPersistenceAdapterCtx;

void MockPersistenceAdapter_Init(tSMockPersistenceAdapterCtx *pCtx);
IPersistencePort_t MockPersistenceAdapter_CreatePort(
  tSMockPersistenceAdapterCtx *pCtx);

#endif /* ADAPTERS_MOCK_PERSISTENCE_ADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
