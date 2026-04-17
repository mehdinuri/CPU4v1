/* App/Domain/NTCIP/Mib1201v0315NewAuxIO/AuxIoV2Objects.h
 *
 * NTCIP 1201 NewAuxIO v2 objects.
 */
#ifndef AUX_IO_V2_OBJECTS_H
#define AUX_IO_V2_OBJECTS_H

#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"

void AuxIoV2ObjectsRegister(NtcipObjectDirectory_t *directory,
                            NtcipContext_t *context);

#endif /* AUX_IO_V2_OBJECTS_H */
