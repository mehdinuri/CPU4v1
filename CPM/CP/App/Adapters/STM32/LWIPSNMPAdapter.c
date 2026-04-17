/* App/Adapters/STM32/LWIPSNMPAdapter.c
 *
 * Thin bridge from SNMP handlers to the domain NTCIP object directory.
 */
#include "LWIPSNMPAdapter.h"

#include "Domain/NTCIP/NTCIP1201.h"
#include "Domain/NTCIP/NTCIP1202.h"

static void StageCommittedConfig(void *ctx)
{
  LWIPSNMPAdapterCtx_t *adapterCtx = (LWIPSNMPAdapterCtx_t *) ctx;

  if ((adapterCtx == NULL) || (adapterCtx->configurationService == NULL)
      || (adapterCtx->activationService == NULL))
  {
    return;
  }

  (void) IntersectionActivationServiceStageCommittedConfig(
    adapterCtx->activationService,
    ConfigurationServiceGetActiveConfig(adapterCtx->configurationService),
    ConfigurationServiceGetActiveSetId(adapterCtx->configurationService));
}

void LWIPSNMPAdapterInit(LWIPSNMPAdapterCtx_t *ctx,
                         ConfigurationService_t *configurationService,
                         IntersectionEngine_t *intersectionEngine,
                         IntersectionController_t *intersectionController)
{
  ctx->configurationService = configurationService;
  ctx->activationService = NULL;
  ctx->intersectionEngine = intersectionEngine;
  ctx->intersectionController = intersectionController;
  ctx->detectorReportService = NULL;
  ctx->unitAlarmPort = NULL;
  ctx->unitClockPort = NULL;
  NtcipDbTransactionServiceInit(&ctx->dbTransactionService,
                                configurationService);
  NtcipContextInit(&ctx->ntcipContext,
                   configurationService,
                   intersectionEngine,
                   intersectionController,
                   &ctx->dbTransactionService);
  NtcipObjectDirectoryInit(&ctx->objectDirectory);
  Ntcip1201RegisterObjects(&ctx->objectDirectory, &ctx->ntcipContext);
  Ntcip1202RegisterObjects(&ctx->objectDirectory, &ctx->ntcipContext);
}

void LWIPSNMPAdapterBindDetectorReportService(
  LWIPSNMPAdapterCtx_t *ctx,
  DetectorReportService_t *detectorReportService)
{
  if (ctx == NULL)
  {
    return;
  }

  ctx->detectorReportService = detectorReportService;
  NtcipContextBindDetectorReportService(&ctx->ntcipContext,
                                        detectorReportService);
}

void LWIPSNMPAdapterBindActivationService(
  LWIPSNMPAdapterCtx_t *ctx,
  IntersectionActivationService_t *activationService)
{
  if (ctx == NULL)
  {
    return;
  }

  ctx->activationService = activationService;
  NtcipDbTransactionServiceBindCommitObserver(&ctx->dbTransactionService,
                                              StageCommittedConfig,
                                              ctx);
}

void LWIPSNMPAdapterBindUnitAlarmPort(LWIPSNMPAdapterCtx_t *ctx,
                                      IUnitAlarmPort_t *unitAlarmPort)
{
  if (ctx == NULL)
  {
    return;
  }

  ctx->unitAlarmPort = unitAlarmPort;
  NtcipContextBindUnitAlarmPort(&ctx->ntcipContext, unitAlarmPort);
}

void LWIPSNMPAdapterBindUnitClockPort(LWIPSNMPAdapterCtx_t *ctx,
                                      IUnitClockPort_t *unitClockPort)
{
  if (ctx == NULL)
  {
    return;
  }

  ctx->unitClockPort = unitClockPort;
  NtcipContextBindUnitClockPort(&ctx->ntcipContext, unitClockPort);
}

NtcipError_t LWIPSNMPAdapterGet(const LWIPSNMPAdapterCtx_t *ctx,
                                const uint32_t *oid,
                                uint8_t oidLength,
                                const NtcipRequestContext_t *requestContext,
                                NtcipValue_t *value)
{
  return NtcipObjectDirectoryGet(&ctx->objectDirectory,
                                 oid,
                                 oidLength,
                                 requestContext,
                                 value);
}

NtcipError_t LWIPSNMPAdapterSetTest(const LWIPSNMPAdapterCtx_t *ctx,
                                    const uint32_t *oid,
                                    uint8_t oidLength,
                                    const NtcipRequestContext_t *requestContext,
                                    const NtcipValue_t *value)
{
  return NtcipObjectDirectorySetTest(&ctx->objectDirectory,
                                     oid,
                                     oidLength,
                                     requestContext,
                                     value);
}

NtcipError_t LWIPSNMPAdapterSetValue(LWIPSNMPAdapterCtx_t *ctx,
                                     const uint32_t *oid,
                                     uint8_t oidLength,
                                     const NtcipRequestContext_t *requestContext,
                                     const NtcipValue_t *value)
{
  return NtcipObjectDirectorySetValue(&ctx->objectDirectory,
                                      oid,
                                      oidLength,
                                      requestContext,
                                      value);
}
