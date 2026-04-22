/* App/Domain/NTCIP/Core/NtcipObjectDirectory.c
 *
 * Exact-match routing from SNMP instance OIDs to small, versioned MIB-group
 * handlers.
 */
#include "NtcipObjectDirectory.h"

#include <stddef.h>
#include <string.h>

static const NtcipRequestContext_t kDefaultRequestContext =
  NTCIP_REQUEST_CONTEXT_INIT(0U, 0U, 0U);

static uint8_t OidPrefixMatches(const uint32_t *oid,
                                const NtcipObjectDescriptor_t *descriptor)
{
  uint8_t i;

  for (i = 0U; i < descriptor->oidLength; i++)
  {
    if (oid[i] != descriptor->oid[i])
    {
      return 0U;
    }
  }

  return 1U;
}

static uint8_t DescriptorPrefixMatches(const uint32_t *oid,
                                       uint8_t oidLength,
                                       const NtcipObjectDescriptor_t *descriptor)
{
  if ((oid == NULL) || (descriptor == NULL) || (descriptor->oid == NULL)
      || (descriptor->kind != NTCIP_OBJECT_KIND_TABLE_COLUMN)
      || (oidLength < descriptor->oidLength))
  {
    return 0U;
  }

  return OidPrefixMatches(oid, descriptor);
}

static uint8_t DescriptorMatches(const uint32_t *oid,
                                 uint8_t oidLength,
                                 const NtcipObjectDescriptor_t *descriptor,
                                 uint32_t *indexes)
{
  uint8_t i;
  uint8_t expectedLength;

  expectedLength = descriptor->oidLength + descriptor->indexCount;

  if (descriptor->kind == NTCIP_OBJECT_KIND_SCALAR)
  {
    expectedLength = (uint8_t) (expectedLength + 1U);
  }

  if ((oid == NULL) || (oidLength != expectedLength)
      || (descriptor->indexCount > NTCIP_OBJECT_DIRECTORY_INDEX_MAX)
      || (OidPrefixMatches(oid, descriptor) == 0U))
  {
    return 0U;
  }

  if (descriptor->kind == NTCIP_OBJECT_KIND_SCALAR)
  {
    if (oid[descriptor->oidLength] != 0U)
    {
      return 0U;
    }

    return 1U;
  }

  for (i = 0U; i < descriptor->indexCount; i++)
  {
    indexes[i] = oid[descriptor->oidLength + i];
  }

  return 1U;
}

static uint8_t ValueTypeMatches(const NtcipObjectDescriptor_t *descriptor,
                                const NtcipValue_t *value)
{
  return (uint8_t) ((descriptor != NULL)
                    && (value != NULL)
                    && (descriptor->valueType == value->type));
}

static uint8_t WriteLockActive(const NtcipObjectDirectory_t *directory)
{
  return (uint8_t) ((directory != NULL)
                    && (directory->writeLockPolicy.isActive != NULL)
                    && (directory->writeLockPolicy.isActive(
                          directory->writeLockPolicy.context) != 0U));
}

static uint8_t WriteAllowedWhileLocked(
  const NtcipObjectDirectory_t *directory,
  const NtcipObjectDescriptor_t *descriptor,
  const NtcipValue_t *value)
{
  return (uint8_t) ((directory != NULL)
                    && (directory->writeLockPolicy.allowsWhenLocked != NULL)
                    && (directory->writeLockPolicy.allowsWhenLocked(
                          directory->writeLockPolicy.context,
                          descriptor,
                          value) != 0U));
}

void NtcipObjectDirectoryInit(NtcipObjectDirectory_t *directory)
{
  if (directory != NULL)
  {
    memset(directory, 0, sizeof(*directory));
  }
}

void NtcipObjectDirectorySetWriteLockPolicy(
  NtcipObjectDirectory_t *directory,
  NtcipWriteLockActiveHandler_t isActive,
  NtcipWriteLockAllowHandler_t allowsWhenLocked,
  void *policyContext)
{
  if (directory == NULL)
  {
    return;
  }

  directory->writeLockPolicy.isActive = isActive;
  directory->writeLockPolicy.allowsWhenLocked = allowsWhenLocked;
  directory->writeLockPolicy.context = policyContext;
}

void NtcipObjectDirectorySetPostSetObserver(
  NtcipObjectDirectory_t *directory,
  NtcipPostSetObserver_t onSetValue,
  void *observerContext)
{
  if (directory == NULL)
  {
    return;
  }

  directory->postSetObserver.onSetValue = onSetValue;
  directory->postSetObserver.context = observerContext;
}

uint8_t NtcipObjectDirectoryRegisterGroup(NtcipObjectDirectory_t *directory,
                                          const char *name,
                                          const NtcipObjectDescriptor_t *
                                          descriptors,
                                          uint16_t descriptorCount,
                                          void *groupContext)
{
  NtcipObjectGroup_t *group;

  if ((directory == NULL) || (name == NULL) || (descriptors == NULL)
      || (descriptorCount == 0U)
      || (directory->groupCount >= NTCIP_OBJECT_DIRECTORY_GROUP_MAX))
  {
    return 0U;
  }

  group = &directory->groups[directory->groupCount];
  group->name = name;
  group->descriptors = descriptors;
  group->descriptorCount = descriptorCount;
  group->context = groupContext;
  directory->groupCount++;

  return 1U;
}

uint8_t NtcipObjectDirectoryResolve(const NtcipObjectDirectory_t *directory,
                                    const uint32_t *oid,
                                    uint8_t oidLength,
                                    NtcipResolvedObject_t *resolvedObject)
{
  uint8_t groupIndex;

  if ((directory == NULL) || (oid == NULL) || (resolvedObject == NULL))
  {
    return 0U;
  }

  memset(resolvedObject, 0, sizeof(*resolvedObject));

  for (groupIndex = 0U; groupIndex < directory->groupCount; groupIndex++)
  {
    const NtcipObjectGroup_t *group = &directory->groups[groupIndex];
    uint16_t objectIndex;

    for (objectIndex = 0U; objectIndex < group->descriptorCount; objectIndex++)
    {
      const NtcipObjectDescriptor_t *descriptor =
        &group->descriptors[objectIndex];

      if (DescriptorMatches(oid,
                            oidLength,
                            descriptor,
                            resolvedObject->indexes) != 0U)
      {
        resolvedObject->descriptor = descriptor;
        resolvedObject->group = group;

        return 1U;
      }
    }
  }

  return 0U;
}

uint8_t NtcipObjectDirectoryMatchesPrefix(
  const NtcipObjectDirectory_t *directory,
  const uint32_t *oid,
  uint8_t oidLength)
{
  uint8_t groupIndex;

  if ((directory == NULL) || (oid == NULL))
  {
    return 0U;
  }

  for (groupIndex = 0U; groupIndex < directory->groupCount; groupIndex++)
  {
    const NtcipObjectGroup_t *group = &directory->groups[groupIndex];
    uint16_t objectIndex;

    for (objectIndex = 0U; objectIndex < group->descriptorCount; objectIndex++)
    {
      if (DescriptorPrefixMatches(oid,
                                  oidLength,
                                  &group->descriptors[objectIndex]) != 0U)
      {
        return 1U;
      }
    }
  }

  return 0U;
}

NtcipError_t NtcipObjectDirectoryGet(const NtcipObjectDirectory_t *directory,
                                     const uint32_t *oid,
                                     uint8_t oidLength,
                                     const NtcipRequestContext_t *requestContext,
                                     NtcipValue_t *value)
{
  NtcipResolvedObject_t resolvedObject;
  const NtcipRequestContext_t *request = requestContext;

  if ((directory == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if (request == NULL)
  {
    request = &kDefaultRequestContext;
  }

  if (NtcipObjectDirectoryResolve(directory, oid, oidLength,
                                  &resolvedObject) == 0U)
  {
    return NTCIP_ERROR_NOT_FOUND;
  }

  if (resolvedObject.descriptor->get == NULL)
  {
    return NTCIP_ERROR_NOT_FOUND;
  }

  return resolvedObject.descriptor->get(resolvedObject.group->context,
                                        resolvedObject.descriptor,
                                        resolvedObject.indexes,
                                        resolvedObject.descriptor->indexCount,
                                        request,
                                        value);
}

NtcipError_t NtcipObjectDirectorySetTest(
  const NtcipObjectDirectory_t *directory,
  const uint32_t *oid,
  uint8_t oidLength,
  const NtcipRequestContext_t *
  requestContext,
  const NtcipValue_t *value)
{
  NtcipResolvedObject_t resolvedObject;
  const NtcipRequestContext_t *request = requestContext;

  if ((directory == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if (request == NULL)
  {
    request = &kDefaultRequestContext;
  }

  if (NtcipObjectDirectoryResolve(directory, oid, oidLength,
                                  &resolvedObject) == 0U)
  {
    return NTCIP_ERROR_NOT_FOUND;
  }

  if ((WriteLockActive(directory) != 0U)
      && (WriteAllowedWhileLocked(directory,
                                  resolvedObject.descriptor,
                                  value) == 0U))
  {
    return NTCIP_ERROR_NO_ACCESS;
  }

  if (resolvedObject.descriptor->access != NTCIP_ACCESS_READ_WRITE)
  {
    return NTCIP_ERROR_READ_ONLY;
  }

  if (ValueTypeMatches(resolvedObject.descriptor, value) == 0U)
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if (resolvedObject.descriptor->setTest == NULL)
  {
    return NTCIP_ERROR_OK;
  }

  return resolvedObject.descriptor->setTest(resolvedObject.group->context,
                                            resolvedObject.descriptor,
                                            resolvedObject.indexes,
                                            resolvedObject.descriptor->
                                            indexCount,
                                            request,
                                            value);
} /* NtcipObjectDirectorySetTest */

NtcipError_t NtcipObjectDirectorySetValue(
  const NtcipObjectDirectory_t *directory,
  const uint32_t *oid,
  uint8_t oidLength,
  const NtcipRequestContext_t *
  requestContext,
  const NtcipValue_t *value)
{
  NtcipResolvedObject_t resolvedObject;
  const NtcipRequestContext_t *request = requestContext;
  NtcipError_t error;

  if ((directory == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if (request == NULL)
  {
    request = &kDefaultRequestContext;
  }

  if (NtcipObjectDirectoryResolve(directory, oid, oidLength,
                                  &resolvedObject) == 0U)
  {
    return NTCIP_ERROR_NOT_FOUND;
  }

  if ((WriteLockActive(directory) != 0U)
      && (WriteAllowedWhileLocked(directory,
                                  resolvedObject.descriptor,
                                  value) == 0U))
  {
    return NTCIP_ERROR_NO_ACCESS;
  }

  if (resolvedObject.descriptor->access != NTCIP_ACCESS_READ_WRITE)
  {
    return NTCIP_ERROR_READ_ONLY;
  }

  if (ValueTypeMatches(resolvedObject.descriptor, value) == 0U)
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if (resolvedObject.descriptor->setValue == NULL)
  {
    return NTCIP_ERROR_GEN_ERROR;
  }

  error = resolvedObject.descriptor->setValue(resolvedObject.group->context,
                                              resolvedObject.descriptor,
                                              resolvedObject.indexes,
                                              resolvedObject.descriptor->
                                              indexCount,
                                              request,
                                              value);

  if ((error == NTCIP_ERROR_OK)
      && (directory->postSetObserver.onSetValue != NULL))
  {
    directory->postSetObserver.onSetValue(directory->postSetObserver.context,
                                          oid,
                                          oidLength,
                                          request,
                                          value);
  }

  return error;
} /* NtcipObjectDirectorySetValue */
