/* App/Ports/IEepromStoragePort.h
 *
 * Raw external EEPROM storage port. Callers operate on physical EEPROM
 * addresses and sizes; semantic object mapping belongs in higher-level
 * persistence adapters.
 */
#ifndef IEEPROM_STORAGE_PORT_H
#define IEEPROM_STORAGE_PORT_H

#include <stdint.h>

typedef struct
{
  void *ctx;

  void (*EnableWriteProtect)(void *ctx);
  void (*DisableWriteProtect)(void *ctx);
  uint8_t (*Read)(void *ctx, uint32_t address, void *dst, uint32_t size);
  uint8_t (*Write)(void *ctx, uint32_t address, const void *src, uint32_t size);
} IEepromStoragePort_t;

static inline void EepromStorageEnableWriteProtect(IEepromStoragePort_t *p)
{
  p->EnableWriteProtect(p->ctx);
}

static inline void EepromStorageDisableWriteProtect(IEepromStoragePort_t *p)
{
  p->DisableWriteProtect(p->ctx);
}

static inline uint8_t EepromStorageRead(IEepromStoragePort_t *p,
                                        uint32_t address,
                                        void *dst,
                                        uint32_t size)
{
  return p->Read(p->ctx, address, dst, size);
}

static inline uint8_t EepromStorageWrite(IEepromStoragePort_t *p,
                                         uint32_t address,
                                         const void *src,
                                         uint32_t size)
{
  return p->Write(p->ctx, address, src, size);
}

#endif /* IEEPROM_STORAGE_PORT_H */
