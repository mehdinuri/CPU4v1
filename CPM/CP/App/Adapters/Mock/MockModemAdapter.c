/* App/Adapters/Mock/MockModemAdapter.c
 *
 * IModemPort_t in-memory test double.
 */
#include "MockModemAdapter.h"
#include <string.h>

static void AdapterOnInit(void *ctx, ISerialPort_t *serialPort)
{
  MockModemAdapterCtx_t *c = (MockModemAdapterCtx_t *) ctx;

  c->serialPort = serialPort;
}

static uint32_t AdapterGetBaudRate(void *ctx)
{
  (void) ctx;

  return 0U;
}

static uint8_t AdapterGetInitialState(void *ctx)
{
  (void) ctx;

  return 0U;
}

static uint8_t AdapterPrepareCommand(void       *ctx,
                                     uint8_t state,
                                     const char *apn,
                                     const char *host,
                                     uint16_t serverPort,
                                     char       *outBuf,
                                     uint16_t maxLen)
{
  MockModemAdapterCtx_t *c = (MockModemAdapterCtx_t *) ctx;

  (void) state;
  (void) apn;
  (void) host;
  (void) serverPort;

  if ((outBuf != NULL) && (maxLen > 0U))
  {
    (void) strncpy(outBuf, c->cmdBuf, (size_t) (maxLen - 1U));
    outBuf[maxLen - 1U] = '\0';
  }

  c->cmdCount++;

  return c->bPrepareResult;
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
  MockModemAdapterCtx_t *c = (MockModemAdapterCtx_t *) ctx;

  (void) state;
  (void) response;
  (void) responseOk;
  (void) pInfo;

  return c->bNextState;
}

static const char *AdapterGetStateLabel(void *ctx, uint8_t state)
{
  (void) ctx;
  (void) state;

  return "MOCK";
}

static uint8_t AdapterIsDisconnected(void       *ctx,
                                     const char *data,
                                     uint16_t len)
{
  MockModemAdapterCtx_t *c = (MockModemAdapterCtx_t *) ctx;

  (void) data;
  (void) len;

  return c->bDisconnected;
}

static uint8_t AdapterGetGreetingType(void *ctx)
{
  MockModemAdapterCtx_t *c = (MockModemAdapterCtx_t *) ctx;

  return c->bGreetingType;
}

static uint8_t AdapterOnConnect(void *ctx)
{
  MockModemAdapterCtx_t *c = (MockModemAdapterCtx_t *) ctx;

  return c->bConnectResult;
}

static void AdapterOnMaintain(void *ctx)
{
  (void) ctx;
}

static void AdapterOnRx(void *ctx, const uint8_t *data, uint16_t len)
{
  (void) ctx;
  (void) data;
  (void) len;
}

static void AdapterOnDisconnect(void *ctx)
{
  (void) ctx;
}

static uint8_t AdapterSend(void *ctx, const uint8_t *data, uint16_t len)
{
  MockModemAdapterCtx_t *c = (MockModemAdapterCtx_t *) ctx;

  if (len <= sizeof(c->cmdBuf))
  {
    memcpy(c->cmdBuf, data, len);
  }

  return 1U;
}

void MockModemAdapterInit(MockModemAdapterCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
  ctx->bConnectResult = 1U;
}

IModemPort_t MockModemAdapterCreatePort(MockModemAdapterCtx_t *ctx)
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

void MockModemAdapterInjectResponse(MockModemAdapterCtx_t *ctx,
                                    uint8_t nextState)
{
  ctx->bNextState = nextState;
}
