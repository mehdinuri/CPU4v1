/* App/Domain/NTCIP/NTCIP1201.c
 *
 * Registers the 1201 v03.15r MIB groups implemented by this firmware.
 */
#include "NTCIP1201.h"

#include "Domain/NTCIP/Mib1201v0315/GlobalConfigurationObjects.h"
#include "Domain/NTCIP/Mib1201v0315/GlobalDbManagementObjects.h"
#include "Domain/NTCIP/Mib1201v0315NewAuxIO/AuxIoV2Objects.h"

void Ntcip1201RegisterObjects(NtcipObjectDirectory_t *directory,
                              NtcipContext_t *context)
{
  GlobalConfigurationObjectsRegister(directory, context);
  GlobalDbManagementObjectsRegister(directory, context);
  AuxIoV2ObjectsRegister(directory, context);
}
