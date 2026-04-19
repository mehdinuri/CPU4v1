/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Include Files */
#include "gps.h"
#include "HardwarePorts.h"
#include "PersistencePorts.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "MSM.h"
#include "cmsis_os.h"
#include "data.h"
#include "defs.h"
#include "time.h"

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Members */
#define GPS_DMA_TX_TIMEOUT 1000

#define GPS_COMM_DELIMITER ','
#define GPS_COMM_START_OF_PACKET '$'
#define GPS_COMM_END_OF_PACKET '\n'

#define GPS_DEFAULT_PORT_TYPE GPS_PORT_TYPE_INTERNAL
#define GPS_DEFAULT_BAUD_RATE_INDEX 9

/* RMC Fields */
#define NMEA_ADDRESS_LEN 5
#define GNGGA_FIELD_NAME_STR "GNGGA"
#define GPGGA_FIELD_NAME_STR "GPGGA"

#define GNGLL_FIELD_NAME_STR "GNGLL"
#define GPGLL_FIELD_NAME_STR "GPGLL"

#define GNGSA_FIELD_NAME_STR "GNGSA"
#define GPGSA_FIELD_NAME_STR "GNGSA"

#define GNGSV_FIELD_NAME_STR "GNGSV"
#define GPGSV_FIELD_NAME_STR "GNGSV"

#define GNRMC_FIELD_NAME_STR "GNRMC"
#define GPRMC_FIELD_NAME_STR "GPRMC"

#define GNVTG_FIELD_NAME_STR "GNVTG"
#define GPVTG_FIELD_NAME_STR "GPVTG"

#define GPRMC_FIELD_NAME 0
#define GPRMC_FIELD_UTC_TIME 1
#define GPRMC_FIELD_STATUS 2 /* states are listed below */
#define GPRMC_FIELD_LATITUDE 3
#define GPRMC_FIELD_LATITUDE_DIRECTION 4
#define GPRMC_FIELD_LONGITUDE 5
#define GPRMC_FIELD_LONGITUDE_DIRECTION 6
#define GPRMC_FIELD_SPEED 7 /* kts */
#define GPRMC_FIELD_TRUE_COURSE 8 /* degree */
#define GPRMC_FIELD_UTC_DATE 9
#define GPRMC_FIELD_MAGNETIC_DEVIATION 10 /* degree */
#define GPRMC_FIELD_MAGNETIC_DEVIATION_DIRECTION 11 /* degree */
#define GPRMC_FIELD_PSM_INDICATION 12 /* psm = positioning system mode */
#define GPRMC_FIELD_CHECKSUM 13
/* status */
#define GPRMC_FIELD_STATUS_SAFE 'A'
#define GPRMC_FIELD_STATUS_UNSAFE 'V'

#define GPS_CFG_UBLOX_MAX_MSGS 6
#define GPS_CFG_TELIT_MAX_MSGS 6
#define GPS_CFG_QUECTEL_MAX_MSGS 7

#define GPS_TIMEOUT_ERROR_MAX 10

#define GPS_TIME_MAX_TIMEOUT 1100 /* 1.1 seconds */

#define GPS_COMM_MAX_TX_PACKET_LENGTH 128
/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Members */
static uint8_t bGpsPort = GPS_PORT_TYPE_NONE;
static uint8_t bGpsBaudRateIndex = 0;

static tSGpsRuntime SGpsRuntime;

static tSGpsReceive SGpsReceive;

static ISerialPort_t *s_port;

__attribute__((section(".ram_d2_dma_buffers"), aligned(32)))
char strGPSDMATxBuffer[GPS_COMM_MAX_TX_PACKET_LENGTH];

/* UBX-G60xx */
/* $PUBX,40,message Id,output rate on DDC,output rate on USART1,output rate on */
/* USART2,output rate on USB,output rate on SPI,reserved*checksum<CR><LF> */
const char *pStrUbloxGPSQueryRateControl[GPS_CFG_UBLOX_MAX_MSGS] =
{ "$PUBX,40,GGA,0,0,0,0,0,0*5A\r\n",                                                                    /* GGA Off */
  "$PUBX,40,GLL,0,0,0,0,0,0*5C\r\n",
  /* GLL Off */
  "$PUBX,40,GSA,0,0,0,0,0,0*4E\r\n",
  /* GSA Off */
  "$PUBX,40,GSV,0,0,0,0,0,0*59\r\n",
  /* GSV Off */
  "$PUBX,40,RMC,0,1,0,0,0,0*46\r\n",
  /* RMC On 1Hz */
  "$PUBX,40,VTG,0,0,0,0,0,0*5E\r\n"
  /* VTG Off */
};

/* Telit JN3 */
/* $PSRF103,message(0=GGA, 1=GLL, 2=GSA, 3=GSV, 4=RMC, 5=VTG),mode(0=set, */
/* 1=query),rate(0-255),checksum enable(0,1)*checksum<CR><LF> */
const char *pStrTelitGPSQueryRateControl[GPS_CFG_TELIT_MAX_MSGS] =
{ "$PSRF103,00,00,00,01*24\r\n",                                                                    /* GGA Off */
  "$PSRF103,01,00,00,01*25\r\n",
  /* GLL Off */
  "$PSRF103,02,00,00,01*26\r\n",
  /* GSA Off */
  "$PSRF103,03,00,00,01*27\r\n",
  /* GSV Off */
  "$PSRF103,04,00,01,01*21\r\n",
  /* RMC On 1Hz */
  "$PSRF103,05,00,00,01*21\r\n"
  /* VTG Off */
};

/* Quectel LC76G */
/* $PAIR066,<GPS_Enabled>,<GLONASS_Enabled>,<Galileo_Enabled>,<BDS_Enabled>,<QZSS_Enabled>,<Res>*<Checksum><CR><LF> */
/* $PAIR062,<Type>,<OutputRate>*<Checksum><CR><LF> */
const char *pStrQuectelGPSQueryRateControl[GPS_CFG_QUECTEL_MAX_MSGS] =
{ "$PAIR066,1,0,1,0,0,0*3A\r\n",                                                                        /* GPS + Galileo */
  "$PAIR062,0,0*3E\r\n",
  /* GGA Off */
  "$PAIR062,1,0*3F\r\n",
  /* GLL Off */
  "$PAIR062,2,0*3C\r\n",
  /* GSA Off */
  "$PAIR062,3,0*3D\r\n",
  /* GSV Off */
  "$PAIR062,4,1*3B\r\n",
  /* RMC On 1Hz */
  "$PAIR062,5,0*3B\r\n"
  /* VTG Off */
};

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Methods */
static void GpsOnRx(void *arg, const uint8_t *data, uint16_t len)
{
  (void) arg;
  GpsRequest((uint8_t *) data, len);
}

void GpsTimeAdjust(tpSTime pTime)
{
  uint8_t bIndex = 0;

  if (GetDeviceTimeZone() > 0)
  {
    for (bIndex = 0; bIndex < GetDeviceTimeZone(); bIndex++)
    {
      TimeHourInc(pTime);
    }
  }
  else
  {
    for (bIndex = 0; bIndex < (-1) * GetDeviceTimeZone(); bIndex++)
    {
      TimeHourDec(pTime);
    }
  }
}

uint8_t GpsPortWrite(void)
{
  return PersistenceWrite(&g_persistencePort,
                          PERSIST_OBJECT_GPS_PORT,
                          0U,
                          &bGpsPort,
                          sizeof(uint8_t));
}

void GpsPortSet(uint8_t bPort)
{
  bGpsPort = bPort;
}

uint8_t GpsPortRead(void)
{
  return PersistenceRead(&g_persistencePort,
                         PERSIST_OBJECT_GPS_PORT,
                         0U,
                         &bGpsPort,
                         sizeof(uint8_t));
}

uint8_t GpsPortGet(void)
{
  return bGpsPort;
}

uint8_t GpsBaudRateIndexWrite(void)
{
  return PersistenceWrite(&g_persistencePort,
                          PERSIST_OBJECT_GPS_BAUD_RATE,
                          0U,
                          &bGpsBaudRateIndex,
                          sizeof(uint8_t));
}

void GpsBaudRateIndexSet(uint8_t bIndex)
{
  bGpsBaudRateIndex = bIndex;
}

uint8_t GpsBaudRateIndexGet(void)
{
  return bGpsBaudRateIndex;
}

uint8_t GpsBaudRateIndexRead(void)
{
  return PersistenceRead(&g_persistencePort,
                         PERSIST_OBJECT_GPS_BAUD_RATE,
                         0U,
                         &bGpsBaudRateIndex,
                         sizeof(uint8_t));
}

uint32_t GpsIndexToBaudRate(uint8_t bGPSBaudRateIndex)
{
  uint32_t lGPSBaudRate;

  switch (bGPSBaudRateIndex)
  {
      case 0x01:
      {
        lGPSBaudRate = 2400;
        break;
      }

      case 0x02:
      {
        lGPSBaudRate = 4800;
        break;
      }

      case 0x03:
      {
        lGPSBaudRate = 9600;
        break;
      }

      case 0x04:
      {
        lGPSBaudRate = 19200;
        break;
      }

      case 0x05:
      {
        lGPSBaudRate = 28800;
        break;
      }

      case 0x06:
      {
        lGPSBaudRate = 38400;
        break;
      }

      case 0x07:
      {
        lGPSBaudRate = 57600;
        break;
      }

      case 0x08:
      {
        lGPSBaudRate = 76800;
        break;
      }

      case 0x09:
      {
        lGPSBaudRate = 115200;
        break;
      }

      case 0x0A:
      {
        lGPSBaudRate = 153600;
        break;
      }

      case 0x0B:
      {
        lGPSBaudRate = 230400;
        break;
      }

      default:
      {
        lGPSBaudRate = 4800;
        break;
      }
  } /* switch */

  return lGPSBaudRate;
} /* GpsIndexToBaudRate */

uint8_t GpsIsPortInternal(void)
{
  return GpsPortGet() == GPS_PORT_TYPE_INTERNAL;
}

void GpsInitRuntimes(void)
{
  memset(&SGpsRuntime.SFlags, 0, sizeof(SGpsRuntime.SFlags));
}

void GpsInit(ISerialPort_t *internalPort, ISerialPort_t *externalPort)
{
  uint8_t bPort = 0;
  uint8_t bIndex = 0;

  GpsInitRuntimes();

  GpsPortRead();
  GpsBaudRateIndexRead();

  bPort = GpsPortGet();
  bIndex = GpsBaudRateIndexGet();

  if ((bPort > GPS_PORT_TYPE_NONE) && (bPort <= GPS_PORT_TYPE_MAX)
      && (bIndex >= GPS_MIN_BAUD_RATE_INDEX)
      && (bIndex <= GPS_MAX_BAUD_RATE_INDEX))
  {
    switch (bPort)
    {
        case COM_PORT_ASSIGNMENT_INTERNAL:
        {
          s_port = internalPort;
          (void) SerialSetBaudRate(s_port, GpsIndexToBaudRate(bIndex));
          SerialSetRxCallback(s_port, GpsOnRx, NULL);

          return;
        }

        case COM_PORT_ASSIGNMENT_EXTERNAL:
        {
          s_port = externalPort;
          (void) SerialSetBaudRate(s_port, GpsIndexToBaudRate(bIndex));
          SerialSetRxCallback(s_port, GpsOnRx, NULL);

          return;
        }

        default:
        {
          break;
        }
    }
  }

  bPort = GPS_DEFAULT_PORT_TYPE;
  bIndex = GPS_DEFAULT_BAUD_RATE_INDEX;

  GpsPortSet(bPort);
  GpsBaudRateIndexSet(bIndex);

  GpsPortWrite();
  GpsBaudRateIndexWrite();

  s_port = internalPort;
  (void) SerialSetBaudRate(s_port, GpsIndexToBaudRate(bIndex));
  SerialSetRxCallback(s_port, GpsOnRx, NULL);
} /* GpsInit */

uint8_t GpsConfigDoneGet(void)
{
  return SGpsRuntime.SFlags.fConfigDone;
}

void GpsConfigDoneSet(uint8_t fState)
{
  SGpsRuntime.SFlags.fConfigDone = fState;
}

uint8_t GpsRTCInitialUpdateDoneGet(void)
{
  return SGpsRuntime.SFlags.fInitialUpdate;
}

void GpsRTCInitialUpdateDoneSet(uint8_t bState)
{
  SGpsRuntime.SFlags.fInitialUpdate = bState;
}

uint8_t GpsRTCHourlyUpdateDoneGet(void)
{
  return SGpsRuntime.SFlags.fHourlyUpdate;
}

void GpsRTCHourlyUpdateDoneSet(uint8_t bState)
{
  SGpsRuntime.SFlags.fHourlyUpdate = bState;
}

uint8_t GpsModemAliveGet(void)
{
  return SGpsRuntime.SFlags.fModemAlive;
}

void GpsModemAliveSet(uint8_t fState)
{
  SGpsRuntime.SFlags.fModemAlive = fState;
}

uint8_t GpsAntStatusGet(void)
{
  return SGpsRuntime.SFlags.fAntStatus;
}

double GpsConvertLatLongToDecimal(uint8_t bDegree, uint8_t bMin, double sSec)
{
  return (double) (bDegree + ((bMin + (sSec / 60)) / 60));
}

void GpsSendConfiguration(void)
{
  uint8_t bIndex = 0;

  if (GpsIsPortInternal())
  {
    for (bIndex = 0; bIndex < GPS_CFG_QUECTEL_MAX_MSGS; bIndex++)
    {
      strcpy(strGPSDMATxBuffer, pStrQuectelGPSQueryRateControl[bIndex]);
      (void) SerialSend(s_port,
                        (const uint8_t *) strGPSDMATxBuffer,
                        (uint16_t) strlen(strGPSDMATxBuffer),
                        GPS_DMA_TX_TIMEOUT);
    }
  }
  else
  {
    for (bIndex = 0; bIndex < GPS_CFG_TELIT_MAX_MSGS; bIndex++)
    {
      strcpy(strGPSDMATxBuffer, pStrTelitGPSQueryRateControl[bIndex]);
      (void) SerialSend(s_port,
                        (const uint8_t *) strGPSDMATxBuffer,
                        (uint16_t) strlen(strGPSDMATxBuffer),
                        GPS_DMA_TX_TIMEOUT);
    }

    for (bIndex = 0; bIndex < GPS_CFG_UBLOX_MAX_MSGS; bIndex++)
    {
      strcpy(strGPSDMATxBuffer, pStrUbloxGPSQueryRateControl[bIndex]);
      (void) SerialSend(s_port,
                        (const uint8_t *) strGPSDMATxBuffer,
                        (uint16_t) strlen(strGPSDMATxBuffer),
                        GPS_DMA_TX_TIMEOUT);
    }
  }
}

void GpsStartReception(void)
{
  memset(&SGpsReceive, 0, sizeof(SGpsReceive));

  SGpsReceive.SFlags.fCheckStarter = TRUE;
  SGpsReceive.SFlags.fCheckTerminator = TRUE;
  SGpsReceive.SFlags.fBusy = TRUE;
}

void GpsRequest(uint8_t *pbData, uint16_t sLength)
{
  tpSGPSRxMsg pSRxMsg = (tpSGPSRxMsg) osMemoryPoolAlloc(GPSRxReqsMemPoolHandle,
                                                        0);

  if (pSRxMsg != NULL)
  {
    memset(pSRxMsg, 0, sizeof(tSGPSRxMsg));

    if (sLength > GPS_COMM_MAX_RX_PACKET_LENGTH)
    {
      sLength = GPS_COMM_MAX_RX_PACKET_LENGTH;
    }

    pSRxMsg->sDataLen = sLength;
    memcpy(pSRxMsg->baData, pbData, sLength);

    if (osMessageQueuePut(GPSRxReqsQueHandle, &pSRxMsg, 0, 0) != osOK)
    {
      osMemoryPoolFree(GPSRxReqsMemPoolHandle, pSRxMsg);
    }
  }
}

void GpsReceiveByte(uint8_t bRxByte)
{
  if (SGpsReceive.SFlags.fBusy)
  {
    if (SGpsReceive.SFlags.fCheckStarter)
    {
      if (bRxByte == GPS_COMM_START_OF_PACKET)
      {
        SGpsReceive.SFlags.fCheckStarter = FALSE;
        memset(SGpsReceive.strAddress, 0, sizeof(SGpsReceive.strAddress));
        SGpsReceive.bAddressIndex = 0;
        SGpsReceive.SFlags.fDateTimeReceived = FALSE;
      }
    }
    else
    {
      if (SGpsReceive.SFlags.fCheckTerminator)
      {
        if (bRxByte == GPS_COMM_END_OF_PACKET)
        {
          if (SGpsReceive.SFlags.fDateTimeReceived
              && SGpsReceive.SFlags.fDataValid)
          {
            tpSTime pSTime = (tpSTime) osMemoryPoolAlloc(GPSTimeMemPoolHandle,
                                                         0);

            if (pSTime != NULL)
            {
              memset(pSTime, 0, sizeof(tSTime));
              memcpy(pSTime, &SGpsReceive.SGpsTime,
                     sizeof(SGpsReceive.SGpsTime));

              if (osMessageQueuePut(GPSTimeQueHandle, &pSTime, 0, 0) != osOK)
              {
                osMemoryPoolFree(GPSTimeMemPoolHandle, pSTime);
              }
            }
          }

          SGpsRuntime.bTimeoutErrorCntr = 0;

          if (GpsModemAliveGet() == FALSE)
          {
            GpsModemAliveSet(TRUE);
          }

          GpsStartReception();
        }
        else if (bRxByte == GPS_COMM_DELIMITER)
        {
          if (SGpsReceive.SFlags.fAddressReceived)
          {
            SGpsReceive.bIndex = 0;
            SGpsReceive.bState++;
          }
        }
        else
        {
          switch (SGpsReceive.bState)
          {
              case GPRMC_FIELD_NAME:
              {
                if (SGpsReceive.SFlags.fAddressReceived == FALSE)
                {
                  SGpsReceive.strAddress[SGpsReceive.bAddressIndex] = bRxByte;
                  SGpsReceive.bAddressIndex++;

                  if (SGpsReceive.bAddressIndex == NMEA_ADDRESS_LEN)
                  {
                    if ((strstr(SGpsReceive.strAddress, GPRMC_FIELD_NAME_STR)
                         != NULL)
                        || (strstr(SGpsReceive.strAddress, GNRMC_FIELD_NAME_STR)
                            != NULL) )
                    {
                      SGpsReceive.SFlags.fAddressReceived = TRUE;
                    }
                  }
                }

                break;
              }

              case GPRMC_FIELD_UTC_TIME:
              {
                switch (SGpsReceive.bIndex)
                {
                    case 0:
                    {
                      SGpsReceive.SGpsTime.SCurrentTime.Hours =
                        (mASCII2Number(bRxByte) * 10);
                      break;
                    }

                    case 1:
                    {
                      SGpsReceive.SGpsTime.SCurrentTime.Hours +=
                        mASCII2Number(bRxByte);
                      break;
                    }

                    case 2:
                    {
                      SGpsReceive.SGpsTime.SCurrentTime.Minutes =
                        (mASCII2Number(bRxByte) * 10);
                      break;
                    }

                    case 3:
                    {
                      SGpsReceive.SGpsTime.SCurrentTime.Minutes +=
                        mASCII2Number(bRxByte);
                      break;
                    }

                    case 4:
                    {
                      SGpsReceive.SGpsTime.SCurrentTime.Seconds =
                        (mASCII2Number(bRxByte) * 10);
                      break;
                    }

                    case 5:
                    {
                      SGpsReceive.SGpsTime.SCurrentTime.Seconds +=
                        mASCII2Number(bRxByte);
                      break;
                    }
                } /* switch */

                SGpsReceive.bIndex++;
                break;
              }

              case GPRMC_FIELD_STATUS:
              {
                SGpsReceive.SFlags.fDataValid = (bRxByte
                                                 == GPRMC_FIELD_STATUS_SAFE)
                                                ? TRUE : FALSE;
                SGpsRuntime.SFlags.fAntStatus = SGpsReceive.SFlags.fDataValid;
                break;
              }

              case GPRMC_FIELD_UTC_DATE:
              {
                switch (SGpsReceive.bIndex)
                {
                    case 0:
                    {
                      SGpsReceive.SGpsTime.SCurrentDate.Date =
                        (mASCII2Number(bRxByte) * 10);
                      break;
                    }

                    case 1:
                    {
                      SGpsReceive.SGpsTime.SCurrentDate.Date +=
                        mASCII2Number(bRxByte);
                      break;
                    }

                    case 2:
                    {
                      SGpsReceive.SGpsTime.SCurrentDate.Month =
                        (mASCII2Number(bRxByte) * 10);
                      break;
                    }

                    case 3:
                    {
                      SGpsReceive.SGpsTime.SCurrentDate.Month +=
                        mASCII2Number(bRxByte);
                      break;
                    }

                    case 4:
                    {
                      SGpsReceive.SGpsTime.SCurrentDate.Year =
                        (mASCII2Number(bRxByte) * 10);
                      break;
                    }

                    case 5:
                    {
                      SGpsReceive.SGpsTime.SCurrentDate.Year +=
                        mASCII2Number(bRxByte);
                      SGpsReceive.SFlags.fDateTimeReceived = TRUE;
                      break;
                    }
                } /* switch */

                SGpsReceive.bIndex++;
                break;
              }

              case GPRMC_FIELD_LATITUDE:
              {
                SGpsReceive.bIndex++;
                break;
              }

              case GPRMC_FIELD_LONGITUDE:
              {
                SGpsReceive.bIndex++;
                break;
              }
          } /* switch */
        }
      }
    }
  }
} /* GpsReceiveByte */

void GPSReceivePacket(tpSGPSRxMsg pSRxMsg)
{
  uint16_t sIndex = 0;

  for (sIndex = 0; sIndex < pSRxMsg->sDataLen; sIndex++)
  {
    GpsReceiveByte(pSRxMsg->baData[sIndex]);
  }
}

void GPSMsgParserTaskFunc(void *argument)
{
  UNUSED(argument);

  tpSGPSRxMsg pSRxMsg;

  while (FOREVER)
  {
    if (osMessageQueueGet(GPSRxReqsQueHandle, &pSRxMsg, NULL,
                          osWaitForever)
        == osOK)
    {
      GpsStartReception();
      GPSReceivePacket(pSRxMsg);

      osMemoryPoolFree(GPSRxReqsMemPoolHandle, pSRxMsg);
    }
  }
}

void GPSTaskFunc(void *argument)
{
  UNUSED(argument);

  tpSTime pSNewTime = NULL;

  osDelay(1000);

  GpsInit(&g_internalGpsPort, &g_auxSerialPort);

  while (FOREVER)
  {
    if (osMessageQueueGet(GPSTimeQueHandle, &pSNewTime, NULL,
                          GPS_TIME_MAX_TIMEOUT)
        == osOK)
    {
      SGpsRuntime.bTimeoutErrorCntr = 0;

      if (GpsAntStatusGet())
      {
        if (GpsRTCInitialUpdateDoneGet())
        {
          if (GpsRTCHourlyUpdateDoneGet() == FALSE)
          {
            if ((TimeSecondOfDayGet() + 1) % 3600 == 0)
            {
              pSNewTime->bCentury = TIME_CURRENT_CENTURY - 1;
              pSNewTime->SCurrentDate.WeekDay =
                TimeWeekDayOfYearCalc(pSNewTime->SCurrentDate.Month,
                                      pSNewTime
                                      ->SCurrentDate.Date,
                                      TimeFullYearCalc
                                        (pSNewTime));

              GpsTimeAdjust(pSNewTime);
              if (TimeIsValid(pSNewTime))
              {
                TimeSet(pSNewTime);
                GpsRTCHourlyUpdateDoneSet(TRUE);
              }
            }
          }
          else
          {
            if ((TimeSecondOfDayGet() + 2) % 3600 == 0) /* Set the flag in order to update RTC and System time hourly */
            {
              GpsRTCHourlyUpdateDoneSet(FALSE);
            }
          }
        }
        else
        {
          pSNewTime->bCentury = TIME_CURRENT_CENTURY - 1;
          pSNewTime->SCurrentDate.WeekDay =
            TimeWeekDayOfYearCalc(pSNewTime->SCurrentDate.Month,
                                  pSNewTime->
                                  SCurrentDate.Date,
                                  TimeFullYearCalc
                                    (pSNewTime));

          GpsTimeAdjust(pSNewTime);
          if (TimeIsValid(pSNewTime))
          {
            TimeSet(pSNewTime);
            GpsRTCInitialUpdateDoneSet(TRUE);
          }
        }
      }

      osMemoryPoolFree(GPSTimeMemPoolHandle, pSNewTime);
    }
    else
    {
      if (SGpsRuntime.bTimeoutErrorCntr++ >= GPS_TIMEOUT_ERROR_MAX)
      {
        GpsInitRuntimes();
        GpsRTCInitialUpdateDoneSet(FALSE);
        TimeSourceSet(TIME_SOURCE_RTC);
      }
    }

    if ((GpsModemAliveGet()) && (GpsConfigDoneGet() == FALSE))
    {
      GpsSendConfiguration();
      GpsConfigDoneSet(TRUE);
    }

    SignalMaintenanceTask(EVENT_FLAGS_MAINTENANCE_GPS_TASK_ACTIVE);
  }
} /* GPSTaskFunc */
