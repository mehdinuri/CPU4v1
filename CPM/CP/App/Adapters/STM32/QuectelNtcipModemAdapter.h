/* App/Adapters/STM32/QuectelNtcipModemAdapter.h
 *
 * IModemPort concrete adapter for the Quectel EG915U NTCIP module
 * (115200 baud, PPP/LwIP stack, SNMP over PPPoS).
 */
#ifndef QUECTEL_NTCIP_MODEM_ADAPTER_H
#define QUECTEL_NTCIP_MODEM_ADAPTER_H

#include "Ports/IModemPort.h"

typedef struct
{
  ISerialPort_t *serialPort;
  uint8_t bSimRetries;
  uint8_t bSigQuaRetries;
  uint8_t bNetRetries;
} QuectelNtcipModemAdapterCtx_t;

void QuectelNtcipModemAdapterInit(QuectelNtcipModemAdapterCtx_t *ctx);
IModemPort_t QuectelNtcipModemAdapterCreatePort(
  QuectelNtcipModemAdapterCtx_t *ctx);

#endif /* QUECTEL_NTCIP_MODEM_ADAPTER_H */
