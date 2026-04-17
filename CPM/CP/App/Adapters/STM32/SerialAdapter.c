/* App/Adapters/STM32/SerialAdapter.c
 *
 * ISerialPort_t concrete implementation for STM32H743 UARTs.
 * Uses HAL_UARTEx_ReceiveToIdle_DMA for RX and HAL_UART_Transmit_DMA
 * for TX.  TX completion is synchronised via a FreeRTOS binary
 * semaphore so that services block in SerialSend() without task-
 * specific thread flags.
 *
 * SerialAdapterRegisterHalCallbacks() is defined in usart.c
 * USER CODE BEGIN 0 — see the declaration in SerialAdapter.h.
 */
#include "SerialAdapter.h"
#include <string.h>

/* ------------------------------------------------------------------
 * Static (file-private) port implementation functions
 * ------------------------------------------------------------------ */

static uint8_t AdapterSend(void          *ctx,
                           const uint8_t *data,
                           uint16_t len,
                           uint32_t timeoutMs)
{
  SerialAdapterCtx_t *c = (SerialAdapterCtx_t *) ctx;
  HAL_StatusTypeDef status;

  /* HAL_UART_Transmit_DMA takes a non-const pointer but only reads the
   * buffer via DMA.  The cast is safe. */
  status = HAL_UART_Transmit_DMA(c->huart,
                                 (uint8_t *) (uintptr_t) data,
                                 len);
  if (status != HAL_OK)
  {
    return 1U;
  }

  if (osSemaphoreAcquire(c->txDoneSem, timeoutMs) != osOK)
  {
    /* Timeout: abort to leave the UART clean for the next call. */
    (void) HAL_UART_Abort(c->huart);

    return 1U;
  }

  return 0U;
}

static uint8_t AdapterSetBaudRate(void *ctx, uint32_t baudRate)
{
  SerialAdapterCtx_t *c = (SerialAdapterCtx_t *) ctx;

  /* Kill any in-progress RX DMA before touching UART registers. */
  (void) HAL_UART_Abort(c->huart);

  c->huart->Init.BaudRate = baudRate;

  if (HAL_UART_Init(c->huart) != HAL_OK)
  {
    return 1U;
  }

  if (HAL_UARTEx_SetTxFifoThreshold(c->huart,
                                    UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    return 1U;
  }

  if (HAL_UARTEx_SetRxFifoThreshold(c->huart,
                                    UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    return 1U;
  }

  if (HAL_UARTEx_DisableFifoMode(c->huart) != HAL_OK)
  {
    return 1U;
  }

  /* Re-arm reception at the new baud rate. */
  memset(c->rxDmaBuf, 0, c->rxDmaBufSize);
  (void) HAL_UARTEx_ReceiveToIdle_DMA(c->huart, c->rxDmaBuf, c->rxDmaBufSize);
  __HAL_DMA_DISABLE_IT(c->hdmaRx, DMA_IT_HT);

  return 0U;
}

static void AdapterSetRxCallback(void    *ctx,
                                 void (*onRx)(void          *arg,
                                              const uint8_t *data,
                                              uint16_t len),
                                 void    *arg)
{
  SerialAdapterCtx_t *c = (SerialAdapterCtx_t *) ctx;

  c->onRx = onRx;
  c->onRxArg = arg;
}

/* ------------------------------------------------------------------
 * HAL callback entry points — called from usart.c USER CODE dispatch
 * ------------------------------------------------------------------ */

void SerialAdapterOnRxEvent(SerialAdapterCtx_t *ctx, uint16_t size)
{
  if ((ctx->onRx != NULL) && (size > 0U))
  {
    ctx->onRx(ctx->onRxArg, ctx->rxDmaBuf, size);
  }

  /* Re-arm: clear buffer then restart idle-line DMA. */
  memset(ctx->rxDmaBuf, 0, ctx->rxDmaBufSize);
  (void) HAL_UARTEx_ReceiveToIdle_DMA(ctx->huart,
                                      ctx->rxDmaBuf,
                                      ctx->rxDmaBufSize);
  __HAL_DMA_DISABLE_IT(ctx->hdmaRx, DMA_IT_HT);
}

void SerialAdapterOnTxCplt(SerialAdapterCtx_t *ctx)
{
  (void) osSemaphoreRelease(ctx->txDoneSem);
}

void SerialAdapterOnError(SerialAdapterCtx_t *ctx)
{
  /* Abort cleanly then re-arm.  Do not invoke onRx — data is corrupt. */
  (void) HAL_UART_AbortReceive(ctx->huart);
  memset(ctx->rxDmaBuf, 0, ctx->rxDmaBufSize);
  (void) HAL_UARTEx_ReceiveToIdle_DMA(ctx->huart,
                                      ctx->rxDmaBuf,
                                      ctx->rxDmaBufSize);
  __HAL_DMA_DISABLE_IT(ctx->hdmaRx, DMA_IT_HT);
}

/* ------------------------------------------------------------------
 * Public API
 * ------------------------------------------------------------------ */

void SerialAdapterInit(SerialAdapterCtx_t *ctx,
                       UART_HandleTypeDef *huart,
                       DMA_HandleTypeDef  *hdmaRx,
                       uint8_t            *rxDmaBuf,
                       uint16_t rxDmaBufSize)
{
  const osSemaphoreAttr_t attr = { .name = "SerialTxDone" };

  ctx->huart = huart;
  ctx->hdmaRx = hdmaRx;
  ctx->rxDmaBuf = rxDmaBuf;
  ctx->rxDmaBufSize = rxDmaBufSize;
  ctx->onRx = NULL;
  ctx->onRxArg = NULL;

  /* Binary semaphore: max count 1, initial count 0 (blocked). */
  ctx->txDoneSem = osSemaphoreNew(1U, 0U, &attr);

  memset(rxDmaBuf, 0, rxDmaBufSize);
  (void) HAL_UARTEx_ReceiveToIdle_DMA(huart, rxDmaBuf, rxDmaBufSize);
  __HAL_DMA_DISABLE_IT(hdmaRx, DMA_IT_HT);
}

ISerialPort_t SerialAdapterCreatePort(SerialAdapterCtx_t *ctx)
{
  ISerialPort_t port;

  port.ctx = ctx;
  port.Send = AdapterSend;
  port.SetBaudRate = AdapterSetBaudRate;
  port.SetRxCallback = AdapterSetRxCallback;

  return port;
}
