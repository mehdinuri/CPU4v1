/**
 ******************************************************************************
 * @file    MCS.c
 * @author  Okan KILIC - Teknotel Electronics
 * @version V1.0.0
 * @date    07/19/2011
 * @brief   Maestro connection supervisor implementation
 ******************************************************************************
 */

#include "MCS.h"
#include "HardwarePorts.h"
#include "PersistencePorts.h"

#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmsis_os.h"
#include "data.h"
#include "defs.h"
#include "gpio.h"
#include "lwip.h"
#include "lwip/dns.h"
#include "rng.h"
#include "snmp_client.h"
#include "snmpv3_client.h"

#define MCS_DMA_TX_TIMEOUT 1000U
#define MCS_DEFAULT_AT_CMD_RECEIVE_TIMEOUT 1000U
#define MCS_MAX_COMMAND_TRY 2U
#define MCS_MAX_NET_TRY 60U
#define MCS_SNMP_RETRY_DELAY_SECONDS 5U
#define MCS_QUECTEL_HARD_RESET_FAILURES 3U
#define MCS_DNS_RESOLVE_MAX_TIMEOUT 30000U

#define MCS_MODEM_NO_RESPONSE "NO RESPONSE"
#define MCS_MODEM_INVALID_RESPONSE "INVALID RESPONSE"

static tSMCSRuntime SMCSRuntime;
static tSMCSConInfo SMCSConfInfo;
static tSMCSSNMPv3State SMCSSNMPv3State;

__attribute__((section(".ram_d2_dma_buffers"), aligned(32)))
char strMCSTransmit[MCS_DATA_PACKET_MAX_SIZE + 1];

static char strMCSResponse[MCS_DATA_PACKET_MAX_SIZE + 1];

char strCurrentJobs[MCS_JOB_LIVE_RECORD_MAX][MCS_JOB_LIVE_RECORD_SIZE_MAX];
static uint8_t bCurrentJobIndex;

#define MCS_RX_RING_SIZE (MCS_DATA_PACKET_MAX_LEN * 2U)
static uint8_t s_rxRing[MCS_RX_RING_SIZE];
static uint16_t s_rxHead;
static uint16_t s_rxTail;
static uint8_t s_rxDataAvailable;

static ISerialPort_t *s_port;
static IModemPort_t *s_driver;
static ModemInfo_t s_modemInfo;
static uint8_t s_activeNetworkType;
static uint8_t fColdStarted = TRUE;

static void ClearAscii(char *buffer, size_t size)
{
  if ((buffer != NULL) && (size > 0U))
  {
    (void) memset(buffer, 0, size);
  }
}

static void CopyAscii(char *dst, size_t dstSize, const char *src)
{
  size_t index;

  if ((dst == NULL) || (dstSize == 0U))
  {
    return;
  }

  (void) memset(dst, 0, dstSize);
  if (src == NULL)
  {
    return;
  }

  for (index = 0U; index < (dstSize - 1U); index++)
  {
    if (src[index] == '\0')
    {
      break;
    }

    dst[index] = src[index];
  }
}

static uint8_t IsValidNetworkType(uint8_t networkType)
{
  return (uint8_t) (networkType < (uint8_t) MCS_NETWORK_TYPE_MAX);
}

static uint8_t PreferredAuthAlgo(void)
{
#if LWIP_SNMP && LWIP_SNMP_V3 && LWIP_SNMP_V3_CRYPTO
  return (uint8_t) SNMP_V3_AUTH_ALGO_SHA;
#else
  return (uint8_t) SNMP_V3_AUTH_ALGO_INVAL;
#endif
}

static uint8_t PreferredPrivAlgo(uint8_t authAlgo)
{
#if LWIP_SNMP && LWIP_SNMP_V3 && LWIP_SNMP_V3_CRYPTO
  if (authAlgo != (uint8_t) SNMP_V3_AUTH_ALGO_INVAL)
  {
    return (uint8_t) SNMP_V3_PRIV_ALGO_AES;
  }
#else
  LWIP_UNUSED_ARG(authAlgo);
#endif

  return (uint8_t) SNMP_V3_PRIV_ALGO_INVAL;
}

static void MCSApplyPowerPolicy(void)
{
  if (s_activeNetworkType == (uint8_t) MCS_NETWORK_TYPE_QUECTEL)
  {
    GPIOGPRSPowerEnable();
  }
  else
  {
    GPIOGPRSPowerDisable();
  }
}

static void MCSGenerateDefaultSnmpEngineId(char *dst, size_t dstSize)
{
  ReadCPUDeviceUID();
  (void) snprintf(dst,
                  dstSize,
                  "MAESTRO-%08" PRIX32 "%08" PRIX32 "%08" PRIX32,
                  GetCPUDeviceUID()->ulaUID[0],
                  GetCPUDeviceUID()->ulaUID[1],
                  GetCPUDeviceUID()->ulaUID[2]);
}

static uint8_t MCSDeriveLocalizedKeys(const char *engineId,
                                      const char *authPassphrase,
                                      const char *privPassphrase,
                                      tSMCSSNMPv3State *state)
{
  uint8_t authAlgo;
  uint8_t privAlgo;

  if ((engineId == NULL) || (state == NULL))
  {
    return FALSE;
  }

  authAlgo = PreferredAuthAlgo();
  privAlgo = PreferredPrivAlgo(authAlgo);

  state->bAuthAlgo = authAlgo;
  state->bPrivAlgo = privAlgo;
  state->bAuthConfigured = FALSE;
  state->bPrivConfigured = FALSE;
  (void) memset(&state->baAuthKey[0], 0, sizeof(state->baAuthKey));
  (void) memset(&state->baPrivKey[0], 0, sizeof(state->baPrivKey));

  if ((authAlgo == (uint8_t) SNMP_V3_AUTH_ALGO_INVAL)
      || (privAlgo == (uint8_t) SNMP_V3_PRIV_ALGO_INVAL))
  {
    return TRUE;
  }

  if ((authPassphrase == NULL) || (privPassphrase == NULL))
  {
    return FALSE;
  }

  if (SNMPv3LocalizeAuthKey(authAlgo,
                            engineId,
                            authPassphrase,
                            &state->baAuthKey[0],
                            sizeof(state->baAuthKey)) != ERR_OK)
  {
    return FALSE;
  }

  if (SNMPv3LocalizePrivKey(authAlgo,
                            engineId,
                            privPassphrase,
                            &state->baPrivKey[0],
                            sizeof(state->baPrivKey)) != ERR_OK)
  {
    (void) memset(&state->baAuthKey[0], 0, sizeof(state->baAuthKey));
    return FALSE;
  }

  state->bAuthConfigured = TRUE;
  state->bPrivConfigured = TRUE;
  return TRUE;
}

static uint8_t MCSIsSNMPv3StateInitialized(void)
{
  return (uint8_t) (SMCSSNMPv3State.strEngineId[0] != '\0');
}

static void MCSClearResolvedManagerIp(void)
{
  ClearAscii(&SMCSRuntime.strManagerIPv4[0], sizeof(SMCSRuntime.strManagerIPv4));
  SMCSRuntime.SResolvedIPv4.addr = 0U;
  SMCSRuntime.SFlags.fDnsResolved = FALSE;
}

static void MCSClearLocalTransportIp(void)
{
  ClearAscii(&SMCSRuntime.strLocalIPv4[0], sizeof(SMCSRuntime.strLocalIPv4));
}

static void MCSResetTransportState(uint8_t hardReset)
{
  if ((s_driver != NULL) && (s_driver->OnDisconnect != NULL))
  {
    ModemOnDisconnect(s_driver);
  }

  if ((hardReset != 0U)
      && (s_activeNetworkType == (uint8_t) MCS_NETWORK_TYPE_QUECTEL))
  {
    if (!GetStandbyState())
    {
      GPIOGPRSPowerDisable();
      osDelay(3100U);
      GPIOGPRSPowerEnable();
      osDelay(1000U);
    }
  }

  if (SNMPClientIsStarted())
  {
    SNMPClientStop();
  }

  MCSSetConnected(FALSE);
  MCSSetSnmpReady(FALSE);
  MCSClearLocalTransportIp();
  MCSClearResolvedManagerIp();
  SMCSRuntime.bATCmdTries = 0U;
  SMCSRuntime.bSnmpRetryCountdown = 0U;
  MCSRingBufferReset();

  if ((s_driver != NULL) && (s_driver->GetInitialState != NULL))
  {
    SMCSRuntime.bState = ModemGetInitialState(s_driver);
  }
  else
  {
    SMCSRuntime.bState = 0U;
  }
}

void MCSRingBufferReset(void)
{
  (void) memset(s_rxRing, 0, sizeof(s_rxRing));
  s_rxHead = 0U;
  s_rxTail = 0U;
  s_rxDataAvailable = 0U;
}

void MCSRingBufferWrite(const uint8_t *data, uint16_t len)
{
  uint16_t index;

  s_rxDataAvailable = 1U;

  for (index = 0U; index < len; index++)
  {
    s_rxRing[s_rxHead] = data[index];
    s_rxHead++;

    if (s_rxHead >= MCS_RX_RING_SIZE)
    {
      s_rxHead = 0U;
    }
  }
}

static void MCSOnRx(void *arg, const uint8_t *data, uint16_t len)
{
  LWIP_UNUSED_ARG(arg);

  if ((s_driver != NULL) && (s_driver->OnRx != NULL))
  {
    ModemOnRx(s_driver, data, len);
  }
}

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
  uint8_t readOk;

  readOk = PersistenceRead(&g_persistencePort,
                           PERSIST_OBJECT_MCS_CONNECTION_INFO,
                           0U,
                           &SMCSConfInfo,
                           sizeof(SMCSConfInfo));

  if ((readOk != FALSE) && (IsValidNetworkType(SMCSConfInfo.bNetworkType) == 0U))
  {
    SMCSConfInfo.bNetworkType = (uint8_t) MCS_NETWORK_TYPE_NONE;
  }

  return readOk;
}

uint8_t MCSWriteSNMPv3State(void)
{
  return PersistenceWrite(&g_persistencePort,
                          PERSIST_OBJECT_SNMPV3_STATE,
                          0U,
                          &SMCSSNMPv3State,
                          sizeof(SMCSSNMPv3State));
}

uint8_t MCSReadSNMPv3State(void)
{
  return PersistenceRead(&g_persistencePort,
                         PERSIST_OBJECT_SNMPV3_STATE,
                         0U,
                         &SMCSSNMPv3State,
                         sizeof(SMCSSNMPv3State));
}

void MCSSetConInfo(tpSMCSConInfo pSInfo)
{
  (void) memcpy(&SMCSConfInfo, pSInfo, sizeof(SMCSConfInfo));
}

void MCSGetConInfo(tpSMCSConInfo pSInfo)
{
  (void) memcpy(pSInfo, &SMCSConfInfo, sizeof(SMCSConfInfo));
}

tpSMCSConInfo MCSGetConInfoPtr(void)
{
  return &SMCSConfInfo;
}

const tSMCSSNMPv3State *MCSGetSNMPv3State(void)
{
  return &SMCSSNMPv3State;
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
  return &SMCSConfInfo.SSNMPInfo.strReadCommunityName[0];
}

char *MCSGetSNMPWriteCommunityName(void)
{
  return &SMCSConfInfo.SSNMPInfo.strWriteCommunityName[0];
}

char *MCSGetSNMPTrapCommunityName(void)
{
  return &SMCSConfInfo.SSNMPInfo.strTrapCommunityName[0];
}

char *MCSGetSNMPv3EngineID(void)
{
  return &SMCSSNMPv3State.strEngineId[0];
}

char *MCSGetSNMPv3Username(void)
{
  return &SMCSConfInfo.SSNMPInfo.strV3Username[0];
}

char *MCSGetSNMPTrapDestination(void)
{
  return &SMCSConfInfo.SSNMPInfo.strTrapDestination[0];
}

uint32_t MCSGetFNV132Hash(const uint8_t *pbData, uint8_t bLen)
{
  uint8_t bIdx = 0U;
  uint32_t ulHash = MCS_FNV_OFFSET_BASIS;

  for (bIdx = 0U; bIdx < bLen; ++bIdx)
  {
    ulHash ^= pbData[bIdx];
    ulHash *= MCS_FNV_PRIME;
  }

  return ulHash;
}

uint32_t MCSGenerateUIDHash(void)
{
  uint8_t bIdx = 0U;
  uint8_t baUIDBytes[UID_MAX_LENGTH * sizeof(uint32_t)];

  ReadCPUDeviceUID();

  for (bIdx = 0U; bIdx < UID_MAX_LENGTH; ++bIdx)
  {
    baUIDBytes[bIdx * sizeof(uint32_t)] =
      (uint8_t) (GetCPUDeviceUID()->ulaUID[bIdx] >> 24);
    baUIDBytes[(bIdx * sizeof(uint32_t)) + 1U] =
      (uint8_t) (GetCPUDeviceUID()->ulaUID[bIdx] >> 16);
    baUIDBytes[(bIdx * sizeof(uint32_t)) + 2U] =
      (uint8_t) (GetCPUDeviceUID()->ulaUID[bIdx] >> 8);
    baUIDBytes[(bIdx * sizeof(uint32_t)) + 3U] =
      (uint8_t) GetCPUDeviceUID()->ulaUID[bIdx];
  }

  return MCSGetFNV132Hash(baUIDBytes, sizeof(baUIDBytes));
}

void MCSInitConInfo(void)
{
  uint32_t ulUIDHash = 0U;
  uint32_t ulRand = 0U;
  uint8_t fRandGenOk = FALSE;

  (void) memset(&SMCSConfInfo, 0, sizeof(SMCSConfInfo));

  ulUIDHash = MCSGenerateUIDHash();
  fRandGenOk = (uint8_t) (HAL_RNG_GenerateRandomNumber(&hrng, &ulRand) == HAL_OK);

  SMCSConfInfo.bInitialized = MCS_CON_INFO_INITIALIZED;
  SMCSConfInfo.bNetworkType = (uint8_t) MCS_NETWORK_TYPE_ETHERNET;
  SMCSConfInfo.SFlags.fEthStaticIp = TRUE;

  SMCSConfInfo.SMACAddress.bAddress0 = MCS_DEFAULT_ETH_MAC_ADDRESS_0;
  SMCSConfInfo.SMACAddress.bAddress1 = (uint8_t) ((ulUIDHash >> 24) & 0xFFU);
  SMCSConfInfo.SMACAddress.bAddress2 = (uint8_t) ((ulUIDHash >> 16) & 0xFFU);
  SMCSConfInfo.SMACAddress.bAddress3 = (uint8_t) ((ulUIDHash >> 8) & 0xFFU);
  SMCSConfInfo.SMACAddress.bAddress4 = (uint8_t) (ulUIDHash & 0xFFU);
  SMCSConfInfo.SMACAddress.bAddress5 = fRandGenOk != FALSE
                                       ? (uint8_t) ((ulRand >> 8) & 0xFFU)
                                       : MCS_DEFAULT_ETH_MAC_ADDRESS_5;

  SMCSConfInfo.SEthLocalIPv4.bAddress0 = MCS_DEFAULT_ETH_STATIC_IPV4_0;
  SMCSConfInfo.SEthLocalIPv4.bAddress1 = MCS_DEFAULT_ETH_STATIC_IPV4_1;
  SMCSConfInfo.SEthLocalIPv4.bAddress2 = MCS_DEFAULT_ETH_STATIC_IPV4_2;
  SMCSConfInfo.SEthLocalIPv4.bAddress3 = fRandGenOk != FALSE
                                         ? (uint8_t) (ulRand & 0xFFU)
                                         : MCS_DEFAULT_ETH_STATIC_IPV4_3;

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
  CopyAscii(&SMCSConfInfo.SSNMPInfo.strReadCommunityName[0],
            sizeof(SMCSConfInfo.SSNMPInfo.strReadCommunityName),
            MCS_DEFAULT_SNMP_READ_COMMUNITY_NAME);
  CopyAscii(&SMCSConfInfo.SSNMPInfo.strWriteCommunityName[0],
            sizeof(SMCSConfInfo.SSNMPInfo.strWriteCommunityName),
            MCS_DEFAULT_SNMP_WRITE_COMMUNITY_NAME);
  CopyAscii(&SMCSConfInfo.SSNMPInfo.strTrapCommunityName[0],
            sizeof(SMCSConfInfo.SSNMPInfo.strTrapCommunityName),
            MCS_DEFAULT_SNMP_TRAP_COMMUNITY_NAME);
  CopyAscii(&SMCSConfInfo.SSNMPInfo.strV3Username[0],
            sizeof(SMCSConfInfo.SSNMPInfo.strV3Username),
            MCS_DEFAULT_SNMPV3_USERNAME);
  CopyAscii(&SMCSConfInfo.SSNMPInfo.strTrapDestination[0],
            sizeof(SMCSConfInfo.SSNMPInfo.strTrapDestination),
            MCS_DEFAULT_SNMP_TRAP_DESTINATION);

  (void) MCSWriteConInfo();
}

void MCSInitSNMPv3State(void)
{
  (void) memset(&SMCSSNMPv3State, 0, sizeof(SMCSSNMPv3State));
  MCSGenerateDefaultSnmpEngineId(&SMCSSNMPv3State.strEngineId[0],
                                 sizeof(SMCSSNMPv3State.strEngineId));
  (void) MCSDeriveLocalizedKeys(&SMCSSNMPv3State.strEngineId[0],
                                MCS_DEFAULT_SNMPV3_PASSWORD,
                                MCS_DEFAULT_SNMPV3_PASSWORD,
                                &SMCSSNMPv3State);
  (void) MCSWriteSNMPv3State();
}

uint8_t MCSSetSNMPv3Username(const char *username)
{
  size_t len;

  if (username == NULL)
  {
    return FALSE;
  }

  len = strlen(username);
  if ((len < SNMPV3_USERNAME_MIN_SIZE) || (len > SNMPV3_USERNAME_MAX_SIZE))
  {
    return FALSE;
  }

  CopyAscii(&SMCSConfInfo.SSNMPInfo.strV3Username[0],
            sizeof(SMCSConfInfo.SSNMPInfo.strV3Username),
            username);
  return MCSWriteConInfo();
}

uint8_t MCSSetSNMPv3Credentials(const char *username,
                                const char *authPassphrase,
                                const char *privPassphrase)
{
  if ((authPassphrase == NULL) || (privPassphrase == NULL))
  {
    return FALSE;
  }

  if ((strlen(authPassphrase) < SNMPV3_PASSWORD_MIN_SIZE)
      || (strlen(authPassphrase) > SNMPV3_PASSWORD_MAX_SIZE)
      || (strlen(privPassphrase) < SNMPV3_PASSWORD_MIN_SIZE)
      || (strlen(privPassphrase) > SNMPV3_PASSWORD_MAX_SIZE))
  {
    return FALSE;
  }

  if ((username != NULL) && (MCSSetSNMPv3Username(username) == FALSE))
  {
    return FALSE;
  }

  if (MCSDeriveLocalizedKeys(&SMCSSNMPv3State.strEngineId[0],
                             authPassphrase,
                             privPassphrase,
                             &SMCSSNMPv3State) == FALSE)
  {
    return FALSE;
  }

  if (MCSWriteSNMPv3State() == FALSE)
  {
    return FALSE;
  }

  if (SNMPClientIsStarted())
  {
    SNMPClientStop();
    MCSSetSnmpReady(FALSE);
    SMCSRuntime.bSnmpRetryCountdown = 0U;
  }

  return TRUE;
}

uint8_t MCSIsConInitialized(void)
{
  return (uint8_t) (SMCSConfInfo.bInitialized == MCS_CON_INFO_INITIALIZED);
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
  if (IsValidNetworkType(bModemType) != 0U)
  {
    SMCSConfInfo.bNetworkType = bModemType;
  }
  else
  {
    SMCSConfInfo.bNetworkType = (uint8_t) MCS_NETWORK_TYPE_NONE;
  }
}

uint8_t MCSGetModemType(void)
{
  return SMCSConfInfo.bNetworkType;
}

void MCSSetRuntimeLocalIPv4(const char *strIp)
{
  CopyAscii(&SMCSRuntime.strLocalIPv4[0],
            sizeof(SMCSRuntime.strLocalIPv4),
            strIp);
}

void MCSSetRuntimeManagerIPv4(const char *strIp)
{
  CopyAscii(&SMCSRuntime.strManagerIPv4[0],
            sizeof(SMCSRuntime.strManagerIPv4),
            strIp);
}

char *MCSGetRuntimeLocalIPv4(void)
{
  return &SMCSRuntime.strLocalIPv4[0];
}

char *MCSGetRuntimeManagerIPv4(void)
{
  return &SMCSRuntime.strManagerIPv4[0];
}

char *MCSGetRuntimeEthernetMAC(void)
{
  return &SMCSRuntime.strMAC[0];
}

char *MCSGetGprsModemIMEI(void)
{
  return &SMCSRuntime.strIMEI[0];
}

char *MCSGetGprsGsmOperator(void)
{
  return &SMCSRuntime.strGsmOperator[0];
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

void MCSSetSnmpReady(uint8_t bState)
{
  SMCSRuntime.SFlags.fSnmpReady = bState;
}

uint8_t MCSGetSnmpReady(void)
{
  return SMCSRuntime.SFlags.fSnmpReady;
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
  (void) memset(strCurrentJobs, 0, sizeof(strCurrentJobs));
  bCurrentJobIndex = 0U;
}

void MCSJobAdd(char *pStrNewJob)
{
  if (pStrNewJob == NULL)
  {
    return;
  }

  (void) memset(strCurrentJobs[bCurrentJobIndex],
                0,
                sizeof(strCurrentJobs[bCurrentJobIndex]));
  CopyAscii(&strCurrentJobs[bCurrentJobIndex][0],
            sizeof(strCurrentJobs[bCurrentJobIndex]),
            pStrNewJob);
  bCurrentJobIndex++;
  bCurrentJobIndex %= MCS_JOB_LIVE_RECORD_MAX;
}

uint8_t MCSJobCurrentGet(char *pStrBuffer, uint8_t bShift)
{
  uint8_t bIndex = 0U;

  if (pStrBuffer == NULL)
  {
    return FALSE;
  }

  if (bShift >= MCS_JOB_LIVE_RECORD_MAX)
  {
    return FALSE;
  }

  if ((bCurrentJobIndex - bShift) >= 1U)
  {
    bIndex = (uint8_t) ((bCurrentJobIndex - bShift) - 1U);
  }
  else
  {
    bIndex = (uint8_t) ((MCS_JOB_LIVE_RECORD_MAX + bCurrentJobIndex - bShift) - 1U);
  }

  CopyAscii(pStrBuffer,
            MCS_JOB_LIVE_RECORD_SIZE_MAX + 1U,
            &strCurrentJobs[bIndex][0]);
  return TRUE;
}

void MCSRuntimeInit(void)
{
  (void) memset(&SMCSRuntime, 0, sizeof(SMCSRuntime));
  (void) memset(strMCSTransmit, 0, sizeof(strMCSTransmit));
  (void) memset(strMCSResponse, 0, sizeof(strMCSResponse));
  MCSJobInit();
  MCSRingBufferReset();
}

void MCSInit(ISerialPort_t *port, IModemPort_t *driver)
{
  uint32_t baud = 0U;

  s_port = port;
  s_driver = driver;

  MCSRuntimeInit();
  (void) memset(&s_modemInfo, 0, sizeof(s_modemInfo));

  if ((MCSReadConInfo() == FALSE) || (MCSIsConInitialized() == FALSE))
  {
    MCSInitConInfo();
  }

  if ((MCSReadSNMPv3State() == FALSE) || (MCSIsSNMPv3StateInitialized() == FALSE))
  {
    MCSInitSNMPv3State();
  }

  s_activeNetworkType = SMCSConfInfo.bNetworkType;
  MCSApplyPowerPolicy();

  if (s_activeNetworkType == (uint8_t) MCS_NETWORK_TYPE_QUECTEL)
  {
    osDelay(1000U);
  }

  if ((s_activeNetworkType != (uint8_t) MCS_NETWORK_TYPE_NONE)
      && (s_driver != NULL)
      && (s_driver->OnInit != NULL))
  {
    ModemOnInit(s_driver, s_port);

    baud = ModemGetBaudRate(s_driver);
    if ((s_port != NULL) && (baud > 0U))
    {
      (void) SerialSetBaudRate(s_port, baud);
    }

    if (s_port != NULL)
    {
      SerialSetRxCallback(s_port, MCSOnRx, NULL);
    }

    SMCSRuntime.bState = ModemGetInitialState(s_driver);
  }
}

void MCSBatteryPowerOff(void)
{
  GPIOGPRSPowerDisable();
}

static void MCSInitDefaultATParams(void)
{
  (void) memset(strMCSTransmit, 0, sizeof(strMCSTransmit));
  (void) memset(strMCSResponse, 0, sizeof(strMCSResponse));
  MCSRingBufferReset();
}

static void MCSTransmitMessage(void)
{
  uint8_t bLength = (uint8_t) strlen(strMCSTransmit);

  if ((bLength != 0U) && (s_port != NULL))
  {
    (void) SerialSend(s_port,
                      (const uint8_t *) strMCSTransmit,
                      bLength,
                      MCS_DMA_TX_TIMEOUT);
  }
}

static uint8_t MCSIPv4AddrAtoN(const char *strAsciiIp, ip_addr_t *pSBinaryIp)
{
  return ip4addr_aton(strAsciiIp, pSBinaryIp);
}

void MCSDNSFoundCallback(const char *strHostname,
                         const ip_addr_t *pSIP,
                         void *pvArg)
{
  LWIP_UNUSED_ARG(strHostname);
  LWIP_UNUSED_ARG(pvArg);

  if ((pSIP != NULL) && (pSIP->addr != 0U))
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
    MCSClearResolvedManagerIp();

    if (DNSEventHandle != NULL)
    {
      osEventFlagsSet(DNSEventHandle, EVENT_FLAGS_DNS_RESOLVE_ERROR);
    }
  }
}

static uint8_t MCSResolveDNSString(const char *strHostname, ip_addr_t *pSResolvedIP)
{
  if ((strHostname == NULL) || (pSResolvedIP == NULL))
  {
    return FALSE;
  }

  if (DNSEventHandle != NULL)
  {
    (void) osEventFlagsClear(DNSEventHandle,
                             EVENT_FLAGS_DNS_RESOLVED
                             | EVENT_FLAGS_DNS_RESOLVE_ERROR);
  }

  switch (dns_gethostbyname(strHostname,
                            pSResolvedIP,
                            MCSDNSFoundCallback,
                            NULL))
  {
      case ERR_OK:
      {
        return TRUE;
      }

      case ERR_INPROGRESS:
      {
        if (osEventFlagsWait(DNSEventHandle,
                             EVENT_FLAGS_DNS_RESOLVED
                             | EVENT_FLAGS_DNS_RESOLVE_ERROR,
                             osFlagsWaitAny,
                             MCS_DNS_RESOLVE_MAX_TIMEOUT)
            == EVENT_FLAGS_DNS_RESOLVED)
        {
          *pSResolvedIP = SMCSRuntime.SResolvedIPv4;
          return TRUE;
        }

        return FALSE;
      }

      default:
      {
        return FALSE;
      }
  }
}

static uint8_t MCSWaitKey(const char *pStrKey,
                          char *pStrDst,
                          const char chStop,
                          uint32_t lTimeout)
{
  uint8_t bKeyIdx = 0U;
  uint8_t bDstIdx = 0U;
  uint32_t lTimeoutCntr = 0U;
  uint8_t bKeyLen;

  if (pStrKey == NULL)
  {
    return FALSE;
  }

  bKeyLen = (uint8_t) strlen(pStrKey);

  while ((s_rxTail == s_rxHead) && (lTimeoutCntr++ < lTimeout))
  {
    osDelay(1U);
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
      osDelay(1U);
      lTimeoutCntr++;
      continue;
    }

    SMCSRuntime.SFlags.fNoResponse = FALSE;

    if (s_rxRing[s_rxTail] == (uint8_t) pStrKey[bKeyIdx])
    {
      uint32_t lKeyTimeoutCntr = lTimeoutCntr;

      while (s_rxRing[s_rxTail] == (uint8_t) pStrKey[bKeyIdx])
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

            while (s_rxRing[s_rxTail] != (uint8_t) chStop)
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

              if (bDstIdx >= MCS_DATA_PACKET_MAX_SIZE)
              {
                break;
              }
            }

            pStrDst[bDstIdx] = '\0';
          }

          return TRUE;
        }
      }
    }
    else
    {
      bKeyIdx = 0U;
      s_rxTail++;
      if (s_rxTail >= MCS_RX_RING_SIZE)
      {
        s_rxTail = 0U;
      }
    }

    lTimeoutCntr++;
  }

  return FALSE;
}

void MCSSetConInfoChanged(uint8_t fState)
{
  SMCSRuntime.SFlags.fConInfoChanged = fState;
}

static uint8_t MCSIsConInfoChanged(void)
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

static void MCSCheckConInfoChange(void)
{
  uint8_t requestedNetworkType;

  if (MCSIsConInfoChanged() == 0U)
  {
    return;
  }

  MCSSetConInfoChanged(FALSE);

  if (MCSReadConInfo() == FALSE)
  {
    return;
  }

  requestedNetworkType = SMCSConfInfo.bNetworkType;
  MCSResetTransportState(FALSE);

  if (requestedNetworkType != s_activeNetworkType)
  {
    s_activeNetworkType = (uint8_t) MCS_NETWORK_TYPE_NONE;
    MCSApplyPowerPolicy();
    MCSJobAdd("NETWORK REBOOT");
    return;
  }

  MCSApplyPowerPolicy();
  if (s_activeNetworkType == (uint8_t) MCS_NETWORK_TYPE_ETHERNET)
  {
    LwIPConfigInterface();
  }
}

static void MCSUpdateRuntimeFromInfo(void)
{
  if (s_modemInfo.strIMEI[0] != '\0')
  {
    CopyAscii(&SMCSRuntime.strIMEI[0],
              sizeof(SMCSRuntime.strIMEI),
              &s_modemInfo.strIMEI[0]);
    ClearAscii(&s_modemInfo.strIMEI[0], sizeof(s_modemInfo.strIMEI));
  }

  if (s_modemInfo.strOperator[0] != '\0')
  {
    CopyAscii(&SMCSRuntime.strGsmOperator[0],
              sizeof(SMCSRuntime.strGsmOperator),
              &s_modemInfo.strOperator[0]);
    ClearAscii(&s_modemInfo.strOperator[0], sizeof(s_modemInfo.strOperator));
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
    MCSJobAdd(&s_modemInfo.strJobLabel[0]);
    ClearAscii(&s_modemInfo.strJobLabel[0], sizeof(s_modemInfo.strJobLabel));
  }

  if (s_modemInfo.strMAC[0] != '\0')
  {
    CopyAscii(&SMCSRuntime.strMAC[0],
              sizeof(SMCSRuntime.strMAC),
              &s_modemInfo.strMAC[0]);
    ClearAscii(&s_modemInfo.strMAC[0], sizeof(s_modemInfo.strMAC));
  }

  if (s_modemInfo.bLocalIPv4Valid != 0U)
  {
    MCSSetRuntimeLocalIPv4(&s_modemInfo.strLocalIPv4[0]);
    MCSJobAdd(MCSGetRuntimeLocalIPv4());
    s_modemInfo.bLocalIPv4Valid = 0U;
  }
}

uint8_t MCSSNMPClientStart(void)
{
  const char *destination;
  const char *resolved;

  if (ServerSettingsNTCIPAvailableGet() == FALSE)
  {
    if (SNMPClientIsStarted())
    {
      SNMPClientStop();
    }
    MCSSetSnmpReady(FALSE);
    return TRUE;
  }

  destination = MCSGetSNMPTrapDestination();
  MCSClearResolvedManagerIp();

  if (!MCSIPv4AddrAtoN(destination, &SMCSRuntime.SResolvedIPv4))
  {
    if (!MCSResolveDNSString(destination, &SMCSRuntime.SResolvedIPv4))
    {
      MCSJobAdd("SNMP DNS ERR");
      MCSSetSnmpReady(FALSE);
      return FALSE;
    }
  }

  resolved = ipaddr_ntoa(&SMCSRuntime.SResolvedIPv4);
  MCSSetRuntimeManagerIPv4(resolved);
  SMCSRuntime.SFlags.fDnsResolved = TRUE;

  if (!SNMPClientIsStarted())
  {
    if (!SNMPClientStart(&SMCSRuntime.SResolvedIPv4))
    {
      MCSJobAdd("SNMP START ERR");
      MCSSetSnmpReady(FALSE);
      return FALSE;
    }

    MCSJobAdd("SNMP STARTED");
    if (MCSGetColdStarted() != FALSE)
    {
      osDelay(1000U);
      SNMPSendColdStartTrap();
      MCSSetColdStarted(FALSE);
    }
  }

  MCSSetSnmpReady(TRUE);
  return TRUE;
}

static void MCSProcessSnmpState(void)
{
  if (ServerSettingsNTCIPAvailableGet() == FALSE)
  {
    if (SNMPClientIsStarted())
    {
      SNMPClientStop();
    }

    MCSSetSnmpReady(FALSE);
    SMCSRuntime.bSnmpRetryCountdown = 0U;
    return;
  }

  if (SNMPClientIsStarted())
  {
    MCSSetSnmpReady(TRUE);
    return;
  }

  if (SMCSRuntime.bSnmpRetryCountdown > 0U)
  {
    SMCSRuntime.bSnmpRetryCountdown--;
    return;
  }

  if (MCSSNMPClientStart() == FALSE)
  {
    SMCSRuntime.bSnmpRetryCountdown = MCS_SNMP_RETRY_DELAY_SECONDS;
  }
}

static void MCSProcessTransportStateMachine(void)
{
  char cmd[MCS_DATA_PACKET_MAX_SIZE + 1U];
  const char *keyword = "OK";
  char stopChar = '\0';
  uint32_t timeoutMs = MCS_DEFAULT_AT_CMD_RECEIVE_TIMEOUT;
  uint8_t maxRetries = MCS_MAX_COMMAND_TRY;

  if ((s_driver == NULL) || (s_activeNetworkType == (uint8_t) MCS_NETWORK_TYPE_NONE))
  {
    return;
  }

  MCSInitDefaultATParams();
  (void) memset(&s_modemInfo, 0, sizeof(s_modemInfo));
  (void) memset(cmd, 0, sizeof(cmd));

  ModemGetWaitParams(s_driver,
                     SMCSRuntime.bState,
                     &keyword,
                     &stopChar,
                     &timeoutMs,
                     &maxRetries);

  if (ModemPrepareCommand(s_driver,
                          SMCSRuntime.bState,
                          GetDeviceInfoAPNName(),
                          MCSGetSNMPTrapDestination(),
                          0U,
                          cmd,
                          (uint16_t) sizeof(cmd)) != 0U)
  {
    CopyAscii(strMCSTransmit, sizeof(strMCSTransmit), cmd);
    MCSTransmitMessage();
  }

  MCSJobAdd((char *) ModemGetStateLabel(s_driver, SMCSRuntime.bState));

  if (MCSWaitKey(keyword, strMCSResponse, stopChar, timeoutMs) != FALSE)
  {
    SMCSRuntime.bATCmdTries = 0U;
    SMCSRuntime.bState =
      ModemHandleResponse(s_driver,
                          SMCSRuntime.bState,
                          strMCSResponse,
                          TRUE,
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
                            FALSE,
                            &s_modemInfo);
    }
    else if (SMCSRuntime.SFlags.fNoResponse != FALSE)
    {
      MCSJobAdd(MCS_MODEM_NO_RESPONSE);
    }
  }

  MCSUpdateRuntimeFromInfo();

  if (SMCSRuntime.bState == MODEM_STATE_CONNECTED)
  {
    MCSSetConnected(ModemIsTransportReady(s_driver));
    SMCSRuntime.bTerminalFailureCount = 0U;
    SMCSRuntime.bSnmpRetryCountdown = 0U;
    MCSProcessSnmpState();
  }
  else if (SMCSRuntime.bState == MODEM_STATE_FAILED)
  {
    uint8_t hardReset = FALSE;

    if (s_activeNetworkType == (uint8_t) MCS_NETWORK_TYPE_QUECTEL)
    {
      SMCSRuntime.bTerminalFailureCount++;
      if (SMCSRuntime.bTerminalFailureCount >= MCS_QUECTEL_HARD_RESET_FAILURES)
      {
        hardReset = TRUE;
        SMCSRuntime.bTerminalFailureCount = 0U;
        MCSJobAdd("MODEM HARD RESET");
      }
    }

    MCSResetTransportState(hardReset);
  }
}

void MCSTaskFunc(void const *pArg)
{
  LWIP_UNUSED_ARG(pArg);

  MCSInit(&g_modemPort, &g_modemDriverPort);

  while (FOREVER)
  {
    MCSCheckConInfoChange();

    if (s_activeNetworkType == (uint8_t) MCS_NETWORK_TYPE_NONE)
    {
      if (SNMPClientIsStarted())
      {
        SNMPClientStop();
      }

      MCSSetConnected(FALSE);
      MCSSetSnmpReady(FALSE);
      osDelay(1000U);
      continue;
    }

    if ((s_driver != NULL) && (s_driver->OnMaintain != NULL))
    {
      ModemOnMaintain(s_driver);
    }

    if (MCSGetConnected() == FALSE)
    {
      MCSProcessTransportStateMachine();
    }
    else if ((s_driver != NULL) && (ModemIsTransportHealthy(s_driver) != FALSE))
    {
      MCSProcessSnmpState();
    }
    else
    {
      MCSJobAdd("TRANSPORT DOWN");
      MCSResetTransportState(FALSE);
    }

    osDelay(1000U);
  }
}
