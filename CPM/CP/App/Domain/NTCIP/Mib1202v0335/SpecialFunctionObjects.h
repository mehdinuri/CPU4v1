/* App/Domain/NTCIP/Mib1202v0335/SpecialFunctionObjects.h
 *
 * 1202 unit special function output table projection.
 */
#ifndef NTCIP_MIB1202_SPECIAL_FUNCTION_OBJECTS_H
#define NTCIP_MIB1202_SPECIAL_FUNCTION_OBJECTS_H

#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"

void SpecialFunctionObjectsRegister(NtcipObjectDirectory_t *directory,
                                    NtcipContext_t *context);

#endif /* NTCIP_MIB1202_SPECIAL_FUNCTION_OBJECTS_H */
