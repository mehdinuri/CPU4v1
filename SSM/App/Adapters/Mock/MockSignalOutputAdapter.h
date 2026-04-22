/**
 ******************************************************************************
 * @file    Adapters/Mock/MockSignalOutputAdapter.h
 * @brief   Test double for ISignalOutputPort. Records the last image and
 *          counts Apply/AllOff invocations.
 ******************************************************************************
 */

#ifndef ADAPTERS_MOCK_SIGNAL_OUTPUT_ADAPTER_H
#define ADAPTERS_MOCK_SIGNAL_OUTPUT_ADAPTER_H

#include "Ports/ISignalOutputPort.h"

typedef struct
{
  uint32_t applyCount;
  uint32_t allOffCount;
  SignalOutputImage_t lastImage;
} MockSignalOutputAdapterCtx_t;

void MockSignalOutputAdapter_Init(MockSignalOutputAdapterCtx_t *ctx);
ISignalOutputPort_t MockSignalOutputAdapter_CreatePort(
  MockSignalOutputAdapterCtx_t *ctx);

#endif /* ADAPTERS_MOCK_SIGNAL_OUTPUT_ADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
