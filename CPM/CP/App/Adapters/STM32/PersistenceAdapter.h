/* App/Adapters/STM32/PersistenceAdapter.h
 *
 * STM32 persistence adapter routing semantic persistence objects to the
 * legacy MSM storage worker.
 */
#ifndef PERSISTENCE_ADAPTER_H
#define PERSISTENCE_ADAPTER_H

#include "Ports/IPersistencePort.h"

typedef struct
{
  uint8_t reserved;
} PersistenceAdapterCtx_t;

void PersistenceAdapterInit(PersistenceAdapterCtx_t *ctx);
IPersistencePort_t PersistenceAdapterCreatePort(PersistenceAdapterCtx_t *ctx);

#endif /* PERSISTENCE_ADAPTER_H */
