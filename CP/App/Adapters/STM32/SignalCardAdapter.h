#pragma once

/*
 * App/Adapters/STM32/SignalCardAdapter.h
 *
 * ISignalOutputPort concrete implementation for STM32H743.
 * Batches lamp state changes and serialises them into FDCAN frames at
 * flush().  CAN ID = 0x200 + cardIdx, 8 bytes per frame covering 8 lamp
 * outputs.  The isReady() flag is set by CANRxTask when the SSM card
 * replies with an ACK frame.
 */
#include "Ports/ISignalOutputPort.h"
#include "Domain/Intersection/Types.h"

#ifdef STM32H743xx
#include "stm32h7xx_hal.h"
#endif

#define SIGNAL_CARD_COUNT_MAX    12U   /* Max SSM cards on the bus          */
#define SIGNAL_OUTPUTS_PER_CARD   8U   /* Lamp outputs per CAN frame/card   */

typedef struct
{
  SignalColor_t pending[SIGNAL_OUTPUTS_MAX];   /* Buffered states per output */
  bool dirty[SIGNAL_OUTPUTS_MAX];              /* True if output changed since last flush */
  bool ackReceived;                            /* Set by CANRxTask on ACK frame */

  #ifdef STM32H743xx
  FDCAN_HandleTypeDef *hfdcan;                 /* HAL FDCAN handle              */
  #else
  void *hfdcan;                                /* Placeholder for host builds   */
  #endif
} SignalCardAdapterCtx_t;

/**
 * Initialise the adapter context and store the FDCAN handle.
 * Must be called before SignalCardAdapter_CreatePort().
 */
#ifdef STM32H743xx
void SignalCardAdapter_Init(SignalCardAdapterCtx_t *ctx,
                            FDCAN_HandleTypeDef     *hfdcan);

#else
void SignalCardAdapter_Init(SignalCardAdapterCtx_t *ctx, void *hfdcan);

#endif

/**
 * Build an ISignalOutputPort_t wired to ctx.
 * Returns a value type — store it on the stack or in a static variable.
 */
ISignalOutputPort_t SignalCardAdapter_CreatePort(SignalCardAdapterCtx_t *ctx);

/**
 * Called by CANRxTask to signal that the SSM has ACK'd the last flush.
 * canId must match 0x200 + cardIdx range; the function sets ackReceived.
 */
void SignalCardAdapter_NotifyAck(SignalCardAdapterCtx_t *ctx, uint32_t canId);
