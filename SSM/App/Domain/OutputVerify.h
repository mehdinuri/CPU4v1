/**
 ******************************************************************************
 * @file    Domain/OutputVerify.h
 * @brief   Post-write output verification with per-channel debouncing.
 *
 *          Each measurement cycle, compare what the controller *commanded*
 *          the 12 signal outputs to be versus what it *observed* them to be
 *          (via the readback GPIOs). A channel that disagrees for N
 *          consecutive cycles signals a real fault — stuck output driver,
 *          broken wiring, burned-out triac, wrong polarity wiring, etc.
 *
 *          Single-cycle mismatches are tolerated (triac fire-angle jitter,
 *          mains zero-cross timing, lamp-warm-up transients). The sticky
 *          fault flag latches until cleared by Reset.
 ******************************************************************************
 */

#ifndef DOMAIN_OUTPUT_VERIFY_H
#define DOMAIN_OUTPUT_VERIFY_H

#include <stdint.h>
#include "Ports/ISignalOutputPort.h"    /* for tSSignalOutputImage */

/* Consecutive-mismatch count that trips the fault. At a 50 Hz task wake
 * cadence, 5 cycles ≈ 100 ms — long enough for triacs to settle, short
 * enough to react well before MUTCD red-signal-failure timers. */
#define OUTPUT_VERIFY_FAULT_THRESHOLD 5U

typedef struct
{
  uint8_t aMismatchCount[SIGNAL_OUTPUT_CHANNEL_COUNT];
  uint8_t bFaultActive;      /* sticky: once set, only Reset clears it */
} tSOutputVerifyState;

/**
 * @brief Reset all counters and clear the fault flag.
 */
void OutputVerify_Reset(tSOutputVerifyState *pState);

/**
 * @brief One cycle of verification. For each channel:
 *          - commanded == observed → reset the channel's mismatch counter.
 *          - commanded != observed → increment; at >= threshold, latch
 *            the fault flag.
 *
 * @note    bFaultActive is sticky; a transient mismatch that resolves on
 *          the next cycle still leaves the flag set if the threshold was
 *          reached. Callers must call Reset to clear.
 */
void OutputVerify_Step(tSOutputVerifyState *pState,
                       const tSSignalOutputImage *pCommanded,
                       const tSSignalOutputImage *pObserved);

#endif /* DOMAIN_OUTPUT_VERIFY_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
