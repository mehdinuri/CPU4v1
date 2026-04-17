/* App/Adapters/STM32/LogRepositoryAdapter.h
 *
 * STM32 log repository backed by the EEPROM storage ranges serviced by MSM.
 */
#ifndef LOG_REPOSITORY_ADAPTER_H
#define LOG_REPOSITORY_ADAPTER_H

#include "Ports/IEepromStoragePort.h"
#include "Ports/ILogRepositoryPort.h"

typedef struct
{
  uint16_t writeIndex;
  uint16_t count;
  uint8_t exists;
} LogRepositoryAdapterCtx_t;

void LogRepositoryAdapterInit(LogRepositoryAdapterCtx_t *ctx,
                              IEepromStoragePort_t *eepromPort);
ILogRepositoryPort_t LogRepositoryAdapterCreatePort(
  LogRepositoryAdapterCtx_t *ctx);

#endif /* LOG_REPOSITORY_ADAPTER_H */
