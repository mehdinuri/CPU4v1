/**
 ******************************************************************************
 * @file    Adapters/STM32/CANTxAdapter.h
 * @brief   STM32 adapter for ICANTxPort — wraps CANTxRequest() on FDCAN1
 *          with fixed classic-CAN / standard-ID / no-BRS settings.
 ******************************************************************************
 */

#ifndef ADAPTERS_STM32_CANTXADAPTER_H
#define ADAPTERS_STM32_CANTXADAPTER_H

#include "Ports/ICANTxPort.h"

typedef struct
{
  uint8_t bInitialised;
} CANTxAdapterCtx_t;

void          CANTxAdapterInit(CANTxAdapterCtx_t *ctx);
ICANTxPort_t  CANTxAdapterCreatePort(CANTxAdapterCtx_t *ctx);

#endif /* ADAPTERS_STM32_CANTXADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
