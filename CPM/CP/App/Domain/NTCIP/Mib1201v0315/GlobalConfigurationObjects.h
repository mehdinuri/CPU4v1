/* App/Domain/NTCIP/Mib1201v0315/GlobalConfigurationObjects.h
 *
 * NTCIP 1201 v03.15r global configuration objects.
 */
#ifndef GLOBAL_CONFIGURATION_OBJECTS_H
#define GLOBAL_CONFIGURATION_OBJECTS_H

#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"

void GlobalConfigurationObjectsRegister(NtcipObjectDirectory_t *directory,
                                        NtcipContext_t *context);

#endif /* GLOBAL_CONFIGURATION_OBJECTS_H */
