/**
 ******************************************************************************
 * @file    Domain/SignalFlashConfig.h
 * @brief   Persisted per-channel flash-assignment bitmap for signal outputs.
 *          Owns the "is-written" marker check and the on-wire layout;
 *          callers think in terms of a clean SignalFlashConfig_t struct.
 ******************************************************************************
 */

#ifndef DOMAIN_SIGNAL_FLASH_CONFIG_H
#define DOMAIN_SIGNAL_FLASH_CONFIG_H

#include <stdint.h>
#include "Ports/IPersistencePort.h"

/* One bit per output channel. Mapping is identical to ISignalOutputPort's
 * channel ordering: bit 0 = R1, bit 1 = Y1, ..., bit 11 = G4.
 * Bits 12..15 are unused and should be written as 1 (the stored format
 * predates any convention — preserve compatibility by not changing it here).
 *
 * Legacy on-flash encoding uses bit = 0 to mean "flashing", bit = 1 to mean
 * "steady". We preserve that encoding when (de)serialising but the domain
 * struct uses a plain boolean-per-channel layout internally.
 */

#define SIGNAL_FLASH_CONFIG_CHANNEL_COUNT 12U

typedef struct
{
  uint8_t isFlashing[SIGNAL_FLASH_CONFIG_CHANNEL_COUNT];      /* 1 = flashing, 0 = steady */
} SignalFlashConfig_t;

/**
 * @brief Load the persisted flash-config. Returns 1 on success, 0 if no valid
 *        record was found (out is left untouched on failure).
 */
uint8_t SignalFlashConfig_Load(IPersistencePort_t *port,
                               SignalFlashConfig_t *out);

/**
 * @brief Save the flash-config. Returns 1 on success.
 */
uint8_t SignalFlashConfig_Save(IPersistencePort_t *port,
                               const SignalFlashConfig_t *in);

#endif /* DOMAIN_SIGNAL_FLASH_CONFIG_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
