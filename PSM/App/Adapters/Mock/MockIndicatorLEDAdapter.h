/**
 ******************************************************************************
 * @file    Adapters/Mock/MockIndicatorLEDAdapter.h
 * @brief   Mock adapter for IIndicatorLEDPort — records all LED state changes.
 ******************************************************************************
 */

#ifndef ADAPTERS_MOCK_MOCKINDICATORLEDADAPTER_H
#define ADAPTERS_MOCK_MOCKINDICATORLEDADAPTER_H

#include "Ports/IIndicatorLEDPort.h"

#define MOCK_LED_COUNT 4U

typedef struct
{
  uint8_t  ledStates[MOCK_LED_COUNT]; /* current state per LED ID */
  uint32_t setCallCount;              /* total SetState calls      */
  uint32_t toggleCallCount;           /* total Toggle calls        */
} MockIndicatorLEDAdapterCtx_t;

void                 MockIndicatorLEDAdapterInit(MockIndicatorLEDAdapterCtx_t *ctx);
IIndicatorLEDPort_t  MockIndicatorLEDAdapterCreatePort(MockIndicatorLEDAdapterCtx_t *ctx);

#endif /* ADAPTERS_MOCK_MOCKINDICATORLEDADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
