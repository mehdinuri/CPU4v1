/* App/Adapters/Mock/MockUserInputAdapter.c */
#include "MockUserInputAdapter.h"

static KeypadSnapshot_t AdapterScanSnapshot(void *ctx)
{
  MockUserInputAdapterCtx_t *c = (MockUserInputAdapterCtx_t *) ctx;
  KeypadSnapshot_t snapshot = c->nextSnapshot;

  c->nextSnapshot = KEYPAD_SNAPSHOT_NONE;
  c->scanCount++;

  return snapshot;
}

void MockUserInputAdapterInit(MockUserInputAdapterCtx_t *ctx)
{
  ctx->nextSnapshot = KEYPAD_SNAPSHOT_NONE;
  ctx->scanCount = 0U;
}

IUserInputPort_t MockUserInputAdapterCreatePort(MockUserInputAdapterCtx_t *ctx)
{
  IUserInputPort_t port;

  port.ctx = ctx;
  port.ScanSnapshot = AdapterScanSnapshot;

  return port;
}
