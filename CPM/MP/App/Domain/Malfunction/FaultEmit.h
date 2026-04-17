/* App/Domain/Malfunction/FaultEmit.h
 *
 * Uniform sink for monitors to publish FaultEvent_t entries during a
 * tick. The engine wires this to the event log and FaultMonitorService.
 */
#ifndef FAULT_EMIT_H
#define FAULT_EMIT_H

#include <stddef.h>

#include "Malfunction/FaultCodes.h"

typedef void (*FaultEmitFn_t)(void *ctx, const FaultEvent_t *event);

typedef struct
{
  FaultEmitFn_t fn;
  void *ctx;
} FaultEmit_t;

static inline void FaultEmitPublish(const FaultEmit_t *emit,
                                    const FaultEvent_t *event)
{
  if ((emit != NULL) && (emit->fn != NULL) && (event != NULL))
  {
    emit->fn(emit->ctx, event);
  }
}

#endif /* FAULT_EMIT_H */
