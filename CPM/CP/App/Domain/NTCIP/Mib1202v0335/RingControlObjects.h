/* App/Domain/NTCIP/Mib1202v0335/RingControlObjects.h
 *
 * Runtime-backed ring control group objects from NTCIP 1202 v03.35e.
 */
#ifndef RING_CONTROL_OBJECTS_H
#define RING_CONTROL_OBJECTS_H

#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"

void RingControlObjectsRegister(NtcipObjectDirectory_t *directory,
                                NtcipContext_t *context);

#endif /* RING_CONTROL_OBJECTS_H */
