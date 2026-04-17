/* App/Adapters/Mock/MockSerialAdapter.c
 *
 * ISerialPort_t in-memory test double.
 */
#include "MockSerialAdapter.h"
#include <string.h>

static uint8_t AdapterSend(void          *ctx,
                           const uint8_t *data,
                           uint16_t len,
                           uint32_t timeoutMs)
{
  MockSerialAdapterCtx_t *c = (MockSerialAdapterCtx_t *) ctx;
  uint16_t copyLen;

  (void) timeoutMs;

  copyLen = (len > MOCK_SERIAL_TX_BUF_SIZE) ? MOCK_SERIAL_TX_BUF_SIZE : len;
  memcpy(c->txBuf, data, copyLen);
  c->txLen = copyLen;
  c->txCount++;

  return c->txResult;
}

static uint8_t AdapterSetBaudRate(void *ctx, uint32_t baudRate)
{
  MockSerialAdapterCtx_t *c = (MockSerialAdapterCtx_t *) ctx;

  c->baudRate = baudRate;

  return 0U;
}

static void AdapterSetRxCallback(void    *ctx,
                                 void (*onRx)(void          *arg,
                                              const uint8_t *data,
                                              uint16_t len),
                                 void    *arg)
{
  MockSerialAdapterCtx_t *c = (MockSerialAdapterCtx_t *) ctx;

  c->onRx = onRx;
  c->onRxArg = arg;
}

void MockSerialAdapterInit(MockSerialAdapterCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
}

ISerialPort_t MockSerialAdapterCreatePort(MockSerialAdapterCtx_t *ctx)
{
  ISerialPort_t port;

  port.ctx = ctx;
  port.Send = AdapterSend;
  port.SetBaudRate = AdapterSetBaudRate;
  port.SetRxCallback = AdapterSetRxCallback;

  return port;
}

void MockSerialAdapterInjectRx(MockSerialAdapterCtx_t *ctx,
                               const uint8_t          *data,
                               uint16_t len)
{
  if (ctx->onRx != NULL)
  {
    ctx->onRx(ctx->onRxArg, data, len);
  }
}
