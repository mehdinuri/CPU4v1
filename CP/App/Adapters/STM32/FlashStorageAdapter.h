#pragma once

/*
 * App/Adapters/STM32/FlashStorageAdapter.h
 *
 * IPersistentStoragePort concrete implementation for STM32H743.
 * Stores key-value pairs in an internal flash sector using a simple
 * log-structured layout.  All flash operations are performed through
 * HAL_FLASH_* calls (TODO stubs).
 *
 * The flash sector is treated as a flat key-value store:
 *   [header: magic(4) | entryCount(2)] [entry: key(2) | len(2) | data(len)] ...
 *
 * On write, entries are appended.  On read, the most recent entry for a key
 * is returned.  Erase resets the entire sector.
 */
#include "Ports/IPersistentStoragePort.h"

#define FLASH_STORAGE_SECTOR_SIZE  (128U * 1024U)  /* 128 KB — one STM32H7 sector */
#define FLASH_STORAGE_MAGIC        0x4E544350U      /* "NTCP" */

typedef struct
{
  uint32_t baseAddr;      /* Flash sector base address (e.g. 0x08100000)    */
  uint32_t writeOffset;   /* Current append offset within the sector (bytes) */
  bool initialised;       /* True once the sector header has been validated  */
} FlashStorageAdapterCtx_t;

/**
 * Initialise the adapter.  Reads the sector header to validate magic and
 * determine writeOffset.  If the sector is blank or corrupt, it is erased.
 *
 * baseAddr: start address of the flash sector used for storage.
 */
void FlashStorageAdapter_Init(FlashStorageAdapterCtx_t *ctx, uint32_t baseAddr);

/** Build an IPersistentStoragePort_t wired to ctx. */
IPersistentStoragePort_t FlashStorageAdapter_CreatePort(
  FlashStorageAdapterCtx_t *ctx);
