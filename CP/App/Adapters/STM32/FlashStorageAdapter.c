/*
 * App/Adapters/STM32/FlashStorageAdapter.c
 *
 * IPersistentStoragePort implementation — STM32H743 internal flash sector.
 *
 * All HAL_FLASH_* calls are TODO stubs.  The function signatures and
 * return codes are correct; wire them once the target flash bank/sector
 * addresses are confirmed in the linker script.
 */
#include "FlashStorageAdapter.h"
#include <string.h>

/* --------------------------------------------------------------------------
 * Forward declarations
 * --------------------------------------------------------------------------*/
static StorageResult_t Flash_Write(void *ctx, uint16_t key,
                                   const void *data, size_t len);
static StorageResult_t Flash_Read(void *ctx, uint16_t key,
                                  void *outBuf, size_t maxLen, size_t *outLen);
static StorageResult_t Flash_Erase(void *ctx, uint16_t key);

/* --------------------------------------------------------------------------
 * Internal helpers
 * --------------------------------------------------------------------------*/

/* Entry header stored in flash before each value blob. */
typedef struct __attribute__((packed))
{
  uint16_t key;
  uint16_t len;      /* Length of value in bytes */
} FlashEntryHeader_t;

#define ENTRY_HDR_SIZE  sizeof(FlashEntryHeader_t)
#define SECTOR_HDR_SIZE 8U   /* magic(4) + entryCount(2) + reserved(2) */

/* --------------------------------------------------------------------------
 * Public API
 * --------------------------------------------------------------------------*/

void FlashStorageAdapter_Init(FlashStorageAdapterCtx_t *ctx, uint32_t baseAddr)
{
  memset(ctx, 0, sizeof(*ctx));
  ctx->baseAddr = baseAddr;
  ctx->writeOffset = SECTOR_HDR_SIZE;
  ctx->initialised = false;

  #ifdef STM32H743xx

  /* TODO: HAL impl — validate sector header.
   *
   * uint32_t magic = *(volatile uint32_t *)baseAddr;
   * if (magic != FLASH_STORAGE_MAGIC) {
   *     // Sector blank or corrupt — erase and write header.
   *     FLASH_EraseSector(baseAddr);
   *     FLASH_WriteWord(baseAddr, FLASH_STORAGE_MAGIC);
   * } else {
   *     // Scan entries to find current writeOffset.
   *     ctx->writeOffset = FlashStorage_ScanEntries(ctx);
   * }
   */
  #endif

  ctx->initialised = true;
}

IPersistentStoragePort_t FlashStorageAdapter_CreatePort(
  FlashStorageAdapterCtx_t *ctx)
{
  IPersistentStoragePort_t port;

  port.ctx = ctx;
  port.write = Flash_Write;
  port.read = Flash_Read;
  port.erase = Flash_Erase;

  return port;
}

/* --------------------------------------------------------------------------
 * Port callbacks
 * --------------------------------------------------------------------------*/

static StorageResult_t Flash_Write(void *vctx, uint16_t key,
                                   const void *data, size_t len)
{
  FlashStorageAdapterCtx_t *ctx = (FlashStorageAdapterCtx_t *) vctx;

  if (!ctx->initialised)
  {
    return STORAGE_ERR_WRITE_FAILED;
  }

  if ((len == 0U) || (data == NULL) )
  {
    return STORAGE_ERR_WRITE_FAILED;
  }

  if ((ctx->writeOffset + ENTRY_HDR_SIZE + len) > FLASH_STORAGE_SECTOR_SIZE)
  {
    return STORAGE_ERR_TOO_LARGE;
  }

  (void) key;

  #ifdef STM32H743xx

  /* TODO: HAL impl — unlock flash, Program entry header + data, lock flash.
   *
   * HAL_FLASH_Unlock();
   *
   * FlashEntryHeader_t hdr = { .key = key, .len = (uint16_t)len };
   * uint32_t addr = ctx->baseAddr + ctx->writeOffset;
   * // Program in 32-bit words (HAL_FLASH_Program with FLASH_TYPEPROGRAM_FLASHWORD).
   * // ... Program hdr ...
   * // ... Program data ...
   *
   * ctx->writeOffset += (uint32_t)(ENTRY_HDR_SIZE + len);
   * // Align to 32-byte HAL flash word boundary if needed.
   *
   * HAL_FLASH_Lock();
   */
  #else
  /* Host build stub — pretend the write succeeded. */
  ctx->writeOffset += (uint32_t) (ENTRY_HDR_SIZE + len);
  #endif

  return STORAGE_OK;
}

static StorageResult_t Flash_Read(void *vctx, uint16_t key,
                                  void *outBuf, size_t maxLen, size_t *outLen)
{
  FlashStorageAdapterCtx_t *ctx = (FlashStorageAdapterCtx_t *) vctx;

  if (!ctx->initialised)
  {
    return STORAGE_ERR_READ_FAILED;
  }

  if ((outBuf == NULL) || (outLen == NULL) || (maxLen == 0U) )
  {
    return STORAGE_ERR_READ_FAILED;
  }

  *outLen = 0U;

  #ifdef STM32H743xx

  /* TODO: HAL impl — scan flash entries from SECTOR_HDR_SIZE to
   * ctx->writeOffset, tracking the most recent entry for `key`.
   * Copy its value into outBuf (up to maxLen bytes).
   *
   * uint32_t scanOffset = SECTOR_HDR_SIZE;
   * uint32_t foundOffset = 0;
   * bool found = false;
   * while (scanOffset + ENTRY_HDR_SIZE <= ctx->writeOffset) {
   *     FlashEntryHeader_t hdr;
   *     memcpy(&hdr, (void*)(ctx->baseAddr + scanOffset), ENTRY_HDR_SIZE);
   *     if (hdr.key == key) { foundOffset = scanOffset; found = true; }
   *     scanOffset += ENTRY_HDR_SIZE + hdr.len;
   * }
   * if (!found) return STORAGE_ERR_NOT_FOUND;
   * FlashEntryHeader_t hdr;
   * memcpy(&hdr, (void*)(ctx->baseAddr + foundOffset), ENTRY_HDR_SIZE);
   * size_t copyLen = (hdr.len < maxLen) ? hdr.len : maxLen;
   * memcpy(outBuf, (void*)(ctx->baseAddr + foundOffset + ENTRY_HDR_SIZE), copyLen);
   * *outLen = copyLen;
   */
  (void) key;

  return STORAGE_ERR_NOT_FOUND;
  #else
  (void) key;

  return STORAGE_ERR_NOT_FOUND;
  #endif
}

static StorageResult_t Flash_Erase(void *vctx, uint16_t key)
{
  FlashStorageAdapterCtx_t *ctx = (FlashStorageAdapterCtx_t *) vctx;

  if (!ctx->initialised)
  {
    return STORAGE_ERR_WRITE_FAILED;
  }

  /* NTCIP requirement: erasing a non-existent key is idempotent. */
  (void) key;

  #ifdef STM32H743xx

  /* TODO: HAL impl — if key == 0xFFFF erase the entire sector; otherwise
   * mark the most recent entry for that key as deleted (tombstone pattern).
   *
   * For a full sector erase:
   * FLASH_EraseInitTypeDef eraseInit = {
   *     .TypeErase = FLASH_TYPEERASE_SECTORS,
   *     .Banks     = FLASH_BANK_2,
   *     .Sector    = <sector number for ctx->baseAddr>,
   *     .NbSectors = 1,
   *     .VoltageRange = FLASH_VOLTAGE_RANGE_3,
   * };
   * uint32_t sectorError = 0;
   * HAL_FLASH_Unlock();
   * HAL_FLASHEx_Erase(&eraseInit, &sectorError);
   * HAL_FLASH_Lock();
   * ctx->writeOffset = SECTOR_HDR_SIZE;
   */
  #endif

  return STORAGE_OK;
}
