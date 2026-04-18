/* App/Domain/NTCIP/Mib1202v0335/SequenceObjects.c
 *
 * NTCIP 1202 sequence plan projection for the controller-core ring subtree.
 */
#include "SequenceObjects.h"

#include <stddef.h>

enum
{
  SEQUENCE_OBJECT_TAG_MAX_SEQUENCES = 1,
  SEQUENCE_OBJECT_TAG_NUMBER,
  SEQUENCE_OBJECT_TAG_RING_NUMBER,
  SEQUENCE_OBJECT_TAG_DATA
};

static const uint32_t kMaxSequencesOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 7U, 2U
};
static const uint32_t kSequenceNumberOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 7U, 3U, 1U, 1U
};
static const uint32_t kSequenceRingNumberOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 7U, 3U, 1U, 2U
};
static const uint32_t kSequenceDataOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 7U, 3U, 1U, 3U
};

static NtcipError_t ValidateDatabaseWrite(const NtcipContext_t *context,
                                          const NtcipRequestContext_t *request)
{
  if ((context == NULL) || (context->dbTransactionService == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  return NtcipDbTransactionServiceValidateDatabaseWrite(
    context->dbTransactionService,
    request);
}

static NtcipError_t DecodeSequenceIndexes(const NtcipContext_t *context,
                                          const uint32_t *indexes,
                                          uint8_t indexCount,
                                          uint8_t *sequenceNumber,
                                          uint8_t *ringIndex)
{
  uint8_t maxSequences;
  uint8_t ringCount;

  if ((context == NULL) || (context->configurationService == NULL)
      || (indexes == NULL) || (indexCount != 2U) || (indexes[0] == 0U)
      || (indexes[1] == 0U))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  maxSequences = ConfigurationServiceGetSequenceCount(
    context->configurationService);
  ringCount = ConfigurationServiceGetRingCount(context->configurationService);

  if ((indexes[0] > maxSequences) || (indexes[1] > ringCount))
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  if (sequenceNumber != NULL)
  {
    *sequenceNumber = (uint8_t) indexes[0];
  }

  if (ringIndex != NULL)
  {
    *ringIndex = (uint8_t) (indexes[1] - 1U);
  }

  return NTCIP_ERROR_OK;
}

static NtcipError_t GetActiveSequenceEntry(const NtcipContext_t *context,
                                           const uint32_t *indexes,
                                           uint8_t indexCount,
                                           uint8_t *sequenceNumber,
                                           uint8_t *ringNumber,
                                           IntersectionRingPlan_t *ringPlan)
{
  NtcipError_t error;
  uint8_t ringIndex = 0U;
  uint8_t localSequenceNumber = 0U;

  error = DecodeSequenceIndexes(context,
                                indexes,
                                indexCount,
                                &localSequenceNumber,
                                &ringIndex);

  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  if ((ringPlan != NULL)
      && (ConfigurationServiceGetActiveSequenceRingPlan(
            context->configurationService,
            localSequenceNumber,
            ringIndex,
            ringPlan) == 0U))
  {
    return NTCIP_ERROR_GEN_ERROR;
  }

  if (sequenceNumber != NULL)
  {
    *sequenceNumber = localSequenceNumber;
  }

  if (ringNumber != NULL)
  {
    *ringNumber = (uint8_t) (ringIndex + 1U);
  }

  return NTCIP_ERROR_OK;
}

static NtcipError_t GetCandidateSequenceEntry(
  const NtcipContext_t *context,
  const uint32_t *indexes,
  uint8_t indexCount,
  uint8_t *ringIndex,
  IntersectionRingPlan_t *ringPlan)
{
  NtcipError_t error;
  uint8_t sequenceNumber = 0U;

  error = DecodeSequenceIndexes(context,
                                indexes,
                                indexCount,
                                &sequenceNumber,
                                ringIndex);

  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  if ((ringPlan == NULL)
      || (ConfigurationServiceGetCandidateSequenceRingPlan(
            context->configurationService,
            sequenceNumber,
            *ringIndex,
            ringPlan) == 0U))
  {
    return NTCIP_ERROR_GEN_ERROR;
  }

  return NTCIP_ERROR_OK;
}

static uint8_t RingPlanContainsPhaseNumber(const IntersectionRingPlan_t *ringPlan,
                                           uint8_t phaseNumber)
{
  uint8_t position;
  uint8_t phaseIndex;

  if ((ringPlan == NULL) || (phaseNumber == 0U))
  {
    return 0U;
  }

  phaseIndex = (uint8_t) (phaseNumber - 1U);

  for (position = 0U; position < ringPlan->phaseCount; position++)
  {
    if (ringPlan->phaseOrder[position] == phaseIndex)
    {
      return 1U;
    }
  }

  return 0U;
}

static NtcipError_t ValidateSequenceData(const NtcipContext_t *context,
                                         const IntersectionRingPlan_t *ringPlan,
                                         const NtcipValue_t *value)
{
  uint8_t seen[INTERSECTION_PHASE_COUNT_MAX] = { 0U };
  uint16_t length;
  uint16_t position;

  if ((context == NULL) || (context->configurationService == NULL)
      || (ringPlan == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  length = value->data.octetString.length;

  if ((length == 0U) || (length != ringPlan->phaseCount))
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  for (position = 0U; position < length; position++)
  {
    uint8_t phaseNumber = value->data.octetString.bytes[position];
    uint8_t phaseIndex;

    if ((phaseNumber == 0U)
        || (phaseNumber
            > ConfigurationServiceGetPhaseCount(context->configurationService)))
    {
      return NTCIP_ERROR_RANGE_ERROR;
    }

    phaseIndex = (uint8_t) (phaseNumber - 1U);

    if ((seen[phaseIndex] != 0U)
        || (RingPlanContainsPhaseNumber(ringPlan, phaseNumber) == 0U))
    {
      return NTCIP_ERROR_RANGE_ERROR;
    }

    seen[phaseIndex] = 1U;
  }

  return NTCIP_ERROR_OK;
}

static NtcipError_t GetSequenceObject(void *groupContext,
                                      const NtcipObjectDescriptor_t *descriptor,
                                      const uint32_t *indexes,
                                      uint8_t indexCount,
                                      const NtcipRequestContext_t *requestContext,
                                      NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  IntersectionRingPlan_t ringPlan;
  uint8_t sequenceNumber = 0U;
  uint8_t ringNumber = 0U;
  uint8_t sequenceData[INTERSECTION_RING_PHASE_COUNT_MAX];
  uint8_t position;
  NtcipError_t error;

  (void) requestContext;

  if ((context == NULL) || (context->configurationService == NULL)
      || (descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if (descriptor->tag == SEQUENCE_OBJECT_TAG_MAX_SEQUENCES)
  {
    NtcipValueSetUnsigned32(value,
                            ConfigurationServiceGetSequenceCount(
                              context->configurationService));

    return NTCIP_ERROR_OK;
  }

  error = GetActiveSequenceEntry(context,
                                 indexes,
                                 indexCount,
                                 &sequenceNumber,
                                 &ringNumber,
                                 &ringPlan);

  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  switch (descriptor->tag)
  {
      case SEQUENCE_OBJECT_TAG_NUMBER:
      {
        NtcipValueSetUnsigned32(value, sequenceNumber);

        return NTCIP_ERROR_OK;
      }

      case SEQUENCE_OBJECT_TAG_RING_NUMBER:
      {
        NtcipValueSetUnsigned32(value, ringNumber);

        return NTCIP_ERROR_OK;
      }

      case SEQUENCE_OBJECT_TAG_DATA:
      {
        for (position = 0U; position < ringPlan.phaseCount; position++)
        {
          sequenceData[position] =
            (uint8_t) (ringPlan.phaseOrder[position] + 1U);
        }

        return NtcipValueSetOctetString(value,
                                        sequenceData,
                                        ringPlan.phaseCount);
      }

      default:
      {
        return NTCIP_ERROR_NOT_FOUND;
      }
  }
}

static NtcipError_t SetTestSequenceObject(
  void *groupContext,
  const NtcipObjectDescriptor_t *descriptor,
  const uint32_t *indexes,
  uint8_t indexCount,
  const NtcipRequestContext_t *requestContext,
  const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  IntersectionRingPlan_t ringPlan;
  uint8_t ringIndex = 0U;
  NtcipError_t error;

  if ((context == NULL) || (descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if (descriptor->tag != SEQUENCE_OBJECT_TAG_DATA)
  {
    return NTCIP_ERROR_READ_ONLY;
  }

  error = ValidateDatabaseWrite(context, requestContext);

  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  error = GetCandidateSequenceEntry(context,
                                    indexes,
                                    indexCount,
                                    &ringIndex,
                                    &ringPlan);

  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  (void) ringIndex;

  return ValidateSequenceData(context, &ringPlan, value);
}

static NtcipError_t SetValueSequenceObject(
  void *groupContext,
  const NtcipObjectDescriptor_t *descriptor,
  const uint32_t *indexes,
  uint8_t indexCount,
  const NtcipRequestContext_t *requestContext,
  const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  uint8_t ringIndex = 0U;
  NtcipError_t error;

  error = SetTestSequenceObject(groupContext,
                                descriptor,
                                indexes,
                                indexCount,
                                requestContext,
                                value);

  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  error = DecodeSequenceIndexes(context,
                                indexes,
                                indexCount,
                                NULL,
                                &ringIndex);

  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  return (ConfigurationServiceSetRingSequenceData(
            context->configurationService,
            (uint8_t) indexes[0],
            ringIndex,
            value->data.octetString.bytes,
            (uint8_t) value->data.octetString.length) != 0U)
         ? NTCIP_ERROR_OK
         : NTCIP_ERROR_BAD_VALUE;
}

static const NtcipObjectDescriptor_t kSequenceObjects[] = {
  { kMaxSequencesOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    SEQUENCE_OBJECT_TAG_MAX_SEQUENCES, GetSequenceObject, NULL, NULL },
  { kSequenceNumberOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    SEQUENCE_OBJECT_TAG_NUMBER, GetSequenceObject, NULL, NULL },
  { kSequenceRingNumberOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    SEQUENCE_OBJECT_TAG_RING_NUMBER, GetSequenceObject, NULL, NULL },
  { kSequenceDataOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 2U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_OCTET_STRING,
    SEQUENCE_OBJECT_TAG_DATA, GetSequenceObject,
    SetTestSequenceObject, SetValueSequenceObject }
};

void SequenceObjectsRegister(NtcipObjectDirectory_t *directory,
                             NtcipContext_t *context)
{
  if ((directory == NULL) || (context == NULL))
  {
    return;
  }

  (void) NtcipObjectDirectoryRegisterGroup(
    directory,
    "1202.sequence",
    kSequenceObjects,
    (uint16_t) (sizeof(kSequenceObjects) / sizeof(kSequenceObjects[0])),
    context);
}
