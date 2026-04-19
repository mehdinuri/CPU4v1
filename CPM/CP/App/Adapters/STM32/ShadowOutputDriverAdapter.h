/* App/Adapters/STM32/ShadowOutputDriverAdapter.h
 *
 * Non-transmitting output driver used while legacy field-bus output messages
 * remain on FDCAN1. It captures the latest canonical image without consuming
 * a hardware transport.
 */
#ifndef SHADOW_OUTPUT_DRIVER_ADAPTER_H
#define SHADOW_OUTPUT_DRIVER_ADAPTER_H

#include "Ports/IOutputDriverPort.h"

typedef struct
{
  OutputDriverImage_t lastImage;
  uint16_t configEpoch;
  uint8_t applyCount;
} ShadowOutputDriverAdapterCtx_t;

void ShadowOutputDriverAdapterInit(ShadowOutputDriverAdapterCtx_t *ctx,
                                   uint16_t configEpoch);
IOutputDriverPort_t ShadowOutputDriverAdapterCreatePort(
  ShadowOutputDriverAdapterCtx_t *ctx);

#endif /* SHADOW_OUTPUT_DRIVER_ADAPTER_H */
