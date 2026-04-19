/* App/Adapters/STM32/ModemConfigAdapter.h
 *
 * Persisted modem-type configuration adapter.
 */
#ifndef MODEM_CONFIG_ADAPTER_H
#define MODEM_CONFIG_ADAPTER_H

#include "Ports/IModemConfigPort.h"

typedef struct
{
  uint8_t reserved;
} ModemConfigAdapterCtx_t;

void ModemConfigAdapterInit(ModemConfigAdapterCtx_t *ctx);
IModemConfigPort_t ModemConfigAdapterCreatePort(ModemConfigAdapterCtx_t *ctx);

#endif /* MODEM_CONFIG_ADAPTER_H */
