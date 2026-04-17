/* App/Ports/IConfigRepositoryPort.h
 *
 * Dedicated repository port for immutable intersection configuration images.
 * Keeps slot/journal concerns out of the generic persistence API.
 */
#ifndef ICONFIG_REPOSITORY_PORT_H
#define ICONFIG_REPOSITORY_PORT_H

#include <stdint.h>

typedef enum
{
  CONFIG_REPOSITORY_OBJECT_NONE = 0,
  CONFIG_REPOSITORY_OBJECT_SLOT_A,
  CONFIG_REPOSITORY_OBJECT_SLOT_B,
  CONFIG_REPOSITORY_OBJECT_MIGRATION_JOURNAL,
  CONFIG_REPOSITORY_OBJECT_LAST
} ConfigRepositoryObjectId_t;

typedef struct
{
  void *ctx;

  uint32_t (*GetCapacity)(void *ctx, ConfigRepositoryObjectId_t objectId);
  uint8_t (*Read)(void *ctx, ConfigRepositoryObjectId_t objectId,
                  uint32_t offset, void *dst, uint32_t size);
  uint8_t (*Write)(void *ctx, ConfigRepositoryObjectId_t objectId,
                   uint32_t offset, const void *src, uint32_t size);
  uint8_t (*Erase)(void *ctx, ConfigRepositoryObjectId_t objectId,
                   uint32_t offset, uint32_t size);
} IConfigRepositoryPort_t;

static inline uint32_t ConfigRepositoryGetCapacity(IConfigRepositoryPort_t *p,
                                                   ConfigRepositoryObjectId_t
                                                   objectId)
{
  return p->GetCapacity(p->ctx, objectId);
}

static inline uint8_t ConfigRepositoryRead(IConfigRepositoryPort_t *p,
                                           ConfigRepositoryObjectId_t objectId,
                                           uint32_t offset,
                                           void *dst,
                                           uint32_t size)
{
  return p->Read(p->ctx, objectId, offset, dst, size);
}

static inline uint8_t ConfigRepositoryWrite(IConfigRepositoryPort_t *p,
                                            ConfigRepositoryObjectId_t objectId,
                                            uint32_t offset,
                                            const void *src,
                                            uint32_t size)
{
  return p->Write(p->ctx, objectId, offset, src, size);
}

static inline uint8_t ConfigRepositoryErase(IConfigRepositoryPort_t *p,
                                            ConfigRepositoryObjectId_t objectId,
                                            uint32_t offset,
                                            uint32_t size)
{
  return p->Erase(p->ctx, objectId, offset, size);
}

#endif /* ICONFIG_REPOSITORY_PORT_H */
