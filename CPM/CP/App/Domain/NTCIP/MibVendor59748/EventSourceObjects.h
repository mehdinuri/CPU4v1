/* App/Domain/NTCIP/MibVendor59748/EventSourceObjects.h
 *
 * Synthetic CP-owned event-source objects that back the 1103 event engine.
 */
#ifndef TEKNOTEL_EVENT_SOURCE_OBJECTS_H
#define TEKNOTEL_EVENT_SOURCE_OBJECTS_H

#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"

void TeknotelEventSourceObjectsRegister(NtcipObjectDirectory_t *directory,
                                        NtcipContext_t *context);

#endif /* TEKNOTEL_EVENT_SOURCE_OBJECTS_H */
