/* App/Adapters/STM32/QuectelModemAdapter.h
 *
 * IModemPort concrete adapter for the Quectel M95 GPRS module
 * (115200 baud, TCP socket via AT+QIOPEN command).
 */
#ifndef QUECTEL_MODEM_ADAPTER_H
#define QUECTEL_MODEM_ADAPTER_H

#include "Ports/IModemPort.h"

typedef struct
{
  ISerialPort_t *serialPort;
  uint8_t bSimRetries;
  uint8_t bSigQuaRetries;
  uint8_t bNetRetries;
} QuectelModemAdapterCtx_t;

void QuectelModemAdapterInit(QuectelModemAdapterCtx_t *ctx);
IModemPort_t QuectelModemAdapterCreatePort(QuectelModemAdapterCtx_t *ctx);

#endif /* QUECTEL_MODEM_ADAPTER_H */
