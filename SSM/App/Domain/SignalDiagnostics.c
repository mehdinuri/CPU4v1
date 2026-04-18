/**
 ******************************************************************************
 * @file    Domain/SignalDiagnostics.c
 ******************************************************************************
 */

#include <string.h>
#include "Domain/SignalDiagnostics.h"

void SignalDiagnostics_Reset(tSSignalDiagnosticsState *pState)
{
  memset(pState, 0, sizeof(*pState));
}

uint8_t SignalDiagnostics_Step(tSSignalDiagnosticsState *pState,
                               const tSSignalOutputImage *pObserved,
                               const tSCurrentMeasurementSnapshot *pSnap)
{
  uint8_t bNewFault = 0U;
  uint8_t bGrpIdx;

  for (bGrpIdx = 0U; bGrpIdx < SIGNAL_GROUP_COUNT; bGrpIdx++)
  {
    /* Calculate the 3-bit combination mask for this group (Red=bit0, Yel=bit1, Grn=bit2) */
    uint8_t bMask = 0U;
    uint8_t i;

    for (i = 0U; i < 3U; i++)
    {
      if (pObserved->aChannels[(bGrpIdx * 3U) + i] != 0U)
      {
        bMask = (uint8_t) (bMask | (uint8_t) (1U << i));
      }
    }

    /* Combination 0 (all dark) is not useful for lamp-out diagnostics. */
    if (bMask == 0U)
    {
      continue;
    }

    uint16_t sMeasured_mA = pSnap->aCurrents_mA[bGrpIdx];
    uint16_t sBaseline_mA = pState->aBaselineCurrent_mA[bGrpIdx][bMask];

    if (sBaseline_mA == 0U)
    {
      /* Learning Phase: Store the baseline if the current is high enough to be valid. */
      if (sMeasured_mA >= SIGNAL_DIAGNOSTICS_LEARN_MIN_MA)
      {
        pState->aBaselineCurrent_mA[bGrpIdx][bMask] = sMeasured_mA;
      }
    }
    else
    {
      /* Monitoring Phase: Check for significant undercurrent. */
      uint32_t lThreshold_mA = ((uint32_t) sBaseline_mA
                                * SIGNAL_DIAGNOSTICS_FAULT_PERCENT) / 100U;

      if (sMeasured_mA < (uint16_t) lThreshold_mA)
      {
        if (pState->aLampOutFault[bGrpIdx] == 0U)
        {
          pState->aLampOutFault[bGrpIdx] = 1U;
          bNewFault = 1U;
        }
      }
    }
  }

  return bNewFault;
} /* SignalDiagnostics_Step */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
