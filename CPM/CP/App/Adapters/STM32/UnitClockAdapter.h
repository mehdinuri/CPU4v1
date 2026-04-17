/* App/Adapters/STM32/UnitClockAdapter.h
 *
 * STM32 unit-clock adapter backed by the legacy time/GNSS services and the
 * canonical configuration service.
 */
#ifndef UNIT_CLOCK_ADAPTER_H
#define UNIT_CLOCK_ADAPTER_H

#include "Domain/Intersection/ConfigurationService.h"
#include "Ports/IUnitClockPort.h"

typedef struct
{
  ConfigurationService_t *configurationService;
} UnitClockAdapterCtx_t;

void UnitClockAdapterInit(UnitClockAdapterCtx_t *ctx,
                          ConfigurationService_t *configurationService);
IUnitClockPort_t UnitClockAdapterCreatePort(UnitClockAdapterCtx_t *ctx);

#endif /* UNIT_CLOCK_ADAPTER_H */
