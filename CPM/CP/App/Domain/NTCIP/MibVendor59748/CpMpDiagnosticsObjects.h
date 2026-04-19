/* App/Domain/NTCIP/MibVendor59748/CpMpDiagnosticsObjects.h
 *
 * Vendor-private CP<->MP diagnostics rooted at enterprise OID 59748.
 */
#ifndef CPMP_DIAGNOSTICS_OBJECTS_H
#define CPMP_DIAGNOSTICS_OBJECTS_H

#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"

void CpMpDiagnosticsObjectsRegister(NtcipObjectDirectory_t *directory,
                                    NtcipContext_t *context);

#endif /* CPMP_DIAGNOSTICS_OBJECTS_H */
