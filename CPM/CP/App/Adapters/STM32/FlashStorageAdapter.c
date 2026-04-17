/* App/Adapters/STM32/FlashStorageAdapter.c
 *
 * IFlashStoragePort concrete implementation using the STM32 internal
 * flash helper module.
 */
#include "FlashStorageAdapter.h"

#include "flash.h"

static uint8_t AdapterRead(void *ctx, uint32_t address, void *dst,
                           uint32_t size)
{
  (void) ctx;

  return FlashRead(address, dst, size);
}

static uint8_t AdapterWrite(void *ctx, uint32_t address, const void *src,
                            uint32_t size)
{
  (void) ctx;

  return FlashWrite(address, (void *) src, size);
}

static uint8_t AdapterErase(void *ctx, uint32_t address, uint32_t size)
{
  (void) ctx;

  return FlashErase(address, size);
}

void FlashStorageAdapterInit(FlashStorageAdapterCtx_t *ctx)
{
  ctx->reserved = 0U;
}

IFlashStoragePort_t FlashStorageAdapterCreatePort(FlashStorageAdapterCtx_t *ctx)
{
  IFlashStoragePort_t port;

  port.ctx = ctx;
  port.Read = AdapterRead;
  port.Write = AdapterWrite;
  port.Erase = AdapterErase;

  return port;
}
