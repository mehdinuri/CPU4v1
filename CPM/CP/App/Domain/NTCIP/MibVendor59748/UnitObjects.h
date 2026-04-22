/* App/Domain/NTCIP/MibVendor59748/UnitObjects.h
 *
 * Teknotel vendor-private 'unit' group under the CPU4 controller arc.
 * Rooted at 1.3.6.1.4.1.59748.4.2.1.3.
 */
#ifndef TEKNOTEL_VENDOR_UNIT_OBJECTS_H
#define TEKNOTEL_VENDOR_UNIT_OBJECTS_H

#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"

void TeknotelUnitObjectsRegister(NtcipObjectDirectory_t *directory,
                                 NtcipContext_t *context);

#endif /* TEKNOTEL_VENDOR_UNIT_OBJECTS_H */
