/**
 ******************************************************************************
 * @file    Domain/SignalSampleAccumulator.c
 ******************************************************************************
 */

#include <string.h>
#include "Domain/SignalSampleAccumulator.h"
#include "Domain/SignalOutput.h"

void SignalSampleAccumulator_Reset(SignalSampleAccumulator_t *acc)
{
  memset(acc, 0, sizeof(*acc));
}

void SignalSampleAccumulator_Observe(SignalSampleAccumulator_t *acc,
                                     const SignalInputSnapshot_t *snap)
{
  uint8_t i;

  for (i = 0U; i < SIGNAL_OUTPUT_CHANNEL_COUNT; i++)
  {
    if (snap->channels[i] != 0U)
    {
      if (acc->onCntr[i] < 0xFFU)
      {
        acc->onCntr[i]++;
      }
    }
    else
    {
      if (acc->offCntr[i] < 0xFFU)
      {
        acc->offCntr[i]++;
      }
    }
  }
}

void SignalSampleAccumulator_Summary(const SignalSampleAccumulator_t *acc,
                                     SignalOutputImage_t *out)
{
  uint8_t i;

  for (i = 0U; i < SIGNAL_OUTPUT_CHANNEL_COUNT; i++)
  {
    out->channels[i] = SignalOutput_IsActive(acc->onCntr[i],
                                             acc->offCntr[i]);
  }
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
