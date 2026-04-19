/* App/Adapters/STM32/UsrModemAdapter.c
 *
 * IModemPort concrete implementation for the USR TCP232-200 transparent
 * TCP bridge module.  The USR module connects automatically at power-up
 * with no AT command exchange; this adapter returns MODEM_STATE_CONNECTED
 * immediately from the single CONNECT state.
 */
#include "UsrModemAdapter.h"

#include <string.h>
#include "MCS.h"
#include "MCSAsynch.h"

/* ------------------------------------------------------------------
 * Private state enum — authoritative state enum for USR TCP232-200 adapter.
 * ------------------------------------------------------------------ */
typedef enum
{
  USR_CONNECT     = 0,
  USR_STATE_TOTAL = 1
} UsrState_t;

/* State label table — indexed by UsrState_t. */
static const char *const s_labels[USR_STATE_TOTAL] =
{
  /* 0 USR_CONNECT */ "CONNECTING...",
};

/* ------------------------------------------------------------------
 * IModemPort_t function implementations
 * ------------------------------------------------------------------ */

static void AdapterOnInit(void *ctx, ISerialPort_t *serialPort)
{
  UsrModemAdapterCtx_t *c = (UsrModemAdapterCtx_t *) ctx;

  c->serialPort = serialPort;
}

/* USR module uses a serial port but the baud rate is pre-configured
 * in the module hardware; the coordinator must not change it. */
static uint32_t AdapterGetBaudRate(void *ctx)
{
  (void) ctx;

  return 0U;
}

static uint8_t AdapterGetInitialState(void *ctx)
{
  (void) ctx;

  return (uint8_t) USR_CONNECT;
}

static uint8_t AdapterPrepareCommand(void       *ctx,
                                     uint8_t state,
                                     const char *apn,
                                     const char *host,
                                     uint16_t serverPort,
                                     char       *outBuf,
                                     uint16_t maxLen)
{
  (void) ctx;
  (void) state;
  (void) apn;
  (void) host;
  (void) serverPort;
  (void) maxLen;

  /* USR module uses transparent TCP bridging; no AT commands. */
  outBuf[0] = '\0';

  return 0U;
}

static void AdapterGetWaitParams(void          *ctx,
                                 uint8_t state,
                                 const char   **kw,
                                 char          *sc,
                                 uint32_t      *tms,
                                 uint8_t       *mr)
{
  (void) ctx;
  (void) state;

  /* Short timeout — HandleResponse advances immediately regardless. */
  *kw = "OK";
  *sc = '\0';
  *tms = 100U;
  *mr = 1U;
}

static uint8_t AdapterHandleResponse(void        *ctx,
                                     uint8_t state,
                                     const char  *response,
                                     uint8_t responseOk,
                                     ModemInfo_t *pInfo)
{
  (void) ctx;
  (void) state;
  (void) response;
  (void) responseOk;

  /* USR module connects immediately; no response validation needed. */
  pInfo->bSignalQuality = 0U;
  pInfo->bSignalQualityValid = 1U;

  return MODEM_STATE_CONNECTED;
}

static uint8_t AdapterOnConnect(void *ctx)
{
  (void) ctx;
  if (!MCSAsynchConnectedGet())
  {
    if (!MCSAsynchStart(MODEM_GREETING_USR_MAC))
    {
      MCSJobAdd("MCS CON. ERROR");
      MCSSetModemAlive(FALSE);

      return 0U;
    }

    MCSJobAdd("MCS CON. SUC.");
  }

  MCSSetModemAlive(MCSAsynchConnectedGet());

  return 1U;
}

static void AdapterOnMaintain(void *ctx)
{
  (void) ctx;
  if (MCSAsynchConnectedGet())
  {
    if (MCSAsynchIsRemoteEndClosed())
    {
      MCSAsynchStop(MCS_ASYNCH_DISC_TYPE_REMOTE_END_DISCON);
    }

    MCSAsynchCheckConnectionTimeout();
  }
}

static void AdapterOnRx(void *ctx, const uint8_t *data, uint16_t len)
{
  (void) ctx;
  if (MCSAsynchConnectedGet())
  {
    (void) MCSAsynchReqRxMsg((uint8_t *) data, len);
  }
  else
  {
    MCSRingBufferWrite(data, len);
  }
}

static void AdapterOnDisconnect(void *ctx)
{
  (void) ctx;
  if (MCSAsynchConnectedGet())
  {
    MCSAsynchStop(MCS_ASYNCH_DISC_TYPE_CON_INFO_CHANGED);
  }
}

static uint8_t AdapterSend(void *ctx, const uint8_t *data, uint16_t len)
{
  UsrModemAdapterCtx_t *c = (UsrModemAdapterCtx_t *) ctx;

  return (SerialSend(c->serialPort, data, len, 1000) == len) ? 1U : 0U;
}

static const char *AdapterGetStateLabel(void *ctx, uint8_t state)
{
  (void) ctx;
  if (state < (uint8_t) USR_STATE_TOTAL)
  {
    return s_labels[state];
  }

  return "USR ?";
}

static uint8_t AdapterIsDisconnected(void       *ctx,
                                     const char *data,
                                     uint16_t len)
{
  /* USR module transparent bridge — no disconnect keywords expected. */
  (void) ctx;
  (void) data;
  (void) len;

  return 0U;
}

static uint8_t AdapterGetGreetingType(void *ctx)
{
  (void) ctx;

  return MODEM_GREETING_USR_MAC;
}

/* ------------------------------------------------------------------
 * Public API
 * ------------------------------------------------------------------ */

void UsrModemAdapterInit(UsrModemAdapterCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
}

IModemPort_t UsrModemAdapterCreatePort(UsrModemAdapterCtx_t *ctx)
{
  IModemPort_t port;

  port.ctx = ctx;
  port.OnInit = AdapterOnInit;
  port.GetBaudRate = AdapterGetBaudRate;
  port.GetInitialState = AdapterGetInitialState;
  port.PrepareCommand = AdapterPrepareCommand;
  port.GetWaitParams = AdapterGetWaitParams;
  port.HandleResponse = AdapterHandleResponse;
  port.GetStateLabel = AdapterGetStateLabel;
  port.IsDisconnected = AdapterIsDisconnected;
  port.GetGreetingType = AdapterGetGreetingType;
  port.OnConnect = AdapterOnConnect;
  port.OnMaintain = AdapterOnMaintain;
  port.OnRx = AdapterOnRx;
  port.OnDisconnect = AdapterOnDisconnect;
  port.Send = AdapterSend;

  return port;
}
