/* App/Adapters/STM32/FieldBusTxAdapter.h
 *
 * Clean FDCAN1 field-bus Tx adapter for the legacy-compatible SSM/PSM
 * messages. It owns only wire formatting and scheduling; the domain still
 * owns aspect derivation and the canonical I/O map.
 */
#ifndef FIELD_BUS_TX_ADAPTER_H
#define FIELD_BUS_TX_ADAPTER_H

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
} FieldBusTxAdapterCtx_t;

void FieldBusTxAdapterInit(FieldBusTxAdapterCtx_t *ctx,
                           ConfigurationService_t *configurationService,
                           IRealtimeClockPort_t *rtcPort,
                           uint16_t configEpoch);
void FieldBusTxAdapterSetConfigEpoch(FieldBusTxAdapterCtx_t *ctx,
                                     uint16_t configEpoch);
IOutputDriverPort_t FieldBusTxAdapterCreatePort(
  FieldBusTxAdapterCtx_t *ctx);
void FieldBusTxAdapterStep(void);

#endif /* FIELD_BUS_TX_ADAPTER_H */
