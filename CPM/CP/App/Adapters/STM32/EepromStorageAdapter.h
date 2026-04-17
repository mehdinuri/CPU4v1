/* App/Adapters/STM32/EepromStorageAdapter.h
 *
 * STM32 external EEPROM storage adapter.
 */
#ifndef EEPROM_STORAGE_ADAPTER_H
#define EEPROM_STORAGE_ADAPTER_H

#include "Ports/IEepromStoragePort.h"

typedef struct
{
  uint8_t reserved;
} EepromStorageAdapterCtx_t;

void EepromStorageAdapterInit(EepromStorageAdapterCtx_t *ctx);
IEepromStoragePort_t EepromStorageAdapterCreatePort(
  EepromStorageAdapterCtx_t *ctx);

#endif /* EEPROM_STORAGE_ADAPTER_H */
