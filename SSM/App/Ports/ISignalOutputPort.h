/**
 ******************************************************************************
 * @file    Ports/ISignalOutputPort.h
 * @brief   Port: drive the 12 signal-output GPIOs (R/Y/G per 4 groups).
 *          Pure interface — no HAL, no FreeRTOS.
 ******************************************************************************
 */

#ifndef PORTS_ISIGNAL_OUTPUT_PORT_H
#define PORTS_ISIGNAL_OUTPUT_PORT_H

#include <stdint.h>

#define SIGNAL_OUTPUT_CHANNEL_COUNT 12U

/* Channel order, left to right:
 *   [0]=R1 [1]=Y1 [2]=G1   [3]=R2 [4]=Y2 [5]=G2
 *   [6]=R3 [7]=Y3 [8]=G3   [9]=R4 [10]=Y4 [11]=G4
 */
typedef struct
{
  uint8_t channels[SIGNAL_OUTPUT_CHANNEL_COUNT];    /* 1 = energised, 0 = off */
} SignalOutputImage_t;

typedef struct ISignalOutputPort
{
  void *ctx;

  void (*Apply)(void *ctx, const SignalOutputImage_t *image);
  void (*AllOff)(void *ctx);
} ISignalOutputPort_t;

static inline void SignalOutput_Apply(ISignalOutputPort_t *port,
                                      const SignalOutputImage_t *image)
{
  port->Apply(port->ctx, image);
}

static inline void SignalOutput_AllOff(ISignalOutputPort_t *port)
{
  port->AllOff(port->ctx);
}

#endif /* PORTS_ISIGNAL_OUTPUT_PORT_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
