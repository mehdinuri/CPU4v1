/* CPM/Libs/CpMpProtocolShared.h
 *
 * Shared private CP <-> MP protocol definitions for the clean CAN FD control
 * link. Field-bus PSM/SSM traffic stays on the legacy wire protocol; this
 * header defines only the controller-local supervision protocol.
 */
#ifndef CPMP_SHARED_PROTOCOL_H
#define CPMP_SHARED_PROTOCOL_H

#include <stdint.h>

#define CPMP_PROTOCOL_VERSION 4U

#define CPMP_FRAME_ID_CP_HEARTBEAT   0x100U
#define CPMP_FRAME_ID_CP_CFG_BEGIN   0x101U
#define CPMP_FRAME_ID_CP_CFG_CHUNK   0x102U
#define CPMP_FRAME_ID_CP_CFG_COMMIT  0x103U
#define CPMP_FRAME_ID_CP_EVENT_ACK   0x104U

#define CPMP_FRAME_ID_MP_HEARTBEAT   0x180U
#define CPMP_FRAME_ID_MP_CFG_STATUS  0x181U
#define CPMP_FRAME_ID_MP_SAFETY      0x182U
#define CPMP_FRAME_ID_MP_FAULTS      0x183U
#define CPMP_FRAME_ID_MP_EVENT       0x186U

#define CPMP_PEER_TIMEOUT_TICKS 5U

#define CPMP_CONFIG_CHUNK_PAYLOAD_BYTES 60U
#define CPMP_CONFIG_CHANNEL_COUNT 32U
#define CPMP_CONFIG_PHASE_COUNT 8U
#define CPMP_CONFIG_IMAGE_MAX_OUTPUT_MAP 96U

typedef enum
{
  CPMP_CONFIG_STATE_EMPTY = 0U,
  CPMP_CONFIG_STATE_LOADING = 1U,
  CPMP_CONFIG_STATE_APPLIED = 2U,
  CPMP_CONFIG_STATE_INVALID = 3U
} CpMpConfigState_t;

typedef enum
{
  CPMP_SAFETY_ACTION_NORMAL = 0U,
  CPMP_SAFETY_ACTION_FLASH = 1U,
  CPMP_SAFETY_ACTION_DARK = 2U
} CpMpSafetyAction_t;

typedef enum
{
  CPMP_OUTPUT_COLOR_NONE = 0U,
  CPMP_OUTPUT_COLOR_RED = 1U,
  CPMP_OUTPUT_COLOR_YELLOW = 2U,
  CPMP_OUTPUT_COLOR_GREEN = 3U
} CpMpOutputColor_t;

typedef enum
{
  CPMP_FAULT_CHANNEL_FLAG_CONFLICT = 0x0001U,
  CPMP_FAULT_CHANNEL_FLAG_DUAL_INDICATION = 0x0002U,
  CPMP_FAULT_CHANNEL_FLAG_RED_FAIL = 0x0004U,
  CPMP_FAULT_CHANNEL_FLAG_DARK = 0x0008U,
  CPMP_FAULT_CHANNEL_FLAG_MIN_YELLOW_SHORT = 0x0010U,
  CPMP_FAULT_CHANNEL_FLAG_CLEARANCE_SHORT = 0x0020U,
  CPMP_FAULT_CHANNEL_FLAG_SIGNAL_SEQUENCE = 0x0040U,
  CPMP_FAULT_CHANNEL_FLAG_LAMP_OPEN = 0x0080U,
  CPMP_FAULT_CHANNEL_FLAG_LAMP_EXTERNALLY_DRIVEN = 0x0100U
} CpMpFaultChannelFlags_t;

typedef enum
{
  CPMP_FAULT_GLOBAL_FLAG_AC_LINE = 0x00000001UL,
  CPMP_FAULT_GLOBAL_FLAG_RAIL_24V = 0x00000002UL,
  CPMP_FAULT_GLOBAL_FLAG_RAIL_5V = 0x00000004UL,
  CPMP_FAULT_GLOBAL_FLAG_CP_MISSING = 0x00000008UL,
  CPMP_FAULT_GLOBAL_FLAG_PSM_MISSING = 0x00000010UL,
  CPMP_FAULT_GLOBAL_FLAG_SSM_MISSING = 0x00000020UL,
  CPMP_FAULT_GLOBAL_FLAG_WATCHDOG = 0x00000040UL,
  CPMP_FAULT_GLOBAL_FLAG_RELAY_FEEDBACK_MISMATCH = 0x00000080UL,
  CPMP_FAULT_GLOBAL_FLAG_CONFIG_INVALID = 0x00000100UL,
  CPMP_FAULT_GLOBAL_FLAG_LOCAL_FLASH_ACTIVE = 0x00000200UL,
  CPMP_FAULT_GLOBAL_FLAG_STARTUP_FLASH_ACTIVE = 0x00000400UL,
  CPMP_FAULT_GLOBAL_FLAG_MMU_FLASH_ACTIVE = 0x00000800UL
} CpMpFaultGlobalFlags_t;

typedef struct
{
  uint8_t outputIndex;
  uint8_t channelIndex;
  uint8_t color;
  uint8_t reserved;
} CpMpOutputMapEntry_t;

typedef struct
{
  uint8_t phaseCount;
  uint8_t ringCount;
  uint8_t channelCount;
  uint8_t outputMapCount;
  uint8_t startupFlashSeconds;
  uint8_t startupFlashMode;
  uint8_t failureFlashPeriodDs;
  uint8_t reserved0;
  uint8_t channelControlType[CPMP_CONFIG_CHANNEL_COUNT];
  uint8_t channelControlSource[CPMP_CONFIG_CHANNEL_COUNT];
  uint8_t channelFlashMask[CPMP_CONFIG_CHANNEL_COUNT];
  uint32_t channelConflictMask[CPMP_CONFIG_CHANNEL_COUNT];
  uint8_t channelMinYellowDs[CPMP_CONFIG_CHANNEL_COUNT];
  uint8_t channelRedClearDs[CPMP_CONFIG_CHANNEL_COUNT];
  uint8_t phaseYellowChangeDs[CPMP_CONFIG_PHASE_COUNT];
  uint8_t phaseRedClearDs[CPMP_CONFIG_PHASE_COUNT];
  uint8_t phaseConcurrencyMask[CPMP_CONFIG_PHASE_COUNT];
  CpMpOutputMapEntry_t outputMap[CPMP_CONFIG_IMAGE_MAX_OUTPUT_MAP];
} CpMpMmuConfigImageV1_t;

typedef struct
{
  uint32_t sequence;
  uint32_t globalFlags;
  uint16_t channelFlags[CPMP_CONFIG_CHANNEL_COUNT];
  uint8_t safetyAction;
  uint8_t safetyReasonCode;
  uint8_t configState;
  uint8_t reserved0;
} CpMpFaultStatusImageV2_t;

typedef struct
{
  uint32_t sequence;
  uint32_t timestampTicks;
  uint32_t param;
  uint16_t source;
  uint8_t code;
  uint8_t severity;
} CpMpFaultEventV1_t;

typedef struct
{
  uint32_t sequence;
} CpMpEventAckV1_t;

#define CPMP_CONFIG_CHUNK_MAX_COUNT \
  ((sizeof(CpMpMmuConfigImageV1_t) + CPMP_CONFIG_CHUNK_PAYLOAD_BYTES - 1U) \
   / CPMP_CONFIG_CHUNK_PAYLOAD_BYTES)
#define CPMP_CONFIG_CHUNK_BITMAP_BYTES \
  ((CPMP_CONFIG_CHUNK_MAX_COUNT + 7U) / 8U)

typedef CpMpMmuConfigImageV1_t CpMpMmuConfigImage_t;
typedef CpMpFaultStatusImageV2_t CpMpFaultStatusImage_t;
typedef CpMpFaultStatusImageV2_t CpMpFaultStatusImageV1_t;

#endif /* CPMP_SHARED_PROTOCOL_H */
