/* App/Domain/NTCIP/Mib1202v0335/CoordObjects.h
 *
 * 1202 coordination subtree projection for controller-core coordination
 * configuration and runtime status.
 */
#ifndef NTCIP_1202_COORD_OBJECTS_H
#define NTCIP_1202_COORD_OBJECTS_H

#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"

void CoordObjectsRegister(NtcipObjectDirectory_t *directory,
                          NtcipContext_t *context);

#endif /* NTCIP_1202_COORD_OBJECTS_H */
