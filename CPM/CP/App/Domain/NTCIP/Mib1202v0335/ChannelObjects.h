/* App/Domain/NTCIP/Mib1202v0335/ChannelObjects.h
 *
 * Controller-core channel objects from NTCIP 1202 v03.35e.
 */
#ifndef CHANNEL_OBJECTS_H
#define CHANNEL_OBJECTS_H

#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"

void ChannelObjectsRegister(NtcipObjectDirectory_t *directory,
                            NtcipContext_t *context);

#endif /* CHANNEL_OBJECTS_H */
