/* App/Domain/NTCIP/Mib1202v0335/TimebaseObjects.h
 *
 * 1202 timebaseAsc subtree handlers.
 */
#ifndef TIMEBASE_OBJECTS_H
#define TIMEBASE_OBJECTS_H

#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"

void TimebaseObjectsRegister(NtcipObjectDirectory_t *directory,
                             NtcipContext_t *context);

#endif /* TIMEBASE_OBJECTS_H */
