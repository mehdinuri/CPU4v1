/* App/Adapters/STM32/ModemAdapter.h
 *
 * Boot-time network-type loader shared by the supported bearer adapters.
 * Runtime modem operations go through IModemPort_t, not this header.
 */
#ifndef MODEM_ADAPTER_H
#define MODEM_ADAPTER_H

#include "Ports/IEepromStoragePort.h"

/*
 * Reads the MCS connection-info record directly from EEPROM via the
 * supplied port, bypassing the MSM task queue.  Safe to call before
 * vTaskStartScheduler(); calling it after risks I2C bus contention
 * with StorageTask.
 *
 * Returns the stored bNetworkType byte when the record is valid.
 * Returns MCS_NETWORK_TYPE_NONE on NULL port, I2C failure, or an
 * uninitialised record.  The caller's switch must have a default clause.
 */
uint8_t ModemAdapterLoadModuleType(IEepromStoragePort_t *eepromPort);

#endif /* MODEM_ADAPTER_H */
