/* App/Adapters/STM32/UsrModemAdapter.h
 *
 * IModemPort concrete adapter for the USR TCP232-200 transparent
 * TCP bridge module (no AT commands; connects directly on power-up).
 */
#ifndef USR_MODEM_ADAPTER_H
#define USR_MODEM_ADAPTER_H

#include "Ports/IModemPort.h"

/* Context is empty — the USR module has no state to track. */
typedef struct
{
  ISerialPort_t *serialPort;
  uint8_t bPadding; /* reserved */
} UsrModemAdapterCtx_t;

void UsrModemAdapterInit(UsrModemAdapterCtx_t *ctx);
IModemPort_t UsrModemAdapterCreatePort(UsrModemAdapterCtx_t *ctx);

#endif /* USR_MODEM_ADAPTER_H */
