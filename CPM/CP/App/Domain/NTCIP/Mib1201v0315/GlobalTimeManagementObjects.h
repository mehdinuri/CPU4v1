/* App/Domain/NTCIP/Mib1201v0315/GlobalTimeManagementObjects.h */
#ifndef GLOBAL_TIME_MANAGEMENT_OBJECTS_H
#define GLOBAL_TIME_MANAGEMENT_OBJECTS_H

#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"

void GlobalTimeManagementObjectsRegister(NtcipObjectDirectory_t *directory,
                                         NtcipContext_t *context);

#endif /* GLOBAL_TIME_MANAGEMENT_OBJECTS_H */
