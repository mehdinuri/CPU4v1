#pragma once

/*
 * App/Ports/IModemPort.h
 *
 * GPRS modem lifecycle and data channel. The concrete adapter drives the
 * UART4 AT command state machine (u-blox/Telit/Quectel) and PPP dial-up.
 * The Domain uses this to determine if a cellular data path is available
 * when Ethernet is absent.
 */
#include <stdint.h>
#include <stdbool.h>

typedef enum
{
  MODEM_STATE_OFF         = 0,
  MODEM_STATE_INITIALIZING,
  MODEM_STATE_REGISTERING,     /* GSM / GPRS registration in progress */
  MODEM_STATE_CONNECTED,       /* IP address assigned, data path active */
  MODEM_STATE_ERROR,
} ModemState_t;

typedef struct IModemPort
{
  void *ctx;

  /* Current modem state. */
  ModemState_t (*getState)(void *ctx);

  /* Signal quality (0–31, 99 = unknown). Maps to AT+CSQ response. */
  uint8_t (*getSignalQuality)(void *ctx);

  /* IMEI string (null-terminated, ≥ 16 bytes in outBuf). */
  void (*getImei)(void *ctx, char *outBuf, uint8_t bufLen);
} IModemPort_t;

static inline ModemState_t Modem_GetState(IModemPort_t *p)
{
  return p->getState(p->ctx);
}

static inline uint8_t Modem_GetSignalQuality(IModemPort_t *p)
{
  return p->getSignalQuality(p->ctx);
}

static inline void Modem_GetImei(IModemPort_t *p, char *buf, uint8_t len)
{
  p->getImei(p->ctx, buf, len);
}
