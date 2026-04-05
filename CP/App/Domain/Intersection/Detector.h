#pragma once

/*
 * App/Domain/Intersection/Detector.h
 *
 * Vehicle Detector demand / occupancy / gap aggregation.
 *
 * Called once per 100 ms tick. Polls the IDetectorInputPort for
 * the current occupancy state and accumulates demand/occupancy counters.
 * The transition rule evaluator reads the accumulated values.
 */
#include "Types.h"
#include "Ports/IDetectorInputPort.h"
#include "Ports/ISNMPNotifierPort.h"

/**
 * Reset all per-period accumulators for a single Detector.
 * Called at the start of each phase or period boundary.
 */
void Detector_ResetPeriod(DetectorRuntime_t *rt);

/**
 * Poll the hardware port and update demand / occupancy / gap / broken
 * counters for one Detector.
 *
 * @param detIdx       Detector index
 * @param rt           Mutable runtime state
 * @param cfg          Immutable configuration
 * @param port         Detector input port to poll
 * @param snmpNotifier Port to notify on broken/recovered transitions
 */
void Detector_Tick(uint8_t detIdx,
                   DetectorRuntime_t    *rt,
                   const DetectorConfig_t *cfg,
                   IDetectorInputPort_t  *port,
                   ISnmpNotifierPort_t   *snmpNotifier);

/**
 * Return true if the Detector has accumulated any demand since the last
 * call to Detector_ResetPeriod().
 */
bool Detector_HasDemand(const DetectorRuntime_t *rt);

/**
 * Calculate the current gap duration (ms since last demand pulse).
 * Returns UINT16_MAX if no demand has been seen in the current period.
 */
uint16_t Detector_GapMs(const DetectorRuntime_t *rt);
