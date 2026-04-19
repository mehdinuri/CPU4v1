/**
 ******************************************************************************
 * @file    MCS.c
 * @author  Okan KILIC - Teknotel Electronics
 * @version V1.0.0
 * @date    07/19/2011
 * @brief  Maestro Central System
 *       This file explains all the definitions and functions of
 * MCS
 ******************************************************************************
 */

/* /////////////////////////// */
/*  include files */
#include "MCS.h"
#include "HardwarePorts.h"
#include "PersistencePorts.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "gpio.h"
#include "MCSAsynch.h"
#include "MSM.h"
#include "PPPOSAsynch.h"
#include "data.h"
#include "defs.h"
#include "eth.h"
#include "lcd.h"
#include "lwip.h"
#include "lwip/apps/snmp.h"
#include "netif/ppp/ppp.h"
#include "program.h"
#include "rng.h"
#include "snmp_client.h"
#include "stm32h7xx_hal_def.h"
#include "tcp_client.h"
#include "udp_probe.h"

/* /////////////////////////// */
/*  Definitions */
#define MCS_DMA_TX_TIMEOUT 1000

#define MCS_DEFAULT_AT_CMD_RECEIVE_TIMEOUT 1000 /* 1s */

#define MCS_MAX_COMMAND_TRY 2
#define MAX_GPRS_CHECK_SIG_QUA_TRY 5
#define MAX_GPRS_CHECK_SIM_TRY 5
#define MAX_GPRS_CHECK_GSM_GPRS_NET_TRY 60

#define MCS_MAX_CONNECT_COMMAND_TRY 4

#define MIN_GPRS_SIGNAL_QUA 0
#define MAX_GPRS_SIGNAL_QUA 31
#define GPRS_SIGNAL_QUA_ERR 99

#define MCS_MAX_PROBE_ERRORS 3
#define MCS_PROBE_TIMEOUT 20

/*  error messages */
#define MCS_MODEM_NO_RESPONSE "NO RESPONSE"
#define MCS_MODEM_INVALID_RESPONSE "INVALID RESPONSE"

/* /////////////////////////// */
/*  members */
/*  Runtime */
static tSMCSRuntime SMCSRuntime;

/*  MCS Connection Info */
static tSMCSConInfo SMCSConfInfo;

/*  Transfer Packets */
__attribute__((section(".ram_d2_dma_buffers"), aligned(32)))
char strMCSTransmit[MCS_DATA_PACKET_MAX_SIZE + 1];

/* MCS Command Response */
static char strMCSResponse[MCS_DATA_PACKET_MAX_SIZE + 1];

/* user job live screen data (currently only LCD screen is used) */
/* job live buffer */
char strCurrentJobs[MCS_JOB_LIVE_RECORD_MAX][MCS_JOB_LIVE_RECORD_SIZE_MAX];
/* job live buffer index pointing empty location for the next job which will be */
/* added */
static uint8_t bCurrentJobIndex;

/* RX ring buffer for AT-command phase (ISR writes sHead; task reads sTail).
 * Single-writer / single-reader — no mutex required on Cortex-M7. */
#define MCS_RX_RING_SIZE (MCS_DATA_PACKET_MAX_LEN * 2U)
static uint8_t s_rxRing[MCS_RX_RING_SIZE];
static uint16_t s_rxHead;
static uint16_t s_rxTail;
static uint8_t s_rxDataAvailable;

static ISerialPort_t *s_port;
static IModemPort_t  *s_driver;
static ModemInfo_t s_modemInfo;

static uint8_t fColdStarted = TRUE;

/* /////////////////////////// */
/*  Ring buffer helpers (called from ISR context via MCSOnRx) */
void MCSRingBufferReset(void)
{
  memset(s_rxRing, 0, sizeof(s_rxRing));
  s_rxHead = 0U;
  s_rxTail = 0U;
  s_rxDataAvailable = 0U;
}

void MCSRingBufferWrite(const uint8_t *data, uint16_t len)
{
  uint16_t i;

  s_rxDataAvailable = 1U;

  for (i = 0U; i < len; i++)
  {
    s_rxRing[s_rxHead] = data[i];
    s_rxHead++;

    if (s_rxHead >= MCS_RX_RING_SIZE)
    {
      s_rxHead = 0U;
    }
  }
}

static void MCSOnRx(void *arg, const uint8_t *data, uint16_t len)
{
  (void) arg;
  ModemOnRx(s_driver, data, len);
}

/* /////////////////////////// */
/*  members */
uint8_t MCSWriteConInfo(void)
{
  return PersistenceWrite(&g_persistencePort,
                          PERSIST_OBJECT_MCS_CONNECTION_INFO,
                          0U,
                          &SMCSConfInfo,
                          sizeof(SMCSConfInfo));
}

uint8_t MCSReadConInfo(void)
{
  return PersistenceRead(&g_persistencePort,
                         PERSIST_OBJECT_MCS_CONNECTION_INFO,
                         0U,
                         &SMCSConfInfo,
                         sizeof(SMCSConfInfo));
}

void MCSSetConInfo(tpSMCSConInfo pSInfo)
{
  memcpy(&SMCSConfInfo, pSInfo, sizeof(SMCSConfInfo));
}

void MCSGetConInfo(tpSMCSConInfo pSInfo)
{
  memcpy(pSInfo, &SMCSConfInfo, sizeof(SMCSConfInfo));
}

tpSMCSConInfo MCSGetConInfoPtr(void)
{
  return &SMCSConfInfo;
}

tpSMCSMACAddress MCSGetEthernetMACAddress(void)
{
  return &SMCSConfInfo.SMACAddress;
}

tpSMCSIPv4 MCSGetLocalIPv4(void)
{
  return &SMCSConfInfo.SEthLocalIPv4;
}

tpSMCSIPv4 MCSGetSubnetMask(void)
{
  return &SMCSConfInfo.SEthSubnetMask;
}

tpSMCSIPv4 MCSGetGateway(void)
{
  return &SMCSConfInfo.SEthGateway;
}

tpSMCSIPv4 MCSGetPrimaryDNSServer(void)
{
  return &SMCSConfInfo.SPrimaryDNSServer;
}

tpSMCSIPv4 MCSGetSecondaryDNSServer(void)
{
  return &SMCSConfInfo.SSecondaryDNSServer;
}

uint8_t MCSIsEthernetStaticIP(void)
{
  return SMCSConfInfo.SFlags.fEthStaticIp;
}

uint32_t MCSGetSNMPDeviceID(void)
{
  return SMCSConfInfo.SSNMPInfo.lDeviceID;
}

uint8_t MCSGetSNMPTrapVersion(void)
{
  return SMCSConfInfo.SSNMPInfo.bTrapVersion;
}

char *MCSGetSNMPReadCommunityName(void)
{
  return SMCSConfInfo.SSNMPInfo.strReadCommunityName;
}

char *MCSGetSNMPWriteCommunityName(void)
{
  return SMCSConfInfo.SSNMPInfo.strWriteCommunityName;
}

char *MCSGetSNMPTrapCommunityName(void)
{
  return SMCSConfInfo.SSNMPInfo.strTrapCommunityName;
}

char *MCSGetSNMPv3EngineID(void)
{
  return SMCSConfInfo.SSNMPInfo.strV3EngineId;
}

char *MCSGetSNMPv3Username(void)
{
  return SMCSConfInfo.SSNMPInfo.strV3Username;
}

char *MCSGetSNMPv3AuthPassword(void)
{
  return SMCSConfInfo.SSNMPInfo.strV3AuthPassword;
}

char *MCSGetSNMPv3PrivPassword(void)
{
  return SMCSConfInfo.SSNMPInfo.strV3PrivPassword;
}

char *MCSGetSNMPTrapDestination(void)
{
  return SMCSConfInfo.SSNMPInfo.strTrapDestination;
}

uint32_t MCSGetFNV132Hash(const uint8_t *pbData, uint8_t bLen)
{
  uint8_t bIdx = 0;
  uint32_t ulHash = MCS_FNV_OFFSET_BASIS;
  const uint32_t ulPrime = MCS_FNV_PRIME;

  for (bIdx = 0; bIdx < bLen; ++bIdx)
  {
    ulHash ^= pbData[bIdx];
    ulHash *= ulPrime;
  }

  return ulHash;
}

uint32_t MCSGenerateUIDHash(void)
{
  uint8_t bIdx = 0;
  uint8_t baUIDBytes[UID_MAX_LENGTH * sizeof(uint32_t)];

  ReadCPUDeviceUID();

  for (bIdx = 0; bIdx < UID_MAX_LENGTH; ++bIdx)
  {
    baUIDBytes[bIdx
               * sizeof(uint32_t)] =
      (uint8_t) (GetCPUDeviceUID()->ulaUID[bIdx] >> 24);
    baUIDBytes[bIdx * sizeof(uint32_t)
               + 1] = (uint8_t) (GetCPUDeviceUID()->ulaUID[bIdx] >> 16);
    baUIDBytes[bIdx * sizeof(uint32_t)
               + 2] = (uint8_t) (GetCPUDeviceUID()->ulaUID[bIdx] >> 8);
    baUIDBytes[bIdx * sizeof(uint32_t)
               + 3] = (uint8_t) (GetCPUDeviceUID()->ulaUID[bIdx]);
  }

  return MCSGetFNV132Hash(baUIDBytes, sizeof(baUIDBytes));
}

void MCSInitConInfo(void)
{
  uint32_t ulUIDHash = 0, ulRand = 0;
  uint8_t fRandGenOk = FALSE;

  memset(&SMCSConfInfo, 0, sizeof(SMCSConfInfo));

  ulUIDHash = MCSGenerateUIDHash();
  fRandGenOk = HAL_RNG_GenerateRandomNumber(&hrng, &ulRand) == HAL_OK;

  SMCSConfInfo.bInitialized = MCS_CON_INFO_INITIALIZED;

  SMCSConfInfo.bModuleType = MCS_MODULE_TYPE_ETH_NTCIP;

  SMCSConfInfo.SFlags.fEthStaticIp = TRUE;

  SMCSConfInfo.SMACAddress.bAddress0 = MCS_DEFAULT_ETH_MAC_ADDRESS_0;
  SMCSConfInfo.SMACAddress.bAddress1 = (uint8_t) ((ulUIDHash >> 24) & 0xFF);
  SMCSConfInfo.SMACAddress.bAddress2 = (uint8_t) ((ulUIDHash >> 16) & 0xFF);
  SMCSConfInfo.SMACAddress.bAddress3 = (uint8_t) ((ulUIDHash >> 8) & 0xFF);
  SMCSConfInfo.SMACAddress.bAddress4 = (uint8_t) (ulUIDHash & 0xFF);
  if (fRandGenOk)
  {
    SMCSConfInfo.SMACAddress.bAddress5 = (uint8_t) ((ulRand >> 8) & 0xFF);
  }
  else
  {
    SMCSConfInfo.SMACAddress.bAddress5 = MCS_DEFAULT_ETH_MAC_ADDRESS_5;
  }

  SMCSConfInfo.SEthLocalIPv4.bAddress0 = MCS_DEFAULT_ETH_STATIC_IPV4_0;
  SMCSConfInfo.SEthLocalIPv4.bAddress1 = MCS_DEFAULT_ETH_STATIC_IPV4_1;
  SMCSConfInfo.SEthLocalIPv4.bAddress2 = MCS_DEFAULT_ETH_STATIC_IPV4_2;
  if (fRandGenOk)
  {
    SMCSConfInfo.SEthLocalIPv4.bAddress3 = (uint8_t) (ulRand & 0xFF);
  }
  else
  {
    SMCSConfInfo.SEthLocalIPv4.bAddress3 = MCS_DEFAULT_ETH_STATIC_IPV4_3;
  }

  SMCSConfInfo.SEthSubnetMask.bAddress0 = MCS_DEFAULT_ETH_STATIC_SUBNET_MASK_0;
  SMCSConfInfo.SEthSubnetMask.bAddress1 = MCS_DEFAULT_ETH_STATIC_SUBNET_MASK_1;
  SMCSConfInfo.SEthSubnetMask.bAddress2 = MCS_DEFAULT_ETH_STATIC_SUBNET_MASK_2;
  SMCSConfInfo.SEthSubnetMask.bAddress3 = MCS_DEFAULT_ETH_STATIC_SUBNET_MASK_3;

  SMCSConfInfo.SEthGateway.bAddress0 = MCS_DEFAULT_ETH_STATIC_GATEWAY_0;
  SMCSConfInfo.SEthGateway.bAddress1 = MCS_DEFAULT_ETH_STATIC_GATEWAY_1;
  SMCSConfInfo.SEthGateway.bAddress2 = MCS_DEFAULT_ETH_STATIC_GATEWAY_2;
  SMCSConfInfo.SEthGateway.bAddress3 = MCS_DEFAULT_ETH_STATIC_GATEWAY_3;

  SMCSConfInfo.SPrimaryDNSServer.bAddress0 =
    MCS_DEFAULT_PRIMARY_DNS_SERVER_IPV4_0;
  SMCSConfInfo.SPrimaryDNSServer.bAddress1 =
    MCS_DEFAULT_PRIMARY_DNS_SERVER_IPV4_1;
  SMCSConfInfo.SPrimaryDNSServer.bAddress2 =
    MCS_DEFAULT_PRIMARY_DNS_SERVER_IPV4_2;
  SMCSConfInfo.SPrimaryDNSServer.bAddress3 =
    MCS_DEFAULT_PRIMARY_DNS_SERVER_IPV4_3;

  SMCSConfInfo.SSecondaryDNSServer.bAddress0 =
    MCS_DEFAULT_SECONDARY_DNS_SERVER_IPV4_0;
  SMCSConfInfo.SSecondaryDNSServer.bAddress1 =
    MCS_DEFAULT_SECONDARY_DNS_SERVER_IPV4_1;
  SMCSConfInfo.SSecondaryDNSServer.bAddress2 =
    MCS_DEFAULT_SECONDARY_DNS_SERVER_IPV4_2;
  SMCSConfInfo.SSecondaryDNSServer.bAddress3 =
    MCS_DEFAULT_SECONDARY_DNS_SERVER_IPV4_3;

  SMCSConfInfo.SSNMPInfo.lDeviceID = ulUIDHash;
  SMCSConfInfo.SSNMPInfo.bTrapVersion = MCS_DEFAULT_SNMP_TRAP_VERSION;
  strcpy(SMCSConfInfo.SSNMPInfo.strReadCommunityName,
         MCS_DEFAULT_SNMP_READ_COMMUNITY_NAME);
  strcpy(SMCSConfInfo.SSNMPInfo.strWriteCommunityName,
         MCS_DEFAULT_SNMP_WRITE_COMMUNITY_NAME);
  strcpy(SMCSConfInfo.SSNMPInfo.strTrapCommunityName,
         MCS_DEFAULT_SNMP_TRAP_COMMUNITY_NAME);
  strcpy(SMCSConfInfo.SSNMPInfo.strV3EngineId, MCS_DEFAULT_SNMPV3_ENGINE_ID);
  strcpy(SMCSConfInfo.SSNMPInfo.strV3Username, MCS_DEFAULT_SNMPV3_USERNAME);
  strcpy(SMCSConfInfo.SSNMPInfo.strV3AuthPassword, MCS_DEFAULT_SNMPV3_PASSWORD);
  strcpy(SMCSConfInfo.SSNMPInfo.strV3PrivPassword, MCS_DEFAULT_SNMPV3_PASSWORD);

  strcpy(SMCSConfInfo.SSNMPInfo.strTrapDestination,
         MCS_DEFAULT_SNMP_TRAP_DESTINATION);

  MCSWriteConInfo();
} /* MCSInitConInfo */

uint8_t MCSIsConInitialized(void)
{
  return SMCSConfInfo.bInitialized == MCS_CON_INFO_INITIALIZED;
}

uint8_t MCSGetGPRSState(void)
{
  return SMCSRuntime.bState;
}

void MCSSetGPRSState(uint8_t bState)
{
  SMCSRuntime.bState = bState;
}

void MCSSetModemType(uint8_t bModemType)
{
  SMCSConfInfo.bModuleType = bModemType;
}

uint8_t MCSGetModemType(void)
{
  return SMCSConfInfo.bModuleType;
}

void MCSSetRuntimeLocalIPv4(char *strIp)
{
  memset(SMCSRuntime.strLocalIPv4, 0, sizeof(SMCSRuntime.strLocalIPv4));
  SMCSRuntime.strLocalIPv4[0] = '\0';
  memcpy(SMCSRuntime.strLocalIPv4, strIp, sizeof(SMCSRuntime.strLocalIPv4));
}

void MCSSetRuntimeRemoteIPv4(char *strIp)
{
  memset(SMCSRuntime.strRemoteIPv4, 0, sizeof(SMCSRuntime.strRemoteIPv4));
  SMCSRuntime.strRemoteIPv4[0] = '\0';
  memcpy(SMCSRuntime.strRemoteIPv4, strIp, sizeof(SMCSRuntime.strRemoteIPv4));
}

char *MCSGetRuntimeLocalIPv4(void)
{
  return SMCSRuntime.strLocalIPv4;
}

char *MCSGetRuntimeRemoteIPv4(void)
{
  return SMCSRuntime.strRemoteIPv4;
}

void MCSSetRuntimeEthernetMAC(tpSMCSMACAddress pSMAC)
{
  memset(SMCSRuntime.strMAC, 0, sizeof(SMCSRuntime.strMAC));
  SMCSRuntime.strLocalIPv4[0] = '\0';
  sprintf(SMCSRuntime.strMAC,
          "%02X%02X%02X%02X%02X%02X",
          pSMAC->bAddress0,
          pSMAC->bAddress1,
          pSMAC->bAddress2,
          pSMAC->bAddress3,
          pSMAC->bAddress4,
          pSMAC->bAddress5);
}

char *MCSGetRuntimeEthernetMAC(void)
{
  return SMCSRuntime.strMAC;
}

void MCSSetGprsModemIMEI(char *ptrIMEI)
{
  memset(SMCSRuntime.strIMEI, 0, sizeof(SMCSRuntime.strIMEI));
  SMCSRuntime.strIMEI[0] = '\0';
  memcpy(&SMCSRuntime.strIMEI, ptrIMEI, strlen(ptrIMEI));
}

char *MCSGetGprsModemIMEI(void)
{
  return SMCSRuntime.strIMEI;
}

char *MCSGetUSRModuleMAC(void)
{
  return GetDeviceInfoAPNName();
}

void MCSSetGprsGsmOperator(char *ptrGSMOperator)
{
  memset(SMCSRuntime.strGsmOperator, 0, sizeof(SMCSRuntime.strGsmOperator));
  SMCSRuntime.strGsmOperator[0] = '\0';
  memcpy(&SMCSRuntime.strGsmOperator, ptrGSMOperator,
         sizeof(SMCSRuntime.strGsmOperator));
}

char *MCSGetGprsGsmOperator(void)
{
  return SMCSRuntime.strGsmOperator;
}

void MCSSetGprsSignalQuality(uint8_t bSigQuality)
{
  SMCSRuntime.bSignalQuality = bSigQuality;
}

uint8_t MCSGetGprsSignalQuality(void)
{
  return SMCSRuntime.bSignalQuality;
}

void MCSSetConnected(uint8_t bState)
{
  SMCSRuntime.SFlags.fConnected = bState;
}

uint8_t MCSGetConnected(void)
{
  return SMCSRuntime.SFlags.fConnected;
}

void MCSSetModemAlive(uint8_t bState)
{
  SMCSRuntime.SFlags.fModemAlive = bState;
}

uint8_t MCSGetModemAlive(void)
{
  return SMCSRuntime.SFlags.fModemAlive;
}

void MCSSimStatusSet(uint8_t fStatus)
{
  SMCSRuntime.SFlags.fSimStatus = fStatus;
}

uint8_t MCSSimStatusGet(void)
{
  return SMCSRuntime.SFlags.fSimStatus;
}

void MCSJobInit(void)
{
  memset(strCurrentJobs, 0, sizeof(strCurrentJobs));
  bCurrentJobIndex = 0;
}

void MCSJobAdd(char *pStrNewJob)
{
  memset(strCurrentJobs[bCurrentJobIndex], 0,
         sizeof(strCurrentJobs[bCurrentJobIndex]));
  strCurrentJobs[bCurrentJobIndex][0] = '\0';
  strcpy(strCurrentJobs[bCurrentJobIndex], pStrNewJob);
  bCurrentJobIndex++;
  bCurrentJobIndex %= MCS_JOB_LIVE_RECORD_MAX;
}

uint8_t MCSJobCurrentGet(char *pStrBuffer, uint8_t bShift)
{
  uint8_t bIndex = 0;

  if ((bCurrentJobIndex - bShift) >= 1)
  {
    bIndex = (bCurrentJobIndex - bShift) - 1;
  }
  else
  {
    bIndex = (MCS_JOB_LIVE_RECORD_MAX + bCurrentJobIndex - bShift) - 1;
  }

  if (bShift < MCS_JOB_LIVE_RECORD_MAX)
  {
    sprintf(pStrBuffer, "%2d", bIndex);
    strcpy(pStrBuffer, strCurrentJobs[bIndex]);

    return TRUE;
  }

  return FALSE;
}

void MCSRuntimeInit(void)
{
  memset(&SMCSRuntime, 0, sizeof(SMCSRuntime));

  /* Initialize Tx - Rx Buffers */
  memset(strMCSTransmit, 0, sizeof(strMCSTransmit));

  /* Initialize command response */
  memset(strMCSResponse, 0, sizeof(strMCSResponse));

  MCSJobInit();
  MCSRingBufferReset();
}

void MCSInit(ISerialPort_t *port, IModemPort_t *driver)
{
  uint32_t baud;

  s_port = port;
  s_driver = driver;

  MCSRuntimeInit();
  memset(&s_modemInfo, 0, sizeof(s_modemInfo));

  if (!MCSReadConInfo() || !MCSIsConInitialized())
  {
    MCSInitConInfo();
  }

  /* Driver one-time setup (LwIP, PPP create, etc.). */
  ModemOnInit(s_driver, s_port);

  /* Configure baud rate; 0 means no serial (Ethernet NTCIP). */
  baud = ModemGetBaudRate(s_driver);
  if (baud > 0U)
  {
    (void) SerialSetBaudRate(s_port, baud);
  }

  SerialSetRxCallback(s_port, MCSOnRx, NULL);

  SMCSRuntime.bState = ModemGetInitialState(s_driver);
} /* MCSInit */

uint8_t MCSReadModuleType(void)
{
  tSMCSConInfo info;

  memset(&info, 0, sizeof(info));
  (void) PersistenceRead(&g_persistencePort,
                         PERSIST_OBJECT_MCS_CONNECTION_INFO,
                         0U, &info, sizeof(info));

  return (info.bInitialized == MCS_CON_INFO_INITIALIZED)
         ? info.bModuleType : (uint8_t) MCS_MODULE_TYPE_NONE;
}

void MCSConSetResponse(const char *pStrSrc,
                       uint8_t bSrcStartIndex,
                       char *pchDst,
                       const char chStop)
{
  uint8_t bIndex = 0;

  for (bIndex =
         0;
       pStrSrc[bSrcStartIndex + bIndex] != chStop
       && bIndex < MCS_DATA_PACKET_MAX_SIZE - bSrcStartIndex;
       bIndex++)
  {
    pchDst[bIndex] = pStrSrc[bSrcStartIndex + bIndex];
  }

  pchDst[bIndex] = '\0';
}

void MCSBatteryPowerOn(void)
{
  SetHeaterState(TRUE);
  GPIOGPRSPowerEnable();
  SetExternalBatteryState(UserSettingsStandbyFlagGet());
}

void MCSBatteryPowerOff(void)
{
  SetHeaterState(FALSE);
  GPIOGPRSPowerDisable();
}

void MCSBatteryPowerReset(void)
{
  if (!GetStandbyState())
  {
    MCSBatteryPowerOff();
    osDelay(3100);
    MCSBatteryPowerOn();
  }
}

uint8_t MCSWaitKey(const char *pStrKey,
                   char *pStrDst,
                   const char chStop,
                   uint32_t lTimeout)
{
  uint8_t bKeyIdx = 0;
  uint8_t bDstIdx = 0;
  uint32_t lTimeoutCntr = 0;

  uint8_t bKeyLen = strlen(pStrKey);

  while ((s_rxTail == s_rxHead) && (lTimeoutCntr++ < lTimeout))
  {
    osDelay(1);
  }

  if (lTimeoutCntr > lTimeout)
  {
    if (s_rxDataAvailable == 0U)
    {
      SMCSRuntime.SFlags.fNoResponse = TRUE;
    }

    return FALSE;
  }

  while (lTimeoutCntr < lTimeout)
  {
    if (s_rxTail == s_rxHead)
    {
      osDelay(1);
      lTimeoutCntr++;

      continue;
    }

    SMCSRuntime.SFlags.fNoResponse = FALSE;

    if (s_rxRing[s_rxTail] == pStrKey[bKeyIdx])
    {
      uint32_t lKeyTimeoutCntr = lTimeoutCntr;

      while (s_rxRing[s_rxTail] == pStrKey[bKeyIdx])
      {
        if (lKeyTimeoutCntr++ > lTimeout)
        {
          return FALSE;
        }

        s_rxTail++;
        bKeyIdx++;

        if (s_rxTail >= MCS_RX_RING_SIZE)
        {
          s_rxTail = 0U;
        }

        if (bKeyIdx == bKeyLen)
        {
          if (pStrDst != NULL)
          {
            uint32_t lStopTimeoutCntr = lTimeoutCntr;

            while (s_rxRing[s_rxTail] != chStop)
            {
              if (lStopTimeoutCntr++ > lTimeout)
              {
                return FALSE;
              }

              if (s_rxTail == s_rxHead)
              {
                return FALSE;
              }

              pStrDst[bDstIdx] = (char) s_rxRing[s_rxTail];

              s_rxTail++;
              bDstIdx++;

              if (s_rxTail >= MCS_RX_RING_SIZE)
              {
                s_rxTail = 0U;
              }
            }
          }

          return TRUE;
        }
      }
    }
    else
    {
      bKeyIdx = 0;
      s_rxTail++;

      if (s_rxTail >= MCS_RX_RING_SIZE)
      {
        s_rxTail = 0U;
      }
    }

    lTimeoutCntr++;
  }

  return FALSE;
} /* MCSWaitKey */

void MCSSetConInfoChanged(uint8_t fState)
{
  SMCSRuntime.SFlags.fConInfoChanged = fState;
}

uint8_t MCSIsConInfoChanged(void)
{
  return SMCSRuntime.SFlags.fConInfoChanged;
}

static void MCSSetColdStarted(uint8_t fState)
{
  fColdStarted = fState;
}

static uint8_t MCSGetColdStarted(void)
{
  return fColdStarted;
}

void MCSCheckConInfoChange(void)
{
  if (MCSIsConInfoChanged())
  {
    /* Disconnect connection in order to be re-established with new Device Info */
    MCSSetConInfoChanged(FALSE);

    ModemOnDisconnect(s_driver);

    if (MCSGetModemType() == MCS_MODULE_TYPE_ETH_NTCIP)
    {
      LwIPConfigInterface();
    }

    MCSRuntimeInit();
  }
}

void MCSCheckRemoteEnd(void)
{
  if (MCSAsynchIsRemoteEndClosed())
  {
    if (MCSAsynchConnectedGet())
    {
      MCSAsynchStop(MCS_ASYNCH_DISC_TYPE_REMOTE_END_DISCON);
    }
  }
}

void MCSWaitProgramDownload(void)
{
  if (ProgramStateGet() == PROGRAM_STATE_LOADING)
  {
    do
    {
      osDelay(1000);
    }while (ProgramStateGet() == PROGRAM_STATE_LOADING);
  }
}

void MCSInitDefaultATParams(void)
{
  memset(strMCSTransmit, 0, sizeof(strMCSTransmit));
  memset(strMCSResponse, 0, sizeof(strMCSResponse));

  MCSRingBufferReset();
}

void MCSTransmitMessage(void)
{
  uint8_t bLength = (uint8_t) strlen(strMCSTransmit);

  if (bLength != 0U)
  {
    (void) SerialSend(s_port,
                      (const uint8_t *) strMCSTransmit,
                      bLength,
                      MCS_DMA_TX_TIMEOUT);
  }
}

uint8_t MCSIPv4AddrAtoN(const char *strAsciiIp, ip_addr_t *pSBinaryIp)
{
  return ip4addr_aton(strAsciiIp,
                      pSBinaryIp);
}

void MCSDNSFoundCallback(const char *strHostname,
                         const ip_addr_t *pSIP,
                         void *pvArg)
{
  if ((pSIP != NULL) && (pSIP->addr != 0))
  {
    SMCSRuntime.SResolvedIPv4.addr = pSIP->addr;
    SMCSRuntime.SFlags.fDnsResolved = TRUE;

    if (DNSEventHandle != NULL)
    {
      osEventFlagsSet(DNSEventHandle, EVENT_FLAGS_DNS_RESOLVED);
    }
  }
  else
  {
    SMCSRuntime.SResolvedIPv4.addr = 0;
    SMCSRuntime.SFlags.fDnsResolved = FALSE;

    if (DNSEventHandle != NULL)
    {
      osEventFlagsSet(DNSEventHandle, EVENT_FLAGS_DNS_RESOLVE_ERROR);
    }
  }
}

uint8_t MCSResolveDNSString(const char *strHostname, ip_addr_t *pSResolvedIP)
{
  uint8_t bRetval = FALSE;

  switch (dns_gethostbyname(strHostname, pSResolvedIP, MCSDNSFoundCallback,
                            NULL))
  {
      case ERR_OK:
      {
        bRetval = TRUE;
        break;
      }

      case ERR_INPROGRESS:
      {
        if (osEventFlagsWait(DNSEventHandle,
                             EVENT_FLAGS_DNS_RESOLVED
                             | EVENT_FLAGS_DNS_RESOLVE_ERROR,
                             osFlagsWaitAny,
                             osWaitForever)
            == EVENT_FLAGS_DNS_RESOLVED)
        {
          bRetval = TRUE;
        }

        break;
      }

      case ERR_VAL:
      {
        ip_addr_t ipaddr;

        IP4_ADDR(&ipaddr,
                 MCSGetPrimaryDNSServer()->bAddress0,
                 MCSGetPrimaryDNSServer()->bAddress1,
                 MCSGetPrimaryDNSServer()->bAddress2,
                 MCSGetPrimaryDNSServer()->bAddress3);
        dns_setserver(0, &ipaddr);
        IP4_ADDR(&ipaddr,
                 MCSGetSecondaryDNSServer()->bAddress0,
                 MCSGetSecondaryDNSServer()->bAddress1,
                 MCSGetSecondaryDNSServer()->bAddress2,
                 MCSGetSecondaryDNSServer()->bAddress3);
        dns_setserver(1, &ipaddr);
        break;
      }
  } /* switch */

  return bRetval;
} /* MCSResolveDNSString */

void MCSExtractOperator(const char *strIn, char *strOut)
{
  const char *strStart, *strEnd;

  strStart = strchr(strIn, '\"');
  if (strStart != NULL)
  {
    strStart++;
    strEnd = strchr(strStart, '\"');
    if (strEnd != NULL)
    {
      uint8_t bIndex = 0;
      uint8_t bLen = strEnd - strStart;

      if (bLen > GSM_OPERATOR_MAX_SIZE)
      {
        bLen = GSM_OPERATOR_MAX_SIZE;
      }

      strncpy(strOut, strStart, bLen);
      strOut[bLen] = '\0';
      for (bIndex = 0; bIndex < bLen; bIndex++)
      {
        strOut[bIndex] = toupper((unsigned char) strOut[bIndex]);
      }
    }
  }
}

uint8_t MCSSNMPClientStart(void)
{
  if (ServerSettingsNTCIPAvailableGet())
  {
    if (!SNMPClientIsStarted())
    {
      if (!MCSIPv4AddrAtoN(MCSGetSNMPTrapDestination(),
                           &SMCSRuntime.SResolvedIPv4))
      {
        if (!MCSResolveDNSString(MCSGetSNMPTrapDestination(),
                                 &SMCSRuntime.SResolvedIPv4))
        {
          MCSJobAdd("SNMP IP ERR.");

          return FALSE;
        }
      }

      if (!SNMPClientStart(&SMCSRuntime.SResolvedIPv4))
      {
        MCSJobAdd("SNMP CL. CON. ERR");

        return FALSE;
      }
      else
      {
        MCSJobAdd("SNMP CL. STARTED");

        if (MCSGetColdStarted())
        {
          osDelay(1000);

          SNMPSendColdStartTrap();
          MCSSetColdStarted(FALSE);
        }
      }
    }
  }
  else
  {
    if (SNMPClientIsStarted())
    {
      SNMPClientStop();
    }
  }

  return TRUE;
} /* MCSSNMPClientStart */

uint8_t MCSUDPProbeStart(void)
{
  if (ServerSettingsNTCIPAvailableGet())
  {
    if (!UDPProbeIsStarted())
    {
      if (!UDPProbeStart())
      {
        MCSJobAdd("UDP PRB. CON. ERR");

        return FALSE;
      }
      else
      {
        MCSJobAdd("UDP PRB. STARTED");
      }
    }
  }
  else
  {
    if (UDPProbeIsStarted())
    {
      UDPProbeStop();
    }
  }

  return TRUE;
}

uint8_t MCSTCPClientConnect(void)
{
  if (ServerSettingsMCSAvailableGet())
  {
    if (!TCPClientIsConnected())
    {
      if (!MCSIPv4AddrAtoN(GetDeviceDomainName(),
                           &SMCSRuntime.SResolvedIPv4))
      {
        if (!MCSResolveDNSString(GetDeviceDomainName(),
                                 &SMCSRuntime.SResolvedIPv4))
        {
          MCSJobAdd("MCS IP ERR.");

          return FALSE;
        }
      }

      if (!TCPClientConnect(&SMCSRuntime.SResolvedIPv4,
                            MCS_DEFAULT_TCP_DESTINATION_PORT))
      {
        MCSJobAdd("TCP CON. ERR. INIT");

        return FALSE;
      }

      if (osEventFlagsWait(TCPClientEventHandle,
                           EVENT_FLAGS_TCP_CLIENT_CONNECTED
                           | EVENT_FLAGS_TCP_CLIENT_ERROR,
                           osFlagsWaitAny,
                           30000) != EVENT_FLAGS_TCP_CLIENT_CONNECTED)
      {
        MCSJobAdd("TCP CON. ERR. TIM");

        return FALSE;
      }

      MCSJobAdd("TCP CON. SUC.");
    }
  }
  else
  {
    if (TCPClientIsConnected())
    {
      TCPClientDisconnect();
    }
  }

  return TRUE;
} /* MCSTCPClientConnect */

/* Copy driver-populated ModemInfo_t fields into SMCSRuntime and trigger
 * any side-effects (stream to LCD, broadcast IMEI, etc.).  Called once
 * per MCSConfigGPRSModules() cycle, after ModemHandleResponse(). */
static void MCSUpdateRuntimeFromInfo(void)
{
  if (s_modemInfo.strIMEI[0] != '\0')
  {
    MCSSetGprsModemIMEI(s_modemInfo.strIMEI);
    memset(s_modemInfo.strIMEI, 0, sizeof(s_modemInfo.strIMEI));
  }

  if (s_modemInfo.strOperator[0] != '\0')
  {
    MCSSetGprsGsmOperator(s_modemInfo.strOperator);
    memset(s_modemInfo.strOperator, 0, sizeof(s_modemInfo.strOperator));
  }

  if (s_modemInfo.bSignalQualityValid != 0U)
  {
    MCSSetGprsSignalQuality(s_modemInfo.bSignalQuality);
    s_modemInfo.bSignalQualityValid = 0U;
  }

  if (s_modemInfo.bModemAliveValid != 0U)
  {
    MCSSetModemAlive(s_modemInfo.bModemAlive);
    s_modemInfo.bModemAliveValid = 0U;
  }

  if (s_modemInfo.bSimReadyValid != 0U)
  {
    MCSSimStatusSet(s_modemInfo.bSimReady);
    s_modemInfo.bSimReadyValid = 0U;
  }

  if (s_modemInfo.strJobLabel[0] != '\0')
  {
    MCSJobAdd(s_modemInfo.strJobLabel);
    memset(s_modemInfo.strJobLabel, 0, sizeof(s_modemInfo.strJobLabel));
  }

  if (s_modemInfo.strMAC[0] != '\0')
  {
    /* Ethernet/USR adapter populated the runtime MAC field. */
    memset(SMCSRuntime.strMAC, 0, sizeof(SMCSRuntime.strMAC));
    (void) strncpy(SMCSRuntime.strMAC, s_modemInfo.strMAC,
                   sizeof(SMCSRuntime.strMAC) - 1U);
    memset(s_modemInfo.strMAC, 0, sizeof(s_modemInfo.strMAC));
  }

  if (s_modemInfo.bLocalIPv4Valid != 0U)
  {
    MCSSetRuntimeLocalIPv4(s_modemInfo.strLocalIPv4);
    MCSJobAdd(MCSGetRuntimeLocalIPv4());
    s_modemInfo.bLocalIPv4Valid = 0U;
  }
} /* MCSUpdateRuntimeFromInfo */

void MCSTryConnect(void)
{
  if (!ModemOnConnect(s_driver))
  {
    if (++SMCSRuntime.bATCmdTries > MCS_MAX_COMMAND_TRY)
    {
      MCSRuntimeInit();
    }
  }
}

/* Transport-agnostic modem configuration coordinator.
 * Drives the active IModemPort_t driver through its state machine one
 * step per call.  Handles all modem types via the adapter interface. */
void MCSConfigGPRSModules(void)
{
  char cmd[MCS_DATA_PACKET_MAX_SIZE + 1];
  const char *keyword;
  char stopChar;
  uint32_t timeoutMs;
  uint8_t maxRetries;

  MCSInitDefaultATParams();
  memset(&s_modemInfo, 0, sizeof(s_modemInfo));
  memset(cmd, 0, sizeof(cmd));

  ModemGetWaitParams(s_driver, SMCSRuntime.bState,
                     &keyword, &stopChar, &timeoutMs, &maxRetries);

  if (ModemPrepareCommand(s_driver,
                          SMCSRuntime.bState,
                          GetDeviceInfoAPNName(),
                          GetDeviceDomainName(),
                          MCS_DEFAULT_TCP_DESTINATION_PORT,
                          cmd,
                          (uint16_t) sizeof(cmd)) != 0U)
  {
    (void) memcpy(strMCSTransmit, cmd, strlen(cmd) + 1U);
    MCSTransmitMessage();
  }

  MCSJobAdd((char *) ModemGetStateLabel(s_driver, SMCSRuntime.bState));

  SMCSRuntime.SFlags.fKeyReceived =
    MCSWaitKey(keyword, strMCSResponse, stopChar, timeoutMs);

  if (SMCSRuntime.SFlags.fKeyReceived != 0U)
  {
    SMCSRuntime.bATCmdTries = 0U;
    SMCSRuntime.bState =
      ModemHandleResponse(s_driver,
                          SMCSRuntime.bState,
                          strMCSResponse,
                          1U,
                          &s_modemInfo);
  }
  else
  {
    SMCSRuntime.bATCmdTries++;
    if (SMCSRuntime.bATCmdTries >= maxRetries)
    {
      SMCSRuntime.bATCmdTries = 0U;
      SMCSRuntime.bState =
        ModemHandleResponse(s_driver,
                            SMCSRuntime.bState,
                            strMCSResponse,
                            0U,
                            &s_modemInfo);
    }
  }

  MCSUpdateRuntimeFromInfo();

  if (SMCSRuntime.bState == MODEM_STATE_CONNECTED)
  {
    MCSTryConnect();
  }
  else if (SMCSRuntime.bState == MODEM_STATE_FAILED)
  {
    MCSBatteryPowerReset();
    MCSRingBufferReset();
    SMCSRuntime.bState = ModemGetInitialState(s_driver);
    SMCSRuntime.bATCmdTries = 0U;
  }
  else
  {
    /* State machine continues on next call. */
  }
} /* MCSConfigGPRSModules */

void MCSUDPProbeResultCb(uint8_t fResult)
{
  if (!fResult)
  {
    if (++SMCSRuntime.bProbeErrCntr > MCS_MAX_PROBE_ERRORS)
    {
      SMCSRuntime.bProbeErrCntr = 0;

      ModemOnDisconnect(s_driver);
    }
  }
  else
  {
    SMCSRuntime.bProbeErrCntr = 0;
  }
}

void MCSUDPConnectionMaintain(void)
{
  if (UDPProbeIsStarted())
  {
    if (++SMCSRuntime.bProbeTimeoutCntr > MCS_PROBE_TIMEOUT)
    {
      SMCSRuntime.bProbeTimeoutCntr = 0;
      UDPProbeSend(MCSUDPProbeResultCb);
    }
  }
  else
  {
    SMCSRuntime.bProbeTimeoutCntr = 0;
  }
}

void MCSModuleConfig(void)
{
  /* All modem types are now driven by the injected IModemPort_t adapter.
   * USrModemAdapter and EthernetNtcipModemAdapter implement the same
   * IModemPort_t interface as the GPRS adapters; no switch needed. */
  MCSConfigGPRSModules();
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  tasks */
void MCSTaskFunc(void const *pArg)
{
  (void) pArg;

  MCSBatteryPowerOn();

  osDelay(1000);

  MCSInit(&g_modemPort, &g_modemDriverPort);

  while (FOREVER)
  {
    MCSWaitProgramDownload();

    MCSCheckConInfoChange();

    ModemOnMaintain(s_driver);

    if (!MCSGetConnected())
    {
      MCSModuleConfig();
    }

    osDelay(1000);
  }
}
