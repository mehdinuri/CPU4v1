/* App/Domain/NTCIP/Mib1202v0335/PhaseObjects.h
 *
 * Controller-core phase objects from NTCIP 1202 v03.35e.
 */
#ifndef PHASE_OBJECTS_H
#define PHASE_OBJECTS_H

#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"

void PhaseObjectsRegister(NtcipObjectDirectory_t *directory,
                          NtcipContext_t *context);

#endif /* PHASE_OBJECTS_H */
