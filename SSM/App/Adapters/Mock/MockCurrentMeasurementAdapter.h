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
  CurrentMeasurementSnapshot_t stored;
  uint32_t getLatestCount;
} MockCurrentMeasurementAdapterCtx_t;

void MockCurrentMeasurementAdapter_Init(
  MockCurrentMeasurementAdapterCtx_t *ctx);
void MockCurrentMeasurementAdapter_SetSnapshot(
  MockCurrentMeasurementAdapterCtx_t *ctx,
  const
  CurrentMeasurementSnapshot_t *
  snap);
ICurrentMeasurementPort_t MockCurrentMeasurementAdapter_CreatePort(
  MockCurrentMeasurementAdapterCtx_t *ctx);

#endif /* ADAPTERS_MOCK_CURRENT_MEASUREMENT_ADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
