/**
 ******************************************************************************
 * @file    MSM.h
 * @author  Okan KILIC - Teknotel Electronics
 * @version V1.0.0
 * @date    08/14/2011
 * @brief  Interapplication Programming Header File
 ******************************************************************************
 */

#ifndef __IAP_H__
#define __IAP_H__

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  include files */
#include <stdint.h>

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  definitions */
#define IAP_BLOCK_WRITE_MAX_SIZE ((uint32_t) 2048)

#define IAP_PACKET_HEAD_SIZE (uint8_t) 7
#define IAP_PACKET_TAIL_SIZE (uint8_t) 1
#define IAP_PACKET_OVERHEAD_SIZE                                               \
        (uint8_t) (IAP_PACKET_HEAD_SIZE + IAP_PACKET_TAIL_SIZE)

#define IAP_MAX_COMMUNICATION_ERRORS 5

#define IAP_DATA_PACKET_SERIAL_MAX_SIZE ((uint8_t) 230)
#define IAP_DATA_PACKET_USB_MAX_SIZE ((uint8_t) 40)

#define IAP_DATA_PACKET_MAX_SIZE ((uint8_t) 230)

#define IAP_BINARY_FILES_INITIALIZED (uint8_t) 0x5A
#define IAP_BINARY_FILE_VALID (uint8_t) 0xA5

typedef enum
{
  IAP_PACKET_TYPE_NONE = 0,
  IAP_PACKET_TYPE_CP,
  IAP_PACKET_TYPE_MAX
} tEIAPPacketType;

typedef enum
{
  IAP_START_MODE_NONE = 0,
  IAP_START_MODE_BASE,
  IAP_START_MODE_BACKUP,
} tEIAPStartMode;

typedef enum
{
  IAP_REQUEST_SOURCE_NONE = 0,
  IAP_REQUEST_SOURCE_MCS,
  IAP_REQUEST_SOURCE_UI_SERIAL,
  IAP_REQUEST_SOURCE_UI_USB
} tEIAPRequestSource;

typedef enum
{
  IAP_TYPE_NONE = 0,
  IAP_TYPE_DOWNLOAD,
  IAP_TYPE_UPLOAD
} tEIAPType;

typedef enum
{
  IAP_STATUS_NONE = 0,
  IAP_STATUS_STARTED,
  IAP_STATUS_RESTART,
  IAP_STATUS_RESUMED,
  IAP_STATUS_CONTINUOUS,
  IAP_STATUS_FINISHED
} tEIAPStatus;

typedef enum
{
  IAP_COMMAND_TYPE_NONE = 0,
  IAP_COMMAND_TYPE_START,
  IAP_COMMAND_TYPE_STATUS,
  IAP_COMMAND_TYPE_DATA
} tEIAPCommandType;

typedef enum
{
  IAP_START_TYPE_NONE = 0,
  IAP_START_TYPE_START,
  IAP_START_TYPE_RESUME,
} tEIAPStartType;

typedef enum
{
  IAP_STATUS_DATA_NONE = 0,
  IAP_STATUS_DATA_SIZE_ERROR,
  IAP_STATUS_DATA_DATA_ERROR,
  IAP_STATUS_DATA_STORAGE_ERROR,
  IAP_STATUS_DATA_COMMUNICATION_ERROR,
  IAP_STATUS_DATA_CHECKSUM_ERROR,
  IAP_STATUS_DATA_FINISHED,
  IAP_STATUS_DATA_SWITCHING_MODE
} tEIAPStatusData;

typedef struct _tSIAPBinInfo
{
  uint8_t fIsInitialized;
  uint8_t fIsUpdated;
  uint32_t ulSize;
} __attribute__((packed)) tSIAPBinInfo, *tpSIAPBinInfo;

typedef struct _tSIAPData
{
  uint8_t bIAPType;
  uint8_t bCommandType;
  uint8_t bPacketType;
  uint16_t sPacketNumber;
  uint16_t sPacketCount;
  uint8_t baData[IAP_DATA_PACKET_MAX_SIZE + 1]; /* Checksum is at the end */
} __attribute__((packed)) tSIAPData, *tpSIAPData;

typedef struct _tSIAPRequest
{
  uint8_t bSource;
  uint8_t bLength;

  union
  {
    uint8_t baData[IAP_PACKET_OVERHEAD_SIZE + IAP_DATA_PACKET_MAX_SIZE];
    tSIAPData SData;
  } __attribute__((packed)) UData;
} __attribute__((packed)) tSIAPRequest, *tpSIAPRequest;

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  public methods */
extern uint8_t IAPReadBinInfo(void);
extern uint8_t IAPWriteBinInfo(void);
extern uint8_t IAPRequest(uint8_t bSrc, uint8_t bLen, void *pvData);

#endif /* ifndef __IAP_H__ */
