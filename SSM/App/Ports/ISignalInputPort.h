/**
 ******************************************************************************
 * @file    Ports/ISignalInputPort.h
 * @brief   Port: sample the 12 signal-output readback pins + the 4-bit card ID.
 ******************************************************************************
 */

#ifndef PORTS_ISIGNAL_INPUT_PORT_H
#define PORTS_ISIGNAL_INPUT_PORT_H

#include <stdint.h>
#include "Ports/ISignalOutputPort.h"    /* for SIGNAL_OUTPUT_CHANNEL_COUNT */

typedef struct
{
  /* 1 = energised high (i.e. GPIO read active). The on-pin is active-low in
   * hardware; the adapter inverts so the domain sees the logical state.
   */
  uint8_t channels[SIGNAL_OUTPUT_CHANNEL_COUNT];
  uint8_t cardId;                              /* 0..15 from ID0..ID3 */
} SignalInputSnapshot_t;

typedef struct ISignalInputPort
{
  void *ctx;

  void (*Sample)(void *ctx, SignalInputSnapshot_t *out);
} ISignalInputPort_t;

static inline void SignalInput_Sample(ISignalInputPort_t *port,
                                      SignalInputSnapshot_t *out)
{
  port->Sample(port->ctx, out);
}

#endif /* PORTS_ISIGNAL_INPUT_PORT_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
