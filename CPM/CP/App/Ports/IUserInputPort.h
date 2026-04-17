/* App/Ports/IUserInputPort.h
 *
 * Port interface for a matrix keypad.
 * UserInputScanSnapshot() returns the currently active logical keys as
 * a bitmask snapshot.
 */
#ifndef IUSER_INPUT_PORT_H
#define IUSER_INPUT_PORT_H

#include "Ports/UserInputTypes.h"

typedef struct
{
  void            *ctx;

  KeypadSnapshot_t (*ScanSnapshot)(void *ctx);
} IUserInputPort_t;

static inline KeypadSnapshot_t UserInputScanSnapshot(IUserInputPort_t *p)
{
  return p->ScanSnapshot(p->ctx);
}

#endif /* IUSER_INPUT_PORT_H */
