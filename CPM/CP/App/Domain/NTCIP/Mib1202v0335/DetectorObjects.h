/* App/Domain/NTCIP/Mib1202v0335/DetectorObjects.h
 *
 * Vehicle and pedestrian detector objects from NTCIP 1202 v03.35e.
 */
#ifndef DETECTOR_OBJECTS_H
#define DETECTOR_OBJECTS_H

#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"

void DetectorObjectsRegister(NtcipObjectDirectory_t *directory,
                             NtcipContext_t *context);

#endif /* DETECTOR_OBJECTS_H */
