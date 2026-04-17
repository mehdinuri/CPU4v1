/* App/Domain/NTCIP/NTCIP1202.h
 *
 * Umbrella registration entry points for NTCIP 1202 v03.35e object groups.
 */
#ifndef NTCIP1202_H
#define NTCIP1202_H

#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"

void Ntcip1202RegisterObjects(NtcipObjectDirectory_t *directory,
                              NtcipContext_t *context);

#endif /* NTCIP1202_H */
