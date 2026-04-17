/* App/Adapters/Mock/MockSerialAdapter.h
 *
 * ISerialPort_t in-memory test double.  No HAL, no FreeRTOS, no
 * hardware.  Tests inspect txBuf/txLen/txCount to verify transmitted
 * data and call MockSerialAdapterInjectRx() to simulate arriving bytes.
 */
#ifndef MOCK_SERIAL_ADAPTER_H
#define MOCK_SERIAL_ADAPTER_H

#include "Ports/ISerialPort.h"

#define MOCK_SERIAL_TX_BUF_SIZE 512U

typedef struct
{
  /* TX inspection fields */
  uint8_t txBuf[MOCK_SERIAL_TX_BUF_SIZE];  /* last transmitted payload */
  uint16_t txLen;                           /* byte count of last TX    */
  uint32_t txCount;                         /* total Send() call count  */
  uint8_t txResult;                         /* value returned by Send() */

  /* Baud rate inspection */
  uint32_t baudRate;

  /* Registered RX callback */
  void (*onRx)(void *arg, const uint8_t *data, uint16_t len);
  void    *onRxArg;
} MockSerialAdapterCtx_t;

void MockSerialAdapterInit(MockSerialAdapterCtx_t *ctx);
ISerialPort_t MockSerialAdapterCreatePort(MockSerialAdapterCtx_t *ctx);

/* Test helper: simulate arriving RX bytes — invokes the registered
 * onRx callback directly, matching what the real adapter does from
 * its HAL_UARTEx_RxEventCallback dispatch. */
void MockSerialAdapterInjectRx(MockSerialAdapterCtx_t *ctx,
                               const uint8_t          *data,
                               uint16_t len);

#endif /* MOCK_SERIAL_ADAPTER_H */
