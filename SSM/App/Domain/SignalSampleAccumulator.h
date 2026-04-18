/**
 ******************************************************************************
 * @file    Domain/SignalSampleAccumulator.h
 * @brief   Accumulates per-channel on/off sample counts over a measurement
 *          cycle and, on demand, collapses them into a boolean-per-channel
 *          "is active" summary via SignalOutput_IsActive.
 ******************************************************************************
 */

#ifndef DOMAIN_SIGNAL_SAMPLE_ACCUMULATOR_H
#define DOMAIN_SIGNAL_SAMPLE_ACCUMULATOR_H

#include <stdint.h>
#include "Ports/ISignalOutputPort.h"
#include "Ports/ISignalInputPort.h"

typedef struct
{
  uint8_t aOnCntr[SIGNAL_OUTPUT_CHANNEL_COUNT];
  uint8_t aOffCntr[SIGNAL_OUTPUT_CHANNEL_COUNT];
} tSSignalSampleAccumulator;

/**
 * @brief Zero both counters for every channel.
 */
void SignalSampleAccumulator_Reset(tSSignalSampleAccumulator *pAcc);

/**
 * @brief Increment on/off counters for each channel, based on one input
 *        snapshot. Safe to call many times before Summary.
 */
void SignalSampleAccumulator_Observe(tSSignalSampleAccumulator *pAcc,
                                     const tSSignalInputSnapshot *pSnap);

/**
 * @brief Write per-channel "is active" into the given image (1 = active).
 *        Uses the SignalOutput_IsActive predicate (strict majority).
 */
void SignalSampleAccumulator_Summary(const tSSignalSampleAccumulator *pAcc,
                                     tSSignalOutputImage *pOut);

#endif /* DOMAIN_SIGNAL_SAMPLE_ACCUMULATOR_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
