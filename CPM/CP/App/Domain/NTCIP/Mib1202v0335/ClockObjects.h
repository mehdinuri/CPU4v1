/* App/Domain/NTCIP/Mib1202v0335/ClockObjects.h
 *
 * 1202 ascClock subtree handlers.
 */
#ifndef CLOCK_OBJECTS_H
#define CLOCK_OBJECTS_H

#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"

void ClockObjectsRegister(NtcipObjectDirectory_t *directory,
                          NtcipContext_t *context);

#endif /* CLOCK_OBJECTS_H */
