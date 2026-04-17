#ifndef _GPS
#define _GPS

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  include files */
#include "Ports/ISerialPort.h"
#include "time.h"

#define GPS_COMM_MAX_RX_PACKET_LENGTH 1024
#define GPS_NMEA_ADDRESS_FIELD_MAX_LEN 5

/*
 *  name: states related to gps antenna
 *  expl:
 */
typedef struct _SGpsRuntime
{
  uint8_t bTimeoutErrorCntr;

  struct
  {
    uint8_t fModemAlive : 1; /* true if modem sends packet to device */
    uint8_t fAntStatus : 1; /* true if antenna location is safe. This shown in */
                            /* status field in RMC packet */
    uint8_t fConfigDone : 1;
    uint8_t fInitialUpdate : 1;
    uint8_t fHourlyUpdate : 1;
    uint8_t fReserved : 3;
  } __attribute__((packed)) SFlags;
} tSGpsRuntime, *tpSGpsRuntime;

/*
 *  name: GPS communication Buffer
 *  expl:
 */
typedef struct _tSGpsReceive
{
  uint8_t bState;
  uint8_t bAddressIndex;
  char strAddress[GPS_NMEA_ADDRESS_FIELD_MAX_LEN + 1];

  uint8_t bIndex;

  tSTime SGpsTime;

  struct
  {
    uint8_t fBusy : 1;
    uint8_t fCheckStarter : 1;
    uint8_t fCheckTerminator : 1;
    uint8_t fAddressReceived : 1;
    uint8_t fDateTimeReceived : 1;
    uint8_t fDataValid : 1;
    uint8_t bReserved : 2;
  } SFlags;
} tSGpsReceive, *tpSGpsReceive;

typedef struct _tSGPSRxMsg
{
  uint8_t baData[GPS_COMM_MAX_RX_PACKET_LENGTH + 1];
  uint16_t sDataLen;
} tSGPSRxMsg, *tpSGPSRxMsg;

typedef enum
{
  GPS_PORT_TYPE_NONE = 0,
  GPS_PORT_TYPE_INTERNAL,
  GPS_PORT_TYPE_EXTERNAL,
  GPS_PORT_TYPE_MAX = GPS_PORT_TYPE_EXTERNAL
} tEGpsPortType;

#define GPS_MIN_BAUD_RATE_INDEX 1
#define GPS_MAX_BAUD_RATE_INDEX 11

#define GPS_RX_MESSAGES_MAX 2
#define GPS_TX_MESSAGES_MAX 2

/* /////////////////////////////////////////// */
/* Public Methods */
extern uint8_t GpsPortWrite(void);
extern void GpsPortSet(uint8_t bPort);
extern uint8_t GpsPortRead(void);
extern uint8_t GpsPortGet(void);
extern uint8_t GpsBaudRateIndexWrite(void);
extern void GpsBaudRateIndexSet(uint8_t bIndex);
extern uint8_t GpsBaudRateIndexGet(void);
extern uint8_t GpsBaudRateIndexRead(void);
extern uint32_t GpsIndexToBaudRate(uint8_t bGPSBaudRateIndex);
extern uint8_t GpsIsPortInternal(void);
extern void GpsTimeAdjust(tpSTime pTime);
extern void GpsInitRuntimes(void);
extern void GpsInit(ISerialPort_t *internalPort, ISerialPort_t *externalPort);
extern uint8_t GpsConfigDoneGet(void);
extern void GpsConfigDoneSet(uint8_t bState);
extern uint8_t GpsRTCInitialUpdateDoneGet(void);
extern void GpsRTCInitialUpdateDoneSet(uint8_t bState);
extern uint8_t GpsRTCHourlyUpdateDoneGet(void);
extern void GpsRTCHourlyUpdateDoneSet(uint8_t bState);
extern uint8_t GpsModemAliveGet(void);
extern uint8_t GpsAntStatusGet(void);
extern double GpsLatitudeGet(void);
extern double GpsLongitudeGet(void);
extern void GpsSendConfiguration(void);
extern void GpsStartReception(void);
extern void GpsRequest(uint8_t *pbData, uint16_t sLength);

#endif /* ifndef _GPS */
