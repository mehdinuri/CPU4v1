/**
 ******************************************************************************
 * @file    Domain/FrequencyCaptureFSM.c
 * @brief   Edge-timing, wraparound, tolerance, and sleep-strike FSM.
 ******************************************************************************
 */

#include "Domain/FrequencyCaptureFSM.h"

#include <stddef.h>

void FrequencyCaptureFSM_Init(FrequencyCaptureFSMState_t *state,
                              uint32_t nowMs)
{
  if (state == NULL)
  {
    return;
  }

  state->prevCaptureValue = 0U;
  state->lastCaptureTimeMs = nowMs;
  state->badReadingsCount = 0U;
  state->measuredFreqHz = 0U;
  state->primed = 0U;
}

uint8_t FrequencyCaptureFSM_OnEdge(FrequencyCaptureFSMState_t *state,
                                   const FrequencyCaptureFSMConfig_t *cfg,
                                   uint32_t captureValue,
                                   uint32_t nowMs)
{
  if ((state == NULL) || (cfg == NULL))
  {
    return 0U;
  }

  state->lastCaptureTimeMs = nowMs;

  if (state->primed == 0U)
  {
    state->prevCaptureValue = captureValue;
    state->primed = 1U;

    return 0U;
  }

  /* Compute wraparound-safe diff (counter is a uint up to counterMax). */
  uint32_t diff = 0U;

  if (captureValue > state->prevCaptureValue)
  {
    diff = captureValue - state->prevCaptureValue;
  }
  else if (captureValue < state->prevCaptureValue)
  {
    diff = (cfg->counterMax - state->prevCaptureValue)
           + captureValue
           + 1U;
  }
  else
  {
    /* Same counter value twice — force re-prime on next edge. */
    diff = 0U;
    state->primed = 0U;
  }

  if (diff != 0U)
  {
    state->measuredFreqHz = (uint8_t) (cfg->clockFreqHz / diff);
  }
  else
  {
    state->measuredFreqHz = 0U;
  }

  state->prevCaptureValue = captureValue;

  return 1U;
} /* FrequencyCaptureFSM_OnEdge */

FrequencyCaptureFSMVerdict_e FrequencyCaptureFSM_Evaluate(
  FrequencyCaptureFSMState_t *state,
  const
  FrequencyCaptureFSMConfig_t
  *cfg,
  uint32_t nowMs)
{
  if ((state == NULL) || (cfg == NULL))
  {
    return FREQ_FSM_VERDICT_BAD;
  }

  uint8_t timedOut = 0U;

  if (state->primed == 0U)
  {
    timedOut = 1U;
  }
  else if ((nowMs - state->lastCaptureTimeMs) > cfg->captureTimeoutMs)
  {
    timedOut = 1U;
  }
  else
  {
    timedOut = 0U;
  }

  if (timedOut != 0U)
  {
    state->measuredFreqHz = 0U;
    state->primed = 0U;
  }

  /* Promote to int16_t before subtracting so a config where tolerance
  * exceeds target cannot underflow past zero into a huge unsigned. */
  int16_t target = (int16_t) cfg->targetFreqHz;
  int16_t tol = (int16_t) cfg->freqToleranceHz;
  int16_t lo = target - tol;
  int16_t hi = target + tol;

  if (lo < 0)
  {
    lo = 0;
  }

  if (hi > 0xFF)
  {
    hi = 0xFF;
  }

  int16_t freq = (int16_t) state->measuredFreqHz;
  uint8_t inBand = ((timedOut == 0U) && (freq >= lo) && (freq <= hi))
                   ? 1U : 0U;

  if (inBand != 0U)
  {
    state->badReadingsCount = 0U;

    return FREQ_FSM_VERDICT_OK;
  }

  state->badReadingsCount++;

  if (state->badReadingsCount >= cfg->badReadingsBeforeSleep)
  {
    return FREQ_FSM_VERDICT_ENTER_FLASH;
  }

  return FREQ_FSM_VERDICT_BAD;
} /* FrequencyCaptureFSM_Evaluate */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
