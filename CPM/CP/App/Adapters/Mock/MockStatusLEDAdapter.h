/* App/Adapters/Mock/MockStatusLEDAdapter.h
 *
 * IStatusLEDPort in-memory test double.
 */
#ifndef MOCK_STATUS_LED_ADAPTER_H
#define MOCK_STATUS_LED_ADAPTER_H

#include "Ports/IStatusLEDPort.h"

typedef struct
{
  uint8_t ledState;
  uint32_t toggleCount;
} MockStatusLEDAdapterCtx_t;

void MockStatusLEDAdapterInit(MockStatusLEDAdapterCtx_t *ctx);
IStatusLEDPort_t MockStatusLEDAdapterCreatePort(MockStatusLEDAdapterCtx_t *ctx);

#endif /* MOCK_STATUS_LED_ADAPTER_H */
