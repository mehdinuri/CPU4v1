/**
 ******************************************************************************
 * @file    Adapters/Mock/MockCurrentMeasurementAdapter.h
 * @brief   Test double for ICurrentMeasurementPort. Holds one canned snapshot;
 *          tests call SetSnapshot() to drive it.
 ******************************************************************************
 */

#ifndef ADAPTERS_MOCK_CURRENT_MEASUREMENT_ADAPTER_H
#define ADAPTERS_MOCK_CURRENT_MEASUREMENT_ADAPTER_H

#include <stdint.h>
#include "Ports/ICurrentMeasurementPort.h"

typedef struct
{
  tSCurrentMeasurementSnapshot SStored;
  uint32_t lGetLatestCount;
} tSMockCurrentMeasurementAdapterCtx;

void MockCurrentMeasurementAdapter_Init(
  tSMockCurrentMeasurementAdapterCtx *pCtx);
void MockCurrentMeasurementAdapter_SetSnapshot(
  tSMockCurrentMeasurementAdapterCtx *pCtx,
  const
  tSCurrentMeasurementSnapshot *
  pSnap);
ICurrentMeasurementPort_t MockCurrentMeasurementAdapter_CreatePort(
  tSMockCurrentMeasurementAdapterCtx *pCtx);

#endif /* ADAPTERS_MOCK_CURRENT_MEASUREMENT_ADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
