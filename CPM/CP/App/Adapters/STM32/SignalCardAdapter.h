/* App/Adapters/STM32/SignalCardAdapter.h
 *
 * FDCAN2-backed output-driver adapter for the new intersection engine.
 * Publishes the atomic output image to the signal-switching module bus.
 */
#ifndef SIGNAL_CARD_ADAPTER_H
#define SIGNAL_CARD_ADAPTER_H

#include <stdint.h>

#include "fdcan.h"
#include "Ports/IOutputDriverPort.h"

typedef struct
{
  FDCAN_HandleTypeDef *hfdcan;
  OutputDriverImage_t lastAppliedImage;
  uint16_t configEpoch;
  uint16_t sequenceCounter;
  uint8_t started;
} SignalCardAdapterCtx_t;

void SignalCardAdapterInit(SignalCardAdapterCtx_t *ctx,
                           FDCAN_HandleTypeDef *hfdcan,
                           uint16_t configEpoch);
void SignalCardAdapterSetConfigEpoch(SignalCardAdapterCtx_t *ctx,
                                     uint16_t configEpoch);
IOutputDriverPort_t SignalCardAdapterCreatePort(SignalCardAdapterCtx_t *ctx);

#endif /* SIGNAL_CARD_ADAPTER_H */
