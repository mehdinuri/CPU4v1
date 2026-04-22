/*
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Dirk Ziegelmeier <dziegel@gmx.de>
 *
 */
#include "lwip/tcpip.h"
#include "lwip/apps/snmp.h"
#include "lwip/apps/snmp_mib2.h"
#include "snmp_client.h"
#include "snmpv3_client.h"
#include "lwip/udp.h"
#include "lwip/apps/snmp.h"
#include "lwip/apps/snmp_mib2.h"
#include "lwip/apps/snmpv3.h"
#include "lwip/apps/snmp_snmpv2_framework.h"
#include "lwip/apps/snmp_snmpv2_usm.h"
#include "snmp_msg.h"

#include "string.h"

#include "main.h"
#include "MCS.h"
#include "DomainServices.h"
#include "HardwarePorts.h"
#include "Adapters/STM32/LWIPSNMPAdapter.h"
#include "Adapters/STM32/LWIPSNMPBridge.h"
#include "Adapters/STM32/LWIPSNMPRootMibs.h"
#include "lwip/sys.h"
#include <string.h>

#define SNMP_TEKNOTEL_ENTERPRISE_OID 59748U
#define TEKNOTEL_POWER_DOWN_SPECIFIC_TRAP 1U
#define TEKNOTEL_DRIVER_MODULE_MISSING_SPECIFIC_TRAP 2U
#define TEKNOTEL_DOOR_OPEN_SPECIFIC_TRAP 3U

#define TEKNOTEL_DEVICE_ENTERPRISE_OID                                         \
        { 1U, 3U, 6U, 1U, 4U, 1U, SNMP_TEKNOTEL_ENTERPRISE_OID }
#define TEKNOTEL_DEVICE_ENTERPRISE_OID_LEN 7U
static struct snmp_obj_id SDeviceEnterpriseOID =
{ TEKNOTEL_DEVICE_ENTERPRISE_OID_LEN, TEKNOTEL_DEVICE_ENTERPRISE_OID };

static const u32_t kDriverModuleStatusTrapOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U,
  SNMP_TEKNOTEL_ENTERPRISE_OID, 4U, 2U, 1U, 21U, 1U
};
static const u32_t kNtcipTrapEnterpriseOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 4U
};
static const u32_t kNtcipTrapDataOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 4U, 1U, 2U
};
static struct snmp_obj_id s_ntcipTrapEnterpriseObjId =
{
  10U, { 0U }
};

static tESNMPClientState eClientState = SNMP_CLIENT_NONE;
static LWIPSNMPAdapterCtx_t s_lwipSnmpAdapterCtx;
static char s_trapCommunityName[17];

static const EventReportCommunityRow_t *FindTrapCommunityRow(
  const EventReportConfiguration_t *config,
  uint8_t communityIndex)
{
  uint8_t index;

  if ((config == NULL) || (communityIndex == 0U))
  {
    return NULL;
  }

  for (index = 0U; index < EVENT_REPORT_COMMUNITY_NAMES_MAX; index++)
  {
    if (config->communityRows[index].communityNameIndex == communityIndex)
    {
      return &config->communityRows[index];
    }
  }

  return NULL;
}

static uint8_t TryGetTrapDestination(ip_addr_t *address)
{
  const EventReportConfiguration_t *config;
  const EventReportLogicalNameRow_t *logicalNameRow;
  const EventReportTrapMgmtRow_t *trapMgmtRow;
  uint8_t index;

  if (address == NULL)
  {
    return FALSE;
  }

  config = EventReportServiceGetActiveConfig(&g_eventReportService);
  if (config == NULL)
  {
    return FALSE;
  }

  trapMgmtRow = &config->trapMgmtRows[0];
  logicalNameRow = &config->logicalNameRows[0];
  if ((trapMgmtRow->trapMgmtRowStatus != EVENT_REPORT_ROW_STATUS_ACTIVE)
      || (logicalNameRow->status != EVENT_REPORT_ROW_STATUS_ACTIVE)
      || (trapMgmtRow->trapMgmtManagerPointer
          != logicalNameRow->logicalNameTranslationIndex))
  {
    return FALSE;
  }

  for (index = 0U; index < 4U; index++)
  {
    if (logicalNameRow->networkAddress[index] != 0U)
    {
      IP_ADDR4(address,
               logicalNameRow->networkAddress[0],
               logicalNameRow->networkAddress[1],
               logicalNameRow->networkAddress[2],
               logicalNameRow->networkAddress[3]);
      return TRUE;
    }
  }

  return FALSE;
}

static void ConfigureTrapReporting(ip_addr_t *fallbackTargetIp)
{
  const EventReportConfiguration_t *config;
  const EventReportTrapMgmtRow_t *trapMgmtRow;
  const EventReportCommunityRow_t *communityRow;
  ip_addr_t targetIp;
  uint8_t targetValid = FALSE;

  config = EventReportServiceGetActiveConfig(&g_eventReportService);
  trapMgmtRow = (config == NULL) ? NULL : &config->trapMgmtRows[0];
  communityRow = (config == NULL) ? NULL
                 : FindTrapCommunityRow(config,
                                        config->trapMgmtRows[0]
                                        .trapMgmtCommunityNamePointer);

  if ((communityRow != NULL) && (communityRow->communityNameLength > 0U))
  {
    (void) memset(&s_trapCommunityName[0], 0, sizeof(s_trapCommunityName));
    (void) memcpy(&s_trapCommunityName[0],
                  &communityRow->communityNameUser[0],
                  communityRow->communityNameLength);
    snmp_set_community_trap(&s_trapCommunityName[0]);
  }
  else
  {
    snmp_set_community_trap(MCSGetSNMPTrapCommunityName());
  }

  if (TryGetTrapDestination(&targetIp) != FALSE)
  {
    targetValid = TRUE;
  }
  else if (fallbackTargetIp != NULL)
  {
    targetIp = *fallbackTargetIp;
    targetValid = TRUE;
  }

  if ((trapMgmtRow != NULL)
      && (trapMgmtRow->trapMgmtRowStatus == EVENT_REPORT_ROW_STATUS_ACTIVE)
      && (targetValid != FALSE))
  {
    snmp_trap_dst_ip_set(0, &targetIp);
    snmp_trap_dst_enable(0, 1);
  }
  else
  {
    snmp_trap_dst_enable(0, 0);
  }
}

static void BindDomainSnmpAdapter(void)
{
  LWIPSNMPAdapterInit(&s_lwipSnmpAdapterCtx,
                      &g_configurationService,
                      &g_intersectionEngine,
                      &g_intersectionController);
  EventReportServiceApplySnmpCommunities(&g_eventReportService,
                                         MCSGetSNMPReadCommunityName(),
                                         MCSGetSNMPWriteCommunityName(),
                                         MCSGetSNMPTrapCommunityName());
  LWIPSNMPAdapterBindDetectorReportService(&s_lwipSnmpAdapterCtx,
                                           &g_detectorReportService);
  LWIPSNMPAdapterBindGlobalTimeManagementService(&s_lwipSnmpAdapterCtx,
                                                 &g_globalTimeManagementService);
  LWIPSNMPAdapterBindEventReportService(&s_lwipSnmpAdapterCtx,
                                        &g_eventReportService);
  LWIPSNMPAdapterBindSnmpSecurityPort(&s_lwipSnmpAdapterCtx,
                                      &g_snmpSecurityPort);
  LWIPSNMPAdapterBindActivationService(&s_lwipSnmpAdapterCtx,
                                       &g_intersectionActivationService);
  LWIPSNMPAdapterBindDoorSensorPort(&s_lwipSnmpAdapterCtx, &g_doorPort);
  LWIPSNMPAdapterBindHeaterPort(&s_lwipSnmpAdapterCtx, &g_heaterPort);
  LWIPSNMPAdapterBindPowerMonitorPort(&s_lwipSnmpAdapterCtx,
                                      &g_powerMonitorPort);
  LWIPSNMPAdapterBindUnitAlarmPort(&s_lwipSnmpAdapterCtx, &g_unitAlarmPort);
  LWIPSNMPAdapterBindUnitClockPort(&s_lwipSnmpAdapterCtx, &g_unitClockPort);
  LWIPSNMPAdapterBindCpMpLinkService(&s_lwipSnmpAdapterCtx, &g_cpMpLinkService);
  EventReportServiceBindObjectDirectory(&g_eventReportService,
                                        &s_lwipSnmpAdapterCtx.objectDirectory);
  LWIPSNMPBridgeBindAdapter(&s_lwipSnmpAdapterCtx);
}

static const struct snmp_mib *SaMibs[] =
{
  &mib2,
  &ntcip1201mib,
  &ntcip1202mib,
  &ntcip1103applicationmib,
  &ntcip1103trapmib,
  &teknotelmib,
  #if LWIP_SNMP_V3
  &snmpframeworkmib,
  &snmpusmmib
  #endif
};

void SNMPClientSetState(tESNMPClientState eState)
{
  eClientState = eState;
}

tESNMPClientState SNMPClientGetState(void)
{
  return eClientState;
}

uint8_t SNMPClientIsStarted(void)
{
  return eClientState == SNMP_CLIENT_STARTED;
}

void SNMPClientBindDomainServices(void)
{
  BindDomainSnmpAdapter();
}

uint8_t SNMPClientStart(ip_addr_t *pSTargetIp)
{
  SNMPClientStop();
  SNMPClientBindDomainServices();

  if (MCSCurrentSnmpProfileValid() == FALSE)
  {
    return FALSE;
  }

  SNMPv3Init();

  snmp_set_device_enterprise_oid(&SDeviceEnterpriseOID);
  snmp_set_community(MCSGetSNMPReadCommunityName());
  snmp_set_community_write(MCSGetSNMPWriteCommunityName());
  snmp_set_mibs(SaMibs, LWIP_ARRAYSIZE(SaMibs));

  snmp_v1_enable(0);
  snmp_v2c_enable(1);
  snmp_v3_enable(1);

  snmp_set_default_trap_version(MCSGetSNMPTrapVersion());
  ConfigureTrapReporting(pSTargetIp);
  snmp_set_auth_traps_enabled(1);
  MEMCPY(s_ntcipTrapEnterpriseObjId.id,
         kNtcipTrapEnterpriseOid,
         sizeof(kNtcipTrapEnterpriseOid));

  snmp_init();

  SNMPClientSetState(SNMP_CLIENT_STARTED);

  return snmp_traps_handle != NULL;
}

void SNMPClientStop(void)
{
  if (snmp_traps_handle != NULL)
  {
    udp_remove(snmp_traps_handle);
    SNMPClientSetState(SNMP_CLIENT_NONE);
  }
}

void SNMPSendColdStartTrap(void)
{
  if (SNMPClientIsStarted())
  {
    snmp_coldstart_trap();
  }
}

void SNMPClientProcessEventReportTrap(void)
{
  NtcipOctetString_t trapData;

  if (!SNMPClientIsStarted()
      || (EventReportServiceCopyPendingTrap(&g_eventReportService, &trapData)
          == 0U))
  {
    return;
  }

  ConfigureTrapReporting(NULL);
  if (trapData.length > 0U)
  {
    struct snmp_obj_id trapDataOid = { 12U, { 0U } };
    struct snmp_varbind trapVarBind;

    MEMCPY(trapDataOid.id, kNtcipTrapDataOid, sizeof(kNtcipTrapDataOid));
    trapVarBind.next = NULL;
    trapVarBind.oid = trapDataOid;
    trapVarBind.type = SNMP_ASN1_TYPE_OCTET_STRING;
    trapVarBind.value_len = trapData.length;
    trapVarBind.value = &trapData.bytes[0];

    EventReportServiceAcknowledgeTrapDispatch(
      &g_eventReportService,
      (uint8_t) (snmp_send_trap(&s_ntcipTrapEnterpriseObjId,
                                SNMP_GENTRAP_ENTERPRISE_SPECIFIC,
                                1,
                                &trapVarBind) == ERR_OK));
  }
}

uint8_t SNMPSendPowerDownTrap(void)
{
  if (SNMPClientIsStarted())
  {
    return snmp_send_trap_specific(TEKNOTEL_POWER_DOWN_SPECIFIC_TRAP,
                                   NULL) == ERR_OK;
  }

  return FALSE;
}

void SNMPSendSSMStatusTrap(uint16_t sSSMStatus)
{
  if (SNMPClientIsStarted())
  {
    struct snmp_obj_id SStausOID = { 10, { 0 } };
    struct snmp_varbind SStatusVarBind;
    s32_t lStatus = (s32_t) sSSMStatus;

    MEMCPY(SStausOID.id,
           kDriverModuleStatusTrapOid,
           sizeof(kDriverModuleStatusTrapOid));
    SStatusVarBind.next = NULL;
    SStatusVarBind.oid = SStausOID;
    SStatusVarBind.type = SNMP_ASN1_TYPE_INTEGER;
    SStatusVarBind.value_len = sizeof(s32_t);
    SStatusVarBind.value = &lStatus;

    snmp_send_trap_specific(TEKNOTEL_DRIVER_MODULE_MISSING_SPECIFIC_TRAP,
                            &SStatusVarBind);
  }
}

void SNMPSendDoorOpenTrap(void)
{
  if (SNMPClientIsStarted())
  {
    snmp_send_trap_specific(TEKNOTEL_DOOR_OPEN_SPECIFIC_TRAP, NULL);
  }
}
