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
  uint32_t lApplyCount;
  uint32_t lAllOffCount;
  tSSignalOutputImage SLastImage;
} tSMockSignalOutputAdapterCtx;

void MockSignalOutputAdapter_Init(tSMockSignalOutputAdapterCtx *pCtx);
ISignalOutputPort_t MockSignalOutputAdapter_CreatePort(
  tSMockSignalOutputAdapterCtx *pCtx);

#endif /* ADAPTERS_MOCK_SIGNAL_OUTPUT_ADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
