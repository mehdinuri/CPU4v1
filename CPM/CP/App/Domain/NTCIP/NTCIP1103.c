/* App/Domain/NTCIP/NTCIP1103.c
 *
 * Registers the implemented 1103 v03.52 event-reporting object groups.
 */
#include "NTCIP1103.h"

#include "Domain/NTCIP/Mib1103v0352/BlockObjects.h"
#include "Domain/NTCIP/Mib1103v0352/GlobalReportObjects.h"
#include "Domain/NTCIP/Mib1103v0352/LogicalNameObjects.h"
#include "Domain/NTCIP/Mib1103v0352/SecurityObjects.h"
#include "Domain/NTCIP/Mib1103v0352/TrapObjects.h"

void Ntcip1103RegisterObjects(NtcipObjectDirectory_t *directory,
                              NtcipContext_t *context)
{
  GlobalReportObjectsRegister(directory, context);
  SecurityObjectsRegister(directory, context);
  LogicalNameObjectsRegister(directory, context);
  BlockObjectsRegister(directory, context);
  TrapObjectsRegister(directory, context);
}
