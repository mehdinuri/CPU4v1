/* App/Domain/NTCIP/NTCIP1201.h
 *
 * Umbrella registration entry points for NTCIP 1201 v03.15r object groups.
 */
#ifndef NTCIP1201_H
#define NTCIP1201_H

#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"

void Ntcip1201RegisterObjects(NtcipObjectDirectory_t *directory,
                              NtcipContext_t *context);

#endif /* NTCIP1201_H */
