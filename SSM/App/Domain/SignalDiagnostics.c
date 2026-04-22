/**
 ******************************************************************************
 * @file    Domain/SignalDiagnostics.c
 ******************************************************************************
 */

#include <string.h>
#include "Domain/SignalDiagnostics.h"

void SignalDiagnostics_Reset(SignalDiagnosticsState_t *state)
{
  memset(state, 0, sizeof(*state));
}

uint8_t SignalDiagnostics_Step(SignalDiagnosticsState_t *state,
                               const SignalOutputImage_t *observed,
                               const CurrentMeasurementSnapshot_t *snap)
{
  uint8_t newFault = 0U;
  uint8_t grpIdx;

  for (grpIdx = 0U; grpIdx < SIGNAL_GROUP_COUNT; grpIdx++)
  {
    /* Calculate the 3-bit combination mask for this group (Red=bit0, Yel=bit1, Grn=bit2) */
    uint8_t mask = 0U;
    uint8_t i;

    for (i = 0U; i < 3U; i++)
    {
      if (observed->channels[(grpIdx * 3U) + i] != 0U)
      {
        mask = (uint8_t) (mask | (uint8_t) (1U << i));
      }
    }

    /* Combination 0 (all dark) is not useful for lamp-out diagnostics. */
    if (mask == 0U)
    {
      continue;
    }

    uint16_t measuredMa = snap->currentsMa[grpIdx];
    uint16_t baselineMa = state->baselineCurrentMa[grpIdx][mask];

    if (baselineMa == 0U)
    {
      /* Learning Phase: Store the baseline if the current is high enough to be valid. */
      if (measuredMa >= SIGNAL_DIAGNOSTICS_LEARN_MIN_MA)
      {
        state->baselineCurrentMa[grpIdx][mask] = measuredMa;
      }
    }
    else
    {
      /* Monitoring Phase: Check for significant undercurrent. */
      uint32_t thresholdMa = ((uint32_t) baselineMa
                              * SIGNAL_DIAGNOSTICS_FAULT_PERCENT) / 100U;

      if (measuredMa < (uint16_t) thresholdMa)
      {
        if (state->lampOutFault[grpIdx] == 0U)
        {
          state->lampOutFault[grpIdx] = 1U;
          newFault = 1U;
        }
      }
    }
  }

  return newFault;
} /* SignalDiagnostics_Step */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
