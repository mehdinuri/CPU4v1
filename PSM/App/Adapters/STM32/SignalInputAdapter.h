/**
 ******************************************************************************
 * @file    Adapters/STM32/SignalInputAdapter.h
 * @brief   STM32 adapter for ISignalInputPort — wraps gpio.h read functions.
 ******************************************************************************
 */

#ifndef ADAPTERS_STM32_SIGNALINPUTADAPTER_H
#define ADAPTERS_STM32_SIGNALINPUTADAPTER_H

#include "Ports/ISignalInputPort.h"

typedef struct
{
  uint8_t initialised;
} SignalInputAdapterCtx_t;

void                SignalInputAdapterInit(SignalInputAdapterCtx_t *ctx);
ISignalInputPort_t  SignalInputAdapterCreatePort(SignalInputAdapterCtx_t *ctx);

#endif /* ADAPTERS_STM32_SIGNALINPUTADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
