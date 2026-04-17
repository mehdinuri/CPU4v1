/* App/Ports/ILogRepositoryPort.h
 *
 * Log repository port.
 * Owns log storage policy, ring-buffer metadata, and record access.
 */
#ifndef ILOG_REPOSITORY_PORT_H
#define ILOG_REPOSITORY_PORT_H

#include <stdint.h>

typedef struct
{
  void *ctx;

  uint8_t (*Append)(void *ctx, const void *record,
                    uint32_t recordSize, uint16_t *writtenIndex);
  uint8_t (*Read)(void *ctx, uint16_t index, void *record,
                  uint32_t recordSize);
  uint8_t (*Clear)(void *ctx);
  uint8_t (*Exists)(void *ctx);
  uint8_t (*IsIndexValid)(void *ctx, uint16_t index);
  uint16_t (*GetCount)(void *ctx);
  uint16_t (*GetWriteIndex)(void *ctx);
} ILogRepositoryPort_t;

static inline uint8_t LogRepositoryAppend(ILogRepositoryPort_t *p,
                                          const void *record,
                                          uint32_t recordSize,
                                          uint16_t *writtenIndex)
{
  return p->Append(p->ctx, record, recordSize, writtenIndex);
}

static inline uint8_t LogRepositoryRead(ILogRepositoryPort_t *p,
                                        uint16_t index,
                                        void *record,
                                        uint32_t recordSize)
{
  return p->Read(p->ctx, index, record, recordSize);
}

static inline uint8_t LogRepositoryClear(ILogRepositoryPort_t *p)
{
  return p->Clear(p->ctx);
}

static inline uint8_t LogRepositoryExists(ILogRepositoryPort_t *p)
{
  return p->Exists(p->ctx);
}

static inline uint8_t LogRepositoryIsIndexValid(ILogRepositoryPort_t *p,
                                                uint16_t index)
{
  return p->IsIndexValid(p->ctx, index);
}

static inline uint16_t LogRepositoryGetCount(ILogRepositoryPort_t *p)
{
  return p->GetCount(p->ctx);
}

static inline uint16_t LogRepositoryGetWriteIndex(ILogRepositoryPort_t *p)
{
  return p->GetWriteIndex(p->ctx);
}

#endif /* ILOG_REPOSITORY_PORT_H */
