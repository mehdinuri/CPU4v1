/**
 ******************************************************************************
 * @file    Domain/SignalFlashConfig.c
 ******************************************************************************
 */

#include <string.h>
#include "Domain/SignalFlashConfig.h"
#include "Domain/Crc32.h"

/* On-flash wire format.
 *
 *   offset 0..3  : magic   (WIRE_MAGIC)
 *   offset 4..5  : flashBits (legacy bit encoding: 0 = flashing, 1 = steady)
 *   offset 6..7  : reserved (must be 0; reserved for a future version field)
 *   offset 8..11 : crc32   (CRC-32/ISO-HDLC over bytes 0..7)
 *
 * Total 12 bytes. The backing flash adapter rounds up to 16 bytes (double-word
 * aligned) on write; the trailing 4 bytes are "don't care".
 *
 * WIRE_MAGIC is chosen so the bytes read as the ASCII "SSMF" in a hex dump
 * (little-endian store of 0x464D5353U). Distinct from the legacy 0xAAAA
 * marker at offset 0..1, so any legacy record fails the magic check and the
 * caller falls through to the "write defaults" path.
 */
typedef struct
{
  uint32_t magic;
  uint16_t flashBits;
  uint16_t reserved;
  uint32_t crc32;
} WireFormat_t;

#define WIRE_MAGIC    0x464D5353U
#define WIRE_CRC_SPAN 8U    /* bytes of the wire format covered by the CRC */

uint8_t SignalFlashConfig_Load(IPersistencePort_t *port,
                               SignalFlashConfig_t *out)
{
  WireFormat_t wire;
  uint8_t i;

  if (Persistence_Read(port, PERSIST_KEY_SIGNAL_OUTPUTS_FLASH,
                       &wire, (uint16_t) sizeof(wire)) != PERSIST_OK)
  {
    return 0U;
  }

  if (wire.magic != WIRE_MAGIC)
  {
    return 0U;
  }

  if (Crc32_Compute(&wire, WIRE_CRC_SPAN) != wire.crc32)
  {
    return 0U;
  }

  for (i = 0U; i < SIGNAL_FLASH_CONFIG_CHANNEL_COUNT; i++)
  {
    uint16_t mask = (uint16_t) (1U << i);

    /* Legacy: bit = 0 → flashing */
    out->isFlashing[i] = ((wire.flashBits & mask) == 0U) ? 1U : 0U;
  }

  return 1U;
}

uint8_t SignalFlashConfig_Save(IPersistencePort_t *port,
                               const SignalFlashConfig_t *in)
{
  WireFormat_t wire;
  uint8_t i;

  wire.magic = WIRE_MAGIC;
  wire.flashBits = 0xFFFFU;      /* all bits high = all steady by default */
  wire.reserved = 0U;

  for (i = 0U; i < SIGNAL_FLASH_CONFIG_CHANNEL_COUNT; i++)
  {
    uint16_t mask = (uint16_t) (1U << i);

    if (in->isFlashing[i] != 0U)
    {
      wire.flashBits = (uint16_t) (wire.flashBits & (uint16_t) (~mask));
    }
  }

  wire.crc32 = Crc32_Compute(&wire, WIRE_CRC_SPAN);

  return (Persistence_Write(port, PERSIST_KEY_SIGNAL_OUTPUTS_FLASH,
                            &wire,
                            (uint16_t) sizeof(wire)) == PERSIST_OK) ? 1U : 0U;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
