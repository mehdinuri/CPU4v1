/* App/Domain/NTCIP/Mib1202v0335/PreemptObjects.h
 *
 * 1202 preempt subtree projection for controller-core preempt config and
 * runtime status.
 */
#ifndef NTCIP_1202_PREEMPT_OBJECTS_H
#define NTCIP_1202_PREEMPT_OBJECTS_H

#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"

void PreemptObjectsRegister(NtcipObjectDirectory_t *directory,
                            NtcipContext_t *context);

#endif /* NTCIP_1202_PREEMPT_OBJECTS_H */
