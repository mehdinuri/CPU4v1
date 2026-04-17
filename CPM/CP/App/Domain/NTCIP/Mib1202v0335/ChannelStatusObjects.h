/* App/Domain/NTCIP/Mib1202v0335/ChannelStatusObjects.h
 *
 * Runtime-backed channel status group objects from NTCIP 1202 v03.35e.
 */
#ifndef CHANNEL_STATUS_OBJECTS_H
#define CHANNEL_STATUS_OBJECTS_H

#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"

void ChannelStatusObjectsRegister(NtcipObjectDirectory_t *directory,
                                  NtcipContext_t *context);

#endif /* CHANNEL_STATUS_OBJECTS_H */
