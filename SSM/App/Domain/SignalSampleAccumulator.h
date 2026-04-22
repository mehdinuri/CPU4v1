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
  uint8_t onCntr[SIGNAL_OUTPUT_CHANNEL_COUNT];
  uint8_t offCntr[SIGNAL_OUTPUT_CHANNEL_COUNT];
} SignalSampleAccumulator_t;

/**
 * @brief Zero both counters for every channel.
 */
void SignalSampleAccumulator_Reset(SignalSampleAccumulator_t *acc);

/**
 * @brief Increment on/off counters for each channel, based on one input
 *        snapshot. Safe to call many times before Summary.
 */
void SignalSampleAccumulator_Observe(SignalSampleAccumulator_t *acc,
                                     const SignalInputSnapshot_t *snap);

/**
 * @brief Write per-channel "is active" into the given image (1 = active).
 *        Uses the SignalOutput_IsActive predicate (strict majority).
 */
void SignalSampleAccumulator_Summary(const SignalSampleAccumulator_t *acc,
                                     SignalOutputImage_t *out);

#endif /* DOMAIN_SIGNAL_SAMPLE_ACCUMULATOR_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
