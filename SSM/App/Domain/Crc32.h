/**
 ******************************************************************************
 * @file    Domain/Crc32.h
 * @brief   CRC-32/ISO-HDLC (poly 0xEDB88320, reflected, init/final 0xFFFFFFFF).
 *
 *          This is the canonical "PNG / Ethernet / zip" CRC32. Matches what
 *          STM32G4's hardware CRC unit produces when configured with the
 *          defaults listed above, so software-computed CRCs on this module
 *          stay compatible if we later move generation into hardware.
 ******************************************************************************
 */

#ifndef DOMAIN_CRC32_H
#define DOMAIN_CRC32_H

#include <stdint.h>

/**
 * @brief Compute CRC-32/ISO-HDLC over a byte buffer.
 *
 * @param pData  Input buffer (non-NULL if sLen > 0).
 * @param sLen   Length in bytes.
 * @return       32-bit checksum.
 */
uint32_t Crc32_Compute(const void *pData, uint16_t sLen);

#endif /* DOMAIN_CRC32_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
