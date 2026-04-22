/**
 ******************************************************************************
 * @file    Domain/SignalDiagnostics.h
 * @brief   Advanced baselining diagnostics for grouped current sensors.
 *
 *          Correlates 12 individual voltage readbacks with 4 group-RMS current
 *          measurements to detect lamp-outs (Dark Approach).
 ******************************************************************************
 */

#ifndef DOMAIN_SIGNAL_DIAGNOSTICS_H
#define DOMAIN_SIGNAL_DIAGNOSTICS_H

#include <stdint.h>
#include "Ports/ISignalOutputPort.h"
#include "Ports/ICurrentMeasurementPort.h"

#define SIGNAL_GROUP_COMBINATIONS 8U
#define SIGNAL_GROUP_COUNT        4U

/* Tie the three constants together: combinations must be the power set of
 * the per-group output count, and channels must distribute evenly across
 * groups. Without these checks, silently renumbering SIGNAL_OUTPUT_CHANNEL_COUNT
 * or SIGNAL_GROUP_COUNT would leave baselineCurrentMa[][] mis-sized.
 */
_Static_assert((SIGNAL_OUTPUT_CHANNEL_COUNT % SIGNAL_GROUP_COUNT) == 0U,
               "SIGNAL_OUTPUT_CHANNEL_COUNT must divide evenly by "
               "SIGNAL_GROUP_COUNT");
_Static_assert(SIGNAL_GROUP_COMBINATIONS
               == (1U << (SIGNAL_OUTPUT_CHANNEL_COUNT / SIGNAL_GROUP_COUNT)),
               "SIGNAL_GROUP_COMBINATIONS must equal "
               "2^(outputs per group)");

/* Threshold: if measured current is < 50% of learned baseline, it's a fault. */
#define SIGNAL_DIAGNOSTICS_FAULT_PERCENT 50U

/* Minimum current required to learn a baseline (prevents learning noise). */
#define SIGNAL_DIAGNOSTICS_LEARN_MIN_MA  50U

typedef struct
{
  /* 4 groups, each has 8 possible ON/OFF combinations (2^3).
   * baselineCurrentMa[group][mask] == 0 means not yet learned. */
  uint16_t baselineCurrentMa[SIGNAL_GROUP_COUNT][SIGNAL_GROUP_COMBINATIONS];

  /* Sticky fault flags per group. Once a lamp-out is detected, it stays set. */
  uint8_t lampOutFault[SIGNAL_GROUP_COUNT];
} SignalDiagnosticsState_t;

/**
 * @brief Zero the diagnostics state.
 */
void SignalDiagnostics_Reset(SignalDiagnosticsState_t *state);

/**
 * @brief Perform one cycle of diagnostics.
 *
 * @param state      Diagnostics state (mutable)
 * @param observed   Observed voltage image (12 channels)
 * @param snap       Current measurement snapshot (4 groups)
 * @return 1 if any NEW fault was latched this cycle, 0 otherwise.
 */
uint8_t SignalDiagnostics_Step(SignalDiagnosticsState_t *state,
                               const SignalOutputImage_t *observed,
                               const CurrentMeasurementSnapshot_t *snap);

#endif /* DOMAIN_SIGNAL_DIAGNOSTICS_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
