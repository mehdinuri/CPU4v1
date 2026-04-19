/* App/Adapters/STM32/FieldOutputCanAdapter.h
 *
 * Clean FDCAN1 publisher for the legacy-compatible SSM/PSM field-bus
 * messages. It owns only wire formatting and scheduling; the domain still
 * owns aspect derivation and the canonical I/O map.
 */
#ifndef FIELD_OUTPUT_CAN_ADAPTER_H
#define FIELD_OUTPUT_CAN_ADAPTER_H

#include <stdint.h>

#include "Domain/Intersection/ConfigurationService.h"
#include "Ports/IOutputDriverPort.h"
#include "Ports/IRealtimeClockPort.h"

typedef struct
{
  ConfigurationService_t *configurationService;
  IRealtimeClockPort_t *rtcPort;
  OutputDriverImage_t lastImage;
  uint16_t configEpoch;
  uint16_t stepCounter;
  uint16_t periodicCounter;
  uint8_t hasImage;
} FieldOutputCanAdapterCtx_t;

void FieldOutputCanAdapterInit(FieldOutputCanAdapterCtx_t *ctx,
                               ConfigurationService_t *configurationService,
                               IRealtimeClockPort_t *rtcPort,
                               uint16_t configEpoch);
void FieldOutputCanAdapterSetConfigEpoch(FieldOutputCanAdapterCtx_t *ctx,
                                         uint16_t configEpoch);
IOutputDriverPort_t FieldOutputCanAdapterCreatePort(
  FieldOutputCanAdapterCtx_t *ctx);
void FieldOutputCanAdapterStep(void);

#endif /* FIELD_OUTPUT_CAN_ADAPTER_H */
