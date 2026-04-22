/**
 ******************************************************************************
 * @file    Adapters/Mock/MockSignalInputAdapter.h
 * @brief   Test double for ISignalInputPort.
 ******************************************************************************
 */

#ifndef ADAPTERS_MOCK_SIGNAL_INPUT_ADAPTER_H
#define ADAPTERS_MOCK_SIGNAL_INPUT_ADAPTER_H

#include <stdint.h>
#include "Ports/ISignalInputPort.h"

typedef struct
{
  SignalInputSnapshot_t canned;
  uint32_t sampleCount;
} MockSignalInputAdapterCtx_t;

void MockSignalInputAdapter_Init(MockSignalInputAdapterCtx_t *ctx);
void MockSignalInputAdapter_SetSnapshot(MockSignalInputAdapterCtx_t *ctx,
                                        const SignalInputSnapshot_t *snap);
ISignalInputPort_t MockSignalInputAdapter_CreatePort(
  MockSignalInputAdapterCtx_t *ctx);

#endif /* ADAPTERS_MOCK_SIGNAL_INPUT_ADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
