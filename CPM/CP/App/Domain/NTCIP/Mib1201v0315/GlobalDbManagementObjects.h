/* App/Domain/NTCIP/Mib1201v0315/GlobalDbManagementObjects.h
 *
 * NTCIP 1201 v03.15r database management objects.
 */
#ifndef GLOBAL_DB_MANAGEMENT_OBJECTS_H
#define GLOBAL_DB_MANAGEMENT_OBJECTS_H

#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"

void GlobalDbManagementObjectsRegister(NtcipObjectDirectory_t *directory,
                                       NtcipContext_t *context);

#endif /* GLOBAL_DB_MANAGEMENT_OBJECTS_H */
