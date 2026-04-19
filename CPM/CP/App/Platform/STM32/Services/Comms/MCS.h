/**
 ******************************************************************************
 * @file    MCS.h
 * @author  Okan KILIC - Teknotel Electronics
 * @version V1.0.0
 * @date    07/19/2011
 * @brief  Maestro Central System
 *       This file explains all the definitions and data
 * structures of MCS
 ******************************************************************************
 */

#ifndef __MCS_H__
#define __MCS_H__

/* /////////////////////////// */
/*  includes */
#include "lwip/ip_addr.h"
#include "Ports/ISerialPort.h"
#include "Ports/IModemPort.h"

/* /////////////////////////// */
/*  definitions */
#define MCS_DATA_PACKET_MAX_LEN 1024
#define MCS_DATA_PACKET_MAX_SIZE 255
#define MCS_SNMP_MAX_COMMUNITY_STR_LEN 32

typedef enum
{
  MCS_MODULE_TYPE_UBLOX = 0,
  MCS_MODULE_TYPE_TELIT,
  MCS_MODULE_TYPE_USR,
  MCS_MODULE_TYPE_NONE,
  MCS_MODULE_TYPE_QUECTEL,
  MCS_MODULE_TYPE_ETH_NTCIP,
  MCS_MODULE_TYPE_QUECTEL_NTCIP,
  MCS_MODULE_TYPE_MAX,
} tEMCSModuleTypes;

#define MCS_MAX_IPV4_LEN 15
#define MCS_MAX_IPV4_PARTS 4
#define MCS_MAX_IMEI_LEN 15
#define MCS_MAX_MAC_LEN 12
#define MCS_MAX_MAC_PARTS 6
#define MCS_MAX_GSM_OPERATOR_LEN 20

/*
 *       name: job live buffer boundary values
 *       expl: we have a buffer called as job live. In this buffer, jobs of MCS
 *  is stored in this circular buffer, last job is pointed by a index pointer. In
 *  the following, you can see size of this buffer and its elements which are
 *  string. The string size is important for the caller. Caller module must
 *  allocate a string in size of this buffer elements to be able to get job
 *               successfully. For the present, LCD interface use this buffer to
 *  inform user of gprs job history.
 */
#define MCS_JOB_LIVE_RECORD_MAX             2
#define MCS_JOB_LIVE_RECORD_SIZE_MAX        20

#define MCS_CON_INFO_INITIALIZED 0xAA
#define IMEI_MAX_SIZE 15
#define MAC_MAX_SIZE 17
#define IPV4_MAX_SIZE 15
#define DOMAIN_MAX_SIZE 50
#define PORT_MAX_SIZE 5
#define APN_MAX_SIZE 50
#define GSM_OPERATOR_MAX_SIZE MCS_MAX_GSM_OPERATOR_LEN

#define SNMP_COMMUNITY_NAME_MIN_SIZE 1
#define SNMPV3_ENGINE_ID_MIN_SIZE 1
#define SNMPV3_ENGINE_ID_MAX_SIZE 32
#define SNMPV3_USERNAME_MIN_SIZE 1
#define SNMPV3_USERNAME_MAX_SIZE 32
#define SNMPV3_PASSWORD_MAX_SIZE 20
#define SNMPV3_PASSWORD_MIN_SIZE 8
#define SNMP_TRAP_DESTINATION_MAX_SIZE 64

#define MCS_DEFAULT_TCP_DESTINATION_PORT (uint16_t) (1234)

#define MCS_DEFAULT_ETH_MAC_ADDRESS_0 (uint8_t) (0x02) /* Local, Unicast */
#define MCS_DEFAULT_ETH_MAC_ADDRESS_5 (uint8_t) (0xFE)

#define MCS_DEFAULT_ETH_STATIC_IPV4_0 (uint8_t) (192)
#define MCS_DEFAULT_ETH_STATIC_IPV4_1 (uint8_t) (168)
#define MCS_DEFAULT_ETH_STATIC_IPV4_2 (uint8_t) (10)
#define MCS_DEFAULT_ETH_STATIC_IPV4_3 (uint8_t) (222)

#define MCS_DEFAULT_ETH_STATIC_SUBNET_MASK_0 (uint8_t) (255)
#define MCS_DEFAULT_ETH_STATIC_SUBNET_MASK_1 (uint8_t) (255)
#define MCS_DEFAULT_ETH_STATIC_SUBNET_MASK_2 (uint8_t) (255)
#define MCS_DEFAULT_ETH_STATIC_SUBNET_MASK_3 (uint8_t) (0)

#define MCS_DEFAULT_ETH_STATIC_GATEWAY_0 (uint8_t) (192)
#define MCS_DEFAULT_ETH_STATIC_GATEWAY_1 (uint8_t) (168)
#define MCS_DEFAULT_ETH_STATIC_GATEWAY_2 (uint8_t) (10)
#define MCS_DEFAULT_ETH_STATIC_GATEWAY_3 (uint8_t) (5)

#define MCS_DEFAULT_PRIMARY_DNS_SERVER_IPV4_0 (uint8_t) (8)
#define MCS_DEFAULT_PRIMARY_DNS_SERVER_IPV4_1 (uint8_t) (8)
#define MCS_DEFAULT_PRIMARY_DNS_SERVER_IPV4_2 (uint8_t) (8)
#define MCS_DEFAULT_PRIMARY_DNS_SERVER_IPV4_3 (uint8_t) (8)

#define MCS_DEFAULT_SECONDARY_DNS_SERVER_IPV4_0 (uint8_t) (8)
#define MCS_DEFAULT_SECONDARY_DNS_SERVER_IPV4_1 (uint8_t) (8)
#define MCS_DEFAULT_SECONDARY_DNS_SERVER_IPV4_2 (uint8_t) (4)
#define MCS_DEFAULT_SECONDARY_DNS_SERVER_IPV4_3 (uint8_t) (4)

#define MCS_DEFAULT_SNMP_TRAP_DESTINATION "192.168.10.226" /* v1 */
#define MCS_DEFAULT_SNMP_TRAP_VERSION 0 /* v1 */

#define MCS_DEFAULT_SNMP_READ_COMMUNITY_NAME "public"
#define MCS_DEFAULT_SNMP_WRITE_COMMUNITY_NAME "private"
#define MCS_DEFAULT_SNMP_TRAP_COMMUNITY_NAME "public"

#define MCS_DEFAULT_SNMPV3_ENGINE_ID "MAESTRO"
#define MCS_DEFAULT_SNMPV3_USERNAME "lwip-agent"
#define MCS_DEFAULT_SNMPV3_PASSWORD "agent@maestro"

#define MCS_FNV_OFFSET_BASIS (uint32_t) 2166136261
#define MCS_FNV_PRIME (uint32_t) 16777619

#define MCS_DNS_RESOLVE_MAX_TIMEOUT 30000

/*
 *       name: MCS State Packet
 *       expl: This structure was created for to watch the states of MCS
 *  connection.
 */
typedef struct _tSMCSRuntime
{
  uint8_t bState;
  uint8_t bATCmdTries;
  uint8_t bSignalQuality;
  uint8_t bProbeTimeoutCntr;
  uint8_t bProbeErrCntr;

  char strIMEI[MCS_MAX_IMEI_LEN + 1];
  char strMAC[MCS_MAX_MAC_LEN + 1];
  char strLocalIPv4[MCS_MAX_IPV4_LEN + 1];
  char strRemoteIPv4[MCS_MAX_IPV4_LEN + 1];
  char strGsmOperator[MCS_MAX_GSM_OPERATOR_LEN + 1];
  ip_addr_t SResolvedIPv4;

  struct
  {
    uint8_t fKeyReceived : 1;
    uint8_t fNoResponse : 1;
    uint8_t fConnected : 1;
    uint8_t fConInfoChanged : 1;
    uint8_t fModemAlive : 1;
    uint8_t fDnsResolved : 1;
    uint8_t fSimStatus : 1;
    uint8_t fReserved : 1;
  } SFlags;
} tSMCSRuntime, *tpSMCSRuntime;

typedef struct _tSMCSSNMPInfo
{
  uint32_t lDeviceID;
  uint8_t bTrapVersion;

  char strReadCommunityName[MCS_SNMP_MAX_COMMUNITY_STR_LEN];
  char strWriteCommunityName[MCS_SNMP_MAX_COMMUNITY_STR_LEN];
  char strTrapCommunityName[MCS_SNMP_MAX_COMMUNITY_STR_LEN];
  char strV3EngineId[SNMPV3_ENGINE_ID_MAX_SIZE];
  char strV3Username[SNMPV3_USERNAME_MAX_SIZE];
  char strV3AuthPassword[SNMPV3_PASSWORD_MAX_SIZE];
  char strV3PrivPassword[SNMPV3_PASSWORD_MAX_SIZE];
  char strTrapDestination[SNMP_TRAP_DESTINATION_MAX_SIZE];
} __attribute__((packed)) tSMCSSNMPInfo, *tpSMCSSNMPInfo;

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

typedef struct _tSMCSConInfo
{
  uint8_t bInitialized;
  uint8_t bModuleType;

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

/*
 *       name: Extern Functions
 */
extern uint8_t MCSGetGPRSState(void);
extern uint8_t MCSGetGprsSignalQuality(void);
extern void MCSSetModemType(uint8_t bModemType);
extern uint8_t MCSGetModemType(void);
extern char *MCSGetGprsModemIMEI(void);
extern char *MCSGetUSRModuleMAC(void);
extern char *MCSGetGprsGsmOperator(void);
extern void MCSSetRuntimeLocalIPv4(char *strIp);
extern void MCSSetRuntimeRemoteIPv4(char *strIp);
extern char *MCSGetRuntimeEthernetMAC(void);
extern void MCSSetConnected(uint8_t bState);
extern uint8_t MCSGetConnected(void);
extern void MCSSetConInfoChanged(uint8_t fState);
extern void MCSSetModemAlive(uint8_t bState);
extern uint8_t MCSGetModemAlive(void);
extern void MCSSimStatusSet(uint8_t fStatus);
extern uint8_t MCSSimStatusGet(void);
extern uint8_t MCSBatteryPowerOffRequestGet(void);
extern void MCSBatteryPowerOffRequestSet(uint8_t fState);
extern uint8_t MCSBatteryPowerOffResponseGet(void);
extern void MCSBatteryPowerOffResponseSet(uint8_t fState);
extern void MCSBatteryPowerOff(void);
extern uint8_t MCSWriteConInfo(void);
extern uint8_t MCSReadConInfo(void);
extern void MCSSetConInfo(tpSMCSConInfo pSInfo);
extern void MCSGetConInfo(tpSMCSConInfo pSInfo);
extern tpSMCSConInfo MCSGetConInfoPtr(void);
extern tpSMCSMACAddress MCSGetEthernetMACAddress(void);
extern tpSMCSIPv4 MCSGetLocalIPv4(void);
extern tpSMCSIPv4 MCSGetSubnetMask(void);
extern tpSMCSIPv4 MCSGetGateway(void);
extern tpSMCSIPv4 MCSGetPrimaryDNSServer(void);
extern tpSMCSIPv4 MCSGetSecondaryDNSServer(void);
extern char *MCSGetRuntimeLocalIPv4(void);
extern char *MCSGetRuntimeRemoteIPv4(void);
extern uint8_t MCSIsEthernetStaticIP(void);
extern uint32_t MCSGetSNMPDeviceID(void);
extern uint8_t MCSGetSNMPTrapVersion(void);
extern char *MCSGetSNMPReadCommunityName(void);
extern char *MCSGetSNMPWriteCommunityName(void);
extern char *MCSGetSNMPTrapCommunityName(void);
extern char *MCSGetSNMPv3EngineID(void);
extern char *MCSGetSNMPv3Username(void);
extern char *MCSGetSNMPv3AuthPassword(void);
extern char *MCSGetSNMPv3PrivPassword(void);
extern char *MCSGetSNMPTrapDestination(void);
extern void MCSInitConInfo(void);
extern uint8_t MCSIsConInitialized(void);
extern void MCSRuntimeInit(void);
extern void MCSRingBufferReset(void);
extern void MCSInit(ISerialPort_t *port, IModemPort_t *driver);
extern uint8_t MCSReadModuleType(void);
extern void MCSJobAdd(char *pStrNewJob);
extern uint8_t MCSJobCurrentGet(char *pStrBuffer, uint8_t bShift);

/* Helper functions for modem adapters */
extern void MCSRingBufferWrite(const uint8_t *data, uint16_t len);
extern uint8_t MCSSNMPClientStart(void);
extern uint8_t MCSUDPProbeStart(void);
extern void MCSUDPConnectionMaintain(void);
extern uint8_t MCSTCPClientConnect(void);
extern void MCSTryConnect(void);

#endif /* ifndef __MCS_H__ */
