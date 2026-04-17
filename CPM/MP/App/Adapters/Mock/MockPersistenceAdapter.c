/* App/Adapters/Mock/MockPersistenceAdapter.c */

#include "MockPersistenceAdapter.h"

#include <string.h>

static uint8_t MockRead(void *ctx,
                        uint32_t address,
                        uint8_t *buffer,
                        uint32_t length)
{
  const MockPersistenceAdapterCtx_t *self =
    (const MockPersistenceAdapterCtx_t *) ctx;

  if ((self == NULL) || (buffer == NULL))
  {
    return 0U;
  }

  if ((address + length) > MOCK_PERSISTENCE_SIZE)
  {
    return 0U;
  }

  (void) memcpy(buffer, &self->bytes[address], length);

  return 1U;
}

static uint8_t MockWrite(void *ctx,
                         uint32_t address,
                         const uint8_t *buffer,
                         uint32_t length)
{
  MockPersistenceAdapterCtx_t *self =
    (MockPersistenceAdapterCtx_t *) ctx;

  if ((self == NULL) || (buffer == NULL))
  {
    return 0U;
  }

  if ((address + length) > MOCK_PERSISTENCE_SIZE)
  {
    return 0U;
  }

  (void) memcpy(&self->bytes[address], buffer, length);

  return 1U;
}

static uint8_t MockSize(void *ctx, uint32_t *size)
{
  (void) ctx;

  if (size == NULL)
  {
    return 0U;
  }

  *size = MOCK_PERSISTENCE_SIZE;

  return 1U;
}

void MockPersistenceAdapterInit(MockPersistenceAdapterCtx_t *ctx)
{
  if (ctx == NULL)
  {
    return;
  }

  (void) memset(ctx, 0, sizeof(*ctx));
}

IPersistencePort_t MockPersistenceAdapterCreatePort(
  MockPersistenceAdapterCtx_t *ctx)
{
  IPersistencePort_t port;

  port.ctx = ctx;
  port.Read = MockRead;
  port.Write = MockWrite;
  port.Size = MockSize;

  return port;
}
