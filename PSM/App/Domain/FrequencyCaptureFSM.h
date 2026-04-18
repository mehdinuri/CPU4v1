/**
 ******************************************************************************
 * @file    Domain/FrequencyCaptureFSM.h
 * @brief   Pure state machine for TIM input-capture frequency measurement.
 *
 *          Platform wrappers (tim.c) feed this FSM raw captures and tick
 *          timestamps; the FSM owns wraparound handling, pair priming,
 *          timeout detection, tolerance validation, and the bad-reading
 *          counter used to drive standby entry.
 *
 *          No HAL, no globals — all state is passed explicitly.
 ******************************************************************************
 */

#ifndef DOMAIN_FREQUENCY_CAPTURE_FSM_H
#define DOMAIN_FREQUENCY_CAPTURE_FSM_H

#include <stdint.h>

/* ---------------------------------------------------------------------------
 * Configuration (constants per physical timer/grid)
 * ---------------------------------------------------------------------------*/
typedef struct
{
  uint32_t lClockFreqHz;        /* timer counter tick rate, e.g. 100000 */
  uint32_t lCounterMax;          /* 32-bit wraparound boundary, e.g. 0xFFFFFFFF */
  uint32_t lCaptureTimeoutMs;    /* no-edge timeout window (ms), e.g. 100 */
  uint8_t  bTargetFreqHz;        /* expected grid frequency, e.g. 50 */
  uint8_t  bFreqToleranceHz;     /* +/- band around target, e.g. 5 */
  uint16_t sBadReadingsBeforeSleep; /* threshold to enter standby, e.g. 10 */
} tSFrequencyCaptureFSMConfig;

/* ---------------------------------------------------------------------------
 * Mutable state (written by OnEdge/Evaluate, read by platform wrapper)
 * ---------------------------------------------------------------------------*/
typedef struct
{
  uint32_t lPrevCaptureValue;    /* last raw capture (for diff)           */
  uint32_t lLastCaptureTimeMs;   /* tick at most recent edge              */
  uint16_t sBadReadingsCount;    /* strike count toward sleep threshold   */
  uint8_t  bMeasuredFreqHz;      /* last published frequency (0 on fail)  */
  uint8_t  fPrimed;               /* 1 after first edge seen              */
} tSFrequencyCaptureFSMState;

/* ---------------------------------------------------------------------------
 * Evaluate verdict
 * ---------------------------------------------------------------------------*/
typedef enum
{
  FREQ_FSM_VERDICT_OK           = 0,  /* freq in tolerance band            */
  FREQ_FSM_VERDICT_BAD          = 1,  /* out of band or no recent capture  */
  FREQ_FSM_VERDICT_ENTER_SLEEP  = 2   /* strike count reached threshold    */
} tEFrequencyCaptureFSMVerdict;

/* ---------------------------------------------------------------------------
 * API
 * ---------------------------------------------------------------------------*/

/**
 * @brief Reset state to post-init (unprimed, zeroed counters).
 *        @p lNowMs seeds the last-capture timestamp so an Evaluate() called
 *        immediately after Init does not misfire a timeout.
 */
void FrequencyCaptureFSM_Init(tSFrequencyCaptureFSMState *pState,
                              uint32_t lNowMs);

/**
 * @brief Feed a fresh input-capture value.
 *
 * First edge after priming just stores the value and returns 0.
 * Every subsequent edge computes the wraparound-safe diff, derives frequency,
 * updates state, and returns 1 — signalling the caller to publish
 * @c pState->bMeasuredFreqHz.
 *
 * @return 1 if a new frequency measurement was produced, 0 on first edge.
 */
uint8_t FrequencyCaptureFSM_OnEdge(tSFrequencyCaptureFSMState *pState,
                                   const tSFrequencyCaptureFSMConfig *pCfg,
                                   uint32_t lCaptureValue,
                                   uint32_t lNowMs);

/**
 * @brief Periodic validation tick.
 *
 * Checks for capture timeout, resets frequency on timeout, applies tolerance
 * band, and maintains the bad-reading strike count.  Returns the verdict
 * the platform layer should act on — callers hand standby entry to hardware
 * when they see FREQ_FSM_VERDICT_ENTER_SLEEP.
 */
tEFrequencyCaptureFSMVerdict FrequencyCaptureFSM_Evaluate(
    tSFrequencyCaptureFSMState *pState,
    const tSFrequencyCaptureFSMConfig *pCfg,
    uint32_t lNowMs);

#endif /* DOMAIN_FREQUENCY_CAPTURE_FSM_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
