/* App/Adapters/STM32/StatusLEDAdapter.h
 *
 * IStatusLEDPort driving the front-panel comm LED on PC0. The blink
 * states translate into a phase + period pair the platform Tick()
 * routine applies to the pin each call.
 */
#ifndef STATUS_LED_ADAPTER_H
#define STATUS_LED_ADAPTER_H

#include "Ports/IStatusLEDPort.h"

typedef struct
{
  StatusLEDState_t state;
  uint32_t lastToggleMs;
} StatusLEDAdapterCtx_t;

void StatusLEDAdapterInit(StatusLEDAdapterCtx_t *ctx);
IStatusLEDPort_t StatusLEDAdapterCreatePort(StatusLEDAdapterCtx_t *ctx);
void StatusLEDAdapterTick(StatusLEDAdapterCtx_t *ctx, uint32_t nowMs);

#endif /* STATUS_LED_ADAPTER_H */
