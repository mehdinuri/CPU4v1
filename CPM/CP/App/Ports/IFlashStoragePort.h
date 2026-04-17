/* App/Ports/IFlashStoragePort.h
 *
 * Raw internal flash storage port. Callers operate on physical flash
 * addresses and sizes; semantic object mapping belongs in higher-level
 * persistence adapters.
 */
#ifndef IFLASH_STORAGE_PORT_H
#define IFLASH_STORAGE_PORT_H

#include <stdint.h>

typedef struct
{
  void *ctx;

  uint8_t (*Read)(void *ctx, uint32_t address, void *dst, uint32_t size);
  uint8_t (*Write)(void *ctx, uint32_t address, const void *src, uint32_t size);
  uint8_t (*Erase)(void *ctx, uint32_t address, uint32_t size);
} IFlashStoragePort_t;

static inline uint8_t FlashStorageRead(IFlashStoragePort_t *p,
                                       uint32_t address,
                                       void *dst,
                                       uint32_t size)
{
  return p->Read(p->ctx, address, dst, size);
}

static inline uint8_t FlashStorageWrite(IFlashStoragePort_t *p,
                                        uint32_t address,
                                        const void *src,
                                        uint32_t size)
{
  return p->Write(p->ctx, address, src, size);
}

static inline uint8_t FlashStorageErase(IFlashStoragePort_t *p,
                                        uint32_t address,
                                        uint32_t size)
{
  return p->Erase(p->ctx, address, size);
}

#endif /* IFLASH_STORAGE_PORT_H */
