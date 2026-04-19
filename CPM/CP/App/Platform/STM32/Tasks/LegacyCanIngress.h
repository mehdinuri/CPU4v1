/* App/Platform/STM32/Tasks/LegacyCanIngress.h
 *
 * Narrow compatibility seam for legacy FDCAN1 ingress. This keeps the active
 * runtime out of CanMsgParser.h while preserving the existing queue-based
 * handoff to the residual legacy parser task.
 */
#ifndef LEGACY_CAN_INGRESS_H
#define LEGACY_CAN_INGRESS_H

#include <stdint.h>

#include "fdcan.h"

#define LEGACY_CAN_CPMP_EXT_ID_FIRST 0x3000U
#define LEGACY_CAN_CPMP_EXT_ID_LAST 0x4700U

uint8_t LegacyCanIngressDlcToLength(uint32_t dlc);
void LegacyCanIngressOnRxFrame(tpSFDCANRxMsg rxMsg);

#endif /* LEGACY_CAN_INGRESS_H */
