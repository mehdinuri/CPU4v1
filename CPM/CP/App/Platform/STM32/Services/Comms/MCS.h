/**
 ******************************************************************************
 * @file    MCS.h
 * @author  Okan KILIC - Teknotel Electronics
 * @version V1.0.0
 * @date    07/19/2011
 * @brief   Maestro connection supervisor public API
 ******************************************************************************
 */

#ifndef __MCS_H__
#define __MCS_H__

#include "lwip/ip_addr.h"
#include "Ports/ISerialPort.h"
#include "Ports/IModemPort.h"

#include <stdint.h>

#define MCS_DATA_PACKET_MAX_LEN 1024U
#define MCS_DATA_PACKET_MAX_SIZE 255U
#define MCS_SNMP_MAX_COMMUNITY_STR_LEN 32U

typedef enum
{
  MCS_NETWORK_TYPE_NONE = 0,
  MCS_NETWORK_TYPE_QUECTEL = 1,
  MCS_NETWORK_TYPE_ETHERNET = 2,
  MCS_NETWORK_TYPE_MAX
} tEMCSNetworkTypes;

#define MCS_MAX_IPV4_LEN 15U
#define MCS_MAX_IMEI_LEN 15U
#define MCS_MAX_MAC_LEN 12U
#define MCS_MAX_GSM_OPERATOR_LEN 20U

#define MCS_JOB_LIVE_RECORD_MAX 2U
#define MCS_JOB_LIVE_RECORD_SIZE_MAX 20U

#define MCS_CON_INFO_INITIALIZED 0xAAU

#define SNMP_COMMUNITY_NAME_MIN_SIZE 1U
#define SNMPV3_ENGINE_ID_MIN_SIZE 1U
#define SNMPV3_ENGINE_ID_MAX_SIZE 32U
#define SNMPV3_USERNAME_MIN_SIZE 1U
#define SNMPV3_USERNAME_MAX_SIZE 32U
#define SNMPV3_PASSWORD_MAX_SIZE 20U
#define SNMPV3_PASSWORD_MIN_SIZE 8U
#define SNMP_TRAP_DESTINATION_MAX_SIZE 64U

#define MCS_DEFAULT_ETH_MAC_ADDRESS_0 ((uint8_t) 0x02U)
#define MCS_DEFAULT_ETH_MAC_ADDRESS_5 ((uint8_t) 0xFEU)

#define MCS_DEFAULT_ETH_STATIC_IPV4_0 ((uint8_t) 192U)
#define MCS_DEFAULT_ETH_STATIC_IPV4_1 ((uint8_t) 168U)
#define MCS_DEFAULT_ETH_STATIC_IPV4_2 ((uint8_t) 10U)
#define MCS_DEFAULT_ETH_STATIC_IPV4_3 ((uint8_t) 222U)

#define MCS_DEFAULT_ETH_STATIC_SUBNET_MASK_0 ((uint8_t) 255U)
#define MCS_DEFAULT_ETH_STATIC_SUBNET_MASK_1 ((uint8_t) 255U)
#define MCS_DEFAULT_ETH_STATIC_SUBNET_MASK_2 ((uint8_t) 255U)
#define MCS_DEFAULT_ETH_STATIC_SUBNET_MASK_3 ((uint8_t) 0U)

#define MCS_DEFAULT_ETH_STATIC_GATEWAY_0 ((uint8_t) 192U)
#define MCS_DEFAULT_ETH_STATIC_GATEWAY_1 ((uint8_t) 168U)
#define MCS_DEFAULT_ETH_STATIC_GATEWAY_2 ((uint8_t) 10U)
#define MCS_DEFAULT_ETH_STATIC_GATEWAY_3 ((uint8_t) 5U)

#define MCS_DEFAULT_PRIMARY_DNS_SERVER_IPV4_0 ((uint8_t) 8U)
#define MCS_DEFAULT_PRIMARY_DNS_SERVER_IPV4_1 ((uint8_t) 8U)
#define MCS_DEFAULT_PRIMARY_DNS_SERVER_IPV4_2 ((uint8_t) 8U)
#define MCS_DEFAULT_PRIMARY_DNS_SERVER_IPV4_3 ((uint8_t) 8U)

#define MCS_DEFAULT_SECONDARY_DNS_SERVER_IPV4_0 ((uint8_t) 8U)
#define MCS_DEFAULT_SECONDARY_DNS_SERVER_IPV4_1 ((uint8_t) 8U)
#define MCS_DEFAULT_SECONDARY_DNS_SERVER_IPV4_2 ((uint8_t) 4U)
#define MCS_DEFAULT_SECONDARY_DNS_SERVER_IPV4_3 ((uint8_t) 4U)

#define MCS_DEFAULT_SNMP_TRAP_DESTINATION "192.168.10.226"
#define MCS_DEFAULT_SNMP_TRAP_VERSION 0U

#define MCS_DEFAULT_SNMP_READ_COMMUNITY_NAME "public"
#define MCS_DEFAULT_SNMP_WRITE_COMMUNITY_NAME "private"
#define MCS_DEFAULT_SNMP_TRAP_COMMUNITY_NAME "public"

#define MCS_DEFAULT_SNMPV3_USERNAME "lwip-agent"
#define MCS_DEFAULT_SNMPV3_PASSWORD "agent@maestro"

#define MCS_FNV_OFFSET_BASIS ((uint32_t) 2166136261UL)
#define MCS_FNV_PRIME ((uint32_t) 16777619UL)

typedef struct _tSMCSRuntime
{
  uint8_t bState;
  uint8_t bATCmdTries;
  uint8_t bSignalQuality;
  uint8_t bSnmpRetryCountdown;
  uint8_t bTerminalFailureCount;

  char strIMEI[MCS_MAX_IMEI_LEN + 1U];
  char strMAC[MCS_MAX_MAC_LEN + 1U];
  char strLocalIPv4[MCS_MAX_IPV4_LEN + 1U];
  char strManagerIPv4[MCS_MAX_IPV4_LEN + 1U];
  char strGsmOperator[MCS_MAX_GSM_OPERATOR_LEN + 1U];
  ip_addr_t SResolvedIPv4;

  struct
  {
    uint8_t fNoResponse : 1;
    uint8_t fConnected : 1;
    uint8_t fConInfoChanged : 1;
    uint8_t fModemAlive : 1;
    uint8_t fDnsResolved : 1;
    uint8_t fSimStatus : 1;
    uint8_t fSnmpReady : 1;
    uint8_t fReserved : 1;
  } SFlags;
} tSMCSRuntime, *tpSMCSRuntime;

typedef struct _tSMCSIPv4
{
  uint8_t bAddress0;
  uint8_t bAddress1;
  uint8_t bAddress2;
  uint8_t bAddress3;
} __attribute__((packed)) tSMCSIPv4, *tpSMCSIPv4;

typedef struct _tSMCSMACAddress
{
  uint8_t bAddress0;
  uint8_t bAddress1;
  uint8_t bAddress2;
  uint8_t bAddress3;
  uint8_t bAddress4;
  uint8_t bAddress5;
} __attribute__((packed)) tSMCSMACAddress, *tpSMCSMACAddress;

typedef struct _tSMCSSNMPInfo
{
  uint32_t lDeviceID;
  uint8_t bTrapVersion;

  char strReadCommunityName[MCS_SNMP_MAX_COMMUNITY_STR_LEN + 1U];
  char strWriteCommunityName[MCS_SNMP_MAX_COMMUNITY_STR_LEN + 1U];
  char strTrapCommunityName[MCS_SNMP_MAX_COMMUNITY_STR_LEN + 1U];
  char strV3Username[SNMPV3_USERNAME_MAX_SIZE + 1U];
  char strTrapDestination[SNMP_TRAP_DESTINATION_MAX_SIZE + 1U];
} __attribute__((packed)) tSMCSSNMPInfo, *tpSMCSSNMPInfo;

typedef struct _tSMCSSNMPv3State
{
  uint32_t lEngineBoots;
  uint8_t bAuthConfigured;
  uint8_t bPrivConfigured;
  uint8_t bAuthAlgo;
  uint8_t bPrivAlgo;
  char strEngineId[SNMPV3_ENGINE_ID_MAX_SIZE + 1U];
  uint8_t baAuthKey[SNMPV3_PASSWORD_MAX_SIZE];
  uint8_t baPrivKey[SNMPV3_PASSWORD_MAX_SIZE];
} __attribute__((packed)) tSMCSSNMPv3State, *tpSMCSSNMPv3State;

typedef struct _tSMCSConInfo
{
  uint8_t bInitialized;
  uint8_t bNetworkType;

  struct
  {
    uint8_t fEthStaticIp : 1;
    uint8_t fReserved : 7;
  } __attribute__((packed)) SFlags;

  tSMCSMACAddress SMACAddress;
  tSMCSIPv4 SEthLocalIPv4;
  tSMCSIPv4 SEthSubnetMask;
  tSMCSIPv4 SEthGateway;
  tSMCSIPv4 SPrimaryDNSServer;
  tSMCSIPv4 SSecondaryDNSServer;
  tSMCSSNMPInfo SSNMPInfo;
} __attribute__((packed)) tSMCSConInfo, *tpSMCSConInfo;

extern uint8_t MCSGetGPRSState(void);
extern void MCSSetGPRSState(uint8_t bState);
extern void MCSSetModemType(uint8_t bModemType);
extern uint8_t MCSGetModemType(void);
extern uint8_t MCSGetGprsSignalQuality(void);
extern char *MCSGetGprsModemIMEI(void);
extern char *MCSGetGprsGsmOperator(void);
extern void MCSSetRuntimeLocalIPv4(const char *strIp);
extern char *MCSGetRuntimeLocalIPv4(void);
extern void MCSSetRuntimeManagerIPv4(const char *strIp);
extern char *MCSGetRuntimeManagerIPv4(void);
extern char *MCSGetRuntimeEthernetMAC(void);
extern void MCSSetConnected(uint8_t bState);
extern uint8_t MCSGetConnected(void);
extern void MCSSetSnmpReady(uint8_t bState);
extern uint8_t MCSGetSnmpReady(void);
extern void MCSSetConInfoChanged(uint8_t fState);
extern void MCSSetModemAlive(uint8_t bState);
extern uint8_t MCSGetModemAlive(void);
extern void MCSSimStatusSet(uint8_t fStatus);
extern uint8_t MCSSimStatusGet(void);
extern void MCSBatteryPowerOff(void);
extern uint8_t MCSWriteConInfo(void);
extern uint8_t MCSReadConInfo(void);
extern uint8_t MCSWriteSNMPv3State(void);
extern uint8_t MCSReadSNMPv3State(void);
extern void MCSSetConInfo(tpSMCSConInfo pSInfo);
extern void MCSGetConInfo(tpSMCSConInfo pSInfo);
extern tpSMCSConInfo MCSGetConInfoPtr(void);
extern const tSMCSSNMPv3State *MCSGetSNMPv3State(void);
extern tpSMCSMACAddress MCSGetEthernetMACAddress(void);
extern tpSMCSIPv4 MCSGetLocalIPv4(void);
extern tpSMCSIPv4 MCSGetSubnetMask(void);
extern tpSMCSIPv4 MCSGetGateway(void);
extern tpSMCSIPv4 MCSGetPrimaryDNSServer(void);
extern tpSMCSIPv4 MCSGetSecondaryDNSServer(void);
extern uint8_t MCSIsEthernetStaticIP(void);
extern uint32_t MCSGetSNMPDeviceID(void);
extern uint8_t MCSGetSNMPTrapVersion(void);
extern char *MCSGetSNMPReadCommunityName(void);
extern char *MCSGetSNMPWriteCommunityName(void);
extern char *MCSGetSNMPTrapCommunityName(void);
extern char *MCSGetSNMPv3EngineID(void);
extern char *MCSGetSNMPv3Username(void);
extern char *MCSGetSNMPTrapDestination(void);
extern uint8_t MCSSetSNMPv3Username(const char *username);
extern uint8_t MCSSetSNMPv3Credentials(const char *username,
                                       const char *authPassphrase,
                                       const char *privPassphrase);
extern void MCSInitConInfo(void);
extern void MCSInitSNMPv3State(void);
extern uint8_t MCSIsConInitialized(void);
extern void MCSRuntimeInit(void);
extern void MCSRingBufferReset(void);
extern void MCSInit(ISerialPort_t *port, IModemPort_t *driver);
extern void MCSJobAdd(char *pStrNewJob);
extern uint8_t MCSJobCurrentGet(char *pStrBuffer, uint8_t bShift);

extern void MCSRingBufferWrite(const uint8_t *data, uint16_t len);
extern uint8_t MCSSNMPClientStart(void);

#endif /* ifndef __MCS_H__ */
