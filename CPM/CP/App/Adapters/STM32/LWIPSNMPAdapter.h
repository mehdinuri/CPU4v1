/* App/Adapters/STM32/LWIPSNMPAdapter.h
 *
 * Thin bridge from the transport-facing SNMP layer into the new domain-side
 * NTCIP object directory.
 */
#ifndef LWIP_SNMP_ADAPTER_H
#define LWIP_SNMP_ADAPTER_H

#include "Domain/Intersection/ConfigurationService.h"
#include "Domain/Intersection/IntersectionActivationService.h"
#include "Domain/Intersection/DetectorReportService.h"
#include "Domain/Intersection/GlobalTimeManagementService.h"
#include "Domain/Intersection/IntersectionController.h"
#include "Domain/Intersection/IntersectionEngine.h"
#include "Domain/NTCIP/Core/NtcipDbTransactionService.h"
#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"
#include "Ports/IDoorSensorPort.h"
#include "Ports/IHeaterPort.h"
#include "Ports/IPowerMonitorPort.h"
#include "Ports/IUnitAlarmPort.h"
#include "Ports/IUnitClockPort.h"

#define LWIP_SNMP_SESSION_CACHE_SIZE 8U

typedef enum
{
  LWIP_SNMP_MANAGED_STATE_UNMANAGED = 0,
  LWIP_SNMP_MANAGED_STATE_PREFIX_ONLY,
  LWIP_SNMP_MANAGED_STATE_EXACT
} LWIPSNMPManagedState_t;

typedef struct
{
  uint32_t sessionKey;
  uint8_t transactionId;
  uint8_t transactionIdValid;
  uint8_t occupied;
  uint32_t lastAccess;
} LWIPSNMPSessionState_t;

typedef struct
{
  ConfigurationService_t *configurationService;
  IntersectionActivationService_t *activationService;
  IntersectionEngine_t *intersectionEngine;
  IntersectionController_t *intersectionController;
  DetectorReportService_t *detectorReportService;
  GlobalTimeManagementService_t *globalTimeManagementService;
  IDoorSensorPort_t *doorSensorPort;
  IHeaterPort_t *heaterPort;
  IPowerMonitorPort_t *powerMonitorPort;
  IUnitAlarmPort_t *unitAlarmPort;
  IUnitClockPort_t *unitClockPort;
  NtcipDbTransactionService_t dbTransactionService;
  NtcipObjectDirectory_t objectDirectory;
  NtcipContext_t ntcipContext;
  LWIPSNMPSessionState_t sessionStates[LWIP_SNMP_SESSION_CACHE_SIZE];
  uint32_t sessionAccessCounter;
} LWIPSNMPAdapterCtx_t;

void LWIPSNMPAdapterInit(LWIPSNMPAdapterCtx_t *ctx,
                         ConfigurationService_t *configurationService,
                         IntersectionEngine_t *intersectionEngine,
                         IntersectionController_t *intersectionController);
void LWIPSNMPAdapterBindDetectorReportService(
  LWIPSNMPAdapterCtx_t *ctx,
  DetectorReportService_t *detectorReportService);
void LWIPSNMPAdapterBindGlobalTimeManagementService(
  LWIPSNMPAdapterCtx_t *ctx,
  GlobalTimeManagementService_t *globalTimeManagementService);
void LWIPSNMPAdapterBindActivationService(
  LWIPSNMPAdapterCtx_t *ctx,
  IntersectionActivationService_t *activationService);
void LWIPSNMPAdapterBindDoorSensorPort(LWIPSNMPAdapterCtx_t *ctx,
                                       IDoorSensorPort_t *doorSensorPort);
void LWIPSNMPAdapterBindHeaterPort(LWIPSNMPAdapterCtx_t *ctx,
                                   IHeaterPort_t *heaterPort);
void LWIPSNMPAdapterBindPowerMonitorPort(LWIPSNMPAdapterCtx_t *ctx,
                                         IPowerMonitorPort_t *powerMonitorPort);
void LWIPSNMPAdapterBindUnitAlarmPort(LWIPSNMPAdapterCtx_t *ctx,
                                      IUnitAlarmPort_t *unitAlarmPort);
void LWIPSNMPAdapterBindUnitClockPort(LWIPSNMPAdapterCtx_t *ctx,
                                      IUnitClockPort_t *unitClockPort);
LWIPSNMPManagedState_t LWIPSNMPAdapterGetManagedState(
  const LWIPSNMPAdapterCtx_t *ctx,
  const uint32_t *oid,
  uint8_t oidLength,
  const NtcipObjectDescriptor_t **descriptorOut);
void LWIPSNMPAdapterBuildRequestContext(LWIPSNMPAdapterCtx_t *ctx,
                                        uint32_t sessionKey,
                                        NtcipRequestContext_t *requestContext);
NtcipError_t LWIPSNMPAdapterGet(const LWIPSNMPAdapterCtx_t *ctx,
                                const uint32_t *oid,
                                uint8_t oidLength,
                                const NtcipRequestContext_t *requestContext,
                                NtcipValue_t *value);
NtcipError_t LWIPSNMPAdapterSetTest(const LWIPSNMPAdapterCtx_t *ctx,
                                    const uint32_t *oid,
                                    uint8_t oidLength,
                                    const NtcipRequestContext_t *requestContext,
                                    const NtcipValue_t *value);
NtcipError_t LWIPSNMPAdapterSetValue(LWIPSNMPAdapterCtx_t *ctx,
                                     const uint32_t *oid,
                                     uint8_t oidLength,
                                     const NtcipRequestContext_t *requestContext,
                                     const NtcipValue_t *value);

#endif /* LWIP_SNMP_ADAPTER_H */
