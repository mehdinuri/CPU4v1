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
#include "data.h"
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
  1U, 3U, 6U, 1U, 4U, 1U, SNMP_TEKNOTEL_ENTERPRISE_OID, 1U, 11U, 1U
};

static tESNMPClientState eClientState = SNMP_CLIENT_NONE;
static LWIPSNMPAdapterCtx_t s_lwipSnmpAdapterCtx;

static void BindDomainSnmpAdapter(void)
{
  LWIPSNMPAdapterInit(&s_lwipSnmpAdapterCtx,
                      &g_configurationService,
                      &g_intersectionEngine,
                      &g_intersectionController);
  LWIPSNMPAdapterBindDetectorReportService(&s_lwipSnmpAdapterCtx,
                                           &g_detectorReportService);
  LWIPSNMPAdapterBindGlobalTimeManagementService(&s_lwipSnmpAdapterCtx,
                                                 &g_globalTimeManagementService);
  LWIPSNMPAdapterBindActivationService(&s_lwipSnmpAdapterCtx,
                                       &g_intersectionActivationService);
  LWIPSNMPAdapterBindDoorSensorPort(&s_lwipSnmpAdapterCtx, &g_doorPort);
  LWIPSNMPAdapterBindHeaterPort(&s_lwipSnmpAdapterCtx, &g_heaterPort);
  LWIPSNMPAdapterBindPowerMonitorPort(&s_lwipSnmpAdapterCtx,
                                      &g_powerMonitorPort);
  LWIPSNMPAdapterBindUnitAlarmPort(&s_lwipSnmpAdapterCtx, &g_unitAlarmPort);
  LWIPSNMPAdapterBindUnitClockPort(&s_lwipSnmpAdapterCtx, &g_unitClockPort);
  LWIPSNMPAdapterBindCpMpLinkService(&s_lwipSnmpAdapterCtx, &g_cpMpLinkService);
  LWIPSNMPBridgeBindAdapter(&s_lwipSnmpAdapterCtx);
}

static const struct snmp_mib *SaMibs[] = { &mib2, &ntcip1201mib, &ntcip1202mib,
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

uint8_t SNMPClientStart(ip_addr_t *pSTargetIp)
{
  SNMPClientStop();
  BindDomainSnmpAdapter();

  SNMPv3Init();

  snmp_set_device_enterprise_oid(&SDeviceEnterpriseOID);
  snmp_set_community(MCSGetSNMPReadCommunityName());
  snmp_set_community_write(MCSGetSNMPWriteCommunityName());
  snmp_set_community_trap(MCSGetSNMPTrapCommunityName());
  snmp_set_mibs(SaMibs, LWIP_ARRAYSIZE(SaMibs));

  snmp_v1_enable(1);
  snmp_v2c_enable(1);
  snmp_v3_enable(1);

  snmp_set_default_trap_version(MCSGetSNMPTrapVersion());

  snmp_trap_dst_ip_set(0, pSTargetIp);
  snmp_trap_dst_enable(0, 1);
  snmp_set_auth_traps_enabled(1);

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
