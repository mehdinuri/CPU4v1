/* App/Adapters/STM32/MmiCanAdapter.c */
#include "MmiCanAdapter.h"

#include <string.h>

#include "stm32h7xx_hal.h"

#define MMI_CAN_ADAPTER_RX_DEPTH 16U
#define MMI_CAN_ADAPTER_TX_DEPTH 32U
#define MMI_CAN_ADAPTER_TRANSFER_BUFFER_BYTES 512U
#define MMI_CAN_ADAPTER_CONTROLLER_ROLE_CP 1U
#define MMI_CAN_ADAPTER_TASK_TICK_MS 10U

static const uint32_t kDbCreateTransactionOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 2U, 1U, 0U
};
static const uint32_t kDbTransactionIdOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 2U, 4U, 0U
};

typedef struct
{
  uint16_t standardId;
  uint8_t length;
  uint8_t data[8];
} MmiQueuedFrame_t;

static MmiCanAdapterCtx_t *s_registeredCtx = NULL;

static uint32_t ReadLe32(const uint8_t *bytes)
{
  return (uint32_t) ((uint32_t) bytes[0]
                     | ((uint32_t) bytes[1] << 8U)
                     | ((uint32_t) bytes[2] << 16U)
                     | ((uint32_t) bytes[3] << 24U));
}

static void WriteLe32(uint8_t *bytes, uint32_t value)
{
  bytes[0] = (uint8_t) (value & 0xFFU);
  bytes[1] = (uint8_t) ((value >> 8U) & 0xFFU);
  bytes[2] = (uint8_t) ((value >> 16U) & 0xFFU);
  bytes[3] = (uint8_t) ((value >> 24U) & 0xFFU);
}

static uint8_t DlcToLength(uint32_t dlc)
{
  switch (dlc)
  {
      case FDCAN_DLC_BYTES_0: return 0U;
      case FDCAN_DLC_BYTES_1: return 1U;
      case FDCAN_DLC_BYTES_2: return 2U;
      case FDCAN_DLC_BYTES_3: return 3U;
      case FDCAN_DLC_BYTES_4: return 4U;
      case FDCAN_DLC_BYTES_5: return 5U;
      case FDCAN_DLC_BYTES_6: return 6U;
      case FDCAN_DLC_BYTES_7: return 7U;
      case FDCAN_DLC_BYTES_8: return 8U;
      default: return 8U;
  }
}

static uint32_t LengthToDlc(uint8_t length)
{
  switch (length)
  {
      case 0U: return FDCAN_DLC_BYTES_0;
      case 1U: return FDCAN_DLC_BYTES_1;
      case 2U: return FDCAN_DLC_BYTES_2;
      case 3U: return FDCAN_DLC_BYTES_3;
      case 4U: return FDCAN_DLC_BYTES_4;
      case 5U: return FDCAN_DLC_BYTES_5;
      case 6U: return FDCAN_DLC_BYTES_6;
      case 7U: return FDCAN_DLC_BYTES_7;
      case 8U:
      default:
      {
        return FDCAN_DLC_BYTES_8;
      }
  }
}

static void CreateOsObjects(MmiCanAdapterCtx_t *ctx)
{
  if (ctx == NULL)
  {
    return;
  }

  if (ctx->rxPool == NULL)
  {
    ctx->rxPool = osMemoryPoolNew(MMI_CAN_ADAPTER_RX_DEPTH,
                                  sizeof(MmiQueuedFrame_t),
                                  NULL);
  }

  if (ctx->txPool == NULL)
  {
    ctx->txPool = osMemoryPoolNew(MMI_CAN_ADAPTER_TX_DEPTH,
                                  sizeof(MmiQueuedFrame_t),
                                  NULL);
  }

  if (ctx->rxQueue == NULL)
  {
    ctx->rxQueue = osMessageQueueNew(MMI_CAN_ADAPTER_RX_DEPTH,
                                     sizeof(MmiQueuedFrame_t *),
                                     NULL);
  }

  if (ctx->txQueue == NULL)
  {
    ctx->txQueue = osMessageQueueNew(MMI_CAN_ADAPTER_TX_DEPTH,
                                     sizeof(MmiQueuedFrame_t *),
                                     NULL);
  }
}

static uint8_t QueueFrame(osMemoryPoolId_t pool,
                          osMessageQueueId_t queue,
                          uint16_t standardId,
                          const uint8_t *data,
                          uint8_t length)
{
  MmiQueuedFrame_t *queuedFrame;

  if ((pool == NULL) || (queue == NULL) || (data == NULL) || (length > 8U))
  {
    return 0U;
  }

  queuedFrame = (MmiQueuedFrame_t *) osMemoryPoolAlloc(pool, 0U);
  if (queuedFrame == NULL)
  {
    return 0U;
  }

  (void) memset(queuedFrame, 0, sizeof(*queuedFrame));
  queuedFrame->standardId = standardId;
  queuedFrame->length = length;
  (void) memcpy(&queuedFrame->data[0], data, length);

  if (osMessageQueuePut(queue, &queuedFrame, 0U, 0U) != osOK)
  {
    (void) osMemoryPoolFree(pool, queuedFrame);
    return 0U;
  }

  return 1U;
}

static uint8_t QueueTxFrame(MmiCanAdapterCtx_t *ctx,
                            uint16_t standardId,
                            const uint8_t *data,
                            uint8_t length)
{
  if ((ctx == NULL) || (QueueFrame(ctx->txPool,
                                   ctx->txQueue,
                                   standardId,
                                   data,
                                   length) == 0U))
  {
    if (ctx != NULL)
    {
      ctx->txDrops++;
    }
    return 0U;
  }

  return 1U;
}

static void ResetRxTransfer(MmiCanAdapterCtx_t *ctx)
{
  if (ctx == NULL)
  {
    return;
  }

  ctx->rxActive = 0U;
  ctx->rxMessageClass = MMI_PROTOCOL_V2_MESSAGE_CLASS_NONE;
  ctx->rxSessionId = 0U;
  ctx->rxTransferId = 0U;
  ctx->rxNextSegmentIndex = 0U;
  ctx->rxExpectedLength = 0U;
  (void) memset(&ctx->rxBuffer[0], 0, sizeof(ctx->rxBuffer));
}

static void ResetSubscribeTransfer(MmiCanAdapterCtx_t *ctx)
{
  if (ctx == NULL)
  {
    return;
  }

  ctx->subscribeActive = 0U;
  ctx->subscribeTransferId = 0U;
  ctx->subscribeNextSegmentIndex = 0U;
  (void) memset(&ctx->subscribeBuffer[0], 0, sizeof(ctx->subscribeBuffer));
}

static uint8_t QueueAck(MmiCanAdapterCtx_t *ctx,
                        MmiProtocolMessageClass_t messageClass,
                        uint8_t transferId,
                        MmiProtocolStatus_t status)
{
  MmiProtocolAckV2_t ack;

  (void) memset(&ack, 0, sizeof(ack));
  ack.messageClass = (uint8_t) messageClass;
  ack.transferId = transferId;
  ack.status = (uint8_t) status;

  return QueueTxFrame(ctx,
                      MMI_PROTOCOL_V2_CAN_ID_ACK,
                      (const uint8_t *) &ack,
                      (uint8_t) sizeof(ack));
}

static uint8_t QueueHelloResponse(MmiCanAdapterCtx_t *ctx,
                                  const MmiProtocolHelloRequestV2_t *request)
{
  MmiProtocolHelloResponseV2_t response;
  uint32_t activeGeneration = 0U;

  if ((ctx == NULL) || (request == NULL))
  {
    return 0U;
  }

  ctx->sessionId = (request->requestedSessionId != 0U)
                   ? request->requestedSessionId
                   : 1U;

  (void) memset(&response, 0, sizeof(response));
  response.protocolVersion = MMI_PROTOCOL_V2_VERSION;
  response.controllerRole = MMI_CAN_ADAPTER_CONTROLLER_ROLE_CP;
  response.capabilityFlags = MMI_PROTOCOL_V2_CAPABILITY_RUNTIME;
  if (ctx->service != NULL)
  {
    response.capabilityFlags |= MMI_PROTOCOL_V2_CAPABILITY_STANDARD_OBJECTS;
    response.capabilityFlags |= MMI_PROTOCOL_V2_CAPABILITY_VENDOR_PRIVATE;
  }
  if (ctx->localSettingsService != NULL)
  {
    response.capabilityFlags |= MMI_PROTOCOL_V2_CAPABILITY_LOCAL_SETTINGS;
  }
  if (ctx->eventLogService != NULL)
  {
    response.capabilityFlags |= MMI_PROTOCOL_V2_CAPABILITY_EVENT_LOG;
  }
  if (ctx->maintenanceService != NULL)
  {
    response.capabilityFlags |= MMI_PROTOCOL_V2_CAPABILITY_MAINTENANCE;
  }
  response.capabilityFlags |= MMI_PROTOCOL_V2_CAPABILITY_SUBSCRIPTIONS;
  response.assignedSessionId = ctx->sessionId;

  if ((ctx->service != NULL) && (ctx->service->configurationService != NULL))
  {
    response.activeConfigSetId = ConfigurationServiceGetActiveSetId(
      ctx->service->configurationService);
    activeGeneration = ConfigurationServiceGetActiveGeneration(
      ctx->service->configurationService);
    response.activeGenerationLow = (uint16_t) (activeGeneration & 0xFFFFU);
  }

  if ((ctx->service != NULL) && (ctx->service->cpMpLinkService != NULL))
  {
    response.authorityReady = CpMpLinkServiceAuthorityReady(
      ctx->service->cpMpLinkService);
    response.peerHealthy = CpMpLinkServicePeerHealthy(
      ctx->service->cpMpLinkService);
  }

  return QueueTxFrame(ctx,
                      MMI_PROTOCOL_V2_CAN_ID_HELLO_RSP,
                      (const uint8_t *) &response,
                      (uint8_t) sizeof(response));
}

static uint8_t QueueSegmentedTransfer(MmiCanAdapterCtx_t *ctx,
                                      uint16_t standardId,
                                      uint8_t sessionId,
                                      uint8_t transferId,
                                      const uint8_t *payload,
                                      uint16_t payloadLength)
{
  MmiProtocolSegmentV2_t segment;
  uint16_t offset = 0U;
  uint8_t segmentIndex = 0U;
  uint8_t bytesToCopy;

  if ((ctx == NULL) || ((payload == NULL) && (payloadLength != 0U)))
  {
    return 0U;
  }

  do
  {
    (void) memset(&segment, 0, sizeof(segment));
    segment.sessionId = sessionId;
    segment.transferId = transferId;
    segment.segmentIndex = segmentIndex;
    segment.flags = 0U;
    if (segmentIndex == 0U)
    {
      segment.flags |= MMI_PROTOCOL_V2_SEGMENT_FLAG_FIRST;
    }

    if ((payloadLength - offset) > MMI_PROTOCOL_V2_SEGMENT_DATA_BYTES)
    {
      bytesToCopy = MMI_PROTOCOL_V2_SEGMENT_DATA_BYTES;
    }
    else
    {
      bytesToCopy = (uint8_t) (payloadLength - offset);
      segment.flags |= MMI_PROTOCOL_V2_SEGMENT_FLAG_LAST;
    }

    if ((bytesToCopy > 0U) && (payload != NULL))
    {
      (void) memcpy(&segment.bytes[0], &payload[offset], bytesToCopy);
    }

    if (QueueTxFrame(ctx,
                     standardId,
                     (const uint8_t *) &segment,
                     (uint8_t) sizeof(segment)) == 0U)
    {
      return 0U;
    }

    offset = (uint16_t) (offset + bytesToCopy);
    segmentIndex++;
  } while (offset < payloadLength);

  if (payloadLength == 0U)
  {
    (void) memset(&segment, 0, sizeof(segment));
    segment.sessionId = sessionId;
    segment.transferId = transferId;
    segment.segmentIndex = 0U;
    segment.flags = (uint8_t) (MMI_PROTOCOL_V2_SEGMENT_FLAG_FIRST
                               | MMI_PROTOCOL_V2_SEGMENT_FLAG_LAST);
    return QueueTxFrame(ctx,
                        standardId,
                        (const uint8_t *) &segment,
                        (uint8_t) sizeof(segment));
  }

  return 1U;
}

static uint8_t AllocatePublishTransferId(MmiCanAdapterCtx_t *ctx)
{
  if (ctx == NULL)
  {
    return 1U;
  }

  ctx->publishTransferId++;
  if (ctx->publishTransferId == 0U)
  {
    ctx->publishTransferId = 1U;
  }

  return ctx->publishTransferId;
}

static uint8_t QueueCommandResponse(MmiCanAdapterCtx_t *ctx,
                                    const MmiProtocolCommandHeaderV2_t *request,
                                    MmiProtocolStatus_t status,
                                    const uint8_t *payload,
                                    uint16_t payloadLength)
{
  MmiProtocolResponseHeaderV2_t responseHeader;
  uint8_t transfer[MMI_CAN_ADAPTER_TRANSFER_BUFFER_BYTES];
  uint16_t totalLength;

  if ((ctx == NULL) || (request == NULL))
  {
    return 0U;
  }

  totalLength = (uint16_t) (sizeof(responseHeader) + payloadLength);
  if (totalLength > sizeof(transfer))
  {
    status = MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
    payload = NULL;
    payloadLength = 0U;
    totalLength = (uint16_t) sizeof(responseHeader);
  }

  responseHeader.opcode = request->opcode;
  responseHeader.namespaceId = request->namespaceId;
  responseHeader.resourceId = request->resourceId;
  responseHeader.recordIndex = request->recordIndex;
  responseHeader.transactionId = request->transactionId;
  responseHeader.status = (uint8_t) status;
  responseHeader.payloadLength = payloadLength;

  (void) memset(&transfer[0], 0, sizeof(transfer));
  (void) memcpy(&transfer[0], &responseHeader, sizeof(responseHeader));
  if ((payloadLength > 0U) && (payload != NULL))
  {
    (void) memcpy(&transfer[sizeof(responseHeader)], payload, payloadLength);
  }

  return QueueSegmentedTransfer(ctx,
                                MMI_PROTOCOL_V2_CAN_ID_RESPONSE_SEG,
                                ctx->sessionId,
                                ctx->rxTransferId,
                                &transfer[0],
                                totalLength);
}

static MmiProtocolStatus_t MapNtcipError(NtcipError_t error)
{
  switch (error)
  {
      case NTCIP_ERROR_OK:
      {
        return MMI_PROTOCOL_V2_STATUS_OK;
      }

      case NTCIP_ERROR_NOT_FOUND:
      {
        return MMI_PROTOCOL_V2_STATUS_NOT_FOUND;
      }

      case NTCIP_ERROR_READ_ONLY:
      {
        return MMI_PROTOCOL_V2_STATUS_NOT_WRITABLE;
      }

      case NTCIP_ERROR_NO_ACCESS:
      {
        return MMI_PROTOCOL_V2_STATUS_BUSY;
      }

      case NTCIP_ERROR_BAD_VALUE:
      case NTCIP_ERROR_RANGE_ERROR:
      {
        return MMI_PROTOCOL_V2_STATUS_INVALID_VALUE;
      }

      case NTCIP_ERROR_NO_TRANSACTION:
      case NTCIP_ERROR_OWNER_MISMATCH:
      case NTCIP_ERROR_TRANSACTION_ID_MISMATCH:
      {
        return MMI_PROTOCOL_V2_STATUS_TRANSACTION_REQUIRED;
      }

      case NTCIP_ERROR_GEN_ERROR:
      case NTCIP_ERROR_COMMIT_FAILED:
      default:
      {
        return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
      }
  }
}

static uint16_t StandardObjectPayloadMinLength(void)
{
  return (uint16_t) sizeof(MmiProtocolObjectPrefixV2_t);
}

static uint8_t DecodeNtcipValue(const MmiProtocolObjectPrefixV2_t *prefix,
                                const uint8_t *encodedValue,
                                NtcipValue_t *value)
{
  uint8_t index;

  if ((prefix == NULL) || (value == NULL)
      || ((prefix->valueLength != 0U) && (encodedValue == NULL)))
  {
    return 0U;
  }

  switch ((NtcipValueType_t) prefix->valueEncoding)
  {
      case NTCIP_VALUE_TYPE_UNSIGNED32:
      {
        if (prefix->valueLength != 4U)
        {
          return 0U;
        }

        NtcipValueSetUnsigned32(value, ReadLe32(encodedValue));
        return 1U;
      }

      case NTCIP_VALUE_TYPE_SIGNED32:
      {
        if (prefix->valueLength != 4U)
        {
          return 0U;
        }

        NtcipValueSetSigned32(value, (int32_t) ReadLe32(encodedValue));
        return 1U;
      }

      case NTCIP_VALUE_TYPE_OBJECT_ID:
      {
        uint32_t oid[NTCIP_OID_MAX_LENGTH];

        if (((prefix->valueLength % 4U) != 0U)
            || ((prefix->valueLength / 4U) > NTCIP_OID_MAX_LENGTH))
        {
          return 0U;
        }

        for (index = 0U; index < (prefix->valueLength / 4U); index++)
        {
          oid[index] = ReadLe32(&encodedValue[(uint16_t) index * 4U]);
        }

        return (uint8_t) (NtcipValueSetObjectId(value,
                                                &oid[0],
                                                (uint8_t) (prefix->valueLength
                                                           / 4U))
                          == NTCIP_ERROR_OK);
      }

      case NTCIP_VALUE_TYPE_OCTET_STRING:
      {
        return (uint8_t) (NtcipValueSetOctetString(value,
                                                   encodedValue,
                                                   prefix->valueLength)
                          == NTCIP_ERROR_OK);
      }

      default:
      {
        return 0U;
      }
  }
}

static uint8_t EncodeNtcipValue(const NtcipValue_t *value,
                                uint8_t *encodedValue,
                                uint16_t *encodedLength,
                                uint8_t *encoding)
{
  uint8_t index;

  if ((value == NULL) || (encodedLength == NULL) || (encoding == NULL))
  {
    return 0U;
  }

  *encodedLength = 0U;
  *encoding = (uint8_t) value->type;

  switch (value->type)
  {
      case NTCIP_VALUE_TYPE_UNSIGNED32:
      {
        if (encodedValue == NULL)
        {
          return 0U;
        }

        WriteLe32(encodedValue, value->data.unsigned32);
        *encodedLength = 4U;
        return 1U;
      }

      case NTCIP_VALUE_TYPE_SIGNED32:
      {
        if (encodedValue == NULL)
        {
          return 0U;
        }

        WriteLe32(encodedValue, (uint32_t) value->data.signed32);
        *encodedLength = 4U;
        return 1U;
      }

      case NTCIP_VALUE_TYPE_OBJECT_ID:
      {
        if ((encodedValue == NULL)
            || (((uint16_t) value->data.objectId.length * 4U)
                > MMI_CAN_ADAPTER_TRANSFER_BUFFER_BYTES))
        {
          return 0U;
        }

        for (index = 0U; index < value->data.objectId.length; index++)
        {
          WriteLe32(&encodedValue[(uint16_t) index * 4U],
                    value->data.objectId.elements[index]);
        }

        *encodedLength = (uint16_t) value->data.objectId.length * 4U;
        return 1U;
      }

      case NTCIP_VALUE_TYPE_OCTET_STRING:
      {
        if ((encodedValue == NULL)
            || (value->data.octetString.length > MMI_CAN_ADAPTER_TRANSFER_BUFFER_BYTES))
        {
          return 0U;
        }

        if (value->data.octetString.length > 0U)
        {
          (void) memcpy(encodedValue,
                        &value->data.octetString.bytes[0],
                        value->data.octetString.length);
        }

        *encodedLength = value->data.octetString.length;
        return 1U;
      }

      default:
      {
        return 0U;
      }
  }
}

static uint8_t ParseStandardObjectPayload(const uint8_t *payload,
                                          uint16_t payloadLength,
                                          MmiProtocolObjectPrefixV2_t *prefix,
                                          uint32_t *oid,
                                          uint8_t *oidLength,
                                          const uint8_t **valueBytes)
{
  uint16_t minimumLength;
  uint16_t oidBytesLength;

  if ((payload == NULL) || (prefix == NULL) || (oid == NULL)
      || (oidLength == NULL) || (valueBytes == NULL))
  {
    return 0U;
  }

  minimumLength = StandardObjectPayloadMinLength();
  if (payloadLength < minimumLength)
  {
    return 0U;
  }

  (void) memcpy(prefix, payload, sizeof(*prefix));
  if (prefix->oidLength > NTCIP_OID_MAX_LENGTH)
  {
    return 0U;
  }

  oidBytesLength = (uint16_t) prefix->oidLength * 4U;
  if (payloadLength != (uint16_t) (minimumLength
                                   + oidBytesLength
                                   + prefix->valueLength))
  {
    return 0U;
  }

  for (*oidLength = 0U; *oidLength < prefix->oidLength; (*oidLength)++)
  {
    oid[*oidLength] = ReadLe32(&payload[minimumLength
                                        + ((uint16_t) *oidLength * 4U)]);
  }

  *valueBytes = &payload[minimumLength + oidBytesLength];
  return 1U;
}

static uint8_t BuildStandardObjectPayload(const uint32_t *oid,
                                          uint8_t oidLength,
                                          const NtcipValue_t *value,
                                          uint8_t *payload,
                                          uint16_t *payloadLength)
{
  MmiProtocolObjectPrefixV2_t prefix;
  uint16_t offset;
  uint16_t valueLength;
  uint8_t valueEncoding;
  uint8_t encodedValue[MMI_CAN_ADAPTER_TRANSFER_BUFFER_BYTES];
  uint8_t index;

  if ((oid == NULL) || (value == NULL) || (payload == NULL)
      || (payloadLength == NULL) || (oidLength > NTCIP_OID_MAX_LENGTH))
  {
    return 0U;
  }

  if (EncodeNtcipValue(value,
                       &encodedValue[0],
                       &valueLength,
                       &valueEncoding) == 0U)
  {
    return 0U;
  }

  offset = StandardObjectPayloadMinLength();
  if ((uint16_t) (offset + ((uint16_t) oidLength * 4U) + valueLength)
      > MMI_CAN_ADAPTER_TRANSFER_BUFFER_BYTES)
  {
    return 0U;
  }

  prefix.oidLength = oidLength;
  prefix.valueEncoding = valueEncoding;
  prefix.valueLength = valueLength;
  (void) memcpy(payload, &prefix, sizeof(prefix));

  for (index = 0U; index < oidLength; index++)
  {
    WriteLe32(&payload[offset + ((uint16_t) index * 4U)], oid[index]);
  }

  offset = (uint16_t) (offset + ((uint16_t) oidLength * 4U));
  if (valueLength > 0U)
  {
    (void) memcpy(&payload[offset], &encodedValue[0], valueLength);
  }

  *payloadLength = (uint16_t) (offset + valueLength);
  return 1U;
}

static void BuildRequestContext(MmiCanAdapterCtx_t *ctx,
                                uint8_t transactionId,
                                NtcipRequestContext_t *requestContext)
{
  if ((ctx == NULL) || (requestContext == NULL))
  {
    return;
  }

  LWIPSNMPAdapterBuildRequestContext(&ctx->ntcipAdapter,
                                     ctx->sessionId,
                                     requestContext);

  if (transactionId != 0U)
  {
    requestContext->transactionIdValid = 1U;
    requestContext->transactionId = transactionId;
  }
}

static MmiProtocolStatus_t StandardTransactionControl(
  MmiCanAdapterCtx_t *ctx,
  MmiProtocolOpcode_t opcode,
  uint8_t transactionId)
{
  NtcipRequestContext_t requestContext;
  NtcipValue_t value;
  NtcipError_t error;

  if ((ctx == NULL) || (transactionId == 0U))
  {
    return MMI_PROTOCOL_V2_STATUS_INVALID_VALUE;
  }

  BuildRequestContext(ctx, transactionId, &requestContext);

  switch (opcode)
  {
      case MMI_PROTOCOL_V2_OPCODE_BEGIN_TRANSACTION:
      {
        NtcipValueSetUnsigned32(&value,
                                (uint32_t) NTCIP_DB_CREATE_STATE_TRANSACTION);
        error = LWIPSNMPAdapterSetValue(&ctx->ntcipAdapter,
                                        &kDbCreateTransactionOid[0],
                                        13U,
                                        &requestContext,
                                        &value);
        if (error != NTCIP_ERROR_OK)
        {
          return MapNtcipError(error);
        }

        NtcipValueSetUnsigned32(&value, transactionId);
        error = LWIPSNMPAdapterSetValue(&ctx->ntcipAdapter,
                                        &kDbTransactionIdOid[0],
                                        13U,
                                        &requestContext,
                                        &value);
        return MapNtcipError(error);
      }

      case MMI_PROTOCOL_V2_OPCODE_VERIFY:
      {
        NtcipValueSetUnsigned32(&value, (uint32_t) NTCIP_DB_CREATE_STATE_VERIFY);
        error = LWIPSNMPAdapterSetValue(&ctx->ntcipAdapter,
                                        &kDbCreateTransactionOid[0],
                                        13U,
                                        &requestContext,
                                        &value);
        return MapNtcipError(error);
      }

      case MMI_PROTOCOL_V2_OPCODE_COMMIT:
      case MMI_PROTOCOL_V2_OPCODE_ROLLBACK:
      {
        NtcipValueSetUnsigned32(&value, (uint32_t) NTCIP_DB_CREATE_STATE_NORMAL);
        error = LWIPSNMPAdapterSetValue(&ctx->ntcipAdapter,
                                        &kDbCreateTransactionOid[0],
                                        13U,
                                        &requestContext,
                                        &value);
        return MapNtcipError(error);
      }

      default:
      {
        return MMI_PROTOCOL_V2_STATUS_UNSUPPORTED;
      }
  }
}

static MmiProtocolStatus_t HandleStandardObjectCommand(
  MmiCanAdapterCtx_t *ctx,
  const MmiProtocolCommandHeaderV2_t *request,
  const uint8_t *payload,
  uint16_t payloadLength,
  uint8_t *responsePayload,
  uint16_t *responsePayloadLength)
{
  MmiProtocolObjectPrefixV2_t prefix;
  NtcipRequestContext_t requestContext;
  NtcipValue_t value;
  NtcipError_t error;
  uint32_t oid[NTCIP_OID_MAX_LENGTH];
  uint8_t oidLength = 0U;
  const uint8_t *valueBytes = NULL;

  if ((ctx == NULL) || (request == NULL) || (responsePayload == NULL)
      || (responsePayloadLength == NULL))
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }

  switch ((MmiProtocolOpcode_t) request->opcode)
  {
      case MMI_PROTOCOL_V2_OPCODE_BEGIN_TRANSACTION:
      case MMI_PROTOCOL_V2_OPCODE_VERIFY:
      case MMI_PROTOCOL_V2_OPCODE_COMMIT:
      case MMI_PROTOCOL_V2_OPCODE_ROLLBACK:
      {
        if (payloadLength != 0U)
        {
          return MMI_PROTOCOL_V2_STATUS_INVALID_VALUE;
        }

        return StandardTransactionControl(ctx,
                                          (MmiProtocolOpcode_t) request->opcode,
                                          request->transactionId);
      }

      case MMI_PROTOCOL_V2_OPCODE_GET:
      {
        if (ParseStandardObjectPayload(payload,
                                       payloadLength,
                                       &prefix,
                                       &oid[0],
                                       &oidLength,
                                       &valueBytes) == 0U)
        {
          return MMI_PROTOCOL_V2_STATUS_INVALID_VALUE;
        }

        BuildRequestContext(ctx, request->transactionId, &requestContext);
        error = LWIPSNMPAdapterGet(&ctx->ntcipAdapter,
                                   &oid[0],
                                   oidLength,
                                   &requestContext,
                                   &value);
        if (error != NTCIP_ERROR_OK)
        {
          return MapNtcipError(error);
        }

        if (BuildStandardObjectPayload(&oid[0],
                                       oidLength,
                                       &value,
                                       responsePayload,
                                       responsePayloadLength) == 0U)
        {
          return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
        }

        return MMI_PROTOCOL_V2_STATUS_OK;
      }

      case MMI_PROTOCOL_V2_OPCODE_SET:
      {
        if (ParseStandardObjectPayload(payload,
                                       payloadLength,
                                       &prefix,
                                       &oid[0],
                                       &oidLength,
                                       &valueBytes) == 0U)
        {
          return MMI_PROTOCOL_V2_STATUS_INVALID_VALUE;
        }

        if (DecodeNtcipValue(&prefix, valueBytes, &value) == 0U)
        {
          return MMI_PROTOCOL_V2_STATUS_INVALID_VALUE;
        }

        BuildRequestContext(ctx, request->transactionId, &requestContext);
        error = LWIPSNMPAdapterSetTest(&ctx->ntcipAdapter,
                                       &oid[0],
                                       oidLength,
                                       &requestContext,
                                       &value);
        if (error != NTCIP_ERROR_OK)
        {
          return MapNtcipError(error);
        }

        error = LWIPSNMPAdapterSetValue(&ctx->ntcipAdapter,
                                        &oid[0],
                                        oidLength,
                                        &requestContext,
                                        &value);
        return MapNtcipError(error);
      }

      default:
      {
        return MMI_PROTOCOL_V2_STATUS_UNSUPPORTED;
      }
  }
}

static MmiProtocolStatus_t HandleLocalSettingsCommand(
  MmiCanAdapterCtx_t *ctx,
  const MmiProtocolCommandHeaderV2_t *request,
  const uint8_t *payload,
  uint16_t payloadLength,
  uint8_t *responsePayload,
  uint16_t *responsePayloadLength)
{
  if ((ctx == NULL) || (request == NULL) || (responsePayload == NULL)
      || (responsePayloadLength == NULL))
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }

  if (ctx->localSettingsService == NULL)
  {
    return MMI_PROTOCOL_V2_STATUS_UNSUPPORTED;
  }

  switch ((MmiProtocolOpcode_t) request->opcode)
  {
      case MMI_PROTOCOL_V2_OPCODE_GET:
      {
        if (payloadLength != 0U)
        {
          return MMI_PROTOCOL_V2_STATUS_INVALID_VALUE;
        }

        return MmiLocalSettingsServiceRead(ctx->localSettingsService,
                                           request->resourceId,
                                           responsePayload,
                                           responsePayloadLength);
      }

      case MMI_PROTOCOL_V2_OPCODE_SET:
      {
        return MmiLocalSettingsServiceWrite(ctx->localSettingsService,
                                            request->resourceId,
                                            payload,
                                            payloadLength);
      }

      default:
      {
        return MMI_PROTOCOL_V2_STATUS_UNSUPPORTED;
      }
  }
}

static MmiProtocolStatus_t HandleEventLogCommand(
  MmiCanAdapterCtx_t *ctx,
  const MmiProtocolCommandHeaderV2_t *request,
  const uint8_t *payload,
  uint16_t payloadLength,
  uint8_t *responsePayload,
  uint16_t *responsePayloadLength)
{
  if ((ctx == NULL) || (request == NULL) || (responsePayload == NULL)
      || (responsePayloadLength == NULL))
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }

  if (ctx->eventLogService == NULL)
  {
    return MMI_PROTOCOL_V2_STATUS_UNSUPPORTED;
  }

  if (request->opcode != MMI_PROTOCOL_V2_OPCODE_GET)
  {
    return MMI_PROTOCOL_V2_STATUS_UNSUPPORTED;
  }

  return MmiEventLogServiceRead(ctx->eventLogService,
                                request->resourceId,
                                payload,
                                payloadLength,
                                responsePayload,
                                MMI_CAN_ADAPTER_TRANSFER_BUFFER_BYTES,
                                responsePayloadLength);
}

static MmiProtocolStatus_t HandleMaintenanceCommand(
  MmiCanAdapterCtx_t *ctx,
  const MmiProtocolCommandHeaderV2_t *request,
  const uint8_t *payload,
  uint16_t payloadLength)
{
  if ((ctx == NULL) || (request == NULL) || (ctx->maintenanceService == NULL))
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }

  if ((request->opcode != MMI_PROTOCOL_V2_OPCODE_COMMAND)
      && (request->opcode != MMI_PROTOCOL_V2_OPCODE_SET))
  {
    return MMI_PROTOCOL_V2_STATUS_UNSUPPORTED;
  }

  return MmiMaintenanceServiceExecute(ctx->maintenanceService,
                                      request->resourceId,
                                      payload,
                                      payloadLength);
}

static uint8_t IsKnownNamespace(uint8_t namespaceId)
{
  switch (namespaceId)
  {
      case MMI_PROTOCOL_V2_NAMESPACE_RUNTIME:
      case MMI_PROTOCOL_V2_NAMESPACE_STANDARD_OBJECT:
      case MMI_PROTOCOL_V2_NAMESPACE_LOCAL_SETTINGS:
      case MMI_PROTOCOL_V2_NAMESPACE_VENDOR_PRIVATE:
      case MMI_PROTOCOL_V2_NAMESPACE_EVENT_LOG:
      case MMI_PROTOCOL_V2_NAMESPACE_MAINTENANCE:
      {
        return 1U;
      }

      default:
      {
        return 0U;
      }
  }
}

static MmiProtocolStatus_t NormalizeRecordIndex(
  const MmiResourceDescriptor_t *descriptor,
  uint16_t recordCount,
  uint8_t *recordIndex)
{
  if ((descriptor == NULL) || (recordIndex == NULL))
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }

  if (descriptor->countSource == MMI_RESOURCE_COUNT_SINGLE)
  {
    if ((*recordIndex != 0U) && (*recordIndex != 1U))
    {
      return MMI_PROTOCOL_V2_STATUS_BAD_INDEX;
    }

    *recordIndex = 0U;
    return MMI_PROTOCOL_V2_STATUS_OK;
  }

  if ((*recordIndex == 0U) || ((uint16_t) *recordIndex > recordCount))
  {
    return MMI_PROTOCOL_V2_STATUS_BAD_INDEX;
  }

  return MMI_PROTOCOL_V2_STATUS_OK;
}

static MmiProtocolStatus_t BuildRuntimePayload(
  const MmiProtocolCommandHeaderV2_t *request,
  uint8_t *payload,
  uint16_t *payloadLength,
  const MmiResourceDescriptor_t *descriptor,
  MmiSnapshotCache_t *snapshotCache)
{
  uint8_t recordIndex;

  if ((request == NULL) || (payload == NULL) || (payloadLength == NULL)
      || (descriptor == NULL) || (snapshotCache == NULL))
  {
    return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
  }

  recordIndex = request->recordIndex;

  switch (request->resourceId)
  {
      case MMI_PROTOCOL_V2_RUNTIME_TOPIC_SUMMARY:
      {
        MmiRuntimeSummaryV2_t record;

        if (MmiSnapshotCacheGetSummary(snapshotCache, &record) == 0U)
        {
          return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
        }

        *payloadLength = (uint16_t) sizeof(record);
        (void) memcpy(payload, &record, sizeof(record));
        break;
      }

      case MMI_PROTOCOL_V2_RUNTIME_TOPIC_RINGS:
      {
        MmiRuntimeRingRecordV2_t record;

        if (MmiSnapshotCacheGetRingRecord(snapshotCache,
                                          recordIndex,
                                          &record) == 0U)
        {
          return MMI_PROTOCOL_V2_STATUS_NOT_FOUND;
        }

        *payloadLength = (uint16_t) sizeof(record);
        (void) memcpy(payload, &record, sizeof(record));
        break;
      }

      case MMI_PROTOCOL_V2_RUNTIME_TOPIC_PHASES:
      {
        MmiRuntimePhaseRecordV2_t record;

        if (MmiSnapshotCacheGetPhaseRecord(snapshotCache,
                                           recordIndex,
                                           &record) == 0U)
        {
          return MMI_PROTOCOL_V2_STATUS_NOT_FOUND;
        }

        *payloadLength = (uint16_t) sizeof(record);
        (void) memcpy(payload, &record, sizeof(record));
        break;
      }

      case MMI_PROTOCOL_V2_RUNTIME_TOPIC_CHANNELS:
      {
        MmiRuntimeChannelRecordV2_t record;

        if (MmiSnapshotCacheGetChannelRecord(snapshotCache,
                                             recordIndex,
                                             &record) == 0U)
        {
          return MMI_PROTOCOL_V2_STATUS_NOT_FOUND;
        }

        *payloadLength = (uint16_t) sizeof(record);
        (void) memcpy(payload, &record, sizeof(record));
        break;
      }

      case MMI_PROTOCOL_V2_RUNTIME_TOPIC_OVERLAPS:
      {
        MmiRuntimeOverlapRecordV2_t record;

        if (MmiSnapshotCacheGetOverlapRecord(snapshotCache,
                                             recordIndex,
                                             &record) == 0U)
        {
          return MMI_PROTOCOL_V2_STATUS_NOT_FOUND;
        }

        *payloadLength = (uint16_t) sizeof(record);
        (void) memcpy(payload, &record, sizeof(record));
        break;
      }

      case MMI_PROTOCOL_V2_RUNTIME_TOPIC_RAW_INPUTS:
      {
        MmiRuntimeRawInputsV2_t record;

        if (MmiSnapshotCacheGetRawInputs(snapshotCache, &record) == 0U)
        {
          return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
        }

        *payloadLength = (uint16_t) sizeof(record);
        (void) memcpy(payload, &record, sizeof(record));
        break;
      }

      case MMI_PROTOCOL_V2_RUNTIME_TOPIC_VEHICLE_DETECTORS:
      {
        MmiRuntimeVehicleDetectorRecordV2_t record;

        if (MmiSnapshotCacheGetVehicleDetectorRecord(snapshotCache,
                                                     recordIndex,
                                                     &record) == 0U)
        {
          return MMI_PROTOCOL_V2_STATUS_NOT_FOUND;
        }

        *payloadLength = (uint16_t) sizeof(record);
        (void) memcpy(payload, &record, sizeof(record));
        break;
      }

      case MMI_PROTOCOL_V2_RUNTIME_TOPIC_PEDESTRIAN_DETECTORS:
      {
        MmiRuntimePedestrianDetectorRecordV2_t record;

        if (MmiSnapshotCacheGetPedestrianDetectorRecord(snapshotCache,
                                                        recordIndex,
                                                        &record) == 0U)
        {
          return MMI_PROTOCOL_V2_STATUS_NOT_FOUND;
        }

        *payloadLength = (uint16_t) sizeof(record);
        (void) memcpy(payload, &record, sizeof(record));
        break;
      }

      case MMI_PROTOCOL_V2_RUNTIME_TOPIC_MODULE_STATUS:
      {
        MmiRuntimeModuleStatusV2_t record;

        if (MmiSnapshotCacheGetModuleStatus(snapshotCache, &record) == 0U)
        {
          return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
        }

        *payloadLength = (uint16_t) sizeof(record);
        (void) memcpy(payload, &record, sizeof(record));
        break;
      }

      case MMI_PROTOCOL_V2_RUNTIME_TOPIC_SAFETY_SUMMARY:
      {
        MmiRuntimeSafetySummaryV2_t record;

        if (MmiSnapshotCacheGetSafetySummary(snapshotCache, &record) == 0U)
        {
          return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
        }

        *payloadLength = (uint16_t) sizeof(record);
        (void) memcpy(payload, &record, sizeof(record));
        break;
      }

      case MMI_PROTOCOL_V2_RUNTIME_TOPIC_SAFETY_CHANNELS:
      {
        MmiRuntimeSafetyChannelRecordV2_t record;

        if (MmiSnapshotCacheGetSafetyChannelRecord(snapshotCache,
                                                   recordIndex,
                                                   &record) == 0U)
        {
          return MMI_PROTOCOL_V2_STATUS_NOT_FOUND;
        }

        *payloadLength = (uint16_t) sizeof(record);
        (void) memcpy(payload, &record, sizeof(record));
        break;
      }

      case MMI_PROTOCOL_V2_RUNTIME_TOPIC_CLOCK:
      {
        MmiRuntimeClockSummaryV2_t record;

        if (MmiSnapshotCacheGetClockSummary(snapshotCache, &record) == 0U)
        {
          return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
        }

        *payloadLength = (uint16_t) sizeof(record);
        (void) memcpy(payload, &record, sizeof(record));
        break;
      }

      case MMI_PROTOCOL_V2_RUNTIME_TOPIC_POWER:
      {
        MmiRuntimePowerSummaryV2_t record;

        if (MmiSnapshotCacheGetPowerSummary(snapshotCache, &record) == 0U)
        {
          return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
        }

        *payloadLength = (uint16_t) sizeof(record);
        (void) memcpy(payload, &record, sizeof(record));
        break;
      }

      case MMI_PROTOCOL_V2_RUNTIME_TOPIC_COMMS:
      {
        MmiRuntimeCommsSummaryV2_t record;

        if (MmiSnapshotCacheGetCommsSummary(snapshotCache, &record) == 0U)
        {
          return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
        }

        *payloadLength = (uint16_t) sizeof(record);
        (void) memcpy(payload, &record, sizeof(record));
        break;
      }

      case MMI_PROTOCOL_V2_RUNTIME_TOPIC_RELAY:
      {
        MmiRuntimeRelaySummaryV2_t record;

        if (MmiSnapshotCacheGetRelaySummary(snapshotCache, &record) == 0U)
        {
          return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
        }

        *payloadLength = (uint16_t) sizeof(record);
        (void) memcpy(payload, &record, sizeof(record));
        break;
      }

      case MMI_PROTOCOL_V2_RUNTIME_TOPIC_OUTPUT_TEST:
      {
        MmiRuntimeOutputTestSummaryV2_t record;

        if (MmiSnapshotCacheGetOutputTestSummary(snapshotCache, &record) == 0U)
        {
          return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
        }

        *payloadLength = (uint16_t) sizeof(record);
        (void) memcpy(payload, &record, sizeof(record));
        break;
      }

      case MMI_PROTOCOL_V2_RUNTIME_TOPIC_DOOR:
      {
        MmiRuntimeDoorSummaryV2_t record;

        if (MmiSnapshotCacheGetDoorSummary(snapshotCache, &record) == 0U)
        {
          return MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR;
        }

        *payloadLength = (uint16_t) sizeof(record);
        (void) memcpy(payload, &record, sizeof(record));
        break;
      }

      default:
      {
        (void) descriptor;
        return MMI_PROTOCOL_V2_STATUS_UNSUPPORTED;
      }
  }

  return MMI_PROTOCOL_V2_STATUS_OK;
}

static uint8_t QueueRuntimePublish(MmiCanAdapterCtx_t *ctx,
                                   uint8_t topicId,
                                   uint8_t recordIndex,
                                   uint8_t sequence)
{
  MmiProtocolCommandHeaderV2_t request;
  MmiProtocolPublishHeaderV2_t publishHeader;
  MmiResourceDescriptor_t descriptor;
  uint16_t recordCount = 0U;
  uint8_t payload[MMI_CAN_ADAPTER_TRANSFER_BUFFER_BYTES];
  uint16_t payloadLength = 0U;
  uint8_t transfer[MMI_CAN_ADAPTER_TRANSFER_BUFFER_BYTES];
  uint16_t totalLength;
  MmiProtocolStatus_t status;

  if ((ctx == NULL) || (ctx->service == NULL) || (ctx->snapshotCache == NULL))
  {
    return 0U;
  }

  if (MmiServiceLookupResource(ctx->service,
                               MMI_PROTOCOL_V2_NAMESPACE_RUNTIME,
                               topicId,
                               &descriptor) == 0U)
  {
    return 0U;
  }

  if (MmiServiceResolveRecordCount(ctx->service, &descriptor, &recordCount) == 0U)
  {
    return 0U;
  }

  (void) memset(&request, 0, sizeof(request));
  request.namespaceId = MMI_PROTOCOL_V2_NAMESPACE_RUNTIME;
  request.resourceId = topicId;
  request.recordIndex = recordIndex;
  status = NormalizeRecordIndex(&descriptor, recordCount, &request.recordIndex);
  if (status != MMI_PROTOCOL_V2_STATUS_OK)
  {
    return 0U;
  }

  status = BuildRuntimePayload(&request,
                               &payload[0],
                               &payloadLength,
                               &descriptor,
                               ctx->snapshotCache);
  if (status != MMI_PROTOCOL_V2_STATUS_OK)
  {
    return 0U;
  }

  (void) memset(&publishHeader, 0, sizeof(publishHeader));
  publishHeader.topicId = topicId;
  publishHeader.recordIndex = request.recordIndex;
  publishHeader.sequence = sequence;
  publishHeader.payloadLength = payloadLength;

  totalLength = (uint16_t) (sizeof(publishHeader) + payloadLength);
  if (totalLength > sizeof(transfer))
  {
    return 0U;
  }

  (void) memcpy(&transfer[0], &publishHeader, sizeof(publishHeader));
  if (payloadLength > 0U)
  {
    (void) memcpy(&transfer[sizeof(publishHeader)], &payload[0], payloadLength);
  }

  return QueueSegmentedTransfer(ctx,
                                MMI_PROTOCOL_V2_CAN_ID_PUBLISH_SEG,
                                ctx->sessionId,
                                AllocatePublishTransferId(ctx),
                                &transfer[0],
                                totalLength);
}

static void MarkSubscriptionsDirty(MmiCanAdapterCtx_t *ctx,
                                   uint8_t topicId,
                                   uint8_t recordIndex)
{
  uint8_t index;

  if (ctx == NULL)
  {
    return;
  }

  for (index = 0U; index < MMI_CAN_ADAPTER_SUBSCRIPTION_MAX; index++)
  {
    if ((ctx->subscriptions[index].active != 0U)
        && (ctx->subscriptions[index].topicId == topicId)
        && ((ctx->subscriptions[index].recordIndex == recordIndex)
            || (ctx->subscriptions[index].recordIndex == 0U)
            || (recordIndex == 0U)))
    {
      ctx->subscriptions[index].dirty = 1U;
    }
  }
}

static void ProcessCommandTransfer(MmiCanAdapterCtx_t *ctx)
{
  MmiProtocolCommandHeaderV2_t request;
  MmiResourceDescriptor_t descriptor;
  MmiProtocolStatus_t status;
  uint16_t recordCount = 0U;
  uint8_t normalizedIndex;
  uint8_t payload[MMI_CAN_ADAPTER_TRANSFER_BUFFER_BYTES];
  uint16_t payloadLength = 0U;
  const uint8_t *requestPayload;

  if ((ctx == NULL) || (ctx->service == NULL) || (ctx->snapshotCache == NULL))
  {
    ResetRxTransfer(ctx);
    return;
  }

  (void) memset(&request, 0, sizeof(request));
  (void) memcpy(&request, &ctx->rxBuffer[0], sizeof(request));

  if ((uint16_t) (sizeof(request) + request.payloadLength) != ctx->rxExpectedLength)
  {
    (void) QueueCommandResponse(ctx,
                                &request,
                                MMI_PROTOCOL_V2_STATUS_INVALID_VALUE,
                                NULL,
                                0U);
    ResetRxTransfer(ctx);
    return;
  }

  requestPayload = &ctx->rxBuffer[sizeof(request)];

  if (IsKnownNamespace(request.namespaceId) == 0U)
  {
    (void) QueueCommandResponse(ctx,
                                &request,
                                MMI_PROTOCOL_V2_STATUS_BAD_NAMESPACE,
                                NULL,
                                0U);
    ResetRxTransfer(ctx);
    return;
  }

  if (MmiServiceLookupResource(ctx->service,
                               request.namespaceId,
                               request.resourceId,
                               &descriptor) == 0U)
  {
    (void) QueueCommandResponse(ctx,
                                &request,
                                MMI_PROTOCOL_V2_STATUS_BAD_RESOURCE,
                                NULL,
                                0U);
    ResetRxTransfer(ctx);
    return;
  }

  if (request.namespaceId != MMI_PROTOCOL_V2_NAMESPACE_RUNTIME)
  {
    if (request.namespaceId == MMI_PROTOCOL_V2_NAMESPACE_STANDARD_OBJECT)
    {
      status = HandleStandardObjectCommand(ctx,
                                           &request,
                                           requestPayload,
                                           request.payloadLength,
                                           &payload[0],
                                           &payloadLength);
      (void) QueueCommandResponse(ctx,
                                  &request,
                                  status,
                                  &payload[0],
                                  (status == MMI_PROTOCOL_V2_STATUS_OK)
                                  ? payloadLength
                                  : 0U);
      ResetRxTransfer(ctx);
      return;
    }

    if (request.namespaceId == MMI_PROTOCOL_V2_NAMESPACE_LOCAL_SETTINGS)
    {
      if (MmiServiceResolveRecordCount(ctx->service, &descriptor, &recordCount)
          == 0U)
      {
        (void) QueueCommandResponse(ctx,
                                    &request,
                                    MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR,
                                    NULL,
                                    0U);
        ResetRxTransfer(ctx);
        return;
      }

      normalizedIndex = request.recordIndex;
      status = NormalizeRecordIndex(&descriptor, recordCount, &normalizedIndex);
      if (status != MMI_PROTOCOL_V2_STATUS_OK)
      {
        (void) QueueCommandResponse(ctx, &request, status, NULL, 0U);
        ResetRxTransfer(ctx);
        return;
      }

      request.recordIndex = normalizedIndex;
      status = HandleLocalSettingsCommand(ctx,
                                          &request,
                                          requestPayload,
                                          request.payloadLength,
                                          &payload[0],
                                          &payloadLength);
      (void) QueueCommandResponse(ctx,
                                  &request,
                                  status,
                                  &payload[0],
                                  (status == MMI_PROTOCOL_V2_STATUS_OK)
                                  ? payloadLength
                                  : 0U);
      ResetRxTransfer(ctx);
      return;
    }

    if (request.namespaceId == MMI_PROTOCOL_V2_NAMESPACE_EVENT_LOG)
    {
      status = HandleEventLogCommand(ctx,
                                     &request,
                                     requestPayload,
                                     request.payloadLength,
                                     &payload[0],
                                     &payloadLength);
      (void) QueueCommandResponse(ctx,
                                  &request,
                                  status,
                                  &payload[0],
                                  (status == MMI_PROTOCOL_V2_STATUS_OK)
                                  ? payloadLength
                                  : 0U);
      ResetRxTransfer(ctx);
      return;
    }

    if (request.namespaceId == MMI_PROTOCOL_V2_NAMESPACE_MAINTENANCE)
    {
      status = HandleMaintenanceCommand(ctx,
                                        &request,
                                        requestPayload,
                                        request.payloadLength);
      (void) QueueCommandResponse(ctx,
                                  &request,
                                  status,
                                  NULL,
                                  0U);
      ResetRxTransfer(ctx);
      return;
    }

    (void) QueueCommandResponse(ctx,
                                &request,
                                MMI_PROTOCOL_V2_STATUS_UNSUPPORTED,
                                NULL,
                                0U);
    ResetRxTransfer(ctx);
    return;
  }

  if ((request.opcode != MMI_PROTOCOL_V2_OPCODE_GET)
      || (request.payloadLength != 0U))
  {
    (void) QueueCommandResponse(ctx,
                                &request,
                                MMI_PROTOCOL_V2_STATUS_UNSUPPORTED,
                                NULL,
                                0U);
    ResetRxTransfer(ctx);
    return;
  }

  if (MmiServiceResolveRecordCount(ctx->service, &descriptor, &recordCount) == 0U)
  {
    (void) QueueCommandResponse(ctx,
                                &request,
                                MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR,
                                NULL,
                                0U);
    ResetRxTransfer(ctx);
    return;
  }

  normalizedIndex = request.recordIndex;
  status = NormalizeRecordIndex(&descriptor, recordCount, &normalizedIndex);
  if (status != MMI_PROTOCOL_V2_STATUS_OK)
  {
    (void) QueueCommandResponse(ctx, &request, status, NULL, 0U);
    ResetRxTransfer(ctx);
    return;
  }

  request.recordIndex = normalizedIndex;
  payloadLength = 0U;

  status = BuildRuntimePayload(&request,
                               &payload[0],
                               &payloadLength,
                               &descriptor,
                               ctx->snapshotCache);
  (void) QueueCommandResponse(ctx,
                              &request,
                              status,
                              &payload[0],
                              (status == MMI_PROTOCOL_V2_STATUS_OK)
                              ? payloadLength
                              : 0U);
  ResetRxTransfer(ctx);
}

static void ProcessSubscribeTransfer(MmiCanAdapterCtx_t *ctx)
{
  MmiProtocolSubscribeRequestV2_t request;
  MmiResourceDescriptor_t descriptor;
  uint16_t recordCount = 0U;
  uint8_t normalizedIndex;
  uint8_t slotIndex;
  uint8_t foundSlot = 0U;

  if ((ctx == NULL) || (ctx->service == NULL))
  {
    ResetSubscribeTransfer(ctx);
    return;
  }

  (void) memcpy(&request, &ctx->subscribeBuffer[0], sizeof(request));
  if (MmiServiceLookupResource(ctx->service,
                               MMI_PROTOCOL_V2_NAMESPACE_RUNTIME,
                               request.topicId,
                               &descriptor) == 0U)
  {
    (void) QueueAck(ctx,
                    MMI_PROTOCOL_V2_MESSAGE_CLASS_SUBSCRIBE,
                    ctx->subscribeTransferId,
                    MMI_PROTOCOL_V2_STATUS_BAD_RESOURCE);
    ResetSubscribeTransfer(ctx);
    return;
  }

  if (descriptor.supportsSubscription == 0U)
  {
    (void) QueueAck(ctx,
                    MMI_PROTOCOL_V2_MESSAGE_CLASS_SUBSCRIBE,
                    ctx->subscribeTransferId,
                    MMI_PROTOCOL_V2_STATUS_UNSUPPORTED);
    ResetSubscribeTransfer(ctx);
    return;
  }

  if (MmiServiceResolveRecordCount(ctx->service, &descriptor, &recordCount) == 0U)
  {
    (void) QueueAck(ctx,
                    MMI_PROTOCOL_V2_MESSAGE_CLASS_SUBSCRIBE,
                    ctx->subscribeTransferId,
                    MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR);
    ResetSubscribeTransfer(ctx);
    return;
  }

  normalizedIndex = request.recordIndex;
  if (NormalizeRecordIndex(&descriptor, recordCount, &normalizedIndex)
      != MMI_PROTOCOL_V2_STATUS_OK)
  {
    (void) QueueAck(ctx,
                    MMI_PROTOCOL_V2_MESSAGE_CLASS_SUBSCRIBE,
                    ctx->subscribeTransferId,
                    MMI_PROTOCOL_V2_STATUS_BAD_INDEX);
    ResetSubscribeTransfer(ctx);
    return;
  }

  for (slotIndex = 0U; slotIndex < MMI_CAN_ADAPTER_SUBSCRIPTION_MAX; slotIndex++)
  {
    if ((ctx->subscriptions[slotIndex].active != 0U)
        && (ctx->subscriptions[slotIndex].topicId == request.topicId)
        && (ctx->subscriptions[slotIndex].recordIndex == normalizedIndex))
    {
      foundSlot = 1U;
      break;
    }
  }

  if (foundSlot == 0U)
  {
    for (slotIndex = 0U; slotIndex < MMI_CAN_ADAPTER_SUBSCRIPTION_MAX; slotIndex++)
    {
      if (ctx->subscriptions[slotIndex].active == 0U)
      {
        foundSlot = 1U;
        break;
      }
    }
  }

  if (foundSlot == 0U)
  {
    (void) QueueAck(ctx,
                    MMI_PROTOCOL_V2_MESSAGE_CLASS_SUBSCRIBE,
                    ctx->subscribeTransferId,
                    MMI_PROTOCOL_V2_STATUS_BUSY);
    ResetSubscribeTransfer(ctx);
    return;
  }

  if ((request.leaseSeconds == 0U) || (request.periodDeciseconds == 0U))
  {
    ctx->subscriptions[slotIndex].active = 0U;
    (void) QueueAck(ctx,
                    MMI_PROTOCOL_V2_MESSAGE_CLASS_SUBSCRIBE,
                    ctx->subscribeTransferId,
                    MMI_PROTOCOL_V2_STATUS_OK);
    ResetSubscribeTransfer(ctx);
    return;
  }

  ctx->subscriptions[slotIndex].active = 1U;
  ctx->subscriptions[slotIndex].topicId = request.topicId;
  ctx->subscriptions[slotIndex].recordIndex = normalizedIndex;
  ctx->subscriptions[slotIndex].periodTicks =
    (uint16_t) request.periodDeciseconds * 10U;
  if (ctx->subscriptions[slotIndex].periodTicks == 0U)
  {
    ctx->subscriptions[slotIndex].periodTicks = 1U;
  }
  ctx->subscriptions[slotIndex].ticksUntilDue = 0U;
  ctx->subscriptions[slotIndex].dirty = 1U;

  (void) QueueAck(ctx,
                  MMI_PROTOCOL_V2_MESSAGE_CLASS_SUBSCRIBE,
                  ctx->subscribeTransferId,
                  MMI_PROTOCOL_V2_STATUS_OK);
  ResetSubscribeTransfer(ctx);
}

static void ProcessSubscribeSegment(MmiCanAdapterCtx_t *ctx,
                                    const MmiProtocolSegmentV2_t *segment)
{
  uint16_t offset;

  if ((ctx == NULL) || (segment == NULL))
  {
    return;
  }

  if (ctx->sessionId == 0U)
  {
    ctx->sessionId = (segment->sessionId != 0U) ? segment->sessionId : 1U;
  }

  if (segment->sessionId != ctx->sessionId)
  {
    (void) QueueAck(ctx,
                    MMI_PROTOCOL_V2_MESSAGE_CLASS_SUBSCRIBE,
                    segment->transferId,
                    MMI_PROTOCOL_V2_STATUS_BUSY);
    return;
  }

  if (MmiProtocolV2SegmentIsFirst(segment) != 0U)
  {
    ResetSubscribeTransfer(ctx);
    ctx->subscribeActive = 1U;
    ctx->subscribeTransferId = segment->transferId;
    ctx->subscribeNextSegmentIndex = 0U;
  }

  if ((ctx->subscribeActive == 0U)
      || (ctx->subscribeTransferId != segment->transferId)
      || (ctx->subscribeNextSegmentIndex != segment->segmentIndex))
  {
    ResetSubscribeTransfer(ctx);
    (void) QueueAck(ctx,
                    MMI_PROTOCOL_V2_MESSAGE_CLASS_SUBSCRIBE,
                    segment->transferId,
                    MMI_PROTOCOL_V2_STATUS_BUSY);
    return;
  }

  offset = (uint16_t) segment->segmentIndex * MMI_PROTOCOL_V2_SEGMENT_DATA_BYTES;
  if ((offset + MMI_PROTOCOL_V2_SEGMENT_DATA_BYTES)
      > sizeof(ctx->subscribeBuffer))
  {
    ResetSubscribeTransfer(ctx);
    (void) QueueAck(ctx,
                    MMI_PROTOCOL_V2_MESSAGE_CLASS_SUBSCRIBE,
                    segment->transferId,
                    MMI_PROTOCOL_V2_STATUS_INVALID_VALUE);
    return;
  }

  (void) memcpy(&ctx->subscribeBuffer[offset],
                &segment->bytes[0],
                MMI_PROTOCOL_V2_SEGMENT_DATA_BYTES);
  ctx->subscribeNextSegmentIndex++;

  if (MmiProtocolV2SegmentIsLast(segment) != 0U)
  {
    ProcessSubscribeTransfer(ctx);
  }
}

static void ProcessSubscriptions(MmiCanAdapterCtx_t *ctx)
{
  uint8_t index;

  if (ctx == NULL)
  {
    return;
  }

  for (index = 0U; index < MMI_CAN_ADAPTER_SUBSCRIPTION_MAX; index++)
  {
    MmiRuntimeSubscription_t *subscription = &ctx->subscriptions[index];

    if (subscription->active == 0U)
    {
      continue;
    }

    if ((subscription->ticksUntilDue == 0U) || (subscription->dirty != 0U))
    {
      if (QueueRuntimePublish(ctx,
                              subscription->topicId,
                              subscription->recordIndex,
                              subscription->sequence) != 0U)
      {
        subscription->sequence++;
      }

      subscription->dirty = 0U;
      subscription->ticksUntilDue = subscription->periodTicks;
    }
    else
    {
      subscription->ticksUntilDue--;
    }
  }
}

static void ProcessCommandSegment(MmiCanAdapterCtx_t *ctx,
                                  const MmiProtocolSegmentV2_t *segment)
{
  uint16_t offset;
  MmiProtocolCommandHeaderV2_t header;

  if ((ctx == NULL) || (segment == NULL))
  {
    return;
  }

  if (ctx->sessionId == 0U)
  {
    ctx->sessionId = (segment->sessionId != 0U) ? segment->sessionId : 1U;
  }

  if (segment->sessionId != ctx->sessionId)
  {
    (void) QueueAck(ctx,
                    MMI_PROTOCOL_V2_MESSAGE_CLASS_COMMAND,
                    segment->transferId,
                    MMI_PROTOCOL_V2_STATUS_BUSY);
    return;
  }

  if (MmiProtocolV2SegmentIsFirst(segment) != 0U)
  {
    ResetRxTransfer(ctx);
    ctx->rxActive = 1U;
    ctx->rxMessageClass = MMI_PROTOCOL_V2_MESSAGE_CLASS_COMMAND;
    ctx->rxSessionId = segment->sessionId;
    ctx->rxTransferId = segment->transferId;
    ctx->rxNextSegmentIndex = 0U;
  }

  if ((ctx->rxActive == 0U)
      || (ctx->rxTransferId != segment->transferId)
      || (ctx->rxNextSegmentIndex != segment->segmentIndex))
  {
    ResetRxTransfer(ctx);
    (void) QueueAck(ctx,
                    MMI_PROTOCOL_V2_MESSAGE_CLASS_COMMAND,
                    segment->transferId,
                    MMI_PROTOCOL_V2_STATUS_BUSY);
    return;
  }

  offset = (uint16_t) segment->segmentIndex
           * MMI_PROTOCOL_V2_SEGMENT_DATA_BYTES;
  if ((offset + MMI_PROTOCOL_V2_SEGMENT_DATA_BYTES) > sizeof(ctx->rxBuffer))
  {
    ResetRxTransfer(ctx);
    (void) QueueAck(ctx,
                    MMI_PROTOCOL_V2_MESSAGE_CLASS_COMMAND,
                    segment->transferId,
                    MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR);
    return;
  }

  (void) memcpy(&ctx->rxBuffer[offset],
                &segment->bytes[0],
                MMI_PROTOCOL_V2_SEGMENT_DATA_BYTES);
  ctx->rxNextSegmentIndex++;

  if ((ctx->rxExpectedLength == 0U)
      && ((offset + MMI_PROTOCOL_V2_SEGMENT_DATA_BYTES) >= sizeof(header)))
  {
    (void) memcpy(&header, &ctx->rxBuffer[0], sizeof(header));
    ctx->rxExpectedLength = (uint16_t) (sizeof(header) + header.payloadLength);
    if (ctx->rxExpectedLength > sizeof(ctx->rxBuffer))
    {
      ResetRxTransfer(ctx);
      (void) QueueAck(ctx,
                      MMI_PROTOCOL_V2_MESSAGE_CLASS_COMMAND,
                      segment->transferId,
                      MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR);
      return;
    }
  }

  if (MmiProtocolV2SegmentIsLast(segment) != 0U)
  {
    if ((ctx->rxExpectedLength == 0U)
        || (ctx->rxExpectedLength > (offset + MMI_PROTOCOL_V2_SEGMENT_DATA_BYTES)))
    {
      ResetRxTransfer(ctx);
      (void) QueueAck(ctx,
                      MMI_PROTOCOL_V2_MESSAGE_CLASS_COMMAND,
                      segment->transferId,
                      MMI_PROTOCOL_V2_STATUS_INTERNAL_ERROR);
      return;
    }

    ProcessCommandTransfer(ctx);
  }
}

static void ProcessRxFrame(MmiCanAdapterCtx_t *ctx,
                           const MmiQueuedFrame_t *frame)
{
  MmiProtocolMessageClass_t messageClass;

  if ((ctx == NULL) || (frame == NULL))
  {
    return;
  }

  if (MmiProtocolV2CanIdToMessageClass(frame->standardId, &messageClass) == 0U)
  {
    return;
  }

  switch (messageClass)
  {
      case MMI_PROTOCOL_V2_MESSAGE_CLASS_HELLO_REQUEST:
      {
        MmiProtocolHelloRequestV2_t request;

        if (frame->length < sizeof(request))
        {
          return;
        }

        (void) memcpy(&request, &frame->data[0], sizeof(request));
        (void) QueueHelloResponse(ctx, &request);
        break;
      }

      case MMI_PROTOCOL_V2_MESSAGE_CLASS_COMMAND:
      {
        MmiProtocolSegmentV2_t segment;

        if (frame->length < sizeof(segment))
        {
          return;
        }

        (void) memcpy(&segment, &frame->data[0], sizeof(segment));
        ProcessCommandSegment(ctx, &segment);
        break;
      }

      case MMI_PROTOCOL_V2_MESSAGE_CLASS_SUBSCRIBE:
      {
        MmiProtocolSegmentV2_t segment;

        if (frame->length < sizeof(segment))
        {
          return;
        }

        (void) memcpy(&segment, &frame->data[0], sizeof(segment));
        ProcessSubscribeSegment(ctx, &segment);
        break;
      }

      default:
      {
        break;
      }
  }
}

void MmiCanAdapterInit(MmiCanAdapterCtx_t *ctx,
                       FDCAN_HandleTypeDef *hfdcan,
                       MmiService_t *service,
                       MmiSnapshotCache_t *snapshotCache)
{
  if (ctx == NULL)
  {
    return;
  }

  (void) memset(ctx, 0, sizeof(*ctx));
  ctx->hfdcan = hfdcan;
  ctx->service = service;
  ctx->snapshotCache = snapshotCache;
  ctx->eventLogService = NULL;
  ctx->localSettingsService = NULL;
  ctx->maintenanceService = NULL;
  if ((service != NULL) && (service->configurationService != NULL)
      && (service->intersectionEngine != NULL)
      && (service->intersectionController != NULL))
  {
    LWIPSNMPAdapterInit(&ctx->ntcipAdapter,
                        service->configurationService,
                        service->intersectionEngine,
                        service->intersectionController);
  }
  CreateOsObjects(ctx);
  ResetRxTransfer(ctx);
  ResetSubscribeTransfer(ctx);
  s_registeredCtx = ctx;
}

void MmiCanAdapterBindActivationService(
  MmiCanAdapterCtx_t *ctx,
  IntersectionActivationService_t *activationService)
{
  if (ctx != NULL)
  {
    LWIPSNMPAdapterBindActivationService(&ctx->ntcipAdapter, activationService);
  }
}

void MmiCanAdapterBindDetectorReportService(
  MmiCanAdapterCtx_t *ctx,
  DetectorReportService_t *detectorReportService)
{
  if (ctx != NULL)
  {
    LWIPSNMPAdapterBindDetectorReportService(&ctx->ntcipAdapter,
                                            detectorReportService);
  }
}

void MmiCanAdapterBindGlobalTimeManagementService(
  MmiCanAdapterCtx_t *ctx,
  GlobalTimeManagementService_t *globalTimeManagementService)
{
  if (ctx != NULL)
  {
    LWIPSNMPAdapterBindGlobalTimeManagementService(&ctx->ntcipAdapter,
                                                   globalTimeManagementService);
  }
}

void MmiCanAdapterBindDoorSensorPort(MmiCanAdapterCtx_t *ctx,
                                     IDoorSensorPort_t *doorSensorPort)
{
  if (ctx != NULL)
  {
    LWIPSNMPAdapterBindDoorSensorPort(&ctx->ntcipAdapter, doorSensorPort);
  }
}

void MmiCanAdapterBindHeaterPort(MmiCanAdapterCtx_t *ctx,
                                 IHeaterPort_t *heaterPort)
{
  if (ctx != NULL)
  {
    LWIPSNMPAdapterBindHeaterPort(&ctx->ntcipAdapter, heaterPort);
  }
}

void MmiCanAdapterBindPowerMonitorPort(MmiCanAdapterCtx_t *ctx,
                                       IPowerMonitorPort_t *powerMonitorPort)
{
  if (ctx != NULL)
  {
    LWIPSNMPAdapterBindPowerMonitorPort(&ctx->ntcipAdapter, powerMonitorPort);
  }
}

void MmiCanAdapterBindUnitAlarmPort(MmiCanAdapterCtx_t *ctx,
                                    IUnitAlarmPort_t *unitAlarmPort)
{
  if (ctx != NULL)
  {
    LWIPSNMPAdapterBindUnitAlarmPort(&ctx->ntcipAdapter, unitAlarmPort);
  }
}

void MmiCanAdapterBindUnitClockPort(MmiCanAdapterCtx_t *ctx,
                                    IUnitClockPort_t *unitClockPort)
{
  if (ctx != NULL)
  {
    LWIPSNMPAdapterBindUnitClockPort(&ctx->ntcipAdapter, unitClockPort);
  }
}

void MmiCanAdapterBindCpMpLinkService(MmiCanAdapterCtx_t *ctx,
                                      CpMpLinkService_t *cpMpLinkService)
{
  if (ctx != NULL)
  {
    LWIPSNMPAdapterBindCpMpLinkService(&ctx->ntcipAdapter, cpMpLinkService);
  }
}

void MmiCanAdapterBindLocalSettingsService(
  MmiCanAdapterCtx_t *ctx,
  MmiLocalSettingsService_t *localSettingsService)
{
  if (ctx != NULL)
  {
    ctx->localSettingsService = localSettingsService;
  }
}

void MmiCanAdapterBindEventLogService(MmiCanAdapterCtx_t *ctx,
                                      MmiEventLogService_t *eventLogService)
{
  if (ctx != NULL)
  {
    ctx->eventLogService = eventLogService;
  }
}

void MmiCanAdapterBindMaintenanceService(MmiCanAdapterCtx_t *ctx,
                                         MmiMaintenanceService_t *service)
{
  if (ctx != NULL)
  {
    ctx->maintenanceService = service;
  }
}

void MmiCanAdapterOnRxIsr(const FDCAN_RxHeaderTypeDef *header,
                          const uint8_t *data)
{
  MmiCanAdapterCtx_t *ctx = s_registeredCtx;
  uint8_t length;

  if ((ctx == NULL) || (header == NULL) || (data == NULL)
      || (header->IdType != FDCAN_STANDARD_ID))
  {
    return;
  }

  length = DlcToLength(header->DataLength);
  if (length > 8U)
  {
    length = 8U;
  }

  if (QueueFrame(ctx->rxPool,
                 ctx->rxQueue,
                 (uint16_t) header->Identifier,
                 data,
                 length) == 0U)
  {
    ctx->rxDrops++;
  }
}

void MmiCanAdapterStep(void)
{
  MmiCanAdapterCtx_t *ctx = s_registeredCtx;
  MmiQueuedFrame_t *queuedFrame;

  if (ctx == NULL)
  {
    return;
  }

  while ((ctx->rxQueue != NULL)
         && (osMessageQueueGet(ctx->rxQueue, &queuedFrame, NULL, 0U) == osOK))
  {
    ProcessRxFrame(ctx, queuedFrame);
    (void) osMemoryPoolFree(ctx->rxPool, queuedFrame);
  }

  ProcessSubscriptions(ctx);

  while ((ctx->txQueue != NULL)
         && (osMessageQueueGet(ctx->txQueue, &queuedFrame, NULL, 0U) == osOK))
  {
    FDCAN_TxHeaderTypeDef header;

    (void) memset(&header, 0, sizeof(header));
    header.Identifier = queuedFrame->standardId;
    header.IdType = FDCAN_STANDARD_ID;
    header.TxFrameType = FDCAN_DATA_FRAME;
    header.DataLength = LengthToDlc(queuedFrame->length);
    header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    header.BitRateSwitch = FDCAN_BRS_OFF;
    header.FDFormat = FDCAN_CLASSIC_CAN;
    header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    header.MessageMarker = 0U;

    if ((ctx->hfdcan == NULL)
        || (HAL_FDCAN_AddMessageToTxFifoQ(ctx->hfdcan,
                                          &header,
                                          &queuedFrame->data[0]) != HAL_OK))
    {
      if ((ctx->txQueue == NULL)
          || (osMessageQueuePut(ctx->txQueue, &queuedFrame, 0U, 0U) != osOK))
      {
        ctx->txErrors++;
        (void) osMemoryPoolFree(ctx->txPool, queuedFrame);
      }
      break;
    }

    (void) osMemoryPoolFree(ctx->txPool, queuedFrame);
  }
}

void MmiCanAdapterNotifyRuntimeTopic(uint8_t topicId, uint8_t recordIndex)
{
  MarkSubscriptionsDirty(s_registeredCtx, topicId, recordIndex);
}
