/**
 ******************************************************************************
 * @file    MSM.h
 * @author  Okan KILIC - Teknotel Electronics
 * @version V1.0.0
 * @date    08/14/2011
 * @brief  Maestro Storage Management Header File
 ******************************************************************************
 */

#ifndef __MSM_H__
#define __MSM_H__

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  include files */
#include "MCS.h"
#include "MLM.h"
#include "SettingsStorage.h"
#include "SystemStartTimeStore.h"
#include "flash.h"
#include "Ports/IEepromStoragePort.h"
#include "Ports/IFlashStoragePort.h"

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  definitions */
typedef struct _tSMSMRequest
{
  osThreadId_t SThreadId;
  uint32_t lAddress;
  void *pvData;
  uint32_t lDataSize;
  uint8_t bReqId;
} tSMSMRequest, *tpSMSMRequest;

typedef enum
{
  MSM_REQ_NONE = 0,
  MSM_REQ_FIRST,
  MSM_REQ_FLASH_READ = MSM_REQ_FIRST,
  MSM_REQ_FLASH_WRITE,
  MSM_REQ_FLASH_WRITE_ASYNCH,
  MSM_REQ_FLASH_ERASE,
  MSM_REQ_EEPROM_READ,
  MSM_REQ_EEPROM_WRITE,
  MSM_REQ_LAST = MSM_REQ_EEPROM_WRITE
} tEMSMRequest;

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  FLASH addresses */
/*  Base address of the Flash sectors */
#define FLASH_STORAGE_ADDR_CP_MAIN ADDR_FLASH_SECTOR_2_BANK1

#define FLASH_STORAGE_ADDR_CONFIG_SLOT_A ADDR_FLASH_SECTOR_6_BANK2
#define FLASH_STORAGE_ADDR_CONFIG_SLOT_B ADDR_FLASH_SECTOR_7_BANK2

#define FLASH_STORAGE_ADDR_SCP ADDR_FLASH_SECTOR_6_BANK2

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  EEPROM addresses */
#define EEPROM_STORAGE_ADDR_BASE 0x0000

/* User settings */
#define EEPROM_STORAGE_ADDR_USER_SETTINGS EEPROM_STORAGE_ADDR_BASE

#define EEPROM_STORAGE_ADDR_BROKEN_INPUT_SETTINGS                              \
        EEPROM_STORAGE_ADDR_USER_SETTINGS + (sizeof(tSUserSettings))

#define EEPROM_STORAGE_ADDR_SERVER_SETTINGS                                    \
        EEPROM_STORAGE_ADDR_BROKEN_INPUT_SETTINGS + (sizeof(tSBrokenInputSettings))

#define EEPROM_STORAGE_ADDR_SYSTEM_START_TIME                                  \
        EEPROM_STORAGE_ADDR_SERVER_SETTINGS + (sizeof(tSServerSettings))

/* LRLF Detect Time Settings */
#define EEPROM_STORAGE_LRLF_DETECT_TIME_SETTTINGS                              \
        EEPROM_STORAGE_ADDR_SYSTEM_START_TIME + (sizeof(tSSystemStartTime))

/* GPRS Modem Type */
#define EEPROM_STORAGE_ADDR_MCS_CON_INFO                                       \
        EEPROM_STORAGE_LRLF_DETECT_TIME_SETTTINGS + (sizeof(uint8_t))

/* GPS Port */
#define EEPROM_STORAGE_ADDR_GPS_PORT                                           \
        EEPROM_STORAGE_ADDR_MCS_CON_INFO + (sizeof(tSMCSConInfo))

/* GPS Baud Rate */
#define EEPROM_STORAGE_ADDR_GPS_BAUD_RATE                                      \
        EEPROM_STORAGE_ADDR_GPS_PORT + (sizeof(uint8_t))

/* Language */
#define EEPROM_STORAGE_ADDR_LANGUAGE                                           \
        EEPROM_STORAGE_ADDR_GPS_BAUD_RATE + (sizeof(uint8_t))

/* Logs */
#define EEPROM_STORAGE_ADDR_LOG                                                \
        EEPROM_STORAGE_ADDR_LANGUAGE + (sizeof(uint8_t))
#define EEPROM_STORAGE_ADDR_LOG_RECORD_NUMBER                                  \
        EEPROM_STORAGE_ADDR_LOG                                                \
        + (sizeof(EventLogStorageRecord_t) * LOG_RECORDS_MAX)
#define EEPROM_STORAGE_ADDR_LOG_EXISTENCE                                      \
        EEPROM_STORAGE_ADDR_LOG_RECORD_NUMBER + (sizeof(uint16_t))
#define EEPROM_STORAGE_ADDR_LOG_INDEXES                                        \
        EEPROM_STORAGE_ADDR_LOG_EXISTENCE + (sizeof(uint8_t))

#define EEPROM_STORAGE_ADDR_CONFIG_MIGRATION_JOURNAL \
        EEPROM_STORAGE_ADDR_LOG_INDEXES + (sizeof(uint16_t))

#define EEPROM_STORAGE_ADDR_SNMPV3_STATE \
        EEPROM_STORAGE_ADDR_CONFIG_MIGRATION_JOURNAL + 64U

#define EEPROM_STORAGE_ADDR_SNMP_BOOTSTRAP_INFO \
        EEPROM_STORAGE_ADDR_SNMPV3_STATE + (sizeof(tSMCSSNMPv3State))

#define EEPROM_STORAGE_ADDR_SNMPV3_BOOTSTRAP_STATE \
        EEPROM_STORAGE_ADDR_SNMP_BOOTSTRAP_INFO + (sizeof(tSMCSSNMPBootstrapInfo))

#define EEPROM_STORAGE_ADDR_AUTH_STATE \
        EEPROM_STORAGE_ADDR_SNMPV3_BOOTSTRAP_STATE \
        + (sizeof(tSMCSSNMPv3State))

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  public methods */
extern void MSMInit(IFlashStoragePort_t *flashPort,
                    IEepromStoragePort_t *eepromPort);
extern uint8_t MSMRequest(uint8_t bReqId, uint32_t lAddress, void *pvData,
                          uint32_t lDataSize);

#endif /* ifndef __MSM_H__ */
