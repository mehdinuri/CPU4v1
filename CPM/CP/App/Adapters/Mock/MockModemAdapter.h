/* App/Adapters/Mock/MockModemAdapter.h
 *
 * IModemPort_t in-memory test double.  No hardware, no RTOS.
 * Tests inspect cmdBuf/cmdCount to verify AT commands and call
 * MockModemAdapterInjectResponse() to drive state transitions.
 */
#ifndef MOCK_MODEM_ADAPTER_H
#define MOCK_MODEM_ADAPTER_H

#include "Ports/IModemPort.h"

#define MOCK_MODEM_CMD_BUF_SIZE 128U

typedef struct
{
  ISerialPort_t *serialPort;

  /* PrepareCommand inspection */
  char cmdBuf[MOCK_MODEM_CMD_BUF_SIZE];    /* last command text */
  uint8_t cmdCount;                        /* total PrepareCommand calls */
  uint8_t bPrepareResult;                  /* value returned by PrepareCommand */

  /* HandleResponse control */
  uint8_t bNextState; /* value returned by the next HandleResponse() call */

  /* IsDisconnected control */
  uint8_t bDisconnected; /* value returned by IsDisconnected() */

  /* GetGreetingType control */
  uint8_t bGreetingType;

  /* OnConnect control */
  uint8_t bConnectResult;
} MockModemAdapterCtx_t;

void MockModemAdapterInit(MockModemAdapterCtx_t *ctx);
IModemPort_t MockModemAdapterCreatePort(MockModemAdapterCtx_t *ctx);

/* Test helper: set the state value that the next HandleResponse() call
 * will return, simulating a driver advancing through its state machine. */
void MockModemAdapterInjectResponse(MockModemAdapterCtx_t *ctx,
                                    uint8_t nextState);

#endif /* MOCK_MODEM_ADAPTER_H */
