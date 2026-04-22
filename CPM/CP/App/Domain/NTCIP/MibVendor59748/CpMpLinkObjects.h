/* App/Domain/NTCIP/MibVendor59748/CpMpLinkObjects.h
 *
 * Teknotel vendor-private 'cpMpLink' group under the CPU4 controller arc.
 * Rooted at 1.3.6.1.4.1.59748.4.2.1.20. Exposes health and safety-authority
 * scalars of the private CP<->MP CAN FD supervision link.
 */
#ifndef TEKNOTEL_VENDOR_CPMP_LINK_OBJECTS_H
#define TEKNOTEL_VENDOR_CPMP_LINK_OBJECTS_H

#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"

void TeknotelCpMpLinkObjectsRegister(NtcipObjectDirectory_t *directory,
                                     NtcipContext_t *context);

#endif /* TEKNOTEL_VENDOR_CPMP_LINK_OBJECTS_H */
