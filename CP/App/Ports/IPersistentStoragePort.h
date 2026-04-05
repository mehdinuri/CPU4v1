#pragma once

/*
 * App/Ports/IPersistentStoragePort.h
 *
 * Persistent key-value storage backed by EEPROM or internal flash.
 * Operations are synchronous from the caller's perspective; the concrete
 * adapter may queue them to an async FreeRTOS task internally.
 *
 * Keys are 16-bit identifiers defined in App/Domain/Services/storage_keys.h.
 */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Result codes for storage operations. */
typedef enum
{
  STORAGE_OK          = 0,
  STORAGE_ERR_NOT_FOUND,
  STORAGE_ERR_TOO_LARGE,
  STORAGE_ERR_WRITE_FAILED,
  STORAGE_ERR_READ_FAILED,
  STORAGE_ERR_BUSY,
} StorageResult_t;

typedef struct IPersistentStoragePort
{
  void *ctx;

  /* Write [len] bytes from [data] under [key]. Overwrites existing value. */
  StorageResult_t (*write)(void *ctx, uint16_t key,
                           const void *data, size_t len);

  /* Read up to [maxLen] bytes from [key] into [outBuf].
  * Sets [outLen] to the actual number of bytes read. */
  StorageResult_t (*read)(void *ctx, uint16_t key,
                          void *outBuf, size_t maxLen, size_t *outLen);

  /* Erase a key (idempotent — STORAGE_OK if key did not exist). */
  StorageResult_t (*erase)(void *ctx, uint16_t key);
} IPersistentStoragePort_t;

static inline StorageResult_t Storage_Write(IPersistentStoragePort_t *p,
                                            uint16_t key,
                                            const void *data,
                                            size_t len)
{
  return p->write(p->ctx, key, data, len);
}

static inline StorageResult_t Storage_Read(IPersistentStoragePort_t *p,
                                           uint16_t key,
                                           void *outBuf,
                                           size_t maxLen,
                                           size_t *outLen)
{
  return p->read(p->ctx, key, outBuf, maxLen, outLen);
}

static inline StorageResult_t Storage_Erase(IPersistentStoragePort_t *p,
                                            uint16_t key)
{
  return p->erase(p->ctx, key);
}
