/* App/Domain/NTCIP/Mib1202v0335/SequenceObjects.h
 *
 * Controller-core sequence plan objects from NTCIP 1202 v03.35e.
 */
#ifndef SEQUENCE_OBJECTS_H
#define SEQUENCE_OBJECTS_H

#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"

void SequenceObjectsRegister(NtcipObjectDirectory_t *directory,
                             NtcipContext_t *context);

#endif /* SEQUENCE_OBJECTS_H */
