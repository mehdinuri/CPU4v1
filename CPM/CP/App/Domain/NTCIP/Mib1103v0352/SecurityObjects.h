/* App/Domain/NTCIP/Mib1103v0352/SecurityObjects.h */
#ifndef SECURITY_OBJECTS_H
#define SECURITY_OBJECTS_H

#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"

void SecurityObjectsRegister(NtcipObjectDirectory_t *directory,
                             NtcipContext_t *context);

#endif /* SECURITY_OBJECTS_H */
