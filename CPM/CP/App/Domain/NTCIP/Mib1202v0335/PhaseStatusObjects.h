/* App/Domain/NTCIP/Mib1202v0335/PhaseStatusObjects.h
 *
 * Runtime-backed phase status group objects from NTCIP 1202 v03.35e.
 */
#ifndef PHASE_STATUS_OBJECTS_H
#define PHASE_STATUS_OBJECTS_H

#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"

void PhaseStatusObjectsRegister(NtcipObjectDirectory_t *directory,
                                NtcipContext_t *context);

#endif /* PHASE_STATUS_OBJECTS_H */
