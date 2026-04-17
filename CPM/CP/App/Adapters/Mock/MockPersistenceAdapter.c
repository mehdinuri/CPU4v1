/* App/Adapters/Mock/MockPersistenceAdapter.c
 *
 * In-memory semantic persistence adapter for host-side tests.
 */
#include "MockPersistenceAdapter.h"

#include <string.h>

static uint8_t AdapterRead(void *ctx, PersistenceObjectId_t objectId,
                           uint32_t offset, void *dst, uint32_t size)
{
  MockPersistenceAdapterCtx_t *c = (MockPersistenceAdapterCtx_t *) ctx;

  if ((dst == NULL) || (objectId <= PERSIST_OBJECT_NONE)
      || (objectId >= PERSIST_OBJECT_LAST))
  {
    return 0U;
  }

  if ((offset > MOCK_PERSISTENCE_MAX_OBJECT_SIZE)
      || (size > (MOCK_PERSISTENCE_MAX_OBJECT_SIZE - offset)))
  {
    return 0U;
  }

  memcpy(dst, &c->data[objectId][offset], size);

  return 1U;
}

static uint32_t AdapterGetCapacity(void *ctx, PersistenceObjectId_t objectId)
{
  (void) ctx;

  if ((objectId <= PERSIST_OBJECT_NONE) || (objectId >= PERSIST_OBJECT_LAST))
  {
    return 0U;
  }

  return MOCK_PERSISTENCE_MAX_OBJECT_SIZE;
}

static uint8_t AdapterWrite(void *ctx, PersistenceObjectId_t objectId,
                            uint32_t offset, const void *src, uint32_t size)
{
  MockPersistenceAdapterCtx_t *c = (MockPersistenceAdapterCtx_t *) ctx;

  if ((src == NULL) || (objectId <= PERSIST_OBJECT_NONE)
      || (objectId >= PERSIST_OBJECT_LAST))
  {
    return 0U;
  }

  if ((offset > MOCK_PERSISTENCE_MAX_OBJECT_SIZE)
      || (size > (MOCK_PERSISTENCE_MAX_OBJECT_SIZE - offset)))
  {
    return 0U;
  }

  memcpy(&c->data[objectId][offset], src, size);

  return 1U;
}

static uint8_t AdapterErase(void *ctx, PersistenceObjectId_t objectId,
                            uint32_t offset, uint32_t size)
{
  MockPersistenceAdapterCtx_t *c = (MockPersistenceAdapterCtx_t *) ctx;

  if ((objectId <= PERSIST_OBJECT_NONE) || (objectId >= PERSIST_OBJECT_LAST))
  {
    return 0U;
  }

  if ((offset > MOCK_PERSISTENCE_MAX_OBJECT_SIZE)
      || (size > (MOCK_PERSISTENCE_MAX_OBJECT_SIZE - offset)))
  {
    return 0U;
  }

  memset(&c->data[objectId][offset], 0xFF, size);

  return 1U;
}

void MockPersistenceAdapterInit(MockPersistenceAdapterCtx_t *ctx)
{
  memset(ctx, 0xFF, sizeof(*ctx));
  ctx->initialised = 1U;
}

IPersistencePort_t MockPersistenceAdapterCreatePort(
  MockPersistenceAdapterCtx_t *ctx)
{
  IPersistencePort_t port;

  port.ctx = ctx;
  port.GetCapacity = AdapterGetCapacity;
  port.Read = AdapterRead;
  port.Write = AdapterWrite;
  port.Erase = AdapterErase;

  return port;
}
