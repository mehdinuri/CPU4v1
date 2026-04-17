/* App/Domain/NTCIP/Mib1202v0335/PhaseControlObjects.h
 *
 * Runtime-backed phase control group objects from NTCIP 1202 v03.35e.
 */
#ifndef PHASE_CONTROL_OBJECTS_H
#define PHASE_CONTROL_OBJECTS_H

#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"

void PhaseControlObjectsRegister(NtcipObjectDirectory_t *directory,
                                 NtcipContext_t *context);

#endif /* PHASE_CONTROL_OBJECTS_H */
