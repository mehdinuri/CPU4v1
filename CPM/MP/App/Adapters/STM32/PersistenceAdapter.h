/* App/Adapters/STM32/PersistenceAdapter.h
 *
 * IPersistencePort over the M24M01 I2C EEPROM (1 Mbit, 16-bit word
 * address). MP's CubeMX exports two I2C buses; this adapter targets
 * the EEPROM bus (I2C3) and relies on DMA-driven transfers.
 *
 * Stub implementation: the primary goal of Phase 3 is to land the
 * port shape. Writing to and verifying the EEPROM is deferred until
 * on-bench bring-up.
 */
#ifndef PERSISTENCE_ADAPTER_H
#define PERSISTENCE_ADAPTER_H

#include "Ports/IPersistencePort.h"

#define PERSISTENCE_EEPROM_DEVICE_ADDRESS 0xA0U
#define PERSISTENCE_EEPROM_SIZE_BYTES (128U * 1024U)
#define PERSISTENCE_EEPROM_PAGE_BYTES 256U

typedef struct
{
  uint32_t bytesWritten;
  uint32_t bytesRead;
} PersistenceAdapterCtx_t;

void PersistenceAdapterInit(PersistenceAdapterCtx_t *ctx);
IPersistencePort_t PersistenceAdapterCreatePort(PersistenceAdapterCtx_t *ctx);

#endif /* PERSISTENCE_ADAPTER_H */
