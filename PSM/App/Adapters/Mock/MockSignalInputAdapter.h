/**
 ******************************************************************************
 * @file    Adapters/Mock/MockSignalInputAdapter.h
 * @brief   Mock adapter for ISignalInputPort — returns configurable pin states.
 ******************************************************************************
 */

#ifndef ADAPTERS_MOCK_MOCKSIGNALINPUTADAPTER_H
#define ADAPTERS_MOCK_MOCKSIGNALINPUTADAPTER_H

#include "Ports/ISignalInputPort.h"

#define MOCK_INPUT_COUNT 3U

typedef struct
{
  /* Pre-set these before calling service methods to simulate hardware state */
  uint8_t inputStates[MOCK_INPUT_COUNT]; /* indexed by SignalInputId_t */
} MockSignalInputAdapterCtx_t;

void               MockSignalInputAdapterInit(MockSignalInputAdapterCtx_t *ctx);
ISignalInputPort_t MockSignalInputAdapterCreatePort(MockSignalInputAdapterCtx_t *ctx);

#endif /* ADAPTERS_MOCK_MOCKSIGNALINPUTADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
