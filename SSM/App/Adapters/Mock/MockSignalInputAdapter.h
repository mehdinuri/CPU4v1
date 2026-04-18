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
  tSSignalInputSnapshot SCanned;
  uint32_t lSampleCount;
} tSMockSignalInputAdapterCtx;

void MockSignalInputAdapter_Init(tSMockSignalInputAdapterCtx *pCtx);
void MockSignalInputAdapter_SetSnapshot(tSMockSignalInputAdapterCtx *pCtx,
                                        const tSSignalInputSnapshot *pSnap);
ISignalInputPort_t MockSignalInputAdapter_CreatePort(
  tSMockSignalInputAdapterCtx *pCtx);

#endif /* ADAPTERS_MOCK_SIGNAL_INPUT_ADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
