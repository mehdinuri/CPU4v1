/* App/Adapters/Mock/MockConfigRepositoryAdapter.c
 *
 * In-memory immutable configuration repository for host-side tests.
 */
#include "MockConfigRepositoryAdapter.h"

#include <string.h>

static uint8_t *ObjectBase(MockConfigRepositoryAdapterCtx_t *ctx,
                           ConfigRepositoryObjectId_t objectId,
                           uint32_t *capacity)
{
  switch (objectId)
  {
      case CONFIG_REPOSITORY_OBJECT_SLOT_A:
      {
        *capacity = MOCK_CONFIG_REPOSITORY_SLOT_SIZE;

        return ctx->slotA;
      }

      case CONFIG_REPOSITORY_OBJECT_SLOT_B:
      {
        *capacity = MOCK_CONFIG_REPOSITORY_SLOT_SIZE;

        return ctx->slotB;
      }

      case CONFIG_REPOSITORY_OBJECT_MIGRATION_JOURNAL:
      {
        *capacity = MOCK_CONFIG_REPOSITORY_JOURNAL_SIZE;

        return ctx->journal;
      }

      case CONFIG_REPOSITORY_OBJECT_NONE:
      case CONFIG_REPOSITORY_OBJECT_LAST:
      default:
      {
        *capacity = 0U;

        return NULL;
      }
  }
}

static uint8_t RangeIsValid(uint32_t capacity, uint32_t offset, uint32_t size)
{
  if (capacity == 0U)
  {
    return 0U;
  }

  if (size == 0U)
  {
    return 0U;
  }

  if (offset > capacity)
  {
    return 0U;
  }

  if (size > (capacity - offset))
  {
    return 0U;
  }

  return 1U;
}

static uint32_t AdapterGetCapacity(void *ctx,
                                   ConfigRepositoryObjectId_t objectId)
{
  MockConfigRepositoryAdapterCtx_t *c =
    (MockConfigRepositoryAdapterCtx_t *) ctx;
  uint32_t capacity;

  if (ObjectBase(c, objectId, &capacity) == NULL)
  {
    return 0U;
  }

  return capacity;
}

static uint8_t AdapterRead(void *ctx, ConfigRepositoryObjectId_t objectId,
                           uint32_t offset, void *dst, uint32_t size)
{
  MockConfigRepositoryAdapterCtx_t *c =
    (MockConfigRepositoryAdapterCtx_t *) ctx;
  uint32_t capacity;
  uint8_t *base = ObjectBase(c, objectId, &capacity);

  if ((dst == NULL) || (base == NULL) || (RangeIsValid(capacity, offset,
                                                       size) == 0U))
  {
    return 0U;
  }

  memcpy(dst, &base[offset], size);

  return 1U;
}

static uint8_t AdapterWrite(void *ctx, ConfigRepositoryObjectId_t objectId,
                            uint32_t offset, const void *src, uint32_t size)
{
  MockConfigRepositoryAdapterCtx_t *c =
    (MockConfigRepositoryAdapterCtx_t *) ctx;
  uint32_t capacity;
  uint8_t *base = ObjectBase(c, objectId, &capacity);

  if ((src == NULL) || (base == NULL) || (RangeIsValid(capacity, offset,
                                                       size) == 0U))
  {
    return 0U;
  }

  memcpy(&base[offset], src, size);

  return 1U;
}

static uint8_t AdapterErase(void *ctx, ConfigRepositoryObjectId_t objectId,
                            uint32_t offset, uint32_t size)
{
  MockConfigRepositoryAdapterCtx_t *c =
    (MockConfigRepositoryAdapterCtx_t *) ctx;
  uint32_t capacity;
  uint8_t *base = ObjectBase(c, objectId, &capacity);

  if ((base == NULL) || (RangeIsValid(capacity, offset, size) == 0U))
  {
    return 0U;
  }

  memset(&base[offset], 0xFF, size);

  return 1U;
}

void MockConfigRepositoryAdapterInit(MockConfigRepositoryAdapterCtx_t *ctx)
{
  memset(ctx, 0xFF, sizeof(*ctx));
  ctx->initialised = 1U;
}

IConfigRepositoryPort_t MockConfigRepositoryAdapterCreatePort(
  MockConfigRepositoryAdapterCtx_t *ctx)
{
  IConfigRepositoryPort_t port;

  port.ctx = ctx;
  port.GetCapacity = AdapterGetCapacity;
  port.Read = AdapterRead;
  port.Write = AdapterWrite;
  port.Erase = AdapterErase;

  return port;
}
