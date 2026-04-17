/* App/Domain/NTCIP/Mib1202v0335/UnitObjects.h
 *
 * Unit status objects from NTCIP 1202 v03.35e.
 */
#ifndef UNIT_OBJECTS_H
#define UNIT_OBJECTS_H

#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"

void UnitObjectsRegister(NtcipObjectDirectory_t *directory,
                         NtcipContext_t *context);

#endif /* UNIT_OBJECTS_H */
