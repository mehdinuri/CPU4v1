/* App/Adapters/STM32/LegacyFieldCanIds.h
 *
 * Legacy-compatible FDCAN1 field-bus IDs used by the active clean adapters.
 * These constants preserve the existing wire contract without pulling in the
 * legacy parser/task interfaces.
 */
#ifndef LEGACY_FIELD_CAN_IDS_H
#define LEGACY_FIELD_CAN_IDS_H

#include <stdint.h>

#define LEGACY_FIELD_CAN_ID_VERSION 0x00FU
#define LEGACY_FIELD_CAN_ID_CPU_SO0 0x040U
#define LEGACY_FIELD_CAN_ID_CPU_SO1 0x041U
#define LEGACY_FIELD_CAN_ID_IO_INPUTS0 0x080U
#define LEGACY_FIELD_CAN_ID_CPU_DATE_TIME 0x100U
#define LEGACY_FIELD_CAN_ID_CPU_FLASH_SIGNALS0 0x0B0U
#define LEGACY_FIELD_CAN_ID_CPU_FLASH_SIGNALS1 0x0B1U

#define LEGACY_FIELD_CAN_ID_LOOP_NMT 0x000U
#define LEGACY_FIELD_CAN_ID_LOOP_SDO_REQUEST_BASE 0x601U

#endif /* LEGACY_FIELD_CAN_IDS_H */
