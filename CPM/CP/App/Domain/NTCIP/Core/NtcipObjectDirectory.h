/* App/Domain/NTCIP/Core/NtcipObjectDirectory.h
 *
 * Small OID-prefix router that dispatches into per-group scalar and table
 * handlers. This replaces the old god-file registry.
 */
#ifndef NTCIP_OBJECT_DIRECTORY_H
#define NTCIP_OBJECT_DIRECTORY_H

#include "Domain/NTCIP/Core/NtcipTypes.h"

#define NTCIP_OBJECT_DIRECTORY_GROUP_MAX 32U
#define NTCIP_OBJECT_DIRECTORY_INDEX_MAX 4U

typedef struct NtcipObjectDescriptor NtcipObjectDescriptor_t;

typedef NtcipError_t (*NtcipObjectGetHandler_t)(void *groupContext,
                                                const NtcipObjectDescriptor_t *
                                                descriptor,
                                                const uint32_t *indexes,
                                                uint8_t indexCount,
                                                const NtcipRequestContext_t *
                                                requestContext,
                                                NtcipValue_t *value);
typedef NtcipError_t (*NtcipObjectSetHandler_t)(void *groupContext,
                                                const NtcipObjectDescriptor_t *
                                                descriptor,
                                                const uint32_t *indexes,
                                                uint8_t indexCount,
                                                const NtcipRequestContext_t *
                                                requestContext,
                                                const NtcipValue_t *value);
typedef uint8_t (*NtcipWriteLockActiveHandler_t)(void *policyContext);
typedef uint8_t (*NtcipWriteLockAllowHandler_t)(
  void *policyContext,
  const NtcipObjectDescriptor_t *descriptor,
  const NtcipValue_t *value);
typedef void (*NtcipPostSetObserver_t)(void *observerContext,
                                       const uint32_t *oid,
                                       uint8_t oidLength,
                                       const NtcipRequestContext_t *requestContext,
                                       const NtcipValue_t *value);

struct NtcipObjectDescriptor
{
  const uint32_t *oid;
  uint8_t oidLength;
  NtcipObjectKind_t kind;
  uint8_t indexCount;
  NtcipAccess_t access;
  NtcipValueType_t valueType;
  uint16_t tag;
  NtcipObjectGetHandler_t get;
  NtcipObjectSetHandler_t setTest;
  NtcipObjectSetHandler_t setValue;
};

typedef struct
{
  const char *name;
  const NtcipObjectDescriptor_t *descriptors;
  uint16_t descriptorCount;
  void *context;
} NtcipObjectGroup_t;

typedef struct
{
  NtcipWriteLockActiveHandler_t isActive;
  NtcipWriteLockAllowHandler_t allowsWhenLocked;
  void *context;
} NtcipWriteLockPolicy_t;

typedef struct
{
  NtcipPostSetObserver_t onSetValue;
  void *context;
} NtcipPostSetObserverConfig_t;

typedef struct
{
  NtcipObjectGroup_t groups[NTCIP_OBJECT_DIRECTORY_GROUP_MAX];
  uint8_t groupCount;
  NtcipWriteLockPolicy_t writeLockPolicy;
  NtcipPostSetObserverConfig_t postSetObserver;
} NtcipObjectDirectory_t;

typedef struct
{
  const NtcipObjectDescriptor_t *descriptor;
  const NtcipObjectGroup_t *group;
  uint32_t indexes[NTCIP_OBJECT_DIRECTORY_INDEX_MAX];
} NtcipResolvedObject_t;

void NtcipObjectDirectoryInit(NtcipObjectDirectory_t *directory);
void NtcipObjectDirectorySetWriteLockPolicy(
  NtcipObjectDirectory_t *directory,
  NtcipWriteLockActiveHandler_t isActive,
  NtcipWriteLockAllowHandler_t allowsWhenLocked,
  void *policyContext);
void NtcipObjectDirectorySetPostSetObserver(
  NtcipObjectDirectory_t *directory,
  NtcipPostSetObserver_t onSetValue,
  void *observerContext);
uint8_t NtcipObjectDirectoryRegisterGroup(NtcipObjectDirectory_t *directory,
                                          const char *name,
                                          const NtcipObjectDescriptor_t *
                                          descriptors,
                                          uint16_t descriptorCount,
                                          void *groupContext);
uint8_t NtcipObjectDirectoryResolve(const NtcipObjectDirectory_t *directory,
                                    const uint32_t *oid,
                                    uint8_t oidLength,
                                    NtcipResolvedObject_t *resolvedObject);
uint8_t NtcipObjectDirectoryMatchesPrefix(
  const NtcipObjectDirectory_t *directory,
  const uint32_t *oid,
  uint8_t oidLength);
NtcipError_t NtcipObjectDirectoryGet(const NtcipObjectDirectory_t *directory,
                                     const uint32_t *oid,
                                     uint8_t oidLength,
                                     const NtcipRequestContext_t *requestContext,
                                     NtcipValue_t *value);
NtcipError_t NtcipObjectDirectorySetTest(
  const NtcipObjectDirectory_t *directory,
  const uint32_t *oid,
  uint8_t oidLength,
  const NtcipRequestContext_t *
  requestContext,
  const NtcipValue_t *value);
NtcipError_t NtcipObjectDirectorySetValue(
  const NtcipObjectDirectory_t *directory,
  const uint32_t *oid,
  uint8_t oidLength,
  const NtcipRequestContext_t *
  requestContext,
  const NtcipValue_t *value);

#endif /* NTCIP_OBJECT_DIRECTORY_H */
