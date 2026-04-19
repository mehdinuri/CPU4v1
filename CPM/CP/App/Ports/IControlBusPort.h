/* App/Ports/IControlBusPort.h
 *
 * Port interface for the private CP <-> MP control bus on FDCAN2.
 * Transport-only: higher-level framing and configuration transfer live in
 * a domain service layered on top of this port.
 */
#ifndef ICONTROL_BUS_PORT_H
#define ICONTROL_BUS_PORT_H

#include <stdint.h>

#define CONTROL_BUS_FRAME_MAX_LENGTH 64U

typedef struct
{
  uint16_t standardId;
  uint8_t length;
  uint8_t data[CONTROL_BUS_FRAME_MAX_LENGTH];
} ControlBusFrame_t;

typedef void (*ControlBusRxCallback_t)(void *cbCtx,
                                       const ControlBusFrame_t *frame);

typedef struct
{
  void *ctx;

  uint8_t (*SendFrame)(void *ctx, const ControlBusFrame_t *frame);
  uint8_t (*RegisterRxCallback)(void *ctx,
                                ControlBusRxCallback_t cb,
                                void *cbCtx);
} IControlBusPort_t;

static inline uint8_t ControlBusSendFrame(IControlBusPort_t *port,
                                          const ControlBusFrame_t *frame)
{
  if ((port == NULL) || (port->SendFrame == NULL))
  {
    return 0U;
  }

  return port->SendFrame(port->ctx, frame);
}

static inline uint8_t ControlBusRegisterRxCallback(IControlBusPort_t *port,
                                                   ControlBusRxCallback_t cb,
                                                   void *cbCtx)
{
  if ((port == NULL) || (port->RegisterRxCallback == NULL))
  {
    return 0U;
  }

  return port->RegisterRxCallback(port->ctx, cb, cbCtx);
}

#endif /* ICONTROL_BUS_PORT_H */
