/**
 ******************************************************************************
 * @file    Adapters/STM32/CanBusAdapter.h
 * @brief   STM32 adapter for ICanBusPort TX side.
 *          Wraps the existing CANTxRequest enqueue-then-CANMsgSenderTask flow.
 *          Adapter is bus-agnostic: tECanBusId → FDCAN handle happens inside.
 ******************************************************************************
 */

#ifndef ADAPTERS_STM32_CAN_BUS_ADAPTER_H
#define ADAPTERS_STM32_CAN_BUS_ADAPTER_H

#include <stdint.h>
#include "Ports/ICanBusPort.h"

typedef struct
{
  uint8_t bReserved;
} tSCanBusAdapterCtx;

void CanBusAdapter_Init(tSCanBusAdapterCtx *pCtx);
ICanBusPort_t CanBusAdapter_CreatePort(tSCanBusAdapterCtx *pCtx);

#endif /* ADAPTERS_STM32_CAN_BUS_ADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
