/**
 ******************************************************************************
 * @file    Adapters/Mock/MockCANRxAdapter.h
 * @brief   Mock adapter for ICANRxPort — records submitted frames.
 ******************************************************************************
 */

#ifndef ADAPTERS_MOCK_MOCKCANRXADAPTER_H
#define ADAPTERS_MOCK_MOCKCANRXADAPTER_H

#include "Ports/ICANRxPort.h"

typedef struct
{
  CanRxFrame_t lastFrame;    /* last frame submitted           */
  uint32_t     submitCount;  /* total submissions              */
} MockCANRxAdapterCtx_t;

void         MockCANRxAdapterInit(MockCANRxAdapterCtx_t *ctx);
ICANRxPort_t MockCANRxAdapterCreatePort(MockCANRxAdapterCtx_t *ctx);

#endif /* ADAPTERS_MOCK_MOCKCANRXADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
