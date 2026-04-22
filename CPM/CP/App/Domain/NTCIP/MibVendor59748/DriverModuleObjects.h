/* App/Domain/NTCIP/MibVendor59748/DriverModuleObjects.h
 *
 * Teknotel vendor-private 'driverModule' group under the CPU4 controller arc.
 * Rooted at 1.3.6.1.4.1.59748.4.2.1.21. Names the varbind carried by the
 * teknotelDriverModuleMissingTrap.
 */
#ifndef TEKNOTEL_VENDOR_DRIVER_MODULE_OBJECTS_H
#define TEKNOTEL_VENDOR_DRIVER_MODULE_OBJECTS_H

#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"

void TeknotelDriverModuleObjectsRegister(NtcipObjectDirectory_t *directory,
                                         NtcipContext_t *context);

#endif /* TEKNOTEL_VENDOR_DRIVER_MODULE_OBJECTS_H */
