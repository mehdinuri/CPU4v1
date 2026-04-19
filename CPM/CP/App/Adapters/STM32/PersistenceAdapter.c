/* App/Adapters/STM32/PersistenceAdapter.c
 *
 * Maps semantic persistence objects to the legacy MSM-managed flash and
 * EEPROM storage layout.
 */
#include "PersistenceAdapter.h"

#include <stddef.h>

#include "IAP.h"
#include "MSM.h"
#include "data.h"

typedef struct
{
  PersistenceObjectId_t objectId;
  uint8_t readReqId;
  uint8_t writeReqId;
  uint8_t eraseReqId;
  uint32_t baseAddress;
  uint32_t size;
} PersistenceLayout_t;

#define STM32_SECTOR_SIZE_BYTES (128U * 1024U)

static const PersistenceLayout_t saLayout[] = {
  { PERSIST_OBJECT_PROGRAM_LOADING_FLAG, MSM_REQ_EEPROM_READ,
    MSM_REQ_EEPROM_WRITE, MSM_REQ_NONE,
    EEPROM_STORAGE_ADDR_PROGRAM_LOADING, sizeof(uint8_t) },
  { PERSIST_OBJECT_DAYLIGHT_SAVING_FLAG, MSM_REQ_EEPROM_READ,
    MSM_REQ_EEPROM_WRITE, MSM_REQ_NONE,
    EEPROM_STORAGE_ADDR_DST_FLAG, sizeof(uint8_t) },
  { PERSIST_OBJECT_ADMIN_VALIDITY, MSM_REQ_EEPROM_READ,
    MSM_REQ_EEPROM_WRITE, MSM_REQ_NONE,
    EEPROM_STORAGE_ADDR_ADMIN_VALIDITY, sizeof(uint8_t) },
  { PERSIST_OBJECT_ADMIN_USERNAME, MSM_REQ_EEPROM_READ,
    MSM_REQ_EEPROM_WRITE, MSM_REQ_NONE,
    EEPROM_STORAGE_ADDR_ADMIN_USERNAME, sizeof(uint16_t) },
  { PERSIST_OBJECT_ADMIN_PASSWORD, MSM_REQ_EEPROM_READ,
    MSM_REQ_EEPROM_WRITE, MSM_REQ_NONE,
    EEPROM_STORAGE_ADDR_ADMIN_PASSWORD, sizeof(uint16_t) },
  { PERSIST_OBJECT_USER_SETTINGS, MSM_REQ_EEPROM_READ,
    MSM_REQ_EEPROM_WRITE, MSM_REQ_NONE,
    EEPROM_STORAGE_ADDR_USER_SETTINGS, sizeof(tSUserSettings) },
  { PERSIST_OBJECT_BROKEN_INPUT_SETTINGS, MSM_REQ_EEPROM_READ,
    MSM_REQ_EEPROM_WRITE, MSM_REQ_NONE,
    EEPROM_STORAGE_ADDR_BROKEN_INPUT_SETTINGS,
    sizeof(tSBrokenInputSettings) },
  { PERSIST_OBJECT_SERVER_SETTINGS, MSM_REQ_EEPROM_READ,
    MSM_REQ_EEPROM_WRITE, MSM_REQ_NONE,
    EEPROM_STORAGE_ADDR_SERVER_SETTINGS, sizeof(tSServerSettings) },
  { PERSIST_OBJECT_PROGRAM_DATA, MSM_REQ_FLASH_READ,
    MSM_REQ_FLASH_WRITE, MSM_REQ_FLASH_ERASE,
    FLASH_STORAGE_ADDR_SCP, STM32_SECTOR_SIZE_BYTES },
  { PERSIST_OBJECT_PROGRAM_LAST_CHANGE_TIME, MSM_REQ_EEPROM_READ,
    MSM_REQ_EEPROM_WRITE, MSM_REQ_NONE,
    EEPROM_STORAGE_ADDR_PROGRAM_LAST_CHANGE_TIME, sizeof(uint32_t) },
  { PERSIST_OBJECT_SIGNAL_OUTPUT_POWERS, MSM_REQ_EEPROM_READ,
    MSM_REQ_EEPROM_WRITE, MSM_REQ_NONE,
    EEPROM_STORAGE_ADDR_SO_POWERS,
    sizeof(tSSOPowerRecord) * SIGNAL_OUTPUTS_MAX },
  { PERSIST_OBJECT_USER_REQUEST, MSM_REQ_EEPROM_READ,
    MSM_REQ_EEPROM_WRITE, MSM_REQ_NONE,
    EEPROM_STORAGE_ADDR_USER_REQUEST, sizeof(tSUserState) },
  { PERSIST_OBJECT_FUNCTION_CONFIG, MSM_REQ_EEPROM_READ,
    MSM_REQ_EEPROM_WRITE, MSM_REQ_NONE,
    EEPROM_STORAGE_ADDR_FUNC_CONF, sizeof(tSFuncConf) },
  { PERSIST_OBJECT_LOG_SETTINGS, MSM_REQ_EEPROM_READ,
    MSM_REQ_EEPROM_WRITE, MSM_REQ_NONE,
    EEPROM_STORAGE_ADDR_LOG_SETTINGS, sizeof(tSLogSettings) },
  { PERSIST_OBJECT_SYSTEM_START_TIME, MSM_REQ_EEPROM_READ,
    MSM_REQ_EEPROM_WRITE, MSM_REQ_NONE,
    EEPROM_STORAGE_ADDR_SYSTEM_START_TIME, sizeof(tSSystemStartTime) },
  { PERSIST_OBJECT_LRLF_DETECT_TIME, MSM_REQ_EEPROM_READ,
    MSM_REQ_EEPROM_WRITE, MSM_REQ_NONE,
    EEPROM_STORAGE_LRLF_DETECT_TIME_SETTTINGS, sizeof(uint8_t) },
  { PERSIST_OBJECT_DEVICE_UID, MSM_REQ_EEPROM_READ,
    MSM_REQ_EEPROM_WRITE, MSM_REQ_NONE,
    EEPROM_STORAGE_ADDR_DEVICE_UID, sizeof(tSSDeviceUID) },
  { PERSIST_OBJECT_MCS_CONNECTION_INFO, MSM_REQ_EEPROM_READ,
    MSM_REQ_EEPROM_WRITE, MSM_REQ_NONE,
    EEPROM_STORAGE_ADDR_MCS_CON_INFO, sizeof(tSMCSConInfo) },
  { PERSIST_OBJECT_GPS_PORT, MSM_REQ_EEPROM_READ,
    MSM_REQ_EEPROM_WRITE, MSM_REQ_NONE,
    EEPROM_STORAGE_ADDR_GPS_PORT, sizeof(uint8_t) },
  { PERSIST_OBJECT_GPS_BAUD_RATE, MSM_REQ_EEPROM_READ,
    MSM_REQ_EEPROM_WRITE, MSM_REQ_NONE,
    EEPROM_STORAGE_ADDR_GPS_BAUD_RATE, sizeof(uint8_t) },
  { PERSIST_OBJECT_LANGUAGE, MSM_REQ_EEPROM_READ,
    MSM_REQ_EEPROM_WRITE, MSM_REQ_NONE,
    EEPROM_STORAGE_ADDR_LANGUAGE, sizeof(uint8_t) },
  { PERSIST_OBJECT_IAP_BINARY_INFO, MSM_REQ_FLASH_READ,
    MSM_REQ_FLASH_WRITE, MSM_REQ_FLASH_ERASE,
    FLASH_STORAGE_ADDR_IAP_BINARY_INFO, sizeof(tSIAPBinInfo) },
  { PERSIST_OBJECT_IAP_BACKUP_IMAGE, MSM_REQ_FLASH_READ,
    MSM_REQ_FLASH_WRITE, MSM_REQ_FLASH_ERASE,
    FLASH_STORAGE_ADDR_CP_BACKUP,
    FLASH_STORAGE_ADDR_CONFIG_SLOT_A - FLASH_STORAGE_ADDR_CP_BACKUP },
  { PERSIST_OBJECT_CONFIG_SLOT_A, MSM_REQ_FLASH_READ,
    MSM_REQ_FLASH_WRITE, MSM_REQ_FLASH_ERASE,
    FLASH_STORAGE_ADDR_CONFIG_SLOT_A, STM32_SECTOR_SIZE_BYTES },
  { PERSIST_OBJECT_CONFIG_SLOT_B, MSM_REQ_FLASH_READ,
    MSM_REQ_FLASH_WRITE, MSM_REQ_FLASH_ERASE,
    FLASH_STORAGE_ADDR_CONFIG_SLOT_B, STM32_SECTOR_SIZE_BYTES },
  { PERSIST_OBJECT_CONFIG_MIGRATION_JOURNAL, MSM_REQ_EEPROM_READ,
    MSM_REQ_EEPROM_WRITE, MSM_REQ_NONE,
    EEPROM_STORAGE_ADDR_CONFIG_MIGRATION_JOURNAL, 64U },
  { PERSIST_OBJECT_SNMPV3_STATE, MSM_REQ_EEPROM_READ,
    MSM_REQ_EEPROM_WRITE, MSM_REQ_NONE,
    EEPROM_STORAGE_ADDR_SNMPV3_STATE, sizeof(tSMCSSNMPv3State) },
  { PERSIST_OBJECT_AUTH_STATE, MSM_REQ_EEPROM_READ,
    MSM_REQ_EEPROM_WRITE, MSM_REQ_NONE,
    EEPROM_STORAGE_ADDR_AUTH_STATE, sizeof(uint32_t) * 4U }
};

static const PersistenceLayout_t *LayoutGet(PersistenceObjectId_t objectId)
{
  uint32_t index;

  for (index = 0U; index < (sizeof(saLayout) / sizeof(saLayout[0])); index++)
  {
    if (saLayout[index].objectId == objectId)
    {
      return &saLayout[index];
    }
  }

  return NULL;
}

static uint8_t RangeIsValid(const PersistenceLayout_t *layout,
                            uint32_t offset,
                            uint32_t size)
{
  if (layout == NULL)
  {
    return FALSE;
  }

  if (size == 0U)
  {
    return FALSE;
  }

  if (offset > layout->size)
  {
    return FALSE;
  }

  if (size > (layout->size - offset))
  {
    return FALSE;
  }

  return TRUE;
}

static uint32_t AdapterGetCapacity(void *ctx, PersistenceObjectId_t objectId)
{
  const PersistenceLayout_t *layout = LayoutGet(objectId);

  (void) ctx;

  if (layout == NULL)
  {
    return 0U;
  }

  return layout->size;
}

static uint8_t AdapterRead(void *ctx, PersistenceObjectId_t objectId,
                           uint32_t offset, void *dst, uint32_t size)
{
  const PersistenceLayout_t *layout = LayoutGet(objectId);

  (void) ctx;

  if ((dst == NULL) || (RangeIsValid(layout, offset, size) == FALSE))
  {
    return FALSE;
  }

  return MSMRequest(layout->readReqId,
                    layout->baseAddress + offset,
                    dst,
                    size);
}

static uint8_t AdapterWrite(void *ctx, PersistenceObjectId_t objectId,
                            uint32_t offset, const void *src, uint32_t size)
{
  const PersistenceLayout_t *layout = LayoutGet(objectId);

  (void) ctx;

  if ((src == NULL) || (RangeIsValid(layout, offset, size) == FALSE))
  {
    return FALSE;
  }

  return MSMRequest(layout->writeReqId,
                    layout->baseAddress + offset,
                    (void *) src,
                    size);
}

static uint8_t AdapterErase(void *ctx, PersistenceObjectId_t objectId,
                            uint32_t offset, uint32_t size)
{
  const PersistenceLayout_t *layout = LayoutGet(objectId);

  (void) ctx;

  if ((RangeIsValid(layout, offset, size) == FALSE)
      || (layout->eraseReqId == MSM_REQ_NONE))
  {
    return FALSE;
  }

  return MSMRequest(layout->eraseReqId,
                    layout->baseAddress + offset,
                    NULL,
                    size);
}

void PersistenceAdapterInit(PersistenceAdapterCtx_t *ctx)
{
  ctx->reserved = 0U;
}

IPersistencePort_t PersistenceAdapterCreatePort(PersistenceAdapterCtx_t *ctx)
{
  IPersistencePort_t port;

  port.ctx = ctx;
  port.GetCapacity = AdapterGetCapacity;
  port.Read = AdapterRead;
  port.Write = AdapterWrite;
  port.Erase = AdapterErase;

  return port;
}
