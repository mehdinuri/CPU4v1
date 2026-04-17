/* App/Adapters/STM32/CommLEDAdapter.h
 *
 * IStatusLEDPort concrete implementation for the communication
 * activity LED (COM_LED_Pin, PA4).  Drives pin directly via HAL.
 */
#ifndef COMM_LED_ADAPTER_H
#define COMM_LED_ADAPTER_H

#include "Ports/IStatusLEDPort.h"

typedef struct
{
  uint8_t state;
} CommLEDAdapterCtx_t;

/* No-op init — pin configured by MX_GPIO_Init. */
void CommLEDAdapterInit(CommLEDAdapterCtx_t *ctx);

/* Build an IStatusLEDPort_t wired to ctx. */
IStatusLEDPort_t CommLEDAdapterCreatePort(CommLEDAdapterCtx_t *ctx);

#endif /* COMM_LED_ADAPTER_H */
