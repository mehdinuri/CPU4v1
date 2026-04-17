/* App/Adapters/STM32/ConfigRepositoryAdapter.c
 *
 * Thin adapter from immutable configuration repository semantics to the
 * platform persistence service.
 */
#include "ConfigRepositoryAdapter.h"

#include <stddef.h>

static PersistenceObjectId_t ObjectToPersistenceId(
  ConfigRepositoryObjectId_t objectId)
{
  switch (objectId)
  {
      case CONFIG_REPOSITORY_OBJECT_SLOT_A:
      {
        return PERSIST_OBJECT_CONFIG_SLOT_A;
      }

      case CONFIG_REPOSITORY_OBJECT_SLOT_B:
      {
        return PERSIST_OBJECT_CONFIG_SLOT_B;
      }

      case CONFIG_REPOSITORY_OBJECT_MIGRATION_JOURNAL:
      {
        return PERSIST_OBJECT_CONFIG_MIGRATION_JOURNAL;
      }

      case CONFIG_REPOSITORY_OBJECT_NONE:
      case CONFIG_REPOSITORY_OBJECT_LAST:
      default:
      {
        return PERSIST_OBJECT_NONE;
      }
  }
}

static uint32_t AdapterGetCapacity(void *ctx,
                                   ConfigRepositoryObjectId_t objectId)
{
  ConfigRepositoryAdapterCtx_t *c = (ConfigRepositoryAdapterCtx_t *) ctx;
  PersistenceObjectId_t persistenceId = ObjectToPersistenceId(objectId);

  if ((c == NULL) || (c->persistencePort == NULL)
      || (persistenceId == PERSIST_OBJECT_NONE))
  {
    return 0U;
  }

  return PersistenceGetCapacity(c->persistencePort, persistenceId);
}

static uint8_t AdapterRead(void *ctx, ConfigRepositoryObjectId_t objectId,
                           uint32_t offset, void *dst, uint32_t size)
{
  ConfigRepositoryAdapterCtx_t *c = (ConfigRepositoryAdapterCtx_t *) ctx;
  PersistenceObjectId_t persistenceId = ObjectToPersistenceId(objectId);

  if ((c == NULL) || (c->persistencePort == NULL)
      || (persistenceId == PERSIST_OBJECT_NONE))
  {
    return 0U;
  }

  return PersistenceRead(c->persistencePort, persistenceId, offset, dst, size);
}

static uint8_t AdapterWrite(void *ctx, ConfigRepositoryObjectId_t objectId,
                            uint32_t offset, const void *src, uint32_t size)
{
  ConfigRepositoryAdapterCtx_t *c = (ConfigRepositoryAdapterCtx_t *) ctx;
  PersistenceObjectId_t persistenceId = ObjectToPersistenceId(objectId);

  if ((c == NULL) || (c->persistencePort == NULL)
      || (persistenceId == PERSIST_OBJECT_NONE))
  {
    return 0U;
  }

  return PersistenceWrite(c->persistencePort, persistenceId, offset, src, size);
}

static uint8_t AdapterErase(void *ctx, ConfigRepositoryObjectId_t objectId,
                            uint32_t offset, uint32_t size)
{
  ConfigRepositoryAdapterCtx_t *c = (ConfigRepositoryAdapterCtx_t *) ctx;
  PersistenceObjectId_t persistenceId = ObjectToPersistenceId(objectId);

  if ((c == NULL) || (c->persistencePort == NULL)
      || (persistenceId == PERSIST_OBJECT_NONE))
  {
    return 0U;
  }

  return PersistenceErase(c->persistencePort, persistenceId, offset, size);
}

void ConfigRepositoryAdapterInit(ConfigRepositoryAdapterCtx_t *ctx,
                                 IPersistencePort_t *persistencePort)
{
  ctx->persistencePort = persistencePort;
}

IConfigRepositoryPort_t ConfigRepositoryAdapterCreatePort(
  ConfigRepositoryAdapterCtx_t *ctx)
{
  IConfigRepositoryPort_t port;

  port.ctx = ctx;
  port.GetCapacity = AdapterGetCapacity;
  port.Read = AdapterRead;
  port.Write = AdapterWrite;
  port.Erase = AdapterErase;

  return port;
}
