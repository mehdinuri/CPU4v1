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
 *   offset 0..3  : lMagic   (WIRE_MAGIC)
 *   offset 4..5  : sFlashBits (legacy bit encoding: 0 = flashing, 1 = steady)
 *   offset 6..7  : sReserved (must be 0; reserved for a future version field)
 *   offset 8..11 : lCrc32   (CRC-32/ISO-HDLC over bytes 0..7)
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
  uint32_t lMagic;
  uint16_t sFlashBits;
  uint16_t sReserved;
  uint32_t lCrc32;
} tSWireFormat;

#define WIRE_MAGIC    0x464D5353U
#define WIRE_CRC_SPAN 8U    /* bytes of the wire format covered by the CRC */

uint8_t SignalFlashConfig_Load(IPersistencePort_t *pPort,
                               tSSignalFlashConfig *pOut)
{
  tSWireFormat wire;
  uint8_t i;

  if (Persistence_Read(pPort, PERSIST_KEY_SIGNAL_OUTPUTS_FLASH,
                       &wire, (uint16_t) sizeof(wire)) != PERSIST_OK)
  {
    return 0U;
  }

  if (wire.lMagic != WIRE_MAGIC)
  {
    return 0U;
  }

  if (Crc32_Compute(&wire, WIRE_CRC_SPAN) != wire.lCrc32)
  {
    return 0U;
  }

  for (i = 0U; i < SIGNAL_FLASH_CONFIG_CHANNEL_COUNT; i++)
  {
    uint16_t sMask = (uint16_t) (1U << i);

    /* Legacy: bit = 0 → flashing */
    pOut->aIsFlashing[i] = ((wire.sFlashBits & sMask) == 0U) ? 1U : 0U;
  }

  return 1U;
}

uint8_t SignalFlashConfig_Save(IPersistencePort_t *pPort,
                               const tSSignalFlashConfig *pIn)
{
  tSWireFormat wire;
  uint8_t i;

  wire.lMagic = WIRE_MAGIC;
  wire.sFlashBits = 0xFFFFU;      /* all bits high = all steady by default */
  wire.sReserved = 0U;

  for (i = 0U; i < SIGNAL_FLASH_CONFIG_CHANNEL_COUNT; i++)
  {
    uint16_t sMask = (uint16_t) (1U << i);

    if (pIn->aIsFlashing[i] != 0U)
    {
      wire.sFlashBits = (uint16_t) (wire.sFlashBits & (uint16_t) (~sMask));
    }
  }

  wire.lCrc32 = Crc32_Compute(&wire, WIRE_CRC_SPAN);

  return (Persistence_Write(pPort, PERSIST_KEY_SIGNAL_OUTPUTS_FLASH,
                            &wire,
                            (uint16_t) sizeof(wire)) == PERSIST_OK) ? 1U : 0U;
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
