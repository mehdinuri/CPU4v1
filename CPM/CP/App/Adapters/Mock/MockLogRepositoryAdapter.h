/* App/Adapters/Mock/MockLogRepositoryAdapter.h
 *
 * In-memory circular log repository used by host-side tests.
 */
#ifndef MOCK_LOG_REPOSITORY_ADAPTER_H
#define MOCK_LOG_REPOSITORY_ADAPTER_H

#include "Ports/ILogRepositoryPort.h"

#define MOCK_LOG_REPOSITORY_MAX_RECORDS 1024U
#define MOCK_LOG_REPOSITORY_MAX_RECORD_SIZE 128U

typedef struct
{
  uint8_t records[MOCK_LOG_REPOSITORY_MAX_RECORDS][
    MOCK_LOG_REPOSITORY_MAX_RECORD_SIZE];
  uint16_t writeIndex;
  uint16_t count;
  uint8_t exists;
} MockLogRepositoryAdapterCtx_t;

void MockLogRepositoryAdapterInit(MockLogRepositoryAdapterCtx_t *ctx);
ILogRepositoryPort_t MockLogRepositoryAdapterCreatePort(
  MockLogRepositoryAdapterCtx_t *ctx);

#endif /* MOCK_LOG_REPOSITORY_ADAPTER_H */
