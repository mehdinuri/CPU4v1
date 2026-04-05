/*
 * App/Adapters/STM32/KeypadAdapter.c
 *
 * IUserInputPort implementation — GPIO keypad matrix → command FIFO.
 *
 * The keypad matrix scan is performed in UITask (polling or EXTI-driven).
 * Raw key-codes are translated to UserCommand_t by KeypadAdapter_PushCommand().
 * The Domain calls hasCommand() / dequeue() to consume them each tick.
 *
 * NOTE: PushCommand() may be called from an ISR context.  On STM32H7 with
 * FreeRTOS the FIFO is protected by disabling interrupts for the duration
 * of the update (taskENTER_CRITICAL_FROM_ISR / taskEXIT_CRITICAL_FROM_ISR).
 * On host builds no locking is needed.
 */
#include "KeypadAdapter.h"
#include <string.h>

#ifdef STM32H743xx
#include "stm32h7xx_hal.h"  /* Pulls in CMSIS core intrinsics: __get_PRIMASK, __disable_irq, __enable_irq */
#include "cmsis_os2.h"
#endif

/* --------------------------------------------------------------------------
 * Forward declarations
 * --------------------------------------------------------------------------*/
static UserCommand_t Keypad_Dequeue(void *ctx);
static bool Keypad_HasCommand(void *ctx);

/* --------------------------------------------------------------------------
 * Public API
 * --------------------------------------------------------------------------*/

void KeypadAdapter_Init(KeypadAdapterCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
}

IUserInputPort_t KeypadAdapter_CreatePort(KeypadAdapterCtx_t *ctx)
{
  IUserInputPort_t port;

  port.ctx = ctx;
  port.dequeue = Keypad_Dequeue;
  port.hasCommand = Keypad_HasCommand;

  return port;
}

void KeypadAdapter_PushCommand(KeypadAdapterCtx_t *ctx, UserCommand_t cmd)
{
  if (cmd == USER_CMD_NONE)
  {
    return;
  }

  #ifdef STM32H743xx
  /* Enter critical section — safe to call from ISR on Cortex-M7. */
  uint32_t primask = __get_PRIMASK();

  __disable_irq();
  #endif

  if (ctx->count < KEYPAD_QUEUE_DEPTH)
  {
    ctx->queue[ctx->tail] = cmd;
    ctx->tail = (uint8_t) ((ctx->tail + 1U) % KEYPAD_QUEUE_DEPTH);
    ctx->count++;
  }

  /* Else: queue full — command is dropped silently. */

  #ifdef STM32H743xx
  if (!primask)
  {
    __enable_irq();
  }

  #endif
}

/* --------------------------------------------------------------------------
 * Port callbacks
 * --------------------------------------------------------------------------*/

static bool Keypad_HasCommand(void *vctx)
{
  KeypadAdapterCtx_t *ctx = (KeypadAdapterCtx_t *) vctx;

  return ctx->count > 0U;
}

static UserCommand_t Keypad_Dequeue(void *vctx)
{
  KeypadAdapterCtx_t *ctx = (KeypadAdapterCtx_t *) vctx;

  if (ctx->count == 0U)
  {
    return USER_CMD_NONE;
  }

  UserCommand_t cmd = ctx->queue[ctx->head];

  ctx->head = (uint8_t) ((ctx->head + 1U) % KEYPAD_QUEUE_DEPTH);
  ctx->count--;

  return cmd;
}
