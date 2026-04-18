/**
 ******************************************************************************
 * @file    Adapters/STM32/EepromAdapter.c
 * @brief   STM32 adapter for IEepromPort — routes through StorageRequest() so
 *          all I2C access remains serialised through StorageTask.
 ******************************************************************************
 */

#include "EepromAdapter.h"
#include "Storage.h"

/* ---------------------------------------------------------------------------
 * Private adapter implementations
 * ---------------------------------------------------------------------------*/
static uint8_t AdapterRead(void *ctx, uint32_t address, void *dst, uint32_t size)
{
  (void) ctx;
  return StorageRequest(STORAGE_REQ_EEPROM_READ, address, dst, size);
}

static uint8_t AdapterWrite(void *ctx, uint32_t address, const void *src, uint32_t size)
{
  (void) ctx;
  /* StorageRequest takes void* for data (legacy interface).  The callee only
   * reads the buffer for a WRITE request.  The cast-through-uintptr_t is the
   * MISRA-compliant way to remove const without triggering -Wcast-qual.
   * TODO: fix StorageRequest() to accept const void* and remove this cast. */
  return StorageRequest(STORAGE_REQ_EEPROM_WRITE, address, (void *)(uintptr_t) src, size);
}

/* ---------------------------------------------------------------------------
 * Public adapter API
 * ---------------------------------------------------------------------------*/
void EepromAdapterInit(EepromAdapterCtx_t *ctx)
{
  ctx->bInitialised = 1U;
}

IEepromPort_t EepromAdapterCreatePort(EepromAdapterCtx_t *ctx)
{
  IEepromPort_t port;
  port.ctx   = ctx;
  port.Read  = AdapterRead;
  port.Write = AdapterWrite;
  return port;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
