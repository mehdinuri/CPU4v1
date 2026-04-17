/* App/Adapters/Mock/MockDisplayAdapter.c */
#include "MockDisplayAdapter.h"
#include <string.h>

static void AdapterClear(void *ctx)
{
  MockDisplayAdapterCtx_t *c = (MockDisplayAdapterCtx_t *) ctx;

  memset(c->framebuffer, ' ', sizeof(c->framebuffer));
  c->clearCount++;
}

static void AdapterWrite(void *ctx, uint8_t row, uint8_t col,
                         const char *text, uint8_t len)
{
  MockDisplayAdapterCtx_t *c = (MockDisplayAdapterCtx_t *) ctx;
  uint8_t i;
  uint8_t destCol;

  if ((text == NULL) || (row >= MOCK_DISPLAY_ROWS))
  {
    return;
  }

  for (i = 0U; i < len; i++)
  {
    destCol = (uint8_t) (col + i);
    if (destCol >= (MOCK_DISPLAY_COLS - 1U))
    {
      break;
    }

    c->framebuffer[row][destCol] = text[i];
  }

  c->writeCount++;
}

static void AdapterPowerOn(void *ctx)
{
  MockDisplayAdapterCtx_t *c = (MockDisplayAdapterCtx_t *) ctx;

  c->powerOn = 1U;
}

static void AdapterPowerOff(void *ctx)
{
  MockDisplayAdapterCtx_t *c = (MockDisplayAdapterCtx_t *) ctx;

  c->powerOn = 0U;
}

void MockDisplayAdapterInit(MockDisplayAdapterCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
  memset(ctx->framebuffer, ' ', sizeof(ctx->framebuffer));
}

IDisplayPort_t MockDisplayAdapterCreatePort(MockDisplayAdapterCtx_t *ctx)
{
  IDisplayPort_t port;

  port.ctx = ctx;
  port.Clear = AdapterClear;
  port.Write = AdapterWrite;
  port.PowerOn = AdapterPowerOn;
  port.PowerOff = AdapterPowerOff;

  return port;
}
