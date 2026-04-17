/* App/Adapters/STM32/SerialAdapter.h
 *
 * ISerialPort_t concrete implementation for STM32H743 UARTs with
 * DMA RX (ReceiveToIdle) and DMA TX.  One instance per physical UART;
 * all state lives in the caller-supplied SerialAdapterCtx_t — no
 * malloc, MISRA-clean.
 *
 * Usage (in main_stm32.c):
 *   SerialAdapterInit(&ctx, &huart4, &hdma_uart4_rx, buf, sizeof(buf));
 *   SerialAdapterRegisterHalCallbacks(&huart4, &ctx);
 *   g_modemPort = SerialAdapterCreatePort(&ctx);
 */
#ifndef SERIAL_ADAPTER_H
#define SERIAL_ADAPTER_H

#include "Ports/ISerialPort.h"
#include "stm32h7xx_hal.h"
#include "cmsis_os.h"

typedef struct
{
  /* HAL handles — set at init, never changed afterwards */
  UART_HandleTypeDef *huart;
  DMA_HandleTypeDef  *hdmaRx;

  /* DMA RX landing buffer — must reside in DMA-accessible SRAM.
   * Memory is owned by the caller (declared static in main_stm32.c). */
  uint8_t            *rxDmaBuf;
  uint16_t rxDmaBufSize;

  /* Binary semaphore: initially unavailable (count 0).
   * AdapterSend acquires it; AdapterOnTxCplt releases it. */
  osSemaphoreId_t txDoneSem;

  /* RX callback registered by the owning service via SetRxCallback. */
  void (*onRx)(void *arg, const uint8_t *data, uint16_t len);
  void               *onRxArg;
} SerialAdapterCtx_t;

/* ------------------------------------------------------------------
 * Public API
 * ------------------------------------------------------------------ */

/* Initialise the adapter: store handles, create the TX semaphore, arm
 * the first ReceiveToIdle_DMA transfer.  Must be called after
 * osKernelInitialize but before any service Init. */
void SerialAdapterInit(SerialAdapterCtx_t *ctx,
                       UART_HandleTypeDef *huart,
                       DMA_HandleTypeDef  *hdmaRx,
                       uint8_t            *rxDmaBuf,
                       uint16_t rxDmaBufSize);

/* Build an ISerialPort_t wired to ctx. */
ISerialPort_t SerialAdapterCreatePort(SerialAdapterCtx_t *ctx);

/* ------------------------------------------------------------------
 * HAL callback entry points — called from the dispatch registry in
 * usart.c USER CODE BEGIN 0.  Not part of the public port interface.
 * ------------------------------------------------------------------ */
void SerialAdapterOnRxEvent(SerialAdapterCtx_t *ctx, uint16_t size);
void SerialAdapterOnTxCplt(SerialAdapterCtx_t  *ctx);
void SerialAdapterOnError(SerialAdapterCtx_t   *ctx);

/* ------------------------------------------------------------------
 * HAL callback registration — defined in usart.c USER CODE BEGIN 0
 * so that it can access the static registry table without exposing
 * it as a global.  Declared here so that main_stm32.c can call it
 * after SerialAdapterInit.
 * ------------------------------------------------------------------ */
void SerialAdapterRegisterHalCallbacks(UART_HandleTypeDef *huart,
                                       SerialAdapterCtx_t *ctx);

#endif /* SERIAL_ADAPTER_H */
