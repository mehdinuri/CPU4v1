/* App/Ports/IPersistencePort.h
 *
 * Semantic persistence port for non-log data.
 * Callers address persisted objects by meaning, not by flash sector,
 * EEPROM address, or storage medium.
 */
#ifndef IPERSISTENCE_PORT_H
#define IPERSISTENCE_PORT_H

#include <stdint.h>

typedef enum
{
  PERSIST_OBJECT_NONE = 0,
  PERSIST_OBJECT_DAYLIGHT_SAVING_FLAG,
  PERSIST_OBJECT_ADMIN_VALIDITY,
  PERSIST_OBJECT_ADMIN_USERNAME,
  PERSIST_OBJECT_ADMIN_PASSWORD,
  PERSIST_OBJECT_USER_SETTINGS,
  PERSIST_OBJECT_BROKEN_INPUT_SETTINGS,
  PERSIST_OBJECT_SERVER_SETTINGS,
  PERSIST_OBJECT_SIGNAL_OUTPUT_POWERS,
  PERSIST_OBJECT_USER_REQUEST,
  PERSIST_OBJECT_FUNCTION_CONFIG,
  PERSIST_OBJECT_LOG_SETTINGS,
  PERSIST_OBJECT_SYSTEM_START_TIME,
  PERSIST_OBJECT_LRLF_DETECT_TIME,
  PERSIST_OBJECT_DEVICE_UID,
  PERSIST_OBJECT_MCS_CONNECTION_INFO,
  PERSIST_OBJECT_GPS_PORT,
  PERSIST_OBJECT_GPS_BAUD_RATE,
  PERSIST_OBJECT_LANGUAGE,
  PERSIST_OBJECT_CONFIG_SLOT_A,
  PERSIST_OBJECT_CONFIG_SLOT_B,
  PERSIST_OBJECT_CONFIG_MIGRATION_JOURNAL,
  PERSIST_OBJECT_SNMPV3_STATE,
  PERSIST_OBJECT_AUTH_STATE,
  PERSIST_OBJECT_LAST
} PersistenceObjectId_t;

typedef struct
{
  void *ctx;

  uint32_t (*GetCapacity)(void *ctx, PersistenceObjectId_t objectId);
  uint8_t (*Read)(void *ctx, PersistenceObjectId_t objectId,
                  uint32_t offset, void *dst, uint32_t size);
  uint8_t (*Write)(void *ctx, PersistenceObjectId_t objectId,
                   uint32_t offset, const void *src, uint32_t size);
  uint8_t (*Erase)(void *ctx, PersistenceObjectId_t objectId,
                   uint32_t offset, uint32_t size);
} IPersistencePort_t;

static inline uint32_t PersistenceGetCapacity(IPersistencePort_t *p,
                                              PersistenceObjectId_t objectId)
{
  return p->GetCapacity(p->ctx, objectId);
}

static inline uint8_t PersistenceRead(IPersistencePort_t *p,
                                      PersistenceObjectId_t objectId,
                                      uint32_t offset,
                                      void *dst,
                                      uint32_t size)
{
  return p->Read(p->ctx, objectId, offset, dst, size);
}

static inline uint8_t PersistenceWrite(IPersistencePort_t *p,
                                       PersistenceObjectId_t objectId,
                                       uint32_t offset,
                                       const void *src,
                                       uint32_t size)
{
  return p->Write(p->ctx, objectId, offset, src, size);
}

static inline uint8_t PersistenceErase(IPersistencePort_t *p,
                                       PersistenceObjectId_t objectId,
                                       uint32_t offset,
                                       uint32_t size)
{
  return p->Erase(p->ctx, objectId, offset, size);
}

#endif /* IPERSISTENCE_PORT_H */
