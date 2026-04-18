/* App/Domain/NTCIP/NTCIP1202.c
 *
 * Registers the controller-core 1202 v03.35e groups implemented by this
 * firmware.
 */
#include "NTCIP1202.h"

#include "Domain/NTCIP/Mib1202v0335/AscIoInputMappingObjects.h"
#include "Domain/NTCIP/Mib1202v0335/AscIoOutputMappingObjects.h"
#include "Domain/NTCIP/Mib1202v0335/CabinetEnvironmentObjects.h"
#include "Domain/NTCIP/Mib1202v0335/ChannelObjects.h"
#include "Domain/NTCIP/Mib1202v0335/ChannelStatusObjects.h"
#include "Domain/NTCIP/Mib1202v0335/ClockObjects.h"
#include "Domain/NTCIP/Mib1202v0335/CoordObjects.h"
#include "Domain/NTCIP/Mib1202v0335/DetectorObjects.h"
#include "Domain/NTCIP/Mib1202v0335/DetectorReportObjects.h"
#include "Domain/NTCIP/Mib1202v0335/OverlapObjects.h"
#include "Domain/NTCIP/Mib1202v0335/OverlapStatusObjects.h"
#include "Domain/NTCIP/Mib1202v0335/PhaseObjects.h"
#include "Domain/NTCIP/Mib1202v0335/PhaseControlObjects.h"
#include "Domain/NTCIP/Mib1202v0335/PhaseStatusObjects.h"
#include "Domain/NTCIP/Mib1202v0335/PreemptObjects.h"
#include "Domain/NTCIP/Mib1202v0335/RingControlObjects.h"
#include "Domain/NTCIP/Mib1202v0335/RingObjects.h"
#include "Domain/NTCIP/Mib1202v0335/SequenceObjects.h"
#include "Domain/NTCIP/Mib1202v0335/RingStatusObjects.h"
#include "Domain/NTCIP/Mib1202v0335/SpecialFunctionObjects.h"
#include "Domain/NTCIP/Mib1202v0335/TimebaseObjects.h"
#include "Domain/NTCIP/Mib1202v0335/UnitObjects.h"

static const uint32_t kUnitControlOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U, 3U, 10U
};

static uint8_t Ntcip1202RemoteWriteLockActive(void *policyContext)
{
  NtcipContext_t *context = (NtcipContext_t *) policyContext;
  uint8_t unitControl = 0U;

  if ((context == NULL) || (context->intersectionEngine == NULL))
  {
    return 0U;
  }

  if (IntersectionEngineGetUnitControl(context->intersectionEngine,
                                       &unitControl) == 0U)
  {
    return 0U;
  }

  return (uint8_t) ((unitControl & 0x02U) != 0U);
}

static uint8_t UnitControlDescriptorMatches(
  const NtcipObjectDescriptor_t *descriptor)
{
  uint8_t index;

  if ((descriptor == NULL) || (descriptor->oidLength != 12U))
  {
    return 0U;
  }

  for (index = 0U; index < 12U; index++)
  {
    if (descriptor->oid[index] != kUnitControlOid[index])
    {
      return 0U;
    }
  }

  return 1U;
}

static uint8_t Ntcip1202RemoteWriteAllowedWhileLocked(
  void *policyContext,
  const NtcipObjectDescriptor_t *descriptor,
  const NtcipValue_t *value)
{
  (void) policyContext;

  if ((UnitControlDescriptorMatches(descriptor) == 0U) || (value == NULL)
      || (value->type != NTCIP_VALUE_TYPE_UNSIGNED32))
  {
    return 0U;
  }

  return (uint8_t) ((value->data.unsigned32 & 0x02U) == 0U);
}

static void Ntcip1202HandleUserDefinedBackupReset(
  void *observerContext,
  const uint32_t *oid,
  uint8_t oidLength,
  const NtcipRequestContext_t *requestContext,
  const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) observerContext;

  (void) requestContext;
  (void) value;

  if ((context == NULL) || (context->configurationService == NULL)
      || (context->intersectionEngine == NULL) || (oid == NULL)
      || (oidLength == 0U))
  {
    return;
  }

  if (ConfigurationServiceMatchesUserDefinedBackupContentOid(
        context->configurationService,
        oid,
        oidLength) != 0U)
  {
    (void) IntersectionEngineResetUserDefinedBackupTimer(
      context->intersectionEngine);
  }
}

void Ntcip1202RegisterObjects(NtcipObjectDirectory_t *directory,
                              NtcipContext_t *context)
{
  NtcipObjectDirectorySetWriteLockPolicy(directory,
                                         Ntcip1202RemoteWriteLockActive,
                                         Ntcip1202RemoteWriteAllowedWhileLocked,
                                         context);
  NtcipObjectDirectorySetPostSetObserver(directory,
                                         Ntcip1202HandleUserDefinedBackupReset,
                                         context);
  PhaseObjectsRegister(directory, context);
  PhaseStatusObjectsRegister(directory, context);
  PhaseControlObjectsRegister(directory, context);
  RingObjectsRegister(directory, context);
  SequenceObjectsRegister(directory, context);
  RingControlObjectsRegister(directory, context);
  RingStatusObjectsRegister(directory, context);
  CoordObjectsRegister(directory, context);
  TimebaseObjectsRegister(directory, context);
  ClockObjectsRegister(directory, context);
  CabinetEnvironmentObjectsRegister(directory, context);
  AscIoInputMappingObjectsRegister(directory, context);
  AscIoOutputMappingObjectsRegister(directory, context);
  DetectorObjectsRegister(directory, context);
  DetectorReportObjectsRegister(directory, context);
  SpecialFunctionObjectsRegister(directory, context);
  ChannelObjectsRegister(directory, context);
  ChannelStatusObjectsRegister(directory, context);
  OverlapObjectsRegister(directory, context);
  OverlapStatusObjectsRegister(directory, context);
  PreemptObjectsRegister(directory, context);
  UnitObjectsRegister(directory, context);
}
