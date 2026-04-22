/**
 ******************************************************************************
 * @file    Domain/OutputVerify.c
 ******************************************************************************
 */

#include <string.h>
#include "Domain/OutputVerify.h"

void OutputVerify_Reset(OutputVerifyState_t *state)
{
  memset(state, 0, sizeof(*state));
}

void OutputVerify_Step(OutputVerifyState_t *state,
                       const SignalOutputImage_t *commanded,
                       const SignalOutputImage_t *observed)
{
  uint8_t i;

  for (i = 0U; i < SIGNAL_OUTPUT_CHANNEL_COUNT; i++)
  {
    uint8_t cmd = (commanded->channels[i] != 0U) ? 1U : 0U;
    uint8_t obs = (observed->channels[i] != 0U) ? 1U : 0U;

    if (cmd == obs)
    {
      state->mismatchCount[i] = 0U;
    }
    else
    {
      if (state->mismatchCount[i] < 0xFFU)
      {
        state->mismatchCount[i]++;
      }

      if (state->mismatchCount[i] >= OUTPUT_VERIFY_FAULT_THRESHOLD)
      {
        state->faultActive = 1U;
      }
    }
  }
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
