/**
 ******************************************************************************
 * @file    Adapters/Mock/MockFrequencySensorAdapter.h
 * @brief   Mock adapter for IFrequencySensorPort — configurable uint8_t value.
 ******************************************************************************
 */

#ifndef ADAPTERS_MOCK_MOCKFREQUENCYSENSORADAPTER_H
#define ADAPTERS_MOCK_MOCKFREQUENCYSENSORADAPTER_H

#include "Ports/IFrequencySensorPort.h"

typedef struct
{
  uint8_t frequency; /* pre-set by test to simulate measured frequency */
} MockFrequencySensorAdapterCtx_t;

void                    MockFrequencySensorAdapterInit(MockFrequencySensorAdapterCtx_t *ctx);
IFrequencySensorPort_t  MockFrequencySensorAdapterCreatePort(MockFrequencySensorAdapterCtx_t *ctx);

#endif /* ADAPTERS_MOCK_MOCKFREQUENCYSENSORADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
