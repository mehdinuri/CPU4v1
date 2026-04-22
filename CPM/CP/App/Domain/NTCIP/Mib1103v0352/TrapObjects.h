/* App/Domain/NTCIP/Mib1103v0352/TrapObjects.h */
#ifndef TRAP_OBJECTS_H
#define TRAP_OBJECTS_H

#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"

void TrapObjectsRegister(NtcipObjectDirectory_t *directory,
                         NtcipContext_t *context);

#endif /* TRAP_OBJECTS_H */
