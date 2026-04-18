/* App/Domain/NTCIP/Core/NtcipDbTransactionService.c
 *
 * 1201 transaction-state semantics layered over the new configuration
 * service. Buffered configuration stays in the configuration candidate until
 * a verify/done/normal transition completes.
 */
#include "NtcipDbTransactionService.h"

#include <stddef.h>
#include <string.h>

static const uint32_t kPhaseMinimumGreenOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 4U
};
static const uint32_t kPhasePassageOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 5U
};
static const uint32_t kPhaseMaximum1Oid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 6U
};
static const uint32_t kPhaseYellowChangeOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 8U
};
static const uint32_t kPhaseRedClearOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 9U
};
static const uint32_t kPhaseMaximumInitialOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 12U
};
static const uint32_t kPhaseOptionsOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 21U
};
static const uint32_t kPhaseRingOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 1U, 2U, 1U, 22U
};

static void ClearErrorState(NtcipDbTransactionService_t *service)
{
  service->verifyStatus = NTCIP_DB_VERIFY_STATUS_NOT_DONE;
  service->errorType = NTCIP_DB_ERROR_TYPE_NO_ERROR;
  service->errorObjectId.length = 0U;
  service->verifyError.length = 0U;
}

static void ClearOwnership(NtcipDbTransactionService_t *service)
{
  service->ownerSessionKey = 0U;
  service->ownerSessionValid = 0U;
  service->transactionId = 0U;
  service->transactionIdValid = 0U;
}

static uint8_t RequestOwnsTransaction(
  const NtcipDbTransactionService_t *service,
  const NtcipRequestContext_t *
  requestContext)
{
  if ((service == NULL) || (requestContext == NULL)
      || (service->ownerSessionValid == 0U))
  {
    return 0U;
  }

  return (uint8_t) (service->ownerSessionKey == requestContext->sessionKey);
}

static void CopyErrorString(NtcipDbTransactionService_t *service,
                            const char *message)
{
  size_t length;

  service->verifyError.length = 0U;

  if (message == NULL)
  {
    return;
  }

  length = strlen(message);

  if (length > NTCIP_OCTET_STRING_MAX_LENGTH)
  {
    length = NTCIP_OCTET_STRING_MAX_LENGTH;
  }

  memcpy(service->verifyError.bytes, message, length);
  service->verifyError.length = (uint16_t) length;
}

static void SetErrorOidForPhase(NtcipDbTransactionService_t *service,
                                const uint32_t *columnOid,
                                uint8_t columnOidLength,
                                uint16_t phaseNumber)
{
  uint8_t i;

  service->errorObjectId.length = (uint8_t) (columnOidLength + 1U);

  for (i = 0U; i < columnOidLength; i++)
  {
    service->errorObjectId.elements[i] = columnOid[i];
  }

  service->errorObjectId.elements[columnOidLength] = phaseNumber;
}

static void PopulateVerifyErrorFromConfig(NtcipDbTransactionService_t *service,
                                          IntersectionConfigErrorInfo_t
                                          errorInfo)
{
  service->errorType = NTCIP_DB_ERROR_TYPE_BAD_VALUE;
  service->errorObjectId.length = 0U;
  service->verifyError.length = 0U;

  switch (errorInfo.type)
  {
      case INTERSECTION_CONFIG_ERROR_MIN_GREEN:
      {
        SetErrorOidForPhase(service,
                            kPhaseMinimumGreenOid,
                            (uint8_t) (sizeof(kPhaseMinimumGreenOid)
                                       / sizeof(uint32_t)),
                            errorInfo.objectIndex);
        CopyErrorString(service, "phaseMinimumGreen is invalid");
        break;
      }

      case INTERSECTION_CONFIG_ERROR_MAX_GREEN:
      {
        SetErrorOidForPhase(service,
                            kPhaseMaximum1Oid,
                            (uint8_t) (sizeof(kPhaseMaximum1Oid)
                                       / sizeof(uint32_t)),
                            errorInfo.objectIndex);
        CopyErrorString(service, "phaseMaximum1 is invalid");
        break;
      }

      case INTERSECTION_CONFIG_ERROR_YELLOW_CHANGE:
      {
        SetErrorOidForPhase(service,
                            kPhaseYellowChangeOid,
                            (uint8_t) (sizeof(kPhaseYellowChangeOid)
                                       / sizeof(uint32_t)),
                            errorInfo.objectIndex);
        CopyErrorString(service, "phaseYellowChange is invalid");
        break;
      }

      case INTERSECTION_CONFIG_ERROR_RED_CLEAR:
      {
        SetErrorOidForPhase(service,
                            kPhaseRedClearOid,
                            (uint8_t) (sizeof(kPhaseRedClearOid)
                                       / sizeof(uint32_t)),
                            errorInfo.objectIndex);
        CopyErrorString(service, "phaseRedClear is invalid");
        break;
      }

      case INTERSECTION_CONFIG_ERROR_PASSAGE:
      {
        SetErrorOidForPhase(service,
                            kPhasePassageOid,
                            (uint8_t) (sizeof(kPhasePassageOid)
                                       / sizeof(uint32_t)),
                            errorInfo.objectIndex);
        CopyErrorString(service, "phasePassage is invalid");
        break;
      }

      case INTERSECTION_CONFIG_ERROR_MAX_INITIAL:
      {
        SetErrorOidForPhase(service,
                            kPhaseMaximumInitialOid,
                            (uint8_t) (sizeof(kPhaseMaximumInitialOid)
                                       / sizeof(uint32_t)),
                            errorInfo.objectIndex);
        CopyErrorString(service, "phaseMaximumInitial is invalid");
        break;
      }

      case INTERSECTION_CONFIG_ERROR_PHASE_ENABLE:
      {
        SetErrorOidForPhase(service,
                            kPhaseOptionsOid,
                            (uint8_t) (sizeof(kPhaseOptionsOid)
                                       / sizeof(uint32_t)),
                            errorInfo.objectIndex);
        CopyErrorString(service, "phaseOptions is invalid");
        break;
      }

      case INTERSECTION_CONFIG_ERROR_PHASE_RING:
      {
        SetErrorOidForPhase(service,
                            kPhaseRingOid,
                            (uint8_t) (sizeof(kPhaseRingOid)
                                       / sizeof(uint32_t)),
                            errorInfo.objectIndex);
        CopyErrorString(service, "phaseRing is invalid");
        break;
      }

      case INTERSECTION_CONFIG_ERROR_PHASE_COUNT:
      {
        CopyErrorString(service, "phaseCount is invalid");
        break;
      }

      case INTERSECTION_CONFIG_ERROR_RING_COUNT:
      {
        CopyErrorString(service, "ringCount is invalid");
        break;
      }

      case INTERSECTION_CONFIG_ERROR_BARRIER_COUNT:
      {
        CopyErrorString(service, "barrierCount is invalid");
        break;
      }

      case INTERSECTION_CONFIG_ERROR_CHANNEL_CONTROL_TYPE:
      {
        CopyErrorString(service,
                        "channelControlType is not supported by runtime");
        break;
      }

      case INTERSECTION_CONFIG_ERROR_PREEMPT_LINK:
      {
        CopyErrorString(service, "preemptLink is invalid");
        break;
      }

      case INTERSECTION_CONFIG_ERROR_PREEMPT_SEQUENCE_NUMBER:
      {
        CopyErrorString(service, "preemptSequenceNumber is invalid");
        break;
      }

      case INTERSECTION_CONFIG_ERROR_PREEMPT_EXIT_TYPE:
      {
        CopyErrorString(service, "preemptExitType is invalid");
        break;
      }

      case INTERSECTION_CONFIG_ERROR_STORAGE:
      case INTERSECTION_CONFIG_ERROR_TRANSACTION_STATE:
      {
        service->errorType = NTCIP_DB_ERROR_TYPE_GEN_ERROR;
        CopyErrorString(service, "configuration transaction failed");
        break;
      }

      case INTERSECTION_CONFIG_ERROR_NONE:
      default:
      {
        service->errorType = NTCIP_DB_ERROR_TYPE_NO_ERROR;
        break;
      }
  } /* switch */
} /* PopulateVerifyErrorFromConfig */

void NtcipDbTransactionServiceInit(NtcipDbTransactionService_t *service,
                                   ConfigurationService_t *configurationService)
{
  if (service == NULL)
  {
    return;
  }

  memset(service, 0, sizeof(*service));
  service->configurationService = configurationService;
  service->state = NTCIP_DB_CREATE_STATE_NORMAL;
  ClearErrorState(service);
}

void NtcipDbTransactionServiceBindCommitObserver(
  NtcipDbTransactionService_t *service,
  NtcipDbCommitObserver_t commitObserver,
  void *commitObserverCtx)
{
  if (service == NULL)
  {
    return;
  }

  service->commitObserver = commitObserver;
  service->commitObserverCtx = commitObserverCtx;
}

NtcipDbCreateState_t NtcipDbTransactionServiceGetState(
  const NtcipDbTransactionService_t *service)
{
  if (service == NULL)
  {
    return NTCIP_DB_CREATE_STATE_NORMAL;
  }

  return service->state;
}

NtcipDbVerifyStatus_t NtcipDbTransactionServiceGetVerifyStatus(
  const NtcipDbTransactionService_t *service)
{
  if (service == NULL)
  {
    return NTCIP_DB_VERIFY_STATUS_NOT_DONE;
  }

  return service->verifyStatus;
}

NtcipDbErrorType_t NtcipDbTransactionServiceGetErrorType(
  const NtcipDbTransactionService_t *service)
{
  if (service == NULL)
  {
    return NTCIP_DB_ERROR_TYPE_NO_ERROR;
  }

  return service->errorType;
}

const NtcipOid_t *NtcipDbTransactionServiceGetErrorObjectId(
  const NtcipDbTransactionService_t *service)
{
  if (service == NULL)
  {
    return NULL;
  }

  return &service->errorObjectId;
}

const NtcipOctetString_t *NtcipDbTransactionServiceGetVerifyError(
  const NtcipDbTransactionService_t *service)
{
  if (service == NULL)
  {
    return NULL;
  }

  return &service->verifyError;
}

uint8_t NtcipDbTransactionServiceGetTransactionId(
  const NtcipDbTransactionService_t *service)
{
  if ((service == NULL) || (service->transactionIdValid == 0U))
  {
    return 0U;
  }

  return service->transactionId;
}

uint8_t NtcipDbTransactionServiceReadMakeId(
  NtcipDbTransactionService_t *service)
{
  if (service == NULL)
  {
    return 0U;
  }

  service->makeIdCounter++;

  return service->makeIdCounter;
}

NtcipError_t NtcipDbTransactionServiceValidateDatabaseWrite(
  const NtcipDbTransactionService_t *service,
  const
  NtcipRequestContext_t
  *requestContext)
{
  if ((service == NULL)
      || (service->state != NTCIP_DB_CREATE_STATE_TRANSACTION))
  {
    return NTCIP_ERROR_NO_TRANSACTION;
  }

  if (RequestOwnsTransaction(service, requestContext) == 0U)
  {
    return NTCIP_ERROR_OWNER_MISMATCH;
  }

  if ((service->transactionIdValid == 0U) || (requestContext == NULL)
      || (requestContext->transactionIdValid == 0U)
      || (requestContext->transactionId != service->transactionId))
  {
    return NTCIP_ERROR_TRANSACTION_ID_MISMATCH;
  }

  return NTCIP_ERROR_OK;
}

NtcipError_t NtcipDbTransactionServiceSetCreateTransactionTest(
  const NtcipDbTransactionService_t *service,
  const
  NtcipRequestContext_t
  *requestContext,
  uint32_t value)
{
  if (service == NULL)
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  switch (service->state)
  {
      case NTCIP_DB_CREATE_STATE_NORMAL:
      {
        return (value == (uint32_t) NTCIP_DB_CREATE_STATE_TRANSACTION)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_BAD_VALUE;
      }

      case NTCIP_DB_CREATE_STATE_TRANSACTION:
      {
        if (RequestOwnsTransaction(service, requestContext) == 0U)
        {
          return NTCIP_ERROR_OWNER_MISMATCH;
        }

        if ((value == (uint32_t) NTCIP_DB_CREATE_STATE_VERIFY)
            || (value == (uint32_t) NTCIP_DB_CREATE_STATE_NORMAL))
        {
          return NTCIP_ERROR_OK;
        }

        return NTCIP_ERROR_BAD_VALUE;
      }

      case NTCIP_DB_CREATE_STATE_VERIFY:
      {
        return NTCIP_ERROR_BAD_VALUE;
      }

      case NTCIP_DB_CREATE_STATE_DONE:
      {
        if (RequestOwnsTransaction(service, requestContext) == 0U)
        {
          return NTCIP_ERROR_OWNER_MISMATCH;
        }

        if ((value == (uint32_t) NTCIP_DB_CREATE_STATE_NORMAL)
            || (value == (uint32_t) NTCIP_DB_CREATE_STATE_TRANSACTION))
        {
          return NTCIP_ERROR_OK;
        }

        return NTCIP_ERROR_BAD_VALUE;
      }

      default:
      {
        return NTCIP_ERROR_BAD_VALUE;
      }
  } /* switch */
} /* NtcipDbTransactionServiceSetCreateTransactionTest */

NtcipError_t NtcipDbTransactionServiceSetCreateTransactionValue(
  NtcipDbTransactionService_t *service,
  const
  NtcipRequestContext_t
  *requestContext,
  uint32_t value)
{
  NtcipError_t testResult;
  IntersectionConfigErrorInfo_t errorInfo;

  testResult = NtcipDbTransactionServiceSetCreateTransactionTest(service,
                                                                 requestContext,
                                                                 value);

  if (testResult != NTCIP_ERROR_OK)
  {
    return testResult;
  }

  switch (service->state)
  {
      case NTCIP_DB_CREATE_STATE_NORMAL:
      {
        if ((service->configurationService == NULL)
            || (ConfigurationServiceCreateTransaction(
                  service->configurationService) == 0U))
        {
          return NTCIP_ERROR_GEN_ERROR;
        }

        service->state = NTCIP_DB_CREATE_STATE_TRANSACTION;
        service->ownerSessionValid = 1U;
        service->ownerSessionKey = (requestContext
                                    != NULL) ? requestContext->sessionKey : 0U;
        service->transactionIdValid = 0U;
        ClearErrorState(service);

        return NTCIP_ERROR_OK;
      }

      case NTCIP_DB_CREATE_STATE_TRANSACTION:
      {
        if (value == (uint32_t) NTCIP_DB_CREATE_STATE_NORMAL)
        {
          ConfigurationServiceRollback(service->configurationService);
          service->state = NTCIP_DB_CREATE_STATE_NORMAL;
          ClearOwnership(service);
          ClearErrorState(service);

          return NTCIP_ERROR_OK;
        }

        service->state = NTCIP_DB_CREATE_STATE_VERIFY;
        errorInfo =
          ConfigurationServiceGetLastError(service->configurationService);

        if (ConfigurationServiceVerify(service->configurationService) != 0U)
        {
          ClearErrorState(service);
          service->verifyStatus = NTCIP_DB_VERIFY_STATUS_DONE_WITH_NO_ERROR;
        }
        else
        {
          errorInfo =
            ConfigurationServiceGetLastError(service->configurationService);
          service->verifyStatus = NTCIP_DB_VERIFY_STATUS_DONE_WITH_ERROR;
          PopulateVerifyErrorFromConfig(service, errorInfo);
        }

        service->state = NTCIP_DB_CREATE_STATE_DONE;

        return NTCIP_ERROR_OK;
      }

      case NTCIP_DB_CREATE_STATE_DONE:
      {
        if (value == (uint32_t) NTCIP_DB_CREATE_STATE_TRANSACTION)
        {
          service->state = NTCIP_DB_CREATE_STATE_TRANSACTION;
          service->verifyStatus = NTCIP_DB_VERIFY_STATUS_NOT_DONE;
          service->errorType = NTCIP_DB_ERROR_TYPE_NO_ERROR;
          service->errorObjectId.length = 0U;
          service->verifyError.length = 0U;

          return NTCIP_ERROR_OK;
        }

        if (service->verifyStatus == NTCIP_DB_VERIFY_STATUS_DONE_WITH_NO_ERROR)
        {
          if (ConfigurationServiceCommit(service->configurationService) == 0U)
          {
            ConfigurationServiceRollback(service->configurationService);
            service->state = NTCIP_DB_CREATE_STATE_NORMAL;
            ClearOwnership(service);
            ClearErrorState(service);

            return NTCIP_ERROR_COMMIT_FAILED;
          }

          if (service->commitObserver != NULL)
          {
            service->commitObserver(service->commitObserverCtx);
          }
        }
        else
        {
          ConfigurationServiceRollback(service->configurationService);
        }

        service->state = NTCIP_DB_CREATE_STATE_NORMAL;
        ClearOwnership(service);
        ClearErrorState(service);

        return NTCIP_ERROR_OK;
      }

      case NTCIP_DB_CREATE_STATE_VERIFY:
      default:
      {
        return NTCIP_ERROR_BAD_VALUE;
      }
  } /* switch */
} /* NtcipDbTransactionServiceSetCreateTransactionValue */

NtcipError_t NtcipDbTransactionServiceSetTransactionIdTest(
  const NtcipDbTransactionService_t *service,
  const
  NtcipRequestContext_t
  *requestContext,
  uint32_t value)
{
  if ((service == NULL) || (value > 255UL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if ((service->state == NTCIP_DB_CREATE_STATE_NORMAL)
      || (service->state == NTCIP_DB_CREATE_STATE_VERIFY))
  {
    return NTCIP_ERROR_NO_TRANSACTION;
  }

  if (RequestOwnsTransaction(service, requestContext) == 0U)
  {
    return NTCIP_ERROR_OWNER_MISMATCH;
  }

  return NTCIP_ERROR_OK;
}

NtcipError_t NtcipDbTransactionServiceSetTransactionIdValue(
  NtcipDbTransactionService_t *service,
  const
  NtcipRequestContext_t
  *requestContext,
  uint32_t value)
{
  NtcipError_t testResult;

  testResult = NtcipDbTransactionServiceSetTransactionIdTest(service,
                                                             requestContext,
                                                             value);

  if (testResult != NTCIP_ERROR_OK)
  {
    return testResult;
  }

  service->transactionId = (uint8_t) value;
  service->transactionIdValid = 1U;

  return NTCIP_ERROR_OK;
}
