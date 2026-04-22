/**
 ******************************************************************************
 * @file    Ports/IPersistencePort.h
 * @brief   Port: persist typed blobs keyed by PersistenceKey_t.
 *          Adapters map keys → backing storage (flash page, EEPROM slot, etc.).
 *          Domain code never sees addresses or sizes — just keys.
 ******************************************************************************
 */

#ifndef PORTS_IPERSISTENCE_PORT_H
#define PORTS_IPERSISTENCE_PORT_H

#include <stdint.h>

typedef enum
{
  PERSIST_KEY_SIGNAL_OUTPUTS_FLASH = 0,      /* SignalFlashConfig_t */
  PERSIST_KEY__COUNT
} PersistenceKey_e;

typedef enum
{
  PERSIST_OK   = 0,
  PERSIST_FAIL = 1
} PersistenceStatus_e;

typedef struct IPersistencePort
{
  void *ctx;

  PersistenceStatus_e (*Read)(void *ctx,
                              PersistenceKey_e eKey,
                              void *out,
                              uint16_t size);
  PersistenceStatus_e (*Write)(void *ctx,
                               PersistenceKey_e eKey,
                               const void *in,
                               uint16_t size);
} IPersistencePort_t;

static inline PersistenceStatus_e Persistence_Read(IPersistencePort_t *port,
                                                   PersistenceKey_e eKey,
                                                   void *out,
                                                   uint16_t size)
{
  return port->Read(port->ctx, eKey, out, size);
}

static inline PersistenceStatus_e Persistence_Write(IPersistencePort_t *port,
                                                    PersistenceKey_e eKey,
                                                    const void *in,
                                                    uint16_t size)
{
  return port->Write(port->ctx, eKey, in, size);
}

#endif /* PORTS_IPERSISTENCE_PORT_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
