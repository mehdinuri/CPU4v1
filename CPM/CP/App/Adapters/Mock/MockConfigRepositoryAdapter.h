/* App/Adapters/Mock/MockConfigRepositoryAdapter.h
 *
 * In-memory immutable configuration repository used by host-side tests.
 */
#ifndef MOCK_CONFIG_REPOSITORY_ADAPTER_H
#define MOCK_CONFIG_REPOSITORY_ADAPTER_H

#include "Ports/IConfigRepositoryPort.h"

#define MOCK_CONFIG_REPOSITORY_SLOT_SIZE (128U * 1024U)
#define MOCK_CONFIG_REPOSITORY_JOURNAL_SIZE 64U

typedef struct
{
  uint8_t slotA[MOCK_CONFIG_REPOSITORY_SLOT_SIZE];
  uint8_t slotB[MOCK_CONFIG_REPOSITORY_SLOT_SIZE];
  uint8_t journal[MOCK_CONFIG_REPOSITORY_JOURNAL_SIZE];
  uint8_t initialised;
} MockConfigRepositoryAdapterCtx_t;

void MockConfigRepositoryAdapterInit(MockConfigRepositoryAdapterCtx_t *ctx);
IConfigRepositoryPort_t MockConfigRepositoryAdapterCreatePort(
  MockConfigRepositoryAdapterCtx_t *ctx);

#endif /* MOCK_CONFIG_REPOSITORY_ADAPTER_H */
