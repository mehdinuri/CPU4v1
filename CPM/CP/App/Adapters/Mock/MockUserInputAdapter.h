/* App/Adapters/Mock/MockUserInputAdapter.h
 *
 * IUserInputPort in-memory test double.
 * Set nextSnapshot before calling the port to inject a keypad state.
 * After one scan returns it, nextSnapshot resets to KEYPAD_SNAPSHOT_NONE.
 */
#ifndef MOCK_USER_INPUT_ADAPTER_H
#define MOCK_USER_INPUT_ADAPTER_H

#include "Ports/IUserInputPort.h"

typedef struct
{
  KeypadSnapshot_t nextSnapshot;
  uint32_t scanCount;
} MockUserInputAdapterCtx_t;

void MockUserInputAdapterInit(MockUserInputAdapterCtx_t *ctx);
IUserInputPort_t MockUserInputAdapterCreatePort(MockUserInputAdapterCtx_t *ctx);

#endif /* MOCK_USER_INPUT_ADAPTER_H */
