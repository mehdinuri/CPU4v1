/* App/Domain/NTCIP/Mib1202v0335/OverlapObjects.h
 *
 * Controller-core overlap objects from NTCIP 1202 v03.35e.
 */
#ifndef OVERLAP_OBJECTS_H
#define OVERLAP_OBJECTS_H

#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"

void OverlapObjectsRegister(NtcipObjectDirectory_t *directory,
                            NtcipContext_t *context);

#endif /* OVERLAP_OBJECTS_H */
