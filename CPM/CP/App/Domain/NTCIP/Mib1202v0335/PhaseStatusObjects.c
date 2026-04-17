/* App/Domain/NTCIP/Mib1202v0335/PhaseStatusObjects.c
 *
 * Runtime-backed phase status group projection.
 */
#include "PhaseStatusObjects.h"

#include <stddef.h>

enum
{
  PHASE_STATUS_OBJECT_TAG_MAX_PHASE_GROUPS = 1,
  PHASE_STATUS_OBJECT_TAG_GROUP_NUMBER,
  PHASE_STATUS_OBJECT_TAG_REDS,
  PHASE_STATUS_OBJECT_TAG_YELLOWS,
  PHASE_STATUS_OBJECT_TAG_GREENS,
  PHASE_STATUS_OBJECT_TAG_DONT_WALKS,
  PHASE_STATUS_OBJECT_TAG_PED_CLEARS,
  PHASE_STATUS_OBJECT_TAG_WALKS,
  PHASE_STATUS_OBJECT_TAG_VEH_CALLS,
  PHASE_STATUS_OBJECT_TAG_PED_CALLS,
  PHASE_STATUS_OBJECT_TAG_PHASE_ONS,
  PHASE_STATUS_OBJECT_TAG_PHASE_NEXTS
};

static const uint32_t kMaxPhaseGroupsOid[] = { 1U, 3U, 6U, 1U, 4U, 1U, 1206U,
                                               4U, 2U, 1U, 3U };
static const uint32_t kPhaseStatusGroupNumberOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                       1206U, 4U, 2U, 1U, 4U,
                                                       1U, 1U };
static const uint32_t kPhaseStatusGroupRedsOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                     1206U, 4U, 2U, 1U, 4U, 1U,
                                                     2U };
static const uint32_t kPhaseStatusGroupYellowsOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                        1206U, 4U, 2U, 1U, 4U,
                                                        1U, 3U };
static const uint32_t kPhaseStatusGroupGreensOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                       1206U, 4U, 2U, 1U, 4U,
                                                       1U, 4U };
static const uint32_t kPhaseStatusGroupDontWalksOid[] = { 1U, 3U, 6U, 1U, 4U,
                                                          1U, 1206U, 4U, 2U, 1U,
                                                          4U, 1U, 5U };
static const uint32_t kPhaseStatusGroupPedClearsOid[] = { 1U, 3U, 6U, 1U, 4U,
                                                          1U, 1206U, 4U, 2U, 1U,
                                                          4U, 1U, 6U };
static const uint32_t kPhaseStatusGroupWalksOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                      1206U, 4U, 2U, 1U, 4U, 1U,
                                                      7U };
static const uint32_t kPhaseStatusGroupVehCallsOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                         1206U, 4U, 2U, 1U, 4U,
                                                         1U, 8U };
static const uint32_t kPhaseStatusGroupPedCallsOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                         1206U, 4U, 2U, 1U, 4U,
                                                         1U, 9U };
static const uint32_t kPhaseStatusGroupPhaseOnsOid[] = { 1U, 3U, 6U, 1U, 4U, 1U,
                                                         1206U, 4U, 2U, 1U, 4U,
                                                         1U, 10U };
static const uint32_t kPhaseStatusGroupPhaseNextsOid[] = { 1U, 3U, 6U, 1U, 4U,
                                                           1U, 1206U, 4U, 2U,
                                                           1U, 4U, 1U, 11U };

static NtcipError_t ReadPhaseStatusGroup(NtcipContext_t *context,
                                         const uint32_t *indexes,
                                         uint8_t indexCount,
                                         IntersectionPhaseStatusGroup_t *
                                         statusGroup)
{
  uint8_t maxGroups;

  if ((context == NULL) || (context->intersectionEngine == NULL)
      || (indexes == NULL) || (indexCount != 1U) || (statusGroup == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  maxGroups =
    (uint8_t) ((ConfigurationServiceGetPhaseCount(
                  context->configurationService) + 7U) / 8U);

  if ((indexes[0] == 0U) || (indexes[0] > maxGroups))
  {
    return NTCIP_ERROR_RANGE_ERROR;
  }

  if (IntersectionEngineGetPhaseStatusGroup(context->intersectionEngine,
                                            (uint8_t) indexes[0],
                                            statusGroup) == 0U)
  {
    return NTCIP_ERROR_GEN_ERROR;
  }

  return NTCIP_ERROR_OK;
}

static NtcipError_t GetPhaseStatusObject(void *groupContext,
                                         const NtcipObjectDescriptor_t *
                                         descriptor,
                                         const uint32_t *indexes,
                                         uint8_t indexCount,
                                         const NtcipRequestContext_t *
                                         requestContext,
                                         NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  IntersectionPhaseStatusGroup_t statusGroup;
  NtcipError_t error;

  (void) requestContext;

  if ((context == NULL) || (descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if (descriptor->tag == PHASE_STATUS_OBJECT_TAG_MAX_PHASE_GROUPS)
  {
    NtcipValueSetUnsigned32(value,
                            (uint32_t) ((ConfigurationServiceGetPhaseCount(
                                           context->configurationService)
                                         + 7U)
                                        / 8U));

    return NTCIP_ERROR_OK;
  }

  error = ReadPhaseStatusGroup(context, indexes, indexCount, &statusGroup);

  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  switch (descriptor->tag)
  {
      case PHASE_STATUS_OBJECT_TAG_GROUP_NUMBER:
      {
        NtcipValueSetUnsigned32(value, indexes[0]);

        return NTCIP_ERROR_OK;
      }

      case PHASE_STATUS_OBJECT_TAG_REDS:
      {
        NtcipValueSetUnsigned32(value, statusGroup.reds);

        return NTCIP_ERROR_OK;
      }

      case PHASE_STATUS_OBJECT_TAG_YELLOWS:
      {
        NtcipValueSetUnsigned32(value, statusGroup.yellows);

        return NTCIP_ERROR_OK;
      }

      case PHASE_STATUS_OBJECT_TAG_GREENS:
      {
        NtcipValueSetUnsigned32(value, statusGroup.greens);

        return NTCIP_ERROR_OK;
      }

      case PHASE_STATUS_OBJECT_TAG_DONT_WALKS:
      {
        NtcipValueSetUnsigned32(value, statusGroup.dontWalks);

        return NTCIP_ERROR_OK;
      }

      case PHASE_STATUS_OBJECT_TAG_PED_CLEARS:
      {
        NtcipValueSetUnsigned32(value, statusGroup.pedClears);

        return NTCIP_ERROR_OK;
      }

      case PHASE_STATUS_OBJECT_TAG_WALKS:
      {
        NtcipValueSetUnsigned32(value, statusGroup.walks);

        return NTCIP_ERROR_OK;
      }

      case PHASE_STATUS_OBJECT_TAG_VEH_CALLS:
      {
        NtcipValueSetUnsigned32(value, statusGroup.vehCalls);

        return NTCIP_ERROR_OK;
      }

      case PHASE_STATUS_OBJECT_TAG_PED_CALLS:
      {
        NtcipValueSetUnsigned32(value, statusGroup.pedCalls);

        return NTCIP_ERROR_OK;
      }

      case PHASE_STATUS_OBJECT_TAG_PHASE_ONS:
      {
        NtcipValueSetUnsigned32(value, statusGroup.phaseOns);

        return NTCIP_ERROR_OK;
      }

      case PHASE_STATUS_OBJECT_TAG_PHASE_NEXTS:
      {
        NtcipValueSetUnsigned32(value, statusGroup.phaseNexts);

        return NTCIP_ERROR_OK;
      }

      default:
      {
        return NTCIP_ERROR_NOT_FOUND;
      }
  } /* switch */
} /* GetPhaseStatusObject */

static const NtcipObjectDescriptor_t kPhaseStatusObjects[] =
{
  { kMaxPhaseGroupsOid, 11U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY,
    NTCIP_VALUE_TYPE_UNSIGNED32, PHASE_STATUS_OBJECT_TAG_MAX_PHASE_GROUPS,
    GetPhaseStatusObject, NULL, NULL },
  { kPhaseStatusGroupNumberOid, 13U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_STATUS_OBJECT_TAG_GROUP_NUMBER, GetPhaseStatusObject, NULL, NULL },
  { kPhaseStatusGroupRedsOid, 13U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_STATUS_OBJECT_TAG_REDS, GetPhaseStatusObject, NULL, NULL },
  { kPhaseStatusGroupYellowsOid, 13U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_STATUS_OBJECT_TAG_YELLOWS, GetPhaseStatusObject, NULL, NULL },
  { kPhaseStatusGroupGreensOid, 13U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_STATUS_OBJECT_TAG_GREENS, GetPhaseStatusObject, NULL, NULL },
  { kPhaseStatusGroupDontWalksOid, 13U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_STATUS_OBJECT_TAG_DONT_WALKS, GetPhaseStatusObject, NULL, NULL },
  { kPhaseStatusGroupPedClearsOid, 13U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_STATUS_OBJECT_TAG_PED_CLEARS, GetPhaseStatusObject, NULL, NULL },
  { kPhaseStatusGroupWalksOid, 13U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_STATUS_OBJECT_TAG_WALKS, GetPhaseStatusObject, NULL, NULL },
  { kPhaseStatusGroupVehCallsOid, 13U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_STATUS_OBJECT_TAG_VEH_CALLS, GetPhaseStatusObject, NULL, NULL },
  { kPhaseStatusGroupPedCallsOid, 13U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_STATUS_OBJECT_TAG_PED_CALLS, GetPhaseStatusObject, NULL, NULL },
  { kPhaseStatusGroupPhaseOnsOid, 13U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_STATUS_OBJECT_TAG_PHASE_ONS, GetPhaseStatusObject, NULL, NULL },
  { kPhaseStatusGroupPhaseNextsOid, 13U, NTCIP_OBJECT_KIND_TABLE_COLUMN, 1U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    PHASE_STATUS_OBJECT_TAG_PHASE_NEXTS, GetPhaseStatusObject, NULL, NULL }
};

void PhaseStatusObjectsRegister(NtcipObjectDirectory_t *directory,
                                NtcipContext_t *context)
{
  (void) NtcipObjectDirectoryRegisterGroup(
    directory,
    "1202.phaseStatus",
    kPhaseStatusObjects,
    (uint16_t) (sizeof(kPhaseStatusObjects) / sizeof(kPhaseStatusObjects[0])),
    context);
}
