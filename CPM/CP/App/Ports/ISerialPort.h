/* App/Ports/ISerialPort.h
 *
 * Port interface for a full-duplex UART channel backed by DMA.
 * One instance per physical UART: UART4 (GPRS modem), UART8 (internal
 * GPS), USART2 (external GPS or UI serial), UART5 (WiFi placeholder).
 *
 * Services call the inline helpers; adapters implement the function
 * pointers.  No HAL, FreeRTOS, or hardware types appear here.
 */
#ifndef ISERIAL_PORT_H
#define ISERIAL_PORT_H

#include <stdint.h>

typedef struct
{
  void    *ctx;

  /* Blocking DMA transmit.  Returns 0 on success, 1 on timeout or
   * HAL error.  Must not be called from ISR context. */
  uint8_t (*Send)(void          *ctx,
                  const uint8_t *data,
                  uint16_t len,
                  uint32_t timeoutMs);

  /* Reconfigure baud rate and re-arm RX DMA.  Returns 0 on success.
   * Must only be called when no DMA transfer is in progress. */
  uint8_t (*SetBaudRate)(void *ctx, uint32_t baudRate);

  /* Register the application RX handler.  The adapter invokes
   * onRx(arg, data, len) from its HAL RxEvent callback dispatch.
   * Pass NULL for onRx to deregister.  arg is the caller's context. */
  void (*SetRxCallback)(void    *ctx,
                        void (*onRx)(void          *arg,
                                     const uint8_t *data,
                                     uint16_t len),
                        void    *arg);
} ISerialPort_t;

/* ------------------------------------------------------------------
 * Inline dispatch helpers (zero overhead at -O2)
 * ------------------------------------------------------------------ */
static inline uint8_t SerialSend(ISerialPort_t *p,
                                 const uint8_t *data,
                                 uint16_t len,
                                 uint32_t timeoutMs)
{
  return p->Send(p->ctx, data, len, timeoutMs);
}

static inline uint8_t SerialSetBaudRate(ISerialPort_t *p, uint32_t baudRate)
{
  return p->SetBaudRate(p->ctx, baudRate);
}

static inline void SerialSetRxCallback(ISerialPort_t *p,
                                       void (*onRx)(void          *arg,
                                                    const uint8_t *data,
                                                    uint16_t len),
                                       void          *arg)
{
  p->SetRxCallback(p->ctx, onRx, arg);
}

#endif /* ISERIAL_PORT_H */
