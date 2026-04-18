/**
 ******************************************************************************
 * @file    Domain/SignalOutput.h
 * @brief   Pure-computation signal-output predicates — no HAL, no FreeRTOS.
 *          Inputs are passed by value; no global mutable state.
 ******************************************************************************
 */

#ifndef DOMAIN_SIGNALOUTPUT_H
#define DOMAIN_SIGNALOUTPUT_H

#include <stdint.h>

/**
 * @brief Decide whether a signal output is active from its on/off sample counters.
 *
 * The measurement loop samples each GPIO several times per cycle and increments
 * one of two counters. The output is active for this cycle iff the on-samples
 * outnumber the off-samples (strict majority). Equal counts count as inactive.
 *
 * @param bOnCntr   On-sample count  (0..255)
 * @param bOffCntr  Off-sample count (0..255)
 * @return 1 if bOnCntr > bOffCntr, 0 otherwise
 */
uint8_t SignalOutput_IsActive(uint8_t bOnCntr, uint8_t bOffCntr);

#endif /* DOMAIN_SIGNALOUTPUT_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
