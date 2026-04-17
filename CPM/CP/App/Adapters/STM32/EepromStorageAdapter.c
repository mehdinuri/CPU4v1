/* App/Adapters/STM32/EepromStorageAdapter.c
 *
 * IEepromStoragePort concrete implementation using STM32 I2C4 and the
 * board-level EEPROM write-protect GPIO.
 */
#include "EepromStorageAdapter.h"

#include "i2c.h"
#include "iwdg.h"

#define I2C_EEPROM_DEVICE_ADDR 0x00A0U
#define I2C_EEPROM_PAGE_SIZE 64U
#define I2C_XFER_TIMEOUT_MAX 300U
#define I2C_EEPROM_MAX_TRIALS 300U

static void AdapterEnableWriteProtect(void *ctx)
{
  (void) ctx;

  HAL_GPIO_WritePin(EEPROM_WP_GPIO_Port, EEPROM_WP_Pin, GPIO_PIN_SET);
}

static void AdapterDisableWriteProtect(void *ctx)
{
  (void) ctx;

  HAL_GPIO_WritePin(EEPROM_WP_GPIO_Port, EEPROM_WP_Pin, GPIO_PIN_RESET);
}

static uint8_t AdapterRead(void *ctx, uint32_t address, void *dst,
                           uint32_t size)
{
  (void) ctx;

  if ((dst == NULL) || (size == 0U))
  {
    return 0U;
  }

  return (HAL_I2C_Mem_Read(&hi2c4,
                           (uint16_t) I2C_EEPROM_DEVICE_ADDR,
                           (uint16_t) address,
                           I2C_MEMADD_SIZE_16BIT,
                           (uint8_t *) dst,
                           size,
                           I2C_XFER_TIMEOUT_MAX) == HAL_OK) ? 1U : 0U;
}

static uint8_t AdapterWrite(void *ctx, uint32_t address, const void *src,
                            uint32_t size)
{
  const uint8_t *pbData = (const uint8_t *) src;
  uint16_t writeAddr = (uint16_t) address;
  uint16_t bytesRemaining = (uint16_t) size;

  (void) ctx;

  if ((src == NULL) || (size == 0U))
  {
    return 0U;
  }

  AdapterDisableWriteProtect(ctx);

  while (bytesRemaining > 0U)
  {
    uint16_t bytesInPage = I2C_EEPROM_PAGE_SIZE
                           - (writeAddr % I2C_EEPROM_PAGE_SIZE);
    uint16_t bytesToWrite = (bytesRemaining < bytesInPage)
                            ? bytesRemaining
                            : bytesInPage;

    IWDGRefresh();

    if (HAL_I2C_Mem_Write(&hi2c4,
                          (uint16_t) I2C_EEPROM_DEVICE_ADDR,
                          writeAddr,
                          I2C_MEMADD_SIZE_16BIT,
                          (uint8_t *) pbData,
                          bytesToWrite,
                          I2C_XFER_TIMEOUT_MAX) != HAL_OK)
    {
      AdapterEnableWriteProtect(ctx);

      return 0U;
    }

    if (HAL_I2C_IsDeviceReady(&hi2c4,
                              I2C_EEPROM_DEVICE_ADDR,
                              I2C_EEPROM_MAX_TRIALS,
                              I2C_XFER_TIMEOUT_MAX) != HAL_OK)
    {
      AdapterEnableWriteProtect(ctx);

      return 0U;
    }

    bytesRemaining -= bytesToWrite;
    pbData += bytesToWrite;
    writeAddr += bytesToWrite;
  }

  AdapterEnableWriteProtect(ctx);

  return 1U;
} /* AdapterWrite */

void EepromStorageAdapterInit(EepromStorageAdapterCtx_t *ctx)
{
  ctx->reserved = 0U;
}

IEepromStoragePort_t EepromStorageAdapterCreatePort(
  EepromStorageAdapterCtx_t *ctx)
{
  IEepromStoragePort_t port;

  port.ctx = ctx;
  port.EnableWriteProtect = AdapterEnableWriteProtect;
  port.DisableWriteProtect = AdapterDisableWriteProtect;
  port.Read = AdapterRead;
  port.Write = AdapterWrite;

  return port;
}
