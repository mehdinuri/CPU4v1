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
#include "Domain/Intersection/IntersectionController.h"
#include "Domain/Intersection/IntersectionEngine.h"
#include "Domain/NTCIP/Core/NtcipDbTransactionService.h"
#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"
#include "Ports/IUnitAlarmPort.h"
#include "Ports/IUnitClockPort.h"

typedef struct
{
  ConfigurationService_t *configurationService;
  IntersectionActivationService_t *activationService;
  IntersectionEngine_t *intersectionEngine;
  IntersectionController_t *intersectionController;
  DetectorReportService_t *detectorReportService;
  IUnitAlarmPort_t *unitAlarmPort;
  IUnitClockPort_t *unitClockPort;
  NtcipDbTransactionService_t dbTransactionService;
  NtcipObjectDirectory_t objectDirectory;
  NtcipContext_t ntcipContext;
} LWIPSNMPAdapterCtx_t;

void LWIPSNMPAdapterInit(LWIPSNMPAdapterCtx_t *ctx,
                         ConfigurationService_t *configurationService,
                         IntersectionEngine_t *intersectionEngine,
                         IntersectionController_t *intersectionController);
void LWIPSNMPAdapterBindDetectorReportService(
  LWIPSNMPAdapterCtx_t *ctx,
  DetectorReportService_t *detectorReportService);
void LWIPSNMPAdapterBindActivationService(
  LWIPSNMPAdapterCtx_t *ctx,
  IntersectionActivationService_t *activationService);
void LWIPSNMPAdapterBindUnitAlarmPort(LWIPSNMPAdapterCtx_t *ctx,
                                      IUnitAlarmPort_t *unitAlarmPort);
void LWIPSNMPAdapterBindUnitClockPort(LWIPSNMPAdapterCtx_t *ctx,
                                      IUnitClockPort_t *unitClockPort);
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
