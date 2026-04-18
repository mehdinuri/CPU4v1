/**
 ******************************************************************************
 * @file    Adapters/STM32/CANRxAdapter.h
 * @brief   STM32 adapter for ICANRxPort — repackages a neutral tSCANRxFrame
 *          into the HAL-typed tSFDCANRxMsg used by the parser task queue.
 ******************************************************************************
 */

#ifndef ADAPTERS_STM32_CANRXADAPTER_H
#define ADAPTERS_STM32_CANRXADAPTER_H

#include "Ports/ICANRxPort.h"

typedef struct
{
  uint8_t bInitialised;
} CANRxAdapterCtx_t;

void         CANRxAdapterInit(CANRxAdapterCtx_t *ctx);
ICANRxPort_t CANRxAdapterCreatePort(CANRxAdapterCtx_t *ctx);

#endif /* ADAPTERS_STM32_CANRXADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
