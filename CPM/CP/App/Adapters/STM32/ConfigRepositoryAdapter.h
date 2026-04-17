/* App/Adapters/STM32/ConfigRepositoryAdapter.h
 *
 * Maps the dedicated configuration repository port onto the semantic
 * persistence service.
 */
#ifndef CONFIG_REPOSITORY_ADAPTER_H
#define CONFIG_REPOSITORY_ADAPTER_H

#include "Ports/IConfigRepositoryPort.h"
#include "Ports/IPersistencePort.h"

typedef struct
{
  IPersistencePort_t *persistencePort;
} ConfigRepositoryAdapterCtx_t;

void ConfigRepositoryAdapterInit(ConfigRepositoryAdapterCtx_t *ctx,
                                 IPersistencePort_t *persistencePort);
IConfigRepositoryPort_t ConfigRepositoryAdapterCreatePort(
  ConfigRepositoryAdapterCtx_t *ctx);

#endif /* CONFIG_REPOSITORY_ADAPTER_H */
