/**
 ******************************************************************************
 * @file    Adapters/STM32/EepromAdapter.h
 * @brief   STM32 adapter for IEepromPort — routes through StorageRequest() so
 *          all I2C access remains serialised through StorageTask.
 ******************************************************************************
 */

#ifndef ADAPTERS_STM32_EEPROMADAPTER_H
#define ADAPTERS_STM32_EEPROMADAPTER_H

#include "Ports/IEepromPort.h"

typedef struct
{
  uint8_t bInitialised;
} EepromAdapterCtx_t;

void          EepromAdapterInit(EepromAdapterCtx_t *ctx);
IEepromPort_t EepromAdapterCreatePort(EepromAdapterCtx_t *ctx);

#endif /* ADAPTERS_STM32_EEPROMADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
