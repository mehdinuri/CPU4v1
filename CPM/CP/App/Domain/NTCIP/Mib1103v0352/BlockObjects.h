/* App/Domain/NTCIP/Mib1103v0352/BlockObjects.h */
#ifndef BLOCK_OBJECTS_H
#define BLOCK_OBJECTS_H

#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"

void BlockObjectsRegister(NtcipObjectDirectory_t *directory,
                          NtcipContext_t *context);

#endif /* BLOCK_OBJECTS_H */
