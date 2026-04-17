/* App/Domain/NTCIP/Mib1202v0335/RingStatusObjects.h
 *
 * Runtime-backed ring status table objects from NTCIP 1202 v03.35e.
 */
#ifndef RING_STATUS_OBJECTS_H
#define RING_STATUS_OBJECTS_H

#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"

void RingStatusObjectsRegister(NtcipObjectDirectory_t *directory,
                               NtcipContext_t *context);

#endif /* RING_STATUS_OBJECTS_H */
