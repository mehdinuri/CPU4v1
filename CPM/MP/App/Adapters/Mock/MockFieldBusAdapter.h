/* App/Adapters/Mock/MockFieldBusAdapter.h
 *
 * IFieldBusPort in-memory test double. Tests set up a scripted
 * FieldBusSnapshot_t; ReadSnapshot() returns it verbatim.
 */
#ifndef MOCK_FIELD_BUS_ADAPTER_H
#define MOCK_FIELD_BUS_ADAPTER_H

#include "Ports/IFieldBusPort.h"

typedef struct
{
  FieldBusSnapshot_t snapshot;
  uint32_t readCount;
} MockFieldBusAdapterCtx_t;

void MockFieldBusAdapterInit(MockFieldBusAdapterCtx_t *ctx);
IFieldBusPort_t MockFieldBusAdapterCreatePort(MockFieldBusAdapterCtx_t *ctx);
void MockFieldBusAdapterSetSnapshot(MockFieldBusAdapterCtx_t *ctx,
                                    const FieldBusSnapshot_t *snapshot);

#endif /* MOCK_FIELD_BUS_ADAPTER_H */
