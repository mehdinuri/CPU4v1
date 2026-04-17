/* App/Adapters/Mock/MockPersistenceAdapter.h
 *
 * IPersistencePort in-memory test double — flat byte array.
 */
#ifndef MOCK_PERSISTENCE_ADAPTER_H
#define MOCK_PERSISTENCE_ADAPTER_H

#include "Ports/IPersistencePort.h"

#define MOCK_PERSISTENCE_SIZE 4096U

typedef struct
{
  uint8_t bytes[MOCK_PERSISTENCE_SIZE];
} MockPersistenceAdapterCtx_t;

void MockPersistenceAdapterInit(MockPersistenceAdapterCtx_t *ctx);
IPersistencePort_t MockPersistenceAdapterCreatePort(
  MockPersistenceAdapterCtx_t *ctx);

#endif /* MOCK_PERSISTENCE_ADAPTER_H */
