/* App/Domain/NTCIP/Core/NtcipDbTransactionService.h
 *
 * Domain-side implementation of NTCIP 1201 database transaction semantics.
 */
#ifndef NTCIP_DB_TRANSACTION_SERVICE_H
#define NTCIP_DB_TRANSACTION_SERVICE_H

#include "Domain/Intersection/ConfigurationService.h"
#include "Domain/NTCIP/Core/NtcipTypes.h"

typedef enum
{
  NTCIP_DB_CREATE_STATE_NORMAL = 1,
  NTCIP_DB_CREATE_STATE_TRANSACTION = 2,
  NTCIP_DB_CREATE_STATE_VERIFY = 3,
  NTCIP_DB_CREATE_STATE_DONE = 6
} NtcipDbCreateState_t;

typedef enum
{
  NTCIP_DB_VERIFY_STATUS_NOT_DONE = 1,
  NTCIP_DB_VERIFY_STATUS_DONE_WITH_ERROR = 2,
  NTCIP_DB_VERIFY_STATUS_DONE_WITH_NO_ERROR = 3
} NtcipDbVerifyStatus_t;

typedef enum
{
  NTCIP_DB_ERROR_TYPE_TOO_BIG = 1,
  NTCIP_DB_ERROR_TYPE_NO_SUCH_NAME = 2,
  NTCIP_DB_ERROR_TYPE_BAD_VALUE = 3,
  NTCIP_DB_ERROR_TYPE_READ_ONLY = 4,
  NTCIP_DB_ERROR_TYPE_GEN_ERROR = 5,
  NTCIP_DB_ERROR_TYPE_UPDATE_ERROR = 6,
  NTCIP_DB_ERROR_TYPE_NO_ERROR = 7
} NtcipDbErrorType_t;

typedef void (*NtcipDbCommitObserver_t)(void *ctx);

typedef struct NtcipDbTransactionService
{
  ConfigurationService_t *configurationService;
  NtcipDbCreateState_t state;
  NtcipDbVerifyStatus_t verifyStatus;
  NtcipDbErrorType_t errorType;
  NtcipOid_t errorObjectId;
  NtcipOctetString_t verifyError;
  uint32_t ownerSessionKey;
  uint8_t ownerSessionValid;
  uint8_t transactionId;
  uint8_t transactionIdValid;
  uint8_t makeIdCounter;
  NtcipDbCommitObserver_t commitObserver;
  void *commitObserverCtx;
} NtcipDbTransactionService_t;

void NtcipDbTransactionServiceInit(NtcipDbTransactionService_t *service,
                                   ConfigurationService_t *configurationService);
void NtcipDbTransactionServiceBindCommitObserver(
  NtcipDbTransactionService_t *service,
  NtcipDbCommitObserver_t commitObserver,
  void *commitObserverCtx);
NtcipDbCreateState_t NtcipDbTransactionServiceGetState(
  const NtcipDbTransactionService_t *service);
NtcipDbVerifyStatus_t NtcipDbTransactionServiceGetVerifyStatus(
  const NtcipDbTransactionService_t *service);
NtcipDbErrorType_t NtcipDbTransactionServiceGetErrorType(
  const NtcipDbTransactionService_t *service);
const NtcipOid_t     *NtcipDbTransactionServiceGetErrorObjectId(
  const NtcipDbTransactionService_t *service);
const NtcipOctetString_t *NtcipDbTransactionServiceGetVerifyError(
  const NtcipDbTransactionService_t *service);
uint8_t NtcipDbTransactionServiceGetTransactionId(
  const NtcipDbTransactionService_t *service);
uint8_t NtcipDbTransactionServiceReadMakeId(
  NtcipDbTransactionService_t *service);
NtcipError_t NtcipDbTransactionServiceValidateDatabaseWrite(
  const NtcipDbTransactionService_t *service,
  const
  NtcipRequestContext_t
  *requestContext);
NtcipError_t NtcipDbTransactionServiceSetCreateTransactionTest(
  const NtcipDbTransactionService_t *service,
  const
  NtcipRequestContext_t
  *requestContext,
  uint32_t value);
NtcipError_t NtcipDbTransactionServiceSetCreateTransactionValue(
  NtcipDbTransactionService_t *service,
  const
  NtcipRequestContext_t
  *requestContext,
  uint32_t value);
NtcipError_t NtcipDbTransactionServiceSetTransactionIdTest(
  const NtcipDbTransactionService_t *service,
  const
  NtcipRequestContext_t
  *requestContext,
  uint32_t value);
NtcipError_t NtcipDbTransactionServiceSetTransactionIdValue(
  NtcipDbTransactionService_t *service,
  const
  NtcipRequestContext_t
  *requestContext,
  uint32_t value);

#endif /* NTCIP_DB_TRANSACTION_SERVICE_H */
