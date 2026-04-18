/* App/Adapters/STM32/LWIPSNMPAdapter.c
 *
 * Thin bridge from SNMP handlers to the domain NTCIP object directory.
 */
#include "LWIPSNMPAdapter.h"

#include "Domain/NTCIP/NTCIP1201.h"
#include "Domain/NTCIP/NTCIP1202.h"

#include <string.h>

static const uint32_t kDbCreateTransactionOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 2U, 1U, 0U
};
static const uint32_t kDbTransactionIdOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 2U, 4U, 0U
};

static uint8_t OidEquals(const uint32_t *left,
                         uint8_t leftLength,
                         const uint32_t *right,
                         uint8_t rightLength)
{
  uint8_t index;

  if ((left == NULL) || (right == NULL) || (leftLength != rightLength))
  {
    return 0U;
  }

  for (index = 0U; index < leftLength; index++)
  {
    if (left[index] != right[index])
    {
      return 0U;
    }
  }

  return 1U;
}

static LWIPSNMPSessionState_t *FindSessionState(LWIPSNMPAdapterCtx_t *ctx,
                                                uint32_t sessionKey)
{
  uint8_t index;

  if ((ctx == NULL) || (sessionKey == 0U))
  {
    return NULL;
  }

  for (index = 0U; index < LWIP_SNMP_SESSION_CACHE_SIZE; index++)
  {
    if ((ctx->sessionStates[index].occupied != 0U)
        && (ctx->sessionStates[index].sessionKey == sessionKey))
    {
      return &ctx->sessionStates[index];
    }
  }

  return NULL;
}

static LWIPSNMPSessionState_t *GetOrCreateSessionState(LWIPSNMPAdapterCtx_t *ctx,
                                                       uint32_t sessionKey)
{
  LWIPSNMPSessionState_t *state;
  LWIPSNMPSessionState_t *candidate;
  uint8_t index;

  state = FindSessionState(ctx, sessionKey);
  if (state != NULL)
  {
    return state;
  }

  if ((ctx == NULL) || (sessionKey == 0U))
  {
    return NULL;
  }

  candidate = &ctx->sessionStates[0];

  for (index = 0U; index < LWIP_SNMP_SESSION_CACHE_SIZE; index++)
  {
    if (ctx->sessionStates[index].occupied == 0U)
    {
      candidate = &ctx->sessionStates[index];
      break;
    }

    if (ctx->sessionStates[index].lastAccess < candidate->lastAccess)
    {
      candidate = &ctx->sessionStates[index];
    }
  }

  candidate->sessionKey = sessionKey;
  candidate->transactionId = 0U;
  candidate->transactionIdValid = 0U;
  candidate->occupied = 1U;
  candidate->lastAccess = 0U;

  return candidate;
}

static void TouchSessionState(LWIPSNMPAdapterCtx_t *ctx,
                              LWIPSNMPSessionState_t *state)
{
  if ((ctx == NULL) || (state == NULL))
  {
    return;
  }

  ctx->sessionAccessCounter++;
  state->lastAccess = ctx->sessionAccessCounter;
}

static void RecordTransactionId(LWIPSNMPAdapterCtx_t *ctx,
                                uint32_t sessionKey,
                                uint8_t transactionId)
{
  LWIPSNMPSessionState_t *state = GetOrCreateSessionState(ctx, sessionKey);

  if (state == NULL)
  {
    return;
  }

  state->transactionId = transactionId;
  state->transactionIdValid = 1U;
  TouchSessionState(ctx, state);
}

static void ClearTransactionId(LWIPSNMPAdapterCtx_t *ctx, uint32_t sessionKey)
{
  LWIPSNMPSessionState_t *state = FindSessionState(ctx, sessionKey);

  if (state == NULL)
  {
    return;
  }

  state->transactionId = 0U;
  state->transactionIdValid = 0U;
  TouchSessionState(ctx, state);
}

static void StageCommittedConfig(void *ctx)
{
  LWIPSNMPAdapterCtx_t *adapterCtx = (LWIPSNMPAdapterCtx_t *) ctx;

  if ((adapterCtx == NULL) || (adapterCtx->configurationService == NULL))
  {
    return;
  }

  if (adapterCtx->activationService != NULL)
  {
    (void) IntersectionActivationServiceStageCommittedConfig(
      adapterCtx->activationService,
      ConfigurationServiceGetActiveConfig(adapterCtx->configurationService),
      ConfigurationServiceGetActiveSetId(adapterCtx->configurationService));
  }

  if (adapterCtx->globalTimeManagementService != NULL)
  {
    GlobalTimeManagementServiceHandleCommittedConfig(
      adapterCtx->globalTimeManagementService,
      &ConfigurationServiceGetActiveConfig(
        adapterCtx->configurationService)->globalTimeManagement);
  }
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
  ctx->globalTimeManagementService = NULL;
  ctx->doorSensorPort = NULL;
  ctx->heaterPort = NULL;
  ctx->powerMonitorPort = NULL;
  ctx->unitAlarmPort = NULL;
  ctx->unitClockPort = NULL;
  memset(ctx->sessionStates, 0, sizeof(ctx->sessionStates));
  ctx->sessionAccessCounter = 0U;
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

void LWIPSNMPAdapterBindGlobalTimeManagementService(
  LWIPSNMPAdapterCtx_t *ctx,
  GlobalTimeManagementService_t *globalTimeManagementService)
{
  if (ctx == NULL)
  {
    return;
  }

  ctx->globalTimeManagementService = globalTimeManagementService;
  NtcipContextBindGlobalTimeManagementService(&ctx->ntcipContext,
                                              globalTimeManagementService);
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

void LWIPSNMPAdapterBindDoorSensorPort(LWIPSNMPAdapterCtx_t *ctx,
                                       IDoorSensorPort_t *doorSensorPort)
{
  if (ctx == NULL)
  {
    return;
  }

  ctx->doorSensorPort = doorSensorPort;
  NtcipContextBindDoorSensorPort(&ctx->ntcipContext, doorSensorPort);
}

void LWIPSNMPAdapterBindHeaterPort(LWIPSNMPAdapterCtx_t *ctx,
                                   IHeaterPort_t *heaterPort)
{
  if (ctx == NULL)
  {
    return;
  }

  ctx->heaterPort = heaterPort;
  NtcipContextBindHeaterPort(&ctx->ntcipContext, heaterPort);
}

void LWIPSNMPAdapterBindPowerMonitorPort(LWIPSNMPAdapterCtx_t *ctx,
                                         IPowerMonitorPort_t *powerMonitorPort)
{
  if (ctx == NULL)
  {
    return;
  }

  ctx->powerMonitorPort = powerMonitorPort;
  NtcipContextBindPowerMonitorPort(&ctx->ntcipContext, powerMonitorPort);
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

LWIPSNMPManagedState_t LWIPSNMPAdapterGetManagedState(
  const LWIPSNMPAdapterCtx_t *ctx,
  const uint32_t *oid,
  uint8_t oidLength,
  const NtcipObjectDescriptor_t **descriptorOut)
{
  NtcipValue_t value;
  NtcipResolvedObject_t resolvedObject;
  NtcipError_t getError;

  if (descriptorOut != NULL)
  {
    *descriptorOut = NULL;
  }

  if ((ctx == NULL) || (oid == NULL))
  {
    return LWIP_SNMP_MANAGED_STATE_UNMANAGED;
  }

  if (NtcipObjectDirectoryResolve(&ctx->objectDirectory,
                                  oid,
                                  oidLength,
                                  &resolvedObject) != 0U)
  {
    if ((resolvedObject.descriptor->kind == NTCIP_OBJECT_KIND_TABLE_COLUMN)
        && (resolvedObject.descriptor->get != NULL))
    {
      getError = NtcipObjectDirectoryGet(&ctx->objectDirectory,
                                         oid,
                                         oidLength,
                                         NULL,
                                         &value);

      if ((getError == NTCIP_ERROR_NOT_FOUND)
          || (getError == NTCIP_ERROR_RANGE_ERROR)
          || (getError == NTCIP_ERROR_BAD_VALUE))
      {
        return LWIP_SNMP_MANAGED_STATE_PREFIX_ONLY;
      }
    }

    if (descriptorOut != NULL)
    {
      *descriptorOut = resolvedObject.descriptor;
    }

    return LWIP_SNMP_MANAGED_STATE_EXACT;
  }

  if (NtcipObjectDirectoryMatchesPrefix(&ctx->objectDirectory,
                                        oid,
                                        oidLength) != 0U)
  {
    return LWIP_SNMP_MANAGED_STATE_PREFIX_ONLY;
  }

  return LWIP_SNMP_MANAGED_STATE_UNMANAGED;
}

void LWIPSNMPAdapterBuildRequestContext(LWIPSNMPAdapterCtx_t *ctx,
                                        uint32_t sessionKey,
                                        NtcipRequestContext_t *requestContext)
{
  LWIPSNMPSessionState_t *state;

  if (requestContext == NULL)
  {
    return;
  }

  memset(requestContext, 0, sizeof(*requestContext));
  requestContext->sessionKey = sessionKey;

  state = FindSessionState(ctx, sessionKey);
  if ((state != NULL) && (state->transactionIdValid != 0U))
  {
    requestContext->transactionIdValid = 1U;
    requestContext->transactionId = state->transactionId;
    TouchSessionState(ctx, state);
  }
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
  NtcipError_t error;

  error = NtcipObjectDirectorySetValue(&ctx->objectDirectory,
                                       oid,
                                       oidLength,
                                       requestContext,
                                       value);

  if ((error != NTCIP_ERROR_OK) || (requestContext == NULL)
      || (requestContext->sessionKey == 0U) || (value == NULL)
      || (value->type != NTCIP_VALUE_TYPE_UNSIGNED32))
  {
    return error;
  }

  if (OidEquals(oid,
                oidLength,
                kDbTransactionIdOid,
                (uint8_t) (sizeof(kDbTransactionIdOid) / sizeof(kDbTransactionIdOid[0])))
      && (value->data.unsigned32 <= 0xFFU))
  {
    RecordTransactionId(ctx,
                        requestContext->sessionKey,
                        (uint8_t) value->data.unsigned32);
  }
  else if (OidEquals(oid,
                     oidLength,
                     kDbCreateTransactionOid,
                     (uint8_t) (sizeof(kDbCreateTransactionOid) / sizeof(kDbCreateTransactionOid[0])))
           && ((value->data.unsigned32
                == (uint32_t) NTCIP_DB_CREATE_STATE_TRANSACTION)
               || (value->data.unsigned32
                   == (uint32_t) NTCIP_DB_CREATE_STATE_NORMAL)))
  {
    ClearTransactionId(ctx, requestContext->sessionKey);
  }

  return error;
}
