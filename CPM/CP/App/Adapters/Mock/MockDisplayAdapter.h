/* App/Adapters/Mock/MockDisplayAdapter.h
 *
 * IDisplayPort in-memory test double.
 * Tests inspect framebuffer[] to verify display output without
 * requiring real LCD hardware.
 */
#ifndef MOCK_DISPLAY_ADAPTER_H
#define MOCK_DISPLAY_ADAPTER_H

#include "Ports/IDisplayPort.h"

#define MOCK_DISPLAY_ROWS 4U
#define MOCK_DISPLAY_COLS 21U /* 20 chars + null terminator */

typedef struct
{
  char framebuffer[MOCK_DISPLAY_ROWS][MOCK_DISPLAY_COLS];
  uint8_t powerOn;
  uint32_t clearCount;
  uint32_t writeCount;
} MockDisplayAdapterCtx_t;

void MockDisplayAdapterInit(MockDisplayAdapterCtx_t *ctx);
IDisplayPort_t MockDisplayAdapterCreatePort(MockDisplayAdapterCtx_t *ctx);

#endif /* MOCK_DISPLAY_ADAPTER_H */
