/* App/Domain/NTCIP/Mib1202v0335/RingObjects.h
 *
 * Controller-core ring scalars from NTCIP 1202 v03.35e.
 */
#ifndef RING_OBJECTS_H
#define RING_OBJECTS_H

#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"

void RingObjectsRegister(NtcipObjectDirectory_t *directory,
                         NtcipContext_t *context);

#endif /* RING_OBJECTS_H */
