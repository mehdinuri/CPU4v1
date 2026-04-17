/* App/Adapters/STM32/KeypadAdapter.h
 *
 * IUserInputPort concrete implementation for the legacy 4x4 LCD keypad.
 * KEYPAD_ROW5 exists in hardware but is kept low because the legacy UI
 * only uses a 4x4 matrix.
 */
#ifndef KEYPAD_ADAPTER_H
#define KEYPAD_ADAPTER_H

#include "Ports/IUserInputPort.h"

/* Number of logical rows and columns on the LCD keypad. */
#define KEYPAD_ROWS 4U
#define KEYPAD_COLS 4U

typedef struct
{
  KeypadSnapshot_t lastSnapshot;
} KeypadAdapterCtx_t;

/* Initialise GPIO row/column pins. */
void KeypadAdapterInit(KeypadAdapterCtx_t *ctx);

/* Build an IUserInputPort_t wired to ctx. */
IUserInputPort_t KeypadAdapterCreatePort(KeypadAdapterCtx_t *ctx);

#endif /* KEYPAD_ADAPTER_H */
