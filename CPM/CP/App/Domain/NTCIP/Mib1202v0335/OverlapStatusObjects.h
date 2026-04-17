/* App/Domain/NTCIP/Mib1202v0335/OverlapStatusObjects.h
 *
 * Runtime-backed overlap status group objects from NTCIP 1202 v03.35e.
 */
#ifndef OVERLAP_STATUS_OBJECTS_H
#define OVERLAP_STATUS_OBJECTS_H

#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"

void OverlapStatusObjectsRegister(NtcipObjectDirectory_t *directory,
                                  NtcipContext_t *context);

#endif /* OVERLAP_STATUS_OBJECTS_H */
