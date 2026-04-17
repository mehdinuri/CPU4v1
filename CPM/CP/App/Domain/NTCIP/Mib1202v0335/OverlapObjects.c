/* App/Domain/NTCIP/Mib1202v0335/OverlapObjects.c
 *
 * Controller-core overlap subtree projection.
 */
#include "OverlapObjects.h"

#include <stddef.h>

enum
{
  OVERLAP_OBJECT_TAG_MAX_OVERLAPS = 1,
  OVERLAP_OBJECT_TAG_NUMBER,
  OVERLAP_OBJECT_TAG_TYPE,
  OVERLAP_OBJECT_TAG_INCLUDED_PHASES,
  OVERLAP_OBJECT_TAG_MODIFIER_PHASES,
  OVERLAP_OBJECT_TAG_TRAIL_GREEN,
  OVERLAP_OBJECT_TAG_TRAIL_YELLOW,
  OVERLAP_OBJECT_TAG_TRAIL_RED,
  OVERLAP_OBJECT_TAG_WALK,
  OVERLAP_OBJECT_TAG_PED_CLEARANCE,
  OVERLAP_OBJECT_TAG_CONFLICTING_PED_PHASES
};

static const uint32_t kMaxOverlapsOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U,
                                            2U, 1U, 9U, 1U };
static const uint32_t kOverlapNumberOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U,
                                              2U, 1U, 9U, 2U, 1U, 1U };
static const uint32_t kOverlapTypeOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U,
                                            2U, 1U, 9U, 2U, 1U, 2U };
static const uint32_t kOverlapIncludedPhasesOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                      1206U, 4U, 2U, 1U, 9U, 2U,
                                                      1U, 3U };
static const uint32_t kOverlapModifierPhasesOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                      1206U, 4U, 2U, 1U, 9U, 2U,
                                                      1U, 4U };
static const uint32_t kOverlapTrailGreenOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U,
                                                  4U, 2U, 1U, 9U, 2U, 1U, 5U };
static const uint32_t kOverlapTrailYellowOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                   1206U, 4U, 2U, 1U, 9U, 2U,
                                                   1U, 6U };
static const uint32_t kOverlapTrailRedOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U,
                                                4U, 2U, 1U, 9U, 2U, 1U, 7U };
static const uint32_t kOverlapWalkOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U,
                                            2U, 1U, 9U, 2U, 1U, 8U };
static const uint32_t kOverlapPedClearanceOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                    1206U, 4U, 2U, 1U, 9U, 2U,
                                                    1U, 9U };
static const uint32_t kOverlapConflictingPedPhasesOid[] = { 1U, 3U, 6U, 1U, 4U,
                                                            1U, 1206U, 4U, 2U,
                                                            1U, 9U, 2U, 1U,
                                                            10U };

static NtcipError_t GetOverlapFromIndex(const NtcipContext_t *context,
                                        const uint32_t *indexes,
                                        uint8_t indexCount,
                                        uint8_t *overlapIndex,
                                        IntersectionOverlapConfig_t *overlap)
{
  if ((context == NULL) || (context->configurationService == NULL)
      || (indexes == NULL) || (indexCount != 1U) || (indexes[0] == 0U)
      || (overlapIndex == NULL) || (overlap == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if (indexes[0]
      > ConfigurationServiceGetOverlapCount(context->configurationService))
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  *overlapIndex = (uint8_t) (indexes[0] - 1U);

  if (ConfigurationServiceGetActiveOverlapConfig(context->configurationService,
                                                 *overlapIndex,
                                                 overlap) == 0U)
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  return NTCIP_ERROR_OK;
}

static NtcipError_t ValidateOverlapWriteRequest(const NtcipContext_t *context,
                                                const uint32_t *indexes,
                                                uint8_t indexCount,
                                                const NtcipRequestContext_t *
                                                requestContext,
                                                IntersectionOverlapConfig_t *
                                                overlap,
                                                uint8_t *overlapIndex)
{
  NtcipError_t error;

  if ((context == NULL) || (context->dbTransactionService == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  error = NtcipDbTransactionServiceValidateDatabaseWrite(
    context->dbTransactionService,
    requestContext);

  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  return GetOverlapFromIndex(context, indexes, indexCount, overlapIndex,
                             overlap);
}

static NtcipError_t GetOverlapObject(void *groupContext,
                                     const NtcipObjectDescriptor_t *descriptor,
                                     const uint32_t *indexes,
                                     uint8_t indexCount,
                                     const NtcipRequestContext_t *requestContext,
                                     NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  IntersectionOverlapConfig_t overlap;
  uint8_t overlapIndex;
  NtcipError_t error;

  (void) requestContext;

  if ((context == NULL) || (descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if (descriptor->tag == OVERLAP_OBJECT_TAG_MAX_OVERLAPS)
  {
    NtcipValueSetUnsigned32(value,
                            ConfigurationServiceGetOverlapCount(
                              context->configurationService));

    return NTCIP_ERROR_OK;
  }

  error = GetOverlapFromIndex(context,
                              indexes,
                              indexCount,
                              &overlapIndex,
                              &overlap);

  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  switch (descriptor->tag)
  {
      case OVERLAP_OBJECT_TAG_NUMBER:
      {
        NtcipValueSetUnsigned32(value, indexes[0]);

        return NTCIP_ERROR_OK;
      }

      case OVERLAP_OBJECT_TAG_TYPE:
      {
        NtcipValueSetUnsigned32(value, overlap.type);

        return NTCIP_ERROR_OK;
      }

      case OVERLAP_OBJECT_TAG_INCLUDED_PHASES:
      {
        return NtcipValueSetOctetString(value,
                                        overlap.includedPhases.values,
                                        overlap.includedPhases.length);
      }

      case OVERLAP_OBJECT_TAG_MODIFIER_PHASES:
      {
        return NtcipValueSetOctetString(value,
                                        overlap.modifierPhases.values,
                                        overlap.modifierPhases.length);
      }

      case OVERLAP_OBJECT_TAG_TRAIL_GREEN:
      {
        NtcipValueSetUnsigned32(value, overlap.trailGreenDs / 10U);

        return NTCIP_ERROR_OK;
      }

      case OVERLAP_OBJECT_TAG_TRAIL_YELLOW:
      {
        NtcipValueSetUnsigned32(value, overlap.trailYellowDs);

        return NTCIP_ERROR_OK;
      }

      case OVERLAP_OBJECT_TAG_TRAIL_RED:
      {
        NtcipValueSetUnsigned32(value, overlap.trailRedDs);

        return NTCIP_ERROR_OK;
      }

      case OVERLAP_OBJECT_TAG_WALK:
      {
        NtcipValueSetUnsigned32(value, overlap.walkSeconds);

        return NTCIP_ERROR_OK;
      }

      case OVERLAP_OBJECT_TAG_PED_CLEARANCE:
      {
        NtcipValueSetUnsigned32(value, overlap.pedClearSeconds);

        return NTCIP_ERROR_OK;
      }

      case OVERLAP_OBJECT_TAG_CONFLICTING_PED_PHASES:
      {
        return NtcipValueSetOctetString(value,
                                        overlap.conflictingPedPhases.values,
                                        overlap.conflictingPedPhases.length);
      }

      default:
      {
        return NTCIP_ERROR_NOT_FOUND;
      }
  } /* switch */
} /* GetOverlapObject */

static NtcipError_t SetTestOverlapObject(void *groupContext,
                                         const NtcipObjectDescriptor_t *
                                         descriptor,
                                         const uint32_t *indexes,
                                         uint8_t indexCount,
                                         const NtcipRequestContext_t *
                                         requestContext,
                                         const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  IntersectionOverlapConfig_t overlap;
  uint8_t overlapIndex;
  NtcipError_t error;

  error = ValidateOverlapWriteRequest(context,
                                      indexes,
                                      indexCount,
                                      requestContext,
                                      &overlap,
                                      &overlapIndex);

  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  (void) overlap;
  (void) overlapIndex;

  switch (descriptor->tag)
  {
      case OVERLAP_OBJECT_TAG_TYPE:
      {
        return ((value->data.unsigned32 >= 1UL)
                && (value->data.unsigned32 <= 10UL))
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      case OVERLAP_OBJECT_TAG_INCLUDED_PHASES:
      case OVERLAP_OBJECT_TAG_MODIFIER_PHASES:
      case OVERLAP_OBJECT_TAG_CONFLICTING_PED_PHASES:
      {
        return (value->data.octetString.length <= INTERSECTION_PHASE_COUNT_MAX)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      case OVERLAP_OBJECT_TAG_TRAIL_GREEN:
      case OVERLAP_OBJECT_TAG_TRAIL_YELLOW:
      case OVERLAP_OBJECT_TAG_TRAIL_RED:
      case OVERLAP_OBJECT_TAG_WALK:
      case OVERLAP_OBJECT_TAG_PED_CLEARANCE:
      {
        return (value->data.unsigned32 <= 255UL)
               ? NTCIP_ERROR_OK
               : NTCIP_ERROR_RANGE_ERROR;
      }

      default:
      {
        return NTCIP_ERROR_READ_ONLY;
      }
  }
} /* SetTestOverlapObject */

static NtcipError_t SetValueOverlapObject(void *groupContext,
                                          const NtcipObjectDescriptor_t *
                                          descriptor,
                                          const uint32_t *indexes,
                                          uint8_t indexCount,
                                          const NtcipRequestContext_t *
                                          requestContext,
                                          const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  IntersectionOverlapConfig_t overlap;
  uint8_t overlapIndex;
  NtcipError_t error;
  uint8_t ok;

  error = ValidateOverlapWriteRequest(context,
                                      indexes,
                                      indexCount,
                                      requestContext,
                                      &overlap,
                                      &overlapIndex);

  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  switch (descriptor->tag)
  {
      case OVERLAP_OBJECT_TAG_TYPE:
      {
        ok = ConfigurationServiceSetOverlapType(context->configurationService,
                                                overlapIndex,
                                                (uint8_t) value->data.unsigned32);
        break;
      }

      case OVERLAP_OBJECT_TAG_INCLUDED_PHASES:
      {
        ok = ConfigurationServiceSetOverlapIncludedPhases(
          context->configurationService,
          overlapIndex,
          value->data.octetString.bytes,
          (uint8_t) value->data.octetString.length);
        break;
      }

      case OVERLAP_OBJECT_TAG_MODIFIER_PHASES:
      {
        ok = ConfigurationServiceSetOverlapModifierPhases(
          context->configurationService,
          overlapIndex,
          value->data.octetString.bytes,
          (uint8_t) value->data.octetString.length);
        break;
      }

      case OVERLAP_OBJECT_TAG_TRAIL_GREEN:
      {
        ok = ConfigurationServiceSetOverlapTrailGreenDs(
          context->configurationService,
          overlapIndex,
          (uint16_t) (value->data.unsigned32 * 10U));
        break;
      }

      case OVERLAP_OBJECT_TAG_TRAIL_YELLOW:
      {
        ok = ConfigurationServiceSetOverlapTrailYellowDs(
          context->configurationService,
          overlapIndex,
          (uint16_t) value->data.unsigned32);
        break;
      }

      case OVERLAP_OBJECT_TAG_TRAIL_RED:
      {
        ok = ConfigurationServiceSetOverlapTrailRedDs(
          context->configurationService,
          overlapIndex,
          (uint16_t) value->data.unsigned32);
        break;
      }

      case OVERLAP_OBJECT_TAG_WALK:
      {
        ok = ConfigurationServiceSetOverlapWalkSeconds(
          context->configurationService,
          overlapIndex,
          (uint16_t) value->data.unsigned32);
        break;
      }

      case OVERLAP_OBJECT_TAG_PED_CLEARANCE:
      {
        ok = ConfigurationServiceSetOverlapPedClearSeconds(
          context->configurationService,
          overlapIndex,
          (uint16_t) value->data.unsigned32);
        break;
      }

      case OVERLAP_OBJECT_TAG_CONFLICTING_PED_PHASES:
      {
        ok = ConfigurationServiceSetOverlapConflictingPedPhases(
          context->configurationService,
          overlapIndex,
          value->data.octetString.bytes,
          (uint8_t) value->data.octetString.length);
        break;
      }

      default:
      {
        return NTCIP_ERROR_READ_ONLY;
      }
  } /* switch */

  return (ok != 0U) ? NTCIP_ERROR_OK : NTCIP_ERROR_BAD_VALUE;
} /* SetValueOverlapObject */

static const NtcipObjectDescriptor_t kOverlapObjects[] =
{
  { kMaxOverlapsOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U, NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, OVERLAP_OBJECT_TAG_MAX_OVERLAPS,
    GetOverlapObject, NULL, NULL },
  { kOverlapNumberOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, OVERLAP_OBJECT_TAG_NUMBER,
    GetOverlapObject, NULL, NULL },
  { kOverlapTypeOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, OVERLAP_OBJECT_TAG_TYPE,
    GetOverlapObject, SetTestOverlapObject, SetValueOverlapObject },
  { kOverlapIncludedPhasesOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_OCTET_STRING, OVERLAP_OBJECT_TAG_INCLUDED_PHASES,
    GetOverlapObject, SetTestOverlapObject, SetValueOverlapObject },
  { kOverlapModifierPhasesOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_OCTET_STRING, OVERLAP_OBJECT_TAG_MODIFIER_PHASES,
    GetOverlapObject, SetTestOverlapObject, SetValueOverlapObject },
  { kOverlapTrailGreenOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, OVERLAP_OBJECT_TAG_TRAIL_GREEN,
    GetOverlapObject, SetTestOverlapObject, SetValueOverlapObject },
  { kOverlapTrailYellowOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, OVERLAP_OBJECT_TAG_TRAIL_YELLOW,
    GetOverlapObject, SetTestOverlapObject, SetValueOverlapObject },
  { kOverlapTrailRedOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, OVERLAP_OBJECT_TAG_TRAIL_RED,
    GetOverlapObject, SetTestOverlapObject, SetValueOverlapObject },
  { kOverlapWalkOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, OVERLAP_OBJECT_TAG_WALK,
    GetOverlapObject, SetTestOverlapObject, SetValueOverlapObject },
  { kOverlapPedClearanceOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_UNSIGNED32, OVERLAP_OBJECT_TAG_PED_CLEARANCE,
    GetOverlapObject, SetTestOverlapObject, SetValueOverlapObject },
  { kOverlapConflictingPedPhasesOid, 14U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_WRITE,
    NTCIP_VALUE_TYPE_OCTET_STRING, OVERLAP_OBJECT_TAG_CONFLICTING_PED_PHASES,
    GetOverlapObject, SetTestOverlapObject, SetValueOverlapObject }
};

void OverlapObjectsRegister(NtcipObjectDirectory_t *directory,
                            NtcipContext_t *context)
{
  (void) NtcipObjectDirectoryRegisterGroup(
    directory,
    "1202.overlap",
    kOverlapObjects,
    (uint16_t) (sizeof(kOverlapObjects) / sizeof(kOverlapObjects[0])),
    context);
}
