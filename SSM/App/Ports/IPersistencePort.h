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
  PERSIST_KEY_SIGNAL_OUTPUTS_FLASH = 0,      /* tSSignalFlashConfig */
  PERSIST_KEY__COUNT
} tEPersistenceKey;

typedef enum
{
  PERSIST_OK   = 0,
  PERSIST_FAIL = 1
} tEPersistenceStatus;

typedef struct IPersistencePort
{
  void *pCtx;

  tEPersistenceStatus (*Read)(void *pCtx,
                              tEPersistenceKey eKey,
                              void *pOut,
                              uint16_t sSize);
  tEPersistenceStatus (*Write)(void *pCtx,
                               tEPersistenceKey eKey,
                               const void *pIn,
                               uint16_t sSize);
} IPersistencePort_t;

static inline tEPersistenceStatus Persistence_Read(IPersistencePort_t *pPort,
                                                   tEPersistenceKey eKey,
                                                   void *pOut,
                                                   uint16_t sSize)
{
  return pPort->Read(pPort->pCtx, eKey, pOut, sSize);
}

static inline tEPersistenceStatus Persistence_Write(IPersistencePort_t *pPort,
                                                    tEPersistenceKey eKey,
                                                    const void *pIn,
                                                    uint16_t sSize)
{
  return pPort->Write(pPort->pCtx, eKey, pIn, sSize);
}

#endif /* PORTS_IPERSISTENCE_PORT_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
