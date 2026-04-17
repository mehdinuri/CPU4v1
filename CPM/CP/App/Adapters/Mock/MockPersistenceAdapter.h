/* App/Adapters/Mock/MockPersistenceAdapter.h
 *
 * In-memory persistence adapter used by host-side tests.
 */
#ifndef MOCK_PERSISTENCE_ADAPTER_H
#define MOCK_PERSISTENCE_ADAPTER_H

#include "Ports/IPersistencePort.h"

#define MOCK_PERSISTENCE_MAX_OBJECT_SIZE 8192U

typedef struct
{
  uint8_t data[PERSIST_OBJECT_LAST][MOCK_PERSISTENCE_MAX_OBJECT_SIZE];
  uint8_t initialised;
} MockPersistenceAdapterCtx_t;

void MockPersistenceAdapterInit(MockPersistenceAdapterCtx_t *ctx);
IPersistencePort_t MockPersistenceAdapterCreatePort(
  MockPersistenceAdapterCtx_t *ctx);

#endif /* MOCK_PERSISTENCE_ADAPTER_H */
