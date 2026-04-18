/**
 ******************************************************************************
 * @file    Ports/IEepromPort.h
 * @brief   Port interface for EEPROM read/write (raw address-based access).
 *          STM32: routes through StorageRequest() queue.
 *          Host: in-memory byte buffer.
 ******************************************************************************
 */

#ifndef PORTS_IEEPROMPORT_H
#define PORTS_IEEPROMPORT_H

#include <stdint.h>

/* ---------------------------------------------------------------------------
 * Port interface
 * ---------------------------------------------------------------------------*/
typedef struct
{
  void *ctx;
  uint8_t (*Read)(void *ctx, uint32_t address, void *dst, uint32_t size);
  uint8_t (*Write)(void *ctx, uint32_t address, const void *src, uint32_t size);
} IEepromPort_t;

/* ---------------------------------------------------------------------------
 * Zero-cost inline dispatch helpers
 * ---------------------------------------------------------------------------*/
static inline uint8_t Eeprom_Read(IEepromPort_t *p,
                                    uint32_t address,
                                    void *dst,
                                    uint32_t size)
{
  return p->Read(p->ctx, address, dst, size);
}

static inline uint8_t Eeprom_Write(IEepromPort_t *p,
                                     uint32_t address,
                                     const void *src,
                                     uint32_t size)
{
  return p->Write(p->ctx, address, src, size);
}

#endif /* PORTS_IEEPROMPORT_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
