/* App/Adapters/STM32/UBloxModemAdapter.h
 *
 * IModemPort adapter for the u-blox LEON G100 GPRS module.
 * Encapsulates the 17-state AT command sequence, response parsing,
 * and disconnect detection for this specific module type.
 *
 * Usage:
 *   static UBloxModemAdapterCtx_t ctx;
 *   UBloxModemAdapterInit(&ctx);
 *   IModemPort_t port = UBloxModemAdapterCreatePort(&ctx);
 */
#ifndef UBLOX_MODEM_ADAPTER_H
#define UBLOX_MODEM_ADAPTER_H

#include "Ports/IModemPort.h"

/* Context struct — caller must supply static storage. */
typedef struct
{
  ISerialPort_t *serialPort;
  uint8_t bSimRetries;    /* counts non-READY +CPIN responses         */
  uint8_t bSigQuaRetries; /* counts 99-error signal quality responses */
  uint8_t bNetRetries;    /* counts not-registered network responses  */
} UBloxModemAdapterCtx_t;

void UBloxModemAdapterInit(UBloxModemAdapterCtx_t *ctx);
IModemPort_t UBloxModemAdapterCreatePort(UBloxModemAdapterCtx_t *ctx);

#endif /* UBLOX_MODEM_ADAPTER_H */
