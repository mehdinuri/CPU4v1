/* App/Ports/IControlBusPort.h
 *
 * Port interface for the dedicated CP <-> MP back-channel (FDCAN2).
 * Transport-only: the domain sends and receives raw standard-ID CAN FD
 * frames. Any segmentation or protocol state machine belongs in a domain
 * service layered on top of this port.
 *
 * MP never transmits on the field bus; all outgoing traffic goes
 * through this port.
 */
#ifndef I_CONTROL_BUS_PORT_H
#define I_CONTROL_BUS_PORT_H

#include <stddef.h>
#include <stdint.h>

#define CONTROL_BUS_FRAME_MAX_LENGTH 64U /* FDCAN FD payload cap */

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

#endif /* I_CONTROL_BUS_PORT_H */
