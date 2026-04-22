/* App/Domain/Services/EventReportService.c */
#include "EventReportService.h"

#include <limits.h>
#include <stddef.h>
#include <string.h>

#define EVENT_REPORT_PERSIST_OP_APPEND 1U

#define EVENT_POWER_ON 1U
#define EVENT_POWER_NORMAL_TO_STAND_BY 28U
#define EVENT_RESET_WINDOW_WATCHDOG 39U
#define EVENT_RESET_INDEPENDENT_WATCHDOG 40U
#define EVENT_RESET_LOW_POWER 41U
#define EVENT_RESET_POWER_ON_CLEAR_CIRCUIT 61U
#define EVENT_DOOR_OPEN 64U
#define EVENT_DOOR_CLOSED 65U
#define EVENT_RESET_SOFTWARE 100U
#define EVENT_RESET_PIN 101U
#define EVENT_RESET_PORRST 102U
#define EVENT_CPMP_LINK_DEGRADED 121U
#define EVENT_CPMP_LINK_RESTORED 122U
#define EVENT_CPMP_MP_EVENT 123U

static const uint32_t kSecurityRootOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U, 5U
};
static const uint32_t kWatchBlocksRootOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U, 6U
};
static const uint32_t kWatchBlockValueOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U, 6U, 4U, 1U, 4U
};
static const uint32_t kReportBlocksRootOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U, 7U
};
static const uint32_t kReportBlockValueOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U, 7U, 4U, 1U, 4U
};
static const uint32_t kSourcePowerOnCountOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 22U, 1U
};
static const uint32_t kSourceResetCauseOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 22U, 2U
};
static const uint32_t kSourceStandbyCountOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 22U, 3U
};
static const uint32_t kSourceDoorOpenCountOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 22U, 4U
};
static const uint32_t kSourceDoorClosedCountOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 22U, 5U
};
static const uint32_t kSourceCpMpLinkDegradedCountOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 22U, 6U
};
static const uint32_t kSourceCpMpLinkRestoredCountOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 22U, 7U
};
static const uint32_t kSourceMpEventCountOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 22U, 8U
};
static const uint32_t kSourceMpEventDataOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 22U, 9U
};

static uint8_t OidIsNull(const NtcipOid_t *oid)
{
  return (uint8_t) ((oid == NULL) || (oid->length == 0U));
}

static uint8_t OidStartsWith(const NtcipOid_t *oid,
                             const uint32_t *prefix,
                             uint8_t prefixLength)
{
  if ((oid == NULL) || (prefix == NULL) || (oid->length < prefixLength))
  {
    return 0U;
  }

  return (uint8_t) (memcmp(&oid->elements[0],
                           prefix,
                           (size_t) prefixLength * sizeof(uint32_t)) == 0);
}

static uint8_t OidMatchesIndexedLeaf(const NtcipOid_t *oid,
                                     const uint32_t *prefix,
                                     uint8_t prefixLength,
                                     uint8_t *indexOut)
{
  if ((oid == NULL) || (prefix == NULL) || (indexOut == NULL)
      || (oid->length != (uint8_t) (prefixLength + 1U))
      || (OidStartsWith(oid, prefix, prefixLength) == 0U)
      || (oid->elements[prefixLength] == 0U)
      || (oid->elements[prefixLength] > UINT8_MAX))
  {
    return 0U;
  }

  *indexOut = (uint8_t) oid->elements[prefixLength];
  return 1U;
}

static uint8_t OidForbiddenForBlockConfiguration(const NtcipOid_t *oid)
{
  return (uint8_t) ((OidStartsWith(oid,
                                   &kSecurityRootOid[0],
                                   (uint8_t) (sizeof(kSecurityRootOid)
                                              / sizeof(kSecurityRootOid[0])))
                      != 0U)
                    || (OidStartsWith(oid,
                                      &kWatchBlocksRootOid[0],
                                      (uint8_t) (sizeof(kWatchBlocksRootOid)
                                                 / sizeof(kWatchBlocksRootOid[0])))
                        != 0U)
                    || (OidStartsWith(
                          oid,
                          &kReportBlocksRootOid[0],
                          (uint8_t) (sizeof(kReportBlocksRootOid)
                                     / sizeof(kReportBlocksRootOid[0])))
                        != 0U));
}

static const EventReportConfiguration_t *GetReadableConfig(
  const EventReportService_t *service)
{
  if (service == NULL)
  {
    return NULL;
  }

  return (service->transactionActive != 0U)
         ? &service->candidateConfig
         : &service->activeConfig;
}

static uint8_t OidIsWatchBlockValue(const NtcipOid_t *oid)
{
  uint8_t blockNumber = 0U;

  return OidMatchesIndexedLeaf(oid,
                               &kWatchBlockValueOid[0],
                               (uint8_t) (sizeof(kWatchBlockValueOid)
                                          / sizeof(kWatchBlockValueOid[0])),
                               &blockNumber);
}

static uint8_t OidIsReportBlockValue(const NtcipOid_t *oid)
{
  uint8_t blockNumber = 0U;

  return OidMatchesIndexedLeaf(oid,
                               &kReportBlockValueOid[0],
                               (uint8_t) (sizeof(kReportBlockValueOid)
                                          / sizeof(kReportBlockValueOid[0])),
                               &blockNumber);
}

static uint8_t ValueIsIntegerLike(const NtcipValue_t *value)
{
  return (uint8_t) ((value != NULL)
                    && ((value->type == NTCIP_VALUE_TYPE_UNSIGNED32)
                        || (value->type == NTCIP_VALUE_TYPE_SIGNED32)));
}

static int32_t ValueToSigned32(const NtcipValue_t *value)
{
  if (value == NULL)
  {
    return 0;
  }

  if (value->type == NTCIP_VALUE_TYPE_SIGNED32)
  {
    return value->data.signed32;
  }

  if (value->data.unsigned32 > (uint32_t) INT32_MAX)
  {
    return INT32_MAX;
  }

  return (int32_t) value->data.unsigned32;
}

static uint8_t ValuesEqual(const NtcipValue_t *left, const NtcipValue_t *right)
{
  if ((left == NULL) || (right == NULL) || (left->type != right->type))
  {
    return 0U;
  }

  switch (left->type)
  {
    case NTCIP_VALUE_TYPE_UNSIGNED32:
      return (uint8_t) (left->data.unsigned32 == right->data.unsigned32);

    case NTCIP_VALUE_TYPE_SIGNED32:
      return (uint8_t) (left->data.signed32 == right->data.signed32);

    case NTCIP_VALUE_TYPE_OBJECT_ID:
      return (uint8_t) ((left->data.objectId.length
                         == right->data.objectId.length)
                        && (memcmp(&left->data.objectId.elements[0],
                                   &right->data.objectId.elements[0],
                                   (size_t) left->data.objectId.length
                                   * sizeof(uint32_t)) == 0));

    case NTCIP_VALUE_TYPE_IP_ADDRESS:
      return (uint8_t) (memcmp(&left->data.ipAddress.bytes[0],
                               &right->data.ipAddress.bytes[0],
                               4U) == 0);

    case NTCIP_VALUE_TYPE_OCTET_STRING:
      return (uint8_t) ((left->data.octetString.length
                         == right->data.octetString.length)
                        && (memcmp(&left->data.octetString.bytes[0],
                                   &right->data.octetString.bytes[0],
                                   left->data.octetString.length) == 0));

    case NTCIP_VALUE_TYPE_OPAQUE:
      return (uint8_t) ((left->data.opaque.length
                         == right->data.opaque.length)
                        && (memcmp(&left->data.opaque.bytes[0],
                                   &right->data.opaque.bytes[0],
                                   left->data.opaque.length) == 0));

    default:
      return 0U;
  }
}

static uint16_t EncodeBerValue(const NtcipValue_t *value, uint8_t *buffer);

static uint16_t EncodeBerLength(uint16_t value, uint8_t *buffer)
{
  if (buffer == NULL)
  {
    return 0U;
  }

  if (value < 128U)
  {
    buffer[0] = (uint8_t) value;
    return 1U;
  }

  if (value <= UINT8_MAX)
  {
    buffer[0] = 0x81U;
    buffer[1] = (uint8_t) value;
    return 2U;
  }

  buffer[0] = 0x82U;
  buffer[1] = (uint8_t) (value >> 8U);
  buffer[2] = (uint8_t) (value & 0xFFU);
  return 3U;
}

static uint16_t EncodeOerLength(uint16_t value, uint8_t *buffer)
{
  if (buffer == NULL)
  {
    return 0U;
  }

  if (value < 128U)
  {
    buffer[0] = (uint8_t) value;
    return 1U;
  }

  if (value <= UINT8_MAX)
  {
    buffer[0] = 0x81U;
    buffer[1] = (uint8_t) value;
    return 2U;
  }

  buffer[0] = 0x82U;
  buffer[1] = (uint8_t) (value >> 8U);
  buffer[2] = (uint8_t) (value & 0xFFU);
  return 3U;
}

static uint16_t EncodeOerOpenType(const NtcipValue_t *value,
                                  uint8_t *buffer,
                                  uint16_t capacity)
{
  uint8_t encodedValue[NTCIP_OCTET_STRING_MAX_LENGTH + 4U];
  uint16_t encodedLength;
  uint16_t lengthLength;

  if ((value == NULL) || (buffer == NULL))
  {
    return 0U;
  }

  encodedLength = EncodeBerValue(value, &encodedValue[0]);
  if ((encodedLength == 0U) || (encodedLength > sizeof(encodedValue)))
  {
    return 0U;
  }

  lengthLength = EncodeOerLength(encodedLength, buffer);
  if ((lengthLength == 0U)
      || ((uint32_t) lengthLength + (uint32_t) encodedLength > capacity))
  {
    return 0U;
  }

  (void) memcpy(&buffer[lengthLength], &encodedValue[0], encodedLength);
  return (uint16_t) (lengthLength + encodedLength);
}

static uint16_t EncodeBerIntegerUnsigned(uint32_t value, uint8_t *buffer)
{
  uint8_t temp[5];
  uint8_t count = 0U;
  uint16_t offset;

  if (buffer == NULL)
  {
    return 0U;
  }

  do
  {
    temp[4U - count] = (uint8_t) (value & 0xFFU);
    value >>= 8U;
    count++;
  } while ((value != 0U) && (count < 4U));

  if ((temp[5U - count] & 0x80U) != 0U)
  {
    temp[4U - count] = 0U;
    count++;
  }

  buffer[0] = 0x02U;
  buffer[1] = count;
  offset = 2U;
  (void) memcpy(&buffer[offset], &temp[5U - count], count);
  return (uint16_t) (offset + count);
}

static uint16_t EncodeBerIntegerSigned(int32_t value, uint8_t *buffer)
{
  uint8_t temp[4];
  uint8_t start = 0U;
  uint8_t nextByte;
  uint8_t index;

  if (buffer == NULL)
  {
    return 0U;
  }

  temp[0] = (uint8_t) ((uint32_t) value >> 24U);
  temp[1] = (uint8_t) ((uint32_t) value >> 16U);
  temp[2] = (uint8_t) ((uint32_t) value >> 8U);
  temp[3] = (uint8_t) (uint32_t) value;

  for (index = 0U; index < 3U; index++)
  {
    nextByte = temp[index + 1U];
    if (((temp[index] == 0x00U) && ((nextByte & 0x80U) == 0U))
        || ((temp[index] == 0xFFU) && ((nextByte & 0x80U) != 0U)))
    {
      start++;
    }
    else
    {
      break;
    }
  }

  buffer[0] = 0x02U;
  buffer[1] = (uint8_t) (4U - start);
  (void) memcpy(&buffer[2], &temp[start], 4U - start);
  return (uint16_t) (2U + (4U - start));
}

static uint16_t EncodeBerObjectId(const NtcipOid_t *oid, uint8_t *buffer)
{
  uint8_t body[NTCIP_OCTET_STRING_MAX_LENGTH];
  uint16_t bodyLength = 0U;
  uint32_t value;
  uint8_t encoded[5];
  uint8_t encodedLength;
  uint8_t index;
  uint8_t lengthBytes;

  if ((oid == NULL) || (buffer == NULL) || (oid->length < 2U))
  {
    return 0U;
  }

  body[bodyLength++] = (uint8_t) (oid->elements[0] * 40U
                                  + oid->elements[1]);
  for (index = 2U; index < oid->length; index++)
  {
    value = oid->elements[index];
    encodedLength = 0U;
    do
    {
      encoded[encodedLength++] = (uint8_t) (value & 0x7FU);
      value >>= 7U;
    } while ((value != 0U) && (encodedLength < sizeof(encoded)));

    while (encodedLength > 0U)
    {
      uint8_t octet = encoded[encodedLength - 1U];

      if (encodedLength > 1U)
      {
        octet |= 0x80U;
      }

      body[bodyLength++] = octet;
      encodedLength--;
    }
  }

  buffer[0] = 0x06U;
  lengthBytes = (uint8_t) EncodeBerLength(bodyLength, &buffer[1]);
  if (lengthBytes == 0U)
  {
    return 0U;
  }

  (void) memcpy(&buffer[1U + lengthBytes], &body[0], bodyLength);
  return (uint16_t) (1U + lengthBytes + bodyLength);
}

static uint16_t EncodeBerString(uint8_t tag,
                                const uint8_t *bytes,
                                uint16_t length,
                                uint8_t *buffer)
{
  uint16_t headerLength;

  if ((buffer == NULL) || ((bytes == NULL) && (length != 0U)))
  {
    return 0U;
  }

  buffer[0] = tag;
  headerLength = EncodeBerLength(length, &buffer[1]);
  if (headerLength == 0U)
  {
    return 0U;
  }

  if (length > 0U)
  {
    (void) memcpy(&buffer[1U + headerLength], bytes, length);
  }

  return (uint16_t) (1U + headerLength + length);
}

static uint16_t EncodeBerValue(const NtcipValue_t *value, uint8_t *buffer)
{
  if ((value == NULL) || (buffer == NULL))
  {
    return 0U;
  }

  switch (value->type)
  {
    case NTCIP_VALUE_TYPE_UNSIGNED32:
      return EncodeBerIntegerUnsigned(value->data.unsigned32, buffer);

    case NTCIP_VALUE_TYPE_SIGNED32:
      return EncodeBerIntegerSigned(value->data.signed32, buffer);

    case NTCIP_VALUE_TYPE_OBJECT_ID:
      return EncodeBerObjectId(&value->data.objectId, buffer);

    case NTCIP_VALUE_TYPE_OCTET_STRING:
      return EncodeBerString(0x04U,
                             &value->data.octetString.bytes[0],
                             value->data.octetString.length,
                             buffer);

    case NTCIP_VALUE_TYPE_OPAQUE:
      return EncodeBerString(0x44U,
                             &value->data.opaque.bytes[0],
                             value->data.opaque.length,
                             buffer);

    case NTCIP_VALUE_TYPE_IP_ADDRESS:
      return EncodeBerString(0x40U,
                             &value->data.ipAddress.bytes[0],
                             4U,
                             buffer);

    default:
      return 0U;
  }
}

static uint8_t CopyText(uint8_t *target,
                        uint8_t targetCapacity,
                        const char *text,
                        uint8_t *lengthOut)
{
  size_t length;

  if ((target == NULL) || (text == NULL) || (lengthOut == NULL))
  {
    return 0U;
  }

  length = strlen(text);
  if (length > targetCapacity)
  {
    return 0U;
  }

  if (length > 0U)
  {
    (void) memcpy(target, text, length);
  }

  *lengthOut = (uint8_t) length;
  return 1U;
}

static void SetOid(NtcipOid_t *oid, const uint32_t *elements, uint8_t length)
{
  if (oid == NULL)
  {
    return;
  }

  (void) memset(oid, 0, sizeof(*oid));
  if ((elements != NULL) && (length <= NTCIP_OID_MAX_LENGTH))
  {
    oid->length = length;
    (void) memcpy(&oid->elements[0],
                  elements,
                  (size_t) length * sizeof(uint32_t));
  }
}

static void InitDefaultClasses(EventReportConfiguration_t *config)
{
  static const char *kDescriptions[4] =
  {
    "power-reset",
    "cabinet-operator",
    "controller-comms",
    "mmu-module"
  };
  uint8_t index;

  if (config == NULL)
  {
    return;
  }

  for (index = 0U; index < EVENT_REPORT_MAX_EVENT_CLASSES; index++)
  {
    EventReportClassConfig_t *row = &config->classes[index];

    (void) memset(row, 0, sizeof(*row));
    row->eventClassNumber = (uint8_t) (index + 1U);
    if (index < 4U)
    {
      row->eventClassLimit = 64U;
      (void) CopyText(&row->eventClassDescription[0],
                      sizeof(row->eventClassDescription),
                      kDescriptions[index],
                      &row->eventClassDescriptionLength);
    }
  }
}

static void InitDefaultCommunities(EventReportConfiguration_t *config)
{
  if (config == NULL)
  {
    return;
  }

  (void) CopyText(&config->communityNameAdmin[0],
                  sizeof(config->communityNameAdmin),
                  "administrator",
                  &config->communityNameAdminLength);

  config->communityRows[0].communityNameIndex = 1U;
  config->communityRows[0].communityNameAccessMask = 0U;
  (void) CopyText(&config->communityRows[0].communityNameUser[0],
                  sizeof(config->communityRows[0].communityNameUser),
                  "public",
                  &config->communityRows[0].communityNameLength);

  config->communityRows[1].communityNameIndex = 2U;
  config->communityRows[1].communityNameAccessMask = 0xFFFFFFFFUL;
  (void) CopyText(&config->communityRows[1].communityNameUser[0],
                  sizeof(config->communityRows[1].communityNameUser),
                  "private",
                  &config->communityRows[1].communityNameLength);

  config->communityRows[2].communityNameIndex = 3U;
  config->communityRows[2].communityNameAccessMask = 0U;
  (void) CopyText(&config->communityRows[2].communityNameUser[0],
                  sizeof(config->communityRows[2].communityNameUser),
                  "reports",
                  &config->communityRows[2].communityNameLength);
}

static void ApplySnmpCommunitiesToConfig(EventReportConfiguration_t *config,
                                         const char *readCommunity,
                                         const char *writeCommunity,
                                         const char *trapCommunity)
{
  if (config == NULL)
  {
    return;
  }

  if (readCommunity != NULL)
  {
    (void) memset(&config->communityRows[0].communityNameUser[0],
                  0,
                  sizeof(config->communityRows[0].communityNameUser));
    (void) CopyText(&config->communityRows[0].communityNameUser[0],
                    sizeof(config->communityRows[0].communityNameUser),
                    readCommunity,
                    &config->communityRows[0].communityNameLength);
  }

  if (writeCommunity != NULL)
  {
    (void) memset(&config->communityRows[1].communityNameUser[0],
                  0,
                  sizeof(config->communityRows[1].communityNameUser));
    (void) CopyText(&config->communityRows[1].communityNameUser[0],
                    sizeof(config->communityRows[1].communityNameUser),
                    writeCommunity,
                    &config->communityRows[1].communityNameLength);
  }

  if (trapCommunity != NULL)
  {
    (void) memset(&config->communityRows[2].communityNameUser[0],
                  0,
                  sizeof(config->communityRows[2].communityNameUser));
    (void) CopyText(&config->communityRows[2].communityNameUser[0],
                    sizeof(config->communityRows[2].communityNameUser),
                    trapCommunity,
                    &config->communityRows[2].communityNameLength);
  }
}

static void InitDefaultLogicalNames(EventReportConfiguration_t *config)
{
  if (config == NULL)
  {
    return;
  }

  config->logicalNameRows[0].logicalNameTranslationIndex = 1U;
  config->logicalNameRows[0].status = EVENT_REPORT_ROW_STATUS_ACTIVE;
  (void) CopyText(&config->logicalNameRows[0].logicalName[0],
                  sizeof(config->logicalNameRows[0].logicalName),
                  "manager",
                  &config->logicalNameRows[0].logicalNameLength);
}

static void InitDefaultWatchRows(EventReportConfiguration_t *config)
{
  uint8_t index;

  if (config == NULL)
  {
    return;
  }

  for (index = 0U; index < EVENT_REPORT_MAX_WATCH_OBJECTS; index++)
  {
    config->watchObjectRows[index].watchId = (uint8_t) (index + 1U);
    config->watchObjectRows[index].watchBlock = 1U;
  }

  for (index = 0U; index < EVENT_REPORT_MAX_WATCH_BLOCKS; index++)
  {
    config->watchBlockRows[index].watchBlockNumber = (uint8_t) (index + 1U);
  }
}

static void InitDefaultReportRows(EventReportConfiguration_t *config)
{
  uint8_t index;

  if (config == NULL)
  {
    return;
  }

  for (index = 0U; index < EVENT_REPORT_MAX_REPORT_OBJECTS; index++)
  {
    config->reportObjectRows[index].reportId = (uint8_t) (index + 1U);
    config->reportObjectRows[index].reportBlock = 1U;
  }

  for (index = 0U; index < EVENT_REPORT_MAX_REPORT_BLOCKS; index++)
  {
    config->reportBlockRows[index].reportBlockNumber = (uint8_t) (index + 1U);
  }
}

static void InitDefaultTrapRows(EventReportConfiguration_t *config)
{
  uint8_t configIndex;

  if (config == NULL)
  {
    return;
  }

  config->trapMgmtRows[0].trapMgmtManagerIndex = 1U;
  config->trapMgmtRows[0].trapMgmtManagerPointer = 1U;
  config->trapMgmtRows[0].trapMgmtCommunityNamePointer = 3U;
  config->trapMgmtRows[0].trapMgmtApplicationProtocol = 2U;
  config->trapMgmtRows[0].trapMgmtTransportProtocol = 3U;
  config->trapMgmtRows[0].trapMgmtPortNum = 162U;
  config->trapMgmtRows[0].trapMgmtRepeatInterval = 60U;
  config->trapMgmtRows[0].trapMgmtDelta = 60U;
  config->trapMgmtRows[0].trapMgmtQueueDepth = 1U;
  config->trapMgmtRows[0].trapMgmtLinkStateStatus =
    EVENT_REPORT_TRAP_LINK_READY;
  config->trapMgmtRows[0].trapMgmtAntiStreamRate = 10U;
  config->trapMgmtRows[0].trapMgmtRowStatus = EVENT_REPORT_ROW_STATUS_ACTIVE;
  config->trapMgmtRows[0].trapMgmtSeqNum = 1U;

  for (configIndex = 0U; configIndex < EVENT_REPORT_MAX_EVENT_LOG_CONFIGS;
       configIndex++)
  {
    config->trapRows[configIndex][0].trapMode = 1U;
  }
}

static void InitDefaultEventConfigs(EventReportConfiguration_t *config)
{
  uint8_t index;

  if (config == NULL)
  {
    return;
  }

  for (index = 0U; index < EVENT_REPORT_MAX_EVENT_LOG_CONFIGS; index++)
  {
    EventReportConfigRow_t *row = &config->configs[index];

    (void) memset(row, 0, sizeof(*row));
    row->eventConfigID = (uint16_t) (index + 1U);
    row->eventConfigClass = 1U;
    row->eventConfigMode = EVENT_REPORT_MODE_ON_CHANGE;
    row->eventConfigAction = EVENT_REPORT_ACTION_DISABLED;
    row->eventConfigStatus = EVENT_REPORT_STATUS_DISABLED;
  }

  config->configs[0].preconfigured = 1U;
  config->configs[0].eventConfigClass = 1U;
  config->configs[0].eventConfigAction = EVENT_REPORT_ACTION_LOG;
  SetOid(&config->configs[0].eventConfigCompareOid,
         &kSourcePowerOnCountOid[0],
         12U);
  SetOid(&config->configs[0].eventConfigLogOid,
         &kSourceResetCauseOid[0],
         12U);
  config->trapRows[0][0].trapDestEnable = 1U;

  config->configs[1].preconfigured = 1U;
  config->configs[1].eventConfigClass = 1U;
  config->configs[1].eventConfigAction = EVENT_REPORT_ACTION_LOG;
  SetOid(&config->configs[1].eventConfigCompareOid,
         &kSourceStandbyCountOid[0],
         12U);
  SetOid(&config->configs[1].eventConfigLogOid,
         &kSourceStandbyCountOid[0],
         12U);
  config->trapRows[1][0].trapDestEnable = 1U;

  config->configs[2].preconfigured = 1U;
  config->configs[2].eventConfigClass = 2U;
  config->configs[2].eventConfigAction = EVENT_REPORT_ACTION_LOG;
  SetOid(&config->configs[2].eventConfigCompareOid,
         &kSourceDoorOpenCountOid[0],
         12U);
  SetOid(&config->configs[2].eventConfigLogOid,
         &kSourceDoorOpenCountOid[0],
         12U);
  config->trapRows[2][0].trapDestEnable = 1U;

  config->configs[3].preconfigured = 1U;
  config->configs[3].eventConfigClass = 2U;
  config->configs[3].eventConfigAction = EVENT_REPORT_ACTION_LOG;
  SetOid(&config->configs[3].eventConfigCompareOid,
         &kSourceDoorClosedCountOid[0],
         12U);
  SetOid(&config->configs[3].eventConfigLogOid,
         &kSourceDoorClosedCountOid[0],
         12U);
  config->trapRows[3][0].trapDestEnable = 1U;

  config->configs[4].preconfigured = 1U;
  config->configs[4].eventConfigClass = 3U;
  config->configs[4].eventConfigAction = EVENT_REPORT_ACTION_LOG;
  SetOid(&config->configs[4].eventConfigCompareOid,
         &kSourceCpMpLinkDegradedCountOid[0],
         12U);
  SetOid(&config->configs[4].eventConfigLogOid,
         &kSourceCpMpLinkDegradedCountOid[0],
         12U);
  config->trapRows[4][0].trapDestEnable = 1U;

  config->configs[5].preconfigured = 1U;
  config->configs[5].eventConfigClass = 3U;
  config->configs[5].eventConfigAction = EVENT_REPORT_ACTION_LOG;
  SetOid(&config->configs[5].eventConfigCompareOid,
         &kSourceCpMpLinkRestoredCountOid[0],
         12U);
  SetOid(&config->configs[5].eventConfigLogOid,
         &kSourceCpMpLinkRestoredCountOid[0],
         12U);
  config->trapRows[5][0].trapDestEnable = 1U;

  config->configs[6].preconfigured = 1U;
  config->configs[6].eventConfigClass = 4U;
  config->configs[6].eventConfigAction = EVENT_REPORT_ACTION_LOG;
  SetOid(&config->configs[6].eventConfigCompareOid,
         &kSourceMpEventCountOid[0],
         12U);
  SetOid(&config->configs[6].eventConfigLogOid,
         &kSourceMpEventDataOid[0],
         12U);
  config->trapRows[6][0].trapDestEnable = 1U;
}

static void InitDefaultConfiguration(EventReportConfiguration_t *config)
{
  if (config == NULL)
  {
    return;
  }

  (void) memset(config, 0, sizeof(*config));
  InitDefaultClasses(config);
  InitDefaultCommunities(config);
  InitDefaultLogicalNames(config);
  InitDefaultWatchRows(config);
  InitDefaultReportRows(config);
  InitDefaultTrapRows(config);
  InitDefaultEventConfigs(config);
}

static uint8_t ReadManagedValue(const EventReportService_t *service,
                                const NtcipOid_t *oid,
                                NtcipValue_t *value)
{
  NtcipOid_t instanceOid;

  if ((service == NULL) || (service->objectDirectory == NULL) || (value == NULL)
      || (OidIsNull(oid) != 0U))
  {
    return 0U;
  }

  if (NtcipObjectDirectoryGet(service->objectDirectory,
                              &oid->elements[0],
                              oid->length,
                              NULL,
                              value) == NTCIP_ERROR_OK)
  {
    return 1U;
  }

  if (oid->length >= NTCIP_OID_MAX_LENGTH)
  {
    return 0U;
  }

  instanceOid = *oid;
  instanceOid.elements[instanceOid.length] = 0U;
  instanceOid.length++;

  return (uint8_t) (NtcipObjectDirectoryGet(service->objectDirectory,
                                            &instanceOid.elements[0],
                                            instanceOid.length,
                                            NULL,
                                            value) == NTCIP_ERROR_OK);
}

uint8_t EventReportServiceValidateWatchObjectOid(
  const EventReportService_t *service,
  const NtcipOid_t *oid)
{
  NtcipValue_t value;

  if ((service == NULL) || (OidIsNull(oid) != 0U)
      || (OidForbiddenForBlockConfiguration(oid) != 0U)
      || (ReadManagedValue(service, oid, &value) == 0U))
  {
    return 0U;
  }

  return ValueIsIntegerLike(&value);
}

uint8_t EventReportServiceValidateReportObjectOid(
  const EventReportService_t *service,
  const NtcipOid_t *oid)
{
  NtcipValue_t value;

  if ((service == NULL) || (OidIsNull(oid) != 0U)
      || (OidForbiddenForBlockConfiguration(oid) != 0U)
      || (ReadManagedValue(service, oid, &value) == 0U))
  {
    return 0U;
  }

  return 1U;
}

static uint8_t AppendBlockEncodedValue(const EventReportService_t *service,
                                       const NtcipOid_t *oid,
                                       NtcipOctetString_t *value)
{
  NtcipValue_t currentValue;
  uint16_t encodedLength;

  if ((service == NULL) || (oid == NULL) || (value == NULL)
      || (ReadManagedValue(service, oid, &currentValue) == 0U))
  {
    return 0U;
  }

  encodedLength = EncodeOerOpenType(&currentValue,
                                    &value->bytes[value->length],
                                    (uint16_t) (sizeof(value->bytes)
                                                - value->length));
  if (encodedLength == 0U)
  {
    return 0U;
  }

  value->length = (uint16_t) (value->length + encodedLength);
  return 1U;
}

static uint8_t ValidateLogOid(const EventReportService_t *service,
                              const NtcipOid_t *oid)
{
  NtcipValue_t value;
  uint8_t ber[EVENT_REPORT_EVENT_VALUE_MAX_LENGTH];
  uint16_t berLength;

  if (OidIsNull(oid) != 0U)
  {
    return 1U;
  }

  if (OidStartsWith(oid,
                    &kWatchBlocksRootOid[0],
                    (uint8_t) (sizeof(kWatchBlocksRootOid)
                               / sizeof(kWatchBlocksRootOid[0])))
      != 0U)
  {
    return 0U;
  }

  if ((OidStartsWith(oid,
                     &kReportBlocksRootOid[0],
                     (uint8_t) (sizeof(kReportBlocksRootOid)
                                / sizeof(kReportBlocksRootOid[0])))
       != 0U)
      && (OidIsReportBlockValue(oid) == 0U))
  {
    return 0U;
  }

  if ((OidStartsWith(oid,
                     &kSecurityRootOid[0],
                     (uint8_t) (sizeof(kSecurityRootOid)
                                / sizeof(kSecurityRootOid[0]))) != 0U)
      || (ReadManagedValue(service, oid, &value) == 0U))
  {
    return 0U;
  }

  berLength = EncodeBerValue(&value, &ber[0]);
  return (uint8_t) ((berLength > 0U)
                    && (berLength <= EVENT_REPORT_EVENT_VALUE_MAX_LENGTH));
}

static uint8_t ValidateEventRow(const EventReportService_t *service,
                                EventReportConfigRow_t *row)
{
  NtcipValue_t value;

  if (row == NULL)
  {
    return 0U;
  }

  if (row->eventConfigAction == EVENT_REPORT_ACTION_DISABLED)
  {
    row->eventConfigStatus = EVENT_REPORT_STATUS_DISABLED;
    return 1U;
  }

  if ((row->eventConfigAction != EVENT_REPORT_ACTION_LOG)
      || (row->eventConfigClass == 0U)
      || (row->eventConfigClass > EVENT_REPORT_MAX_EVENT_CLASSES))
  {
    row->eventConfigStatus = EVENT_REPORT_STATUS_ERROR;
    return 0U;
  }

  switch (row->eventConfigMode)
  {
    case EVENT_REPORT_MODE_ON_CHANGE:
      if ((OidStartsWith(
             &row->eventConfigCompareOid,
             &kWatchBlocksRootOid[0],
             (uint8_t) (sizeof(kWatchBlocksRootOid)
                        / sizeof(kWatchBlocksRootOid[0])))
           != 0U)
          && (OidIsWatchBlockValue(&row->eventConfigCompareOid) == 0U))
      {
        row->eventConfigStatus = EVENT_REPORT_STATUS_ERROR;
        return 0U;
      }

      if (OidStartsWith(&row->eventConfigCompareOid,
                        &kReportBlocksRootOid[0],
                        (uint8_t) (sizeof(kReportBlocksRootOid)
                                   / sizeof(kReportBlocksRootOid[0])))
          != 0U)
      {
        row->eventConfigStatus = EVENT_REPORT_STATUS_ERROR;
        return 0U;
      }

      if ((OidIsNull(&row->eventConfigCompareOid) != 0U)
          || (ReadManagedValue(service, &row->eventConfigCompareOid, &value) == 0U))
      {
        row->eventConfigStatus = EVENT_REPORT_STATUS_ERROR;
        return 0U;
      }
      break;

    case EVENT_REPORT_MODE_GREATER_THAN_VALUE:
    case EVENT_REPORT_MODE_SMALLER_THAN_VALUE:
    case EVENT_REPORT_MODE_HYSTERESIS_BOUND:
    case EVENT_REPORT_MODE_ANDED_WITH_VALUE:
      if ((OidStartsWith(
             &row->eventConfigCompareOid,
             &kWatchBlocksRootOid[0],
             (uint8_t) (sizeof(kWatchBlocksRootOid)
                        / sizeof(kWatchBlocksRootOid[0])))
           != 0U)
          || (OidStartsWith(
                &row->eventConfigCompareOid,
                &kReportBlocksRootOid[0],
                (uint8_t) (sizeof(kReportBlocksRootOid)
                           / sizeof(kReportBlocksRootOid[0])))
              != 0U))
      {
        row->eventConfigStatus = EVENT_REPORT_STATUS_ERROR;
        return 0U;
      }

      if ((OidIsNull(&row->eventConfigCompareOid) != 0U)
          || (ReadManagedValue(service, &row->eventConfigCompareOid, &value) == 0U)
          || (ValueIsIntegerLike(&value) == 0U))
      {
        row->eventConfigStatus = EVENT_REPORT_STATUS_ERROR;
        return 0U;
      }
      break;

    case EVENT_REPORT_MODE_PERIODIC:
      if (row->eventConfigCompareValue <= 0)
      {
        row->eventConfigStatus = EVENT_REPORT_STATUS_ERROR;
        return 0U;
      }
      break;

    default:
      row->eventConfigStatus = EVENT_REPORT_STATUS_ERROR;
      return 0U;
  }

  if (ValidateLogOid(service, &row->eventConfigLogOid) == 0U)
  {
    row->eventConfigStatus = EVENT_REPORT_STATUS_ERROR;
    return 0U;
  }

  row->eventConfigStatus = EVENT_REPORT_STATUS_LOG;
  return 1U;
}

static void RefreshConfigStatus(EventReportService_t *service,
                                EventReportConfiguration_t *config)
{
  uint8_t index;

  if ((service == NULL) || (config == NULL))
  {
    return;
  }

  for (index = 0U; index < EVENT_REPORT_MAX_EVENT_LOG_CONFIGS; index++)
  {
    (void) ValidateEventRow(service, &config->configs[index]);
  }
}

static uint32_t ResolveEventTime(const EventReportService_t *service,
                                 uint32_t nowMs,
                                 uint16_t *milliseconds)
{
  uint32_t value = 0U;
  uint16_t fraction = (uint16_t) (nowMs % 1000U);

  if (milliseconds != NULL)
  {
    *milliseconds = fraction;
  }

  if ((service != NULL)
      && (service->globalTimeManagementService != NULL)
      && (GlobalTimeManagementServiceGetGlobalTimeWithMilliseconds(
            service->globalTimeManagementService,
            &value,
            &fraction) != 0U))
  {
    if (milliseconds != NULL)
    {
      *milliseconds = fraction;
    }

    return value;
  }

  return nowMs / 1000U;
}

static uint8_t BuildEventLogValue(const EventReportService_t *service,
                                  const NtcipOid_t *oid,
                                  EventReportLogRecord_t *record)
{
  NtcipValue_t value;
  uint16_t length;

  if ((record == NULL) || (service == NULL))
  {
    return 0U;
  }

  record->eventLogValueLength = 0U;
  (void) memset(&record->eventLogValue[0], 0, sizeof(record->eventLogValue));

  if (OidIsNull(oid) != 0U)
  {
    return 1U;
  }

  if (ReadManagedValue(service, oid, &value) == 0U)
  {
    return 0U;
  }

  length = EncodeBerValue(&value, &record->eventLogValue[0]);
  if ((length == 0U) || (length > EVENT_REPORT_EVENT_VALUE_MAX_LENGTH))
  {
    return 0U;
  }

  record->eventLogValueLength = (uint8_t) length;
  return 1U;
}

static uint16_t GetOldestIndex(const EventReportService_t *service)
{
  if (service == NULL)
  {
    return 0U;
  }

  if (service->count < EVENT_REPORT_MAX_EVENT_LOG_SIZE)
  {
    return 0U;
  }

  return service->writeIndex;
}

static uint16_t AdvanceIndex(uint16_t index)
{
  index++;
  if (index >= EVENT_REPORT_MAX_EVENT_LOG_SIZE)
  {
    index = 0U;
  }

  return index;
}

static uint16_t RetreatIndex(uint16_t index)
{
  if (index == 0U)
  {
    return (uint16_t) (EVENT_REPORT_MAX_EVENT_LOG_SIZE - 1U);
  }

  return (uint16_t) (index - 1U);
}

static uint8_t CopyLogsChronological(const EventReportService_t *service,
                                     EventReportLogRecord_t *records,
                                     uint16_t *count)
{
  uint16_t index;
  uint16_t current;

  if ((service == NULL) || (records == NULL) || (count == NULL))
  {
    return 0U;
  }

  *count = service->count;
  current = GetOldestIndex(service);
  for (index = 0U; index < service->count; index++)
  {
    records[index] = service->logRecords[current];
    current = AdvanceIndex(current);
  }

  return 1U;
}

static void ReplaceLogRecords(EventReportService_t *service,
                              const EventReportLogRecord_t *records,
                              uint16_t count)
{
  uint16_t index;

  if ((service == NULL) || ((records == NULL) && (count != 0U))
      || (count > EVENT_REPORT_MAX_EVENT_LOG_SIZE))
  {
    return;
  }

  (void) memset(&service->logRecords[0], 0, sizeof(service->logRecords));
  for (index = 0U; index < count; index++)
  {
    service->logRecords[index] = records[index];
  }

  service->count = count;
  if (count >= EVENT_REPORT_MAX_EVENT_LOG_SIZE)
  {
    service->writeIndex = 0U;
  }
  else
  {
    service->writeIndex = count;
  }
  service->fullSyncPending = 1U;
}

static void PruneByClassLimit(EventReportService_t *service, uint8_t eventClass)
{
  EventReportLogRecord_t records[EVENT_REPORT_MAX_EVENT_LOG_SIZE];
  EventReportLogRecord_t kept[EVENT_REPORT_MAX_EVENT_LOG_SIZE];
  uint16_t recordCount = 0U;
  uint16_t keptCount = 0U;
  uint16_t remainingForClass;
  uint16_t index;

  if ((service == NULL) || (eventClass == 0U)
      || (eventClass > EVENT_REPORT_MAX_EVENT_CLASSES))
  {
    return;
  }

  remainingForClass =
    service->activeConfig.classes[eventClass - 1U].eventClassLimit;
  (void) CopyLogsChronological(service, &records[0], &recordCount);

  for (index = recordCount; index > 0U; index--)
  {
    const EventReportLogRecord_t *source = &records[index - 1U];

    if ((source->eventLogClass == eventClass) && (remainingForClass == 0U))
    {
      continue;
    }

    if (source->eventLogClass == eventClass)
    {
      remainingForClass--;
    }

    kept[keptCount++] = *source;
  }

  for (index = 0U; index < keptCount / 2U; index++)
  {
    EventReportLogRecord_t temp = kept[index];

    kept[index] = kept[keptCount - 1U - index];
    kept[keptCount - 1U - index] = temp;
  }

  ReplaceLogRecords(service, &kept[0], keptCount);
}

static void RemoveLogsByPredicate(EventReportService_t *service,
                                  uint8_t eventClass,
                                  uint16_t eventId,
                                  uint8_t matchClass,
                                  uint8_t matchId)
{
  EventReportLogRecord_t records[EVENT_REPORT_MAX_EVENT_LOG_SIZE];
  EventReportLogRecord_t kept[EVENT_REPORT_MAX_EVENT_LOG_SIZE];
  uint16_t recordCount = 0U;
  uint16_t keptCount = 0U;
  uint16_t index;

  if (service == NULL)
  {
    return;
  }

  (void) CopyLogsChronological(service, &records[0], &recordCount);
  for (index = 0U; index < recordCount; index++)
  {
    uint8_t drop = 0U;

    if ((matchClass != 0U) && (records[index].eventLogClass == eventClass))
    {
      drop = 1U;
    }

    if ((matchId != 0U) && (records[index].eventLogID == eventId))
    {
      drop = 1U;
    }

    if (drop == 0U)
    {
      kept[keptCount++] = records[index];
    }
  }

  ReplaceLogRecords(service, &kept[0], keptCount);
}

static uint8_t QueuePersistAppend(EventReportService_t *service,
                                  const EventReportLogRecord_t *record)
{
  EventReportPersistenceOp_t *op;

  if ((service == NULL) || (record == NULL))
  {
    return 0U;
  }

  if (service->persistCount >= EVENT_REPORT_PERSIST_QUEUE_CAPACITY)
  {
    service->persistDropped++;
    service->fullSyncPending = 1U;
    return 0U;
  }

  op = &service->persistQueue[service->persistHead];
  op->kind = EVENT_REPORT_PERSIST_OP_APPEND;
  op->record = *record;
  service->persistHead++;
  service->persistHead %= EVENT_REPORT_PERSIST_QUEUE_CAPACITY;
  service->persistCount++;
  return 1U;
}

static void ResetPersistQueue(EventReportService_t *service)
{
  if (service == NULL)
  {
    return;
  }

  service->persistHead = 0U;
  service->persistTail = 0U;
  service->persistCount = 0U;
}

static uint8_t AppendToMemory(EventReportService_t *service,
                              const EventReportLogRecord_t *record)
{
  uint16_t currentIndex;
  uint8_t eventClass;

  if ((service == NULL) || (record == NULL) || (record->eventLogClass == 0U)
      || (record->eventLogClass > EVENT_REPORT_MAX_EVENT_CLASSES))
  {
    return 0U;
  }

  eventClass = record->eventLogClass;
  if (service->activeConfig.classes[eventClass - 1U].eventClassLimit == 0U)
  {
    return 1U;
  }

  while (EventReportServiceGetClassCount(service, eventClass)
         >= service->activeConfig.classes[eventClass - 1U].eventClassLimit)
  {
    PruneByClassLimit(service, eventClass);
  }

  currentIndex = service->writeIndex;
  service->logRecords[currentIndex] = *record;

  if (service->count < EVENT_REPORT_MAX_EVENT_LOG_SIZE)
  {
    service->count++;
  }

  service->writeIndex = AdvanceIndex(service->writeIndex);
  if (service->count < EVENT_REPORT_MAX_EVENT_LOG_SIZE)
  {
    service->writeIndex = service->count;
  }

  (void) QueuePersistAppend(service, record);
  return 1U;
}

static uint8_t TrapEnabledForConfig(const EventReportService_t *service,
                                    uint16_t eventConfigId)
{
  const EventReportTrapMgmtRow_t *trapRow;

  if ((service == NULL) || (eventConfigId == 0U)
      || (eventConfigId > EVENT_REPORT_MAX_EVENT_LOG_CONFIGS)
      || (service->activeConfig.trapControl == 0U))
  {
    return 0U;
  }

  trapRow = &service->activeConfig.trapMgmtRows[0];
  if (trapRow->trapMgmtRowStatus != EVENT_REPORT_ROW_STATUS_ACTIVE)
  {
    return 0U;
  }

  return (uint8_t) (service->activeConfig.trapRows[eventConfigId - 1U][0].
                    trapDestEnable != 0U);
}

static void BuildTrapData(EventReportService_t *service,
                          const EventReportLogRecord_t *record)
{
  NtcipValue_t encodedValue;
  uint16_t encodedLength;
  uint16_t offset = 0U;
  EventReportTrapMgmtRow_t *trapRow;
  EventReportTrapRow_t *trapConfig;
  uint8_t sequenceNumber;

  if ((service == NULL) || (record == NULL))
  {
    return;
  }

  trapRow = &service->activeConfig.trapMgmtRows[0];
  trapConfig = &service->activeConfig.trapRows[record->eventLogID - 1U][0];

  if (service->trapPending != 0U)
  {
    trapRow->trapMgmtLostTraps++;
  }

  sequenceNumber = trapRow->trapMgmtSeqNum;
  (void) memset(&service->latestTrapData, 0, sizeof(service->latestTrapData));
  service->latestTrapData.bytes[offset++] = sequenceNumber;
  service->latestTrapData.bytes[offset++] = trapRow->trapMgmtManagerIndex;
  NtcipValueSetUnsigned32(&encodedValue, record->eventLogID);
  encodedLength = EncodeOerOpenType(&encodedValue,
                                    &service->latestTrapData.bytes[offset],
                                    (uint16_t) (sizeof(service->latestTrapData
                                                       .bytes)
                                                - offset));
  if (encodedLength == 0U)
  {
    return;
  }
  offset = (uint16_t) (offset + encodedLength);
  NtcipValueSetUnsigned32(&encodedValue, record->eventLogTime);
  encodedLength = EncodeOerOpenType(&encodedValue,
                                    &service->latestTrapData.bytes[offset],
                                    (uint16_t) (sizeof(service->latestTrapData
                                                       .bytes)
                                                - offset));
  if (encodedLength == 0U)
  {
    return;
  }
  offset = (uint16_t) (offset + encodedLength);
  NtcipValueSetUnsigned32(&encodedValue, record->eventLogTimeMilliseconds);
  encodedLength = EncodeOerOpenType(&encodedValue,
                                    &service->latestTrapData.bytes[offset],
                                    (uint16_t) (sizeof(service->latestTrapData
                                                       .bytes)
                                                - offset));
  if (encodedLength == 0U)
  {
    return;
  }
  offset = (uint16_t) (offset + encodedLength);
  (void) NtcipValueSetOctetString(&encodedValue,
                                  &record->eventLogValue[0],
                                  record->eventLogValueLength);
  encodedLength = EncodeOerOpenType(&encodedValue,
                                    &service->latestTrapData.bytes[offset],
                                    (uint16_t) (sizeof(service->latestTrapData
                                                       .bytes)
                                                - offset));
  if (encodedLength == 0U)
  {
    return;
  }
  offset = (uint16_t) (offset + encodedLength);

  service->latestTrapData.length = offset;
  service->trapPending = 1U;
  trapRow->trapMgmtLinkStateStatus = EVENT_REPORT_TRAP_LINK_PENDING;
  trapRow->trapMgmtErrStatus = 0U;
  trapRow->trapMgmtSeqNumAck = sequenceNumber;
  trapConfig->trapCounter++;
  trapRow->trapMgmtSeqNum++;
  if (trapRow->trapMgmtSeqNum == 0U)
  {
    trapRow->trapMgmtSeqNum = 1U;
  }
}

static void HandleTriggeredEvent(EventReportService_t *service,
                                 const EventReportConfigRow_t *row,
                                 uint32_t nowMs)
{
  EventReportLogRecord_t record;

  if ((service == NULL) || (row == NULL)
      || (row->eventConfigStatus != EVENT_REPORT_STATUS_LOG))
  {
    return;
  }

  (void) memset(&record, 0, sizeof(record));
  record.eventLogClass = row->eventConfigClass;
  record.eventLogID = row->eventConfigID;
  record.eventLogTime = ResolveEventTime(service,
                                         nowMs,
                                         &record.eventLogTimeMilliseconds);

  if (record.eventLogTime
      <= service->activeConfig.classes[row->eventConfigClass - 1U].
      eventClassClearTime)
  {
    return;
  }

  if (BuildEventLogValue(service, &row->eventConfigLogOid, &record) == 0U)
  {
    return;
  }

  service->classEventCounters[row->eventConfigClass - 1U]++;
  service->totalEvents = (uint16_t) (service->totalEvents + 1U);

  if (service->activeConfig.classes[row->eventConfigClass - 1U].
      eventClassLimit > 0U)
  {
    (void) AppendToMemory(service, &record);
  }

  if (TrapEnabledForConfig(service, row->eventConfigID) != 0U)
  {
    BuildTrapData(service, &record);
  }
}

static void EvaluateRow(EventReportService_t *service,
                        EventReportConfigRow_t *row,
                        EventReportRuntimeRow_t *runtime,
                        uint32_t nowMs)
{
  NtcipValue_t currentValue;
  uint8_t triggered = 0U;
  uint8_t condition = 0U;
  int32_t signedValue;
  int32_t lowerBound;
  int32_t upperBound;
  uint32_t dwellMs;

  if ((service == NULL) || (row == NULL) || (runtime == NULL)
      || (row->eventConfigAction != EVENT_REPORT_ACTION_LOG)
      || (row->eventConfigStatus != EVENT_REPORT_STATUS_LOG))
  {
    return;
  }

  if (row->eventConfigMode == EVENT_REPORT_MODE_PERIODIC)
  {
    if ((runtime->lastPeriodicMs == 0U)
        || ((nowMs - runtime->lastPeriodicMs)
            >= ((uint32_t) row->eventConfigCompareValue * 1000U)))
    {
      runtime->lastPeriodicMs = nowMs;
      HandleTriggeredEvent(service, row, nowMs);
    }

    return;
  }

  if (ReadManagedValue(service, &row->eventConfigCompareOid, &currentValue) == 0U)
  {
    return;
  }

  switch (row->eventConfigMode)
  {
    case EVENT_REPORT_MODE_ON_CHANGE:
      if (runtime->lastValueValid != 0U)
      {
        triggered = (uint8_t) (ValuesEqual(&runtime->lastValue, &currentValue)
                               == 0U);
      }
      runtime->lastValue = currentValue;
      runtime->lastValueValid = 1U;
      break;

    case EVENT_REPORT_MODE_GREATER_THAN_VALUE:
      signedValue = ValueToSigned32(&currentValue);
      condition = (uint8_t) (signedValue > row->eventConfigCompareValue);
      break;

    case EVENT_REPORT_MODE_SMALLER_THAN_VALUE:
      signedValue = ValueToSigned32(&currentValue);
      condition = (uint8_t) (signedValue < row->eventConfigCompareValue);
      break;

    case EVENT_REPORT_MODE_HYSTERESIS_BOUND:
      signedValue = ValueToSigned32(&currentValue);
      lowerBound = row->eventConfigCompareValue;
      upperBound = row->eventConfigCompareValue2;
      if (lowerBound > upperBound)
      {
        int32_t swap = lowerBound;

        lowerBound = upperBound;
        upperBound = swap;
      }

      if ((runtime->lastStatus != 1U) && (signedValue > upperBound))
      {
        runtime->lastStatus = 1U;
        triggered = 1U;
      }
      else if ((runtime->lastStatus != 2U) && (signedValue < lowerBound))
      {
        runtime->lastStatus = 2U;
        triggered = 1U;
      }
      break;

    case EVENT_REPORT_MODE_ANDED_WITH_VALUE:
      signedValue = ValueToSigned32(&currentValue);
      condition = (uint8_t) (((uint32_t) signedValue
                              & (uint32_t) row->eventConfigCompareValue) != 0U);
      break;

    default:
      break;
  }

  if ((row->eventConfigMode == EVENT_REPORT_MODE_GREATER_THAN_VALUE)
      || (row->eventConfigMode == EVENT_REPORT_MODE_SMALLER_THAN_VALUE)
      || (row->eventConfigMode == EVENT_REPORT_MODE_ANDED_WITH_VALUE))
  {
    dwellMs = (row->eventConfigCompareValue2 <= 0) ? 0U
              : (uint32_t) row->eventConfigCompareValue2 * 100U;

    if (condition != 0U)
    {
      if (runtime->lastStatus == 0U)
      {
        runtime->lastStatus = 1U;
        runtime->lastTrueSinceMs = nowMs;
      }

      if ((dwellMs == 0U) || ((nowMs - runtime->lastTrueSinceMs) >= dwellMs))
      {
        if (runtime->lastValueValid == 0U)
        {
          triggered = 1U;
        }
        else if (ValuesEqual(&runtime->lastValue, &currentValue) == 0U)
        {
          triggered = 1U;
        }
      }
    }
    else
    {
      runtime->lastStatus = 0U;
      runtime->lastTrueSinceMs = 0U;
    }
  }

  runtime->lastValue = currentValue;
  runtime->lastValueValid = 1U;

  if (triggered != 0U)
  {
    HandleTriggeredEvent(service, row, nowMs);
  }
}

void EventReportServiceInit(EventReportService_t *service)
{
  if (service == NULL)
  {
    return;
  }

  (void) memset(service, 0, sizeof(*service));
  InitDefaultConfiguration(&service->activeConfig);
  service->candidateConfig = service->activeConfig;
}

void EventReportServiceBindLogRepository(EventReportService_t *service,
                                         ILogRepositoryPort_t *logRepositoryPort)
{
  if (service != NULL)
  {
    service->logRepositoryPort = logRepositoryPort;
  }
}

void EventReportServiceBindObjectDirectory(
  EventReportService_t *service,
  const NtcipObjectDirectory_t *objectDirectory)
{
  if (service != NULL)
  {
    service->objectDirectory = objectDirectory;
    RefreshConfigStatus(service, &service->activeConfig);
    RefreshConfigStatus(service, &service->candidateConfig);
  }
}

void EventReportServiceBindGlobalTimeManagementService(
  EventReportService_t *service,
  GlobalTimeManagementService_t *globalTimeManagementService)
{
  if (service != NULL)
  {
    service->globalTimeManagementService = globalTimeManagementService;
  }
}

void EventReportServiceLoadPersistedLog(EventReportService_t *service)
{
  EventReportLogRecord_t records[EVENT_REPORT_MAX_EVENT_LOG_SIZE];
  uint16_t count;
  uint16_t writeIndex;
  uint16_t currentIndex;
  uint16_t index;

  if ((service == NULL) || (service->logRepositoryPort == NULL)
      || (LogRepositoryExists(service->logRepositoryPort) == 0U))
  {
    return;
  }

  count = LogRepositoryGetCount(service->logRepositoryPort);
  writeIndex = LogRepositoryGetWriteIndex(service->logRepositoryPort);
  currentIndex = (count < EVENT_REPORT_MAX_EVENT_LOG_SIZE) ? 0U : writeIndex;

  for (index = 0U; index < count; index++)
  {
    if (LogRepositoryRead(service->logRepositoryPort,
                          currentIndex,
                          &records[index],
                          sizeof(records[index])) == 0U)
    {
      break;
    }

    currentIndex = AdvanceIndex(currentIndex);
  }

  ReplaceLogRecords(service, &records[0], index);
  service->fullSyncPending = 0U;
}

void EventReportServicePrime(EventReportService_t *service)
{
  uint8_t index;
  NtcipValue_t value;

  if ((service == NULL) || (service->primed != 0U))
  {
    return;
  }

  for (index = 0U; index < EVENT_REPORT_MAX_EVENT_LOG_CONFIGS; index++)
  {
    EventReportConfigRow_t *row = &service->activeConfig.configs[index];

    if ((row->eventConfigAction == EVENT_REPORT_ACTION_LOG)
        && (row->eventConfigStatus == EVENT_REPORT_STATUS_LOG)
        && (row->eventConfigMode != EVENT_REPORT_MODE_PERIODIC)
        && (ReadManagedValue(service, &row->eventConfigCompareOid, &value) != 0U))
    {
      service->runtimeRows[index].lastValue = value;
      service->runtimeRows[index].lastValueValid = 1U;
    }
  }

  service->primed = 1U;
}

void EventReportServiceStep(EventReportService_t *service, uint32_t nowMs)
{
  uint8_t index;

  if (service == NULL)
  {
    return;
  }

  if (service->primed == 0U)
  {
    EventReportServicePrime(service);
  }

  for (index = 0U; index < EVENT_REPORT_MAX_EVENT_LOG_CONFIGS; index++)
  {
    EvaluateRow(service,
                &service->activeConfig.configs[index],
                &service->runtimeRows[index],
                nowMs);
  }
}

void EventReportServiceRefreshWorkingConfig(EventReportService_t *service)
{
  EventReportConfiguration_t *config;

  if (service == NULL)
  {
    return;
  }

  config = (service->transactionActive != 0U)
           ? &service->candidateConfig
           : &service->activeConfig;
  RefreshConfigStatus(service, config);
}

void EventReportServiceApplySnmpCommunities(EventReportService_t *service,
                                            const char *readCommunity,
                                            const char *writeCommunity,
                                            const char *trapCommunity)
{
  if (service == NULL)
  {
    return;
  }

  ApplySnmpCommunitiesToConfig(&service->activeConfig,
                               readCommunity,
                               writeCommunity,
                               trapCommunity);
  ApplySnmpCommunitiesToConfig(&service->candidateConfig,
                               readCommunity,
                               writeCommunity,
                               trapCommunity);
}

void EventReportServiceCreateTransaction(EventReportService_t *service)
{
  if (service == NULL)
  {
    return;
  }

  service->candidateConfig = service->activeConfig;
  service->transactionActive = 1U;
  service->transactionVerified = 0U;
}

void EventReportServiceVerifyTransaction(EventReportService_t *service)
{
  if (service == NULL)
  {
    return;
  }

  RefreshConfigStatus(service, &service->candidateConfig);
  service->transactionVerified = 1U;
}

void EventReportServiceCommitTransaction(EventReportService_t *service)
{
  uint8_t classIndex;

  if (service == NULL)
  {
    return;
  }

  service->activeConfig = service->candidateConfig;
  RefreshConfigStatus(service, &service->activeConfig);
  for (classIndex = 0U; classIndex < EVENT_REPORT_MAX_EVENT_CLASSES;
       classIndex++)
  {
    PruneByClassLimit(service, (uint8_t) (classIndex + 1U));
  }
  service->primed = 0U;
  service->transactionActive = 0U;
  service->transactionVerified = 0U;
  service->fullSyncPending = 1U;
}

void EventReportServiceRollbackTransaction(EventReportService_t *service)
{
  if (service == NULL)
  {
    return;
  }

  service->candidateConfig = service->activeConfig;
  service->transactionActive = 0U;
  service->transactionVerified = 0U;
}

void EventReportServiceAppendLegacyEvent(EventReportService_t *service,
                                         uint8_t eventCode,
                                         uint8_t eventParam,
                                         uint16_t eventShortParam,
                                         uint32_t eventLongParam)
{
  if (service == NULL)
  {
    return;
  }

  switch (eventCode)
  {
    case EVENT_POWER_ON:
      service->eventSources.powerOnCount++;
      break;

    case EVENT_RESET_WINDOW_WATCHDOG:
    case EVENT_RESET_INDEPENDENT_WATCHDOG:
    case EVENT_RESET_LOW_POWER:
    case EVENT_RESET_POWER_ON_CLEAR_CIRCUIT:
    case EVENT_RESET_SOFTWARE:
    case EVENT_RESET_PIN:
    case EVENT_RESET_PORRST:
      service->eventSources.resetCause = eventCode;
      break;

    case EVENT_POWER_NORMAL_TO_STAND_BY:
      service->eventSources.standbyCount++;
      break;

    case EVENT_DOOR_OPEN:
      service->eventSources.doorOpenCount++;
      service->doorStateValid = 1U;
      service->doorOpen = 1U;
      break;

    case EVENT_DOOR_CLOSED:
      service->eventSources.doorClosedCount++;
      service->doorStateValid = 1U;
      service->doorOpen = 0U;
      break;

    case EVENT_CPMP_LINK_DEGRADED:
      service->eventSources.cpMpLinkDegradedCount++;
      service->cpMpLinkHealthyValid = 1U;
      service->cpMpLinkHealthy = 0U;
      break;

    case EVENT_CPMP_LINK_RESTORED:
      service->eventSources.cpMpLinkRestoredCount++;
      service->cpMpLinkHealthyValid = 1U;
      service->cpMpLinkHealthy = 1U;
      break;

    case EVENT_CPMP_MP_EVENT:
      service->eventSources.mpEventCount++;
      service->eventSources.mpEventData[0] = eventParam;
      service->eventSources.mpEventData[1] =
        (uint8_t) (eventShortParam & 0xFFU);
      service->eventSources.mpEventData[2] =
        (uint8_t) ((eventShortParam >> 8U) & 0xFFU);
      service->eventSources.mpEventData[3] =
        (uint8_t) (eventLongParam & 0xFFU);
      service->eventSources.mpEventData[4] =
        (uint8_t) ((eventLongParam >> 8U) & 0xFFU);
      service->eventSources.mpEventData[5] =
        (uint8_t) ((eventLongParam >> 16U) & 0xFFU);
      service->eventSources.mpEventData[6] =
        (uint8_t) ((eventLongParam >> 24U) & 0xFFU);
      break;

    default:
      break;
  }
}

void EventReportServiceUpdateDoorState(EventReportService_t *service,
                                       uint8_t open)
{
  if (service == NULL)
  {
    return;
  }

  if ((service->doorStateValid == 0U) || (service->doorOpen != open))
  {
    EventReportServiceAppendLegacyEvent(service,
                                        (open != 0U) ? EVENT_DOOR_OPEN
                                        : EVENT_DOOR_CLOSED,
                                        0U,
                                        0U,
                                        0U);
  }
}

void EventReportServiceUpdateCpMpLinkHealthy(EventReportService_t *service,
                                             uint8_t healthy)
{
  if (service == NULL)
  {
    return;
  }

  if ((service->cpMpLinkHealthyValid == 0U)
      || (service->cpMpLinkHealthy != healthy))
  {
    EventReportServiceAppendLegacyEvent(service,
                                        (healthy != 0U)
                                        ? EVENT_CPMP_LINK_RESTORED
                                        : EVENT_CPMP_LINK_DEGRADED,
                                        0U,
                                        0U,
                                        0U);
  }
}

uint8_t EventReportServiceReadWatchBlockValue(
  const EventReportService_t *service,
  uint8_t blockNumber,
  NtcipOctetString_t *value)
{
  const EventReportConfiguration_t *config = GetReadableConfig(service);
  uint8_t index;

  if ((config == NULL) || (value == NULL) || (blockNumber == 0U)
      || (blockNumber > EVENT_REPORT_MAX_WATCH_BLOCKS)
      || (config->watchBlockRows[blockNumber - 1U].watchBlockStatus
          != EVENT_REPORT_ROW_STATUS_ACTIVE))
  {
    return 0U;
  }

  (void) memset(value, 0, sizeof(*value));
  for (index = 0U; index < EVENT_REPORT_MAX_WATCH_OBJECTS; index++)
  {
    const EventReportWatchObjectRow_t *row = &config->watchObjectRows[index];

    if ((row->watchStatus != EVENT_REPORT_ROW_STATUS_ACTIVE)
        || (row->watchBlock != blockNumber)
        || (EventReportServiceValidateWatchObjectOid(service, &row->watchOid)
            == 0U)
        || (AppendBlockEncodedValue(service, &row->watchOid, value) == 0U))
    {
      if ((row->watchStatus == EVENT_REPORT_ROW_STATUS_ACTIVE)
          && (row->watchBlock == blockNumber))
      {
        return 0U;
      }
    }
  }

  return 1U;
}

uint8_t EventReportServiceReadReportBlockValue(
  const EventReportService_t *service,
  uint8_t blockNumber,
  NtcipOctetString_t *value)
{
  const EventReportConfiguration_t *config = GetReadableConfig(service);
  uint8_t index;

  if ((config == NULL) || (value == NULL) || (blockNumber == 0U)
      || (blockNumber > EVENT_REPORT_MAX_REPORT_BLOCKS)
      || (config->reportBlockRows[blockNumber - 1U].reportBlockStatus
          != EVENT_REPORT_ROW_STATUS_ACTIVE))
  {
    return 0U;
  }

  (void) memset(value, 0, sizeof(*value));
  for (index = 0U; index < EVENT_REPORT_MAX_REPORT_OBJECTS; index++)
  {
    const EventReportReportObjectRow_t *row = &config->reportObjectRows[index];

    if ((row->reportStatus != EVENT_REPORT_ROW_STATUS_ACTIVE)
        || (row->reportBlock != blockNumber)
        || (EventReportServiceValidateReportObjectOid(service, &row->reportOid)
            == 0U)
        || (AppendBlockEncodedValue(service, &row->reportOid, value) == 0U))
    {
      if ((row->reportStatus == EVENT_REPORT_ROW_STATUS_ACTIVE)
          && (row->reportBlock == blockNumber))
      {
        return 0U;
      }
    }
  }

  return 1U;
}

uint8_t EventReportServiceProcessPersistence(EventReportService_t *service)
{
  EventReportPersistenceOp_t *op;
  EventReportLogRecord_t records[EVENT_REPORT_MAX_EVENT_LOG_SIZE];
  uint16_t count;
  uint16_t index;

  if ((service == NULL) || (service->logRepositoryPort == NULL))
  {
    return 0U;
  }

  if (service->fullSyncPending != 0U)
  {
    if (LogRepositoryClear(service->logRepositoryPort) == 0U)
    {
      return 0U;
    }

    (void) CopyLogsChronological(service, &records[0], &count);
    for (index = 0U; index < count; index++)
    {
      if (LogRepositoryAppend(service->logRepositoryPort,
                              &records[index],
                              sizeof(records[index]),
                              NULL) == 0U)
      {
        return 0U;
      }
    }

    ResetPersistQueue(service);
    service->fullSyncPending = 0U;
    return 1U;
  }

  if (service->persistCount == 0U)
  {
    return 0U;
  }

  op = &service->persistQueue[service->persistTail];
  if ((op->kind != EVENT_REPORT_PERSIST_OP_APPEND)
      || (LogRepositoryAppend(service->logRepositoryPort,
                              &op->record,
                              sizeof(op->record),
                              NULL) == 0U))
  {
    return 0U;
  }

  service->persistTail++;
  service->persistTail %= EVENT_REPORT_PERSIST_QUEUE_CAPACITY;
  service->persistCount--;
  return 1U;
}

const EventReportConfiguration_t *EventReportServiceGetActiveConfig(
  const EventReportService_t *service)
{
  return (service == NULL) ? NULL : &service->activeConfig;
}

EventReportConfiguration_t *EventReportServiceGetCandidateConfig(
  EventReportService_t *service)
{
  return (service == NULL) ? NULL
         : (service->transactionActive != 0U) ? &service->candidateConfig
         : &service->activeConfig;
}

uint16_t EventReportServiceGetLatestLogIndex(const EventReportService_t *service)
{
  if ((service == NULL) || (service->count == 0U))
  {
    return 0xFFFFU;
  }

  if (service->count < EVENT_REPORT_MAX_EVENT_LOG_SIZE)
  {
    return (uint16_t) (service->count - 1U);
  }

  return RetreatIndex(service->writeIndex);
}

uint8_t EventReportServiceCanReadLogIndex(const EventReportService_t *service,
                                          uint16_t index)
{
  if ((service == NULL) || (service->count == 0U)
      || (index >= EVENT_REPORT_MAX_EVENT_LOG_SIZE))
  {
    return 0U;
  }

  if (service->count < EVENT_REPORT_MAX_EVENT_LOG_SIZE)
  {
    return (uint8_t) (index < service->count);
  }

  return 1U;
}

uint8_t EventReportServiceReadLogRecord(const EventReportService_t *service,
                                        uint16_t index,
                                        EventReportLogRecord_t *record)
{
  if ((EventReportServiceCanReadLogIndex(service, index) == 0U)
      || (record == NULL))
  {
    return 0U;
  }

  *record = service->logRecords[index];
  return 1U;
}

uint8_t EventReportServiceGetEventNumberForIndex(
  const EventReportService_t *service,
  uint16_t index,
  uint8_t *eventNumber)
{
  uint16_t current;
  uint16_t scanned;
  uint8_t count = 0U;
  uint8_t eventClass;

  if ((eventNumber == NULL)
      || (EventReportServiceCanReadLogIndex(service, index) == 0U))
  {
    return 0U;
  }

  eventClass = service->logRecords[index].eventLogClass;
  current = GetOldestIndex(service);
  for (scanned = 0U; scanned < service->count; scanned++)
  {
    if (service->logRecords[current].eventLogClass == eventClass)
    {
      count++;
    }

    if (current == index)
    {
      *eventNumber = count;
      return 1U;
    }

    current = AdvanceIndex(current);
  }

  return 0U;
}

uint8_t EventReportServiceFindLatestEventId(const EventReportService_t *service,
                                            uint16_t eventId,
                                            uint16_t *index)
{
  uint16_t current;
  uint16_t scanned;

  if ((service == NULL) || (index == NULL))
  {
    return 0U;
  }

  *index = 0xFFFFU;
  if (service->count == 0U)
  {
    return 1U;
  }

  current = EventReportServiceGetLatestLogIndex(service);
  for (scanned = 0U; scanned < service->count; scanned++)
  {
    if (service->logRecords[current].eventLogID == eventId)
    {
      *index = current;
      return 1U;
    }

    current = RetreatIndex(current);
  }

  return 1U;
}

uint8_t EventReportServiceReadLogByClassNumber(
  const EventReportService_t *service,
  uint8_t eventClass,
  uint8_t eventNumber,
  EventReportLogRecord_t *record)
{
  uint16_t current;
  uint16_t scanned;
  uint8_t count = 0U;

  if ((service == NULL) || (record == NULL) || (eventClass == 0U)
      || (eventNumber == 0U))
  {
    return 0U;
  }

  current = GetOldestIndex(service);
  for (scanned = 0U; scanned < service->count; scanned++)
  {
    if (service->logRecords[current].eventLogClass == eventClass)
    {
      count++;
      if (count == eventNumber)
      {
        *record = service->logRecords[current];
        return 1U;
      }
    }

    current = AdvanceIndex(current);
  }

  return 0U;
}

uint8_t EventReportServiceClearLog(EventReportService_t *service)
{
  ReplaceLogRecords(service, NULL, 0U);
  return 1U;
}

uint8_t EventReportServiceClearEventClass(EventReportService_t *service,
                                          uint8_t eventClass)
{
  if ((service == NULL) || (eventClass == 0U)
      || (eventClass > EVENT_REPORT_MAX_EVENT_CLASSES) || (eventClass <= 4U))
  {
    return 0U;
  }

  RemoveLogsByPredicate(service, eventClass, 0U, 1U, 0U);
  return 1U;
}

uint8_t EventReportServiceClearEventConfig(EventReportService_t *service,
                                           uint16_t eventConfigId)
{
  if ((service == NULL) || (eventConfigId == 0U)
      || (eventConfigId > EVENT_REPORT_MAX_EVENT_LOG_CONFIGS)
      || (service->activeConfig.configs[eventConfigId - 1U].preconfigured != 0U))
  {
    return 0U;
  }

  RemoveLogsByPredicate(service, 0U, eventConfigId, 0U, 1U);
  return 1U;
}

uint8_t EventReportServiceGetClassCount(const EventReportService_t *service,
                                        uint8_t eventClass)
{
  uint16_t current;
  uint16_t scanned;
  uint8_t count = 0U;

  if ((service == NULL) || (eventClass == 0U))
  {
    return 0U;
  }

  current = GetOldestIndex(service);
  for (scanned = 0U; scanned < service->count; scanned++)
  {
    if (service->logRecords[current].eventLogClass == eventClass)
    {
      count++;
    }

    current = AdvanceIndex(current);
  }

  return count;
}

uint16_t EventReportServiceGetNumEvents(const EventReportService_t *service)
{
  return (service == NULL) ? 0U : service->totalEvents;
}

uint16_t EventReportServiceGetPersistDropped(
  const EventReportService_t *service)
{
  return (service == NULL) ? 0U
         : (service->persistDropped > UINT16_MAX) ? UINT16_MAX
         : (uint16_t) service->persistDropped;
}

const NtcipOctetString_t *EventReportServiceGetLatestTrapData(
  const EventReportService_t *service)
{
  return (service == NULL) ? NULL : &service->latestTrapData;
}

uint8_t EventReportServiceCopyPendingTrap(const EventReportService_t *service,
                                          NtcipOctetString_t *trapData)
{
  if ((service == NULL) || (trapData == NULL) || (service->trapPending == 0U))
  {
    return 0U;
  }

  *trapData = service->latestTrapData;
  return 1U;
}

void EventReportServiceAcknowledgeTrapDispatch(EventReportService_t *service,
                                               uint8_t success)
{
  EventReportTrapMgmtRow_t *trapRow;

  if (service == NULL)
  {
    return;
  }

  trapRow = &service->activeConfig.trapMgmtRows[0];
  if (service->trapPending == 0U)
  {
    return;
  }

  service->trapPending = 0U;
  if (success != 0U)
  {
    return;
  }

  trapRow->trapMgmtLinkStateStatus = EVENT_REPORT_TRAP_LINK_ERROR;
  trapRow->trapMgmtErrStatus = 1U;
  trapRow->trapMgmtLostTraps++;
}

const EventReportEventSourceState_t *EventReportServiceGetEventSources(
  const EventReportService_t *service)
{
  return (service == NULL) ? NULL : &service->eventSources;
}
