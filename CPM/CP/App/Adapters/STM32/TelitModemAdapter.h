/* App/Adapters/STM32/TelitModemAdapter.h
 *
 * IModemPort concrete adapter for the Telit GL865 Dual GPRS module
 * (9600 baud, TCP socket via AT#SD command).
 */
#ifndef TELIT_MODEM_ADAPTER_H
#define TELIT_MODEM_ADAPTER_H

#include "Ports/IModemPort.h"

typedef struct
{
  ISerialPort_t *serialPort;
  uint8_t bSimRetries;
  uint8_t bSigQuaRetries;
  uint8_t bNetRetries;
} TelitModemAdapterCtx_t;

void TelitModemAdapterInit(TelitModemAdapterCtx_t *ctx);
IModemPort_t TelitModemAdapterCreatePort(TelitModemAdapterCtx_t *ctx);

#endif /* TELIT_MODEM_ADAPTER_H */
