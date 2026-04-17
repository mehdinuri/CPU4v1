/* App/Adapters/STM32/PersistenceAdapter.c */

#include "PersistenceAdapter.h"

#include <stddef.h>

#include "i2c.h"
#include "stm32g4xx_hal.h"

#define PERSISTENCE_I2C_TIMEOUT_MS 100U

static uint8_t AdapterRead(void *ctx,
                           uint32_t address,
                           uint8_t *buffer,
                           uint32_t length)
{
  PersistenceAdapterCtx_t *self = (PersistenceAdapterCtx_t *) ctx;

  if ((self == NULL) || (buffer == NULL))
  {
    return 0U;
  }

  if ((address + length) > PERSISTENCE_EEPROM_SIZE_BYTES)
  {
    return 0U;
  }

  if (HAL_I2C_Mem_Read(&hi2c3,
                       PERSISTENCE_EEPROM_DEVICE_ADDRESS,
                       (uint16_t) address,
                       I2C_MEMADD_SIZE_16BIT,
                       buffer,
                       (uint16_t) length,
                       PERSISTENCE_I2C_TIMEOUT_MS) != HAL_OK)
  {
    return 0U;
  }

  self->bytesRead += length;

  return 1U;
}

static uint8_t AdapterWrite(void *ctx,
                            uint32_t address,
                            const uint8_t *buffer,
                            uint32_t length)
{
  PersistenceAdapterCtx_t *self = (PersistenceAdapterCtx_t *) ctx;
  uint32_t remaining = length;
  uint32_t cursor = 0U;

  if ((self == NULL) || (buffer == NULL))
  {
    return 0U;
  }

  if ((address + length) > PERSISTENCE_EEPROM_SIZE_BYTES)
  {
    return 0U;
  }

  while (remaining > 0U)
  {
    uint32_t pageOffset = (address + cursor) % PERSISTENCE_EEPROM_PAGE_BYTES;
    uint32_t chunk = PERSISTENCE_EEPROM_PAGE_BYTES - pageOffset;

    if (chunk > remaining)
    {
      chunk = remaining;
    }

    if (HAL_I2C_Mem_Write(&hi2c3,
                          PERSISTENCE_EEPROM_DEVICE_ADDRESS,
                          (uint16_t) (address + cursor),
                          I2C_MEMADD_SIZE_16BIT,
                          (uint8_t *) &buffer[cursor],
                          (uint16_t) chunk,
                          PERSISTENCE_I2C_TIMEOUT_MS) != HAL_OK)
    {
      return 0U;
    }

    /* EEPROM write cycle completion poll. */
    while (HAL_I2C_IsDeviceReady(&hi2c3,
                                 PERSISTENCE_EEPROM_DEVICE_ADDRESS,
                                 1U,
                                 PERSISTENCE_I2C_TIMEOUT_MS) != HAL_OK)
    {
      /* spin */
    }

    cursor += chunk;
    remaining -= chunk;
  }

  self->bytesWritten += length;

  return 1U;
} /* AdapterWrite */

static uint8_t AdapterSize(void *ctx, uint32_t *size)
{
  (void) ctx;

  if (size == NULL)
  {
    return 0U;
  }

  *size = PERSISTENCE_EEPROM_SIZE_BYTES;

  return 1U;
}

void PersistenceAdapterInit(PersistenceAdapterCtx_t *ctx)
{
  if (ctx == NULL)
  {
    return;
  }

  ctx->bytesWritten = 0U;
  ctx->bytesRead = 0U;
}

IPersistencePort_t PersistenceAdapterCreatePort(PersistenceAdapterCtx_t *ctx)
{
  IPersistencePort_t port;

  port.ctx = ctx;
  port.Read = AdapterRead;
  port.Write = AdapterWrite;
  port.Size = AdapterSize;

  return port;
}
