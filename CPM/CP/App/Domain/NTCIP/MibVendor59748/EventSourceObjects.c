/* App/Domain/NTCIP/MibVendor59748/EventSourceObjects.c
 *
 * Synthetic event-source objects under the vendor subtree. These are CP-owned
 * counters and payloads that the 1103 event engine evaluates.
 */
#include "EventSourceObjects.h"

#include "Domain/Services/EventReportService.h"

enum
{
  EVENT_SOURCE_TAG_POWER_ON_COUNT = 1,
  EVENT_SOURCE_TAG_RESET_CAUSE = 2,
  EVENT_SOURCE_TAG_STANDBY_COUNT = 3,
  EVENT_SOURCE_TAG_DOOR_OPEN_COUNT = 4,
  EVENT_SOURCE_TAG_DOOR_CLOSED_COUNT = 5,
  EVENT_SOURCE_TAG_CPMP_LINK_DEGRADED_COUNT = 6,
  EVENT_SOURCE_TAG_CPMP_LINK_RESTORED_COUNT = 7,
  EVENT_SOURCE_TAG_MP_EVENT_COUNT = 8,
  EVENT_SOURCE_TAG_MP_EVENT_DATA = 9
};

static const uint32_t kPowerOnCountOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 22U, 1U
};
static const uint32_t kResetCauseOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 22U, 2U
};
static const uint32_t kStandbyCountOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 22U, 3U
};
static const uint32_t kDoorOpenCountOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 22U, 4U
};
static const uint32_t kDoorClosedCountOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 22U, 5U
};
static const uint32_t kCpMpLinkDegradedCountOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 22U, 6U
};
static const uint32_t kCpMpLinkRestoredCountOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 22U, 7U
};
static const uint32_t kMpEventCountOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 22U, 8U
};
static const uint32_t kMpEventDataOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 22U, 9U
};

static NtcipError_t GetEventSourceObject(void *groupContext,
                                         const NtcipObjectDescriptor_t *descriptor,
                                         const uint32_t *indexes,
                                         uint8_t indexCount,
                                         const NtcipRequestContext_t *requestContext,
                                         NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  const EventReportEventSourceState_t *sources;

  (void) indexes;
  (void) indexCount;
  (void) requestContext;

  if ((context == NULL) || (descriptor == NULL) || (value == NULL)
      || (context->eventReportService == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  sources = EventReportServiceGetEventSources(context->eventReportService);
  if (sources == NULL)
  {
    return NTCIP_ERROR_GEN_ERROR;
  }

  switch (descriptor->tag)
  {
    case EVENT_SOURCE_TAG_POWER_ON_COUNT:
      NtcipValueSetUnsigned32(value, sources->powerOnCount);
      return NTCIP_ERROR_OK;

    case EVENT_SOURCE_TAG_RESET_CAUSE:
      NtcipValueSetUnsigned32(value, sources->resetCause);
      return NTCIP_ERROR_OK;

    case EVENT_SOURCE_TAG_STANDBY_COUNT:
      NtcipValueSetUnsigned32(value, sources->standbyCount);
      return NTCIP_ERROR_OK;

    case EVENT_SOURCE_TAG_DOOR_OPEN_COUNT:
      NtcipValueSetUnsigned32(value, sources->doorOpenCount);
      return NTCIP_ERROR_OK;

    case EVENT_SOURCE_TAG_DOOR_CLOSED_COUNT:
      NtcipValueSetUnsigned32(value, sources->doorClosedCount);
      return NTCIP_ERROR_OK;

    case EVENT_SOURCE_TAG_CPMP_LINK_DEGRADED_COUNT:
      NtcipValueSetUnsigned32(value, sources->cpMpLinkDegradedCount);
      return NTCIP_ERROR_OK;

    case EVENT_SOURCE_TAG_CPMP_LINK_RESTORED_COUNT:
      NtcipValueSetUnsigned32(value, sources->cpMpLinkRestoredCount);
      return NTCIP_ERROR_OK;

    case EVENT_SOURCE_TAG_MP_EVENT_COUNT:
      NtcipValueSetUnsigned32(value, sources->mpEventCount);
      return NTCIP_ERROR_OK;

    case EVENT_SOURCE_TAG_MP_EVENT_DATA:
      return NtcipValueSetOctetString(value,
                                      &sources->mpEventData[0],
                                      sizeof(sources->mpEventData));

    default:
      return NTCIP_ERROR_NOT_FOUND;
  }
}

static const NtcipObjectDescriptor_t kEventSourceObjects[] =
{
  { kPowerOnCountOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    EVENT_SOURCE_TAG_POWER_ON_COUNT, GetEventSourceObject, NULL, NULL },
  { kResetCauseOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    EVENT_SOURCE_TAG_RESET_CAUSE, GetEventSourceObject, NULL, NULL },
  { kStandbyCountOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    EVENT_SOURCE_TAG_STANDBY_COUNT, GetEventSourceObject, NULL, NULL },
  { kDoorOpenCountOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    EVENT_SOURCE_TAG_DOOR_OPEN_COUNT, GetEventSourceObject, NULL, NULL },
  { kDoorClosedCountOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    EVENT_SOURCE_TAG_DOOR_CLOSED_COUNT, GetEventSourceObject, NULL, NULL },
  { kCpMpLinkDegradedCountOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    EVENT_SOURCE_TAG_CPMP_LINK_DEGRADED_COUNT, GetEventSourceObject, NULL,
    NULL },
  { kCpMpLinkRestoredCountOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    EVENT_SOURCE_TAG_CPMP_LINK_RESTORED_COUNT, GetEventSourceObject, NULL,
    NULL },
  { kMpEventCountOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_UNSIGNED32,
    EVENT_SOURCE_TAG_MP_EVENT_COUNT, GetEventSourceObject, NULL, NULL },
  { kMpEventDataOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_ONLY, NTCIP_VALUE_TYPE_OCTET_STRING,
    EVENT_SOURCE_TAG_MP_EVENT_DATA, GetEventSourceObject, NULL, NULL }
};

void TeknotelEventSourceObjectsRegister(NtcipObjectDirectory_t *directory,
                                        NtcipContext_t *context)
{
  (void) NtcipObjectDirectoryRegisterGroup(
    directory,
    "59748.eventSource",
    kEventSourceObjects,
    (uint16_t) (sizeof(kEventSourceObjects) / sizeof(kEventSourceObjects[0])),
    context);
}
