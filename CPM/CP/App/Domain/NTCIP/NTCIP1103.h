/* App/Domain/NTCIP/NTCIP1103.h
 *
 * Umbrella registration entry points for the implemented NTCIP 1103 v03.52
 * object groups.
 */
#ifndef NTCIP1103_H
#define NTCIP1103_H

#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"

void Ntcip1103RegisterObjects(NtcipObjectDirectory_t *directory,
                              NtcipContext_t *context);

#endif /* NTCIP1103_H */
