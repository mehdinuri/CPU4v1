#pragma once

/*
 * App/Domain/Intersection/Phase.h
 *
 * Phase timing state machine.
 * Tracks elapsed time, enforces min/max green times, and handles
 * actuated extensions (positive = extend, negative = shorten).
 */
#include "Types.h"
#include "Ports/ISystemClockPort.h"

/**
 * Reset a phase runtime to "not started" state.
 */
void Phase_Reset(PhaseRuntime_t *rt);

/**
 * Advance the phase timer by one tick (100 ms = 0.1 s).
 * Returns true when elapsedSeconds reaches maxGreenTime.
 */
bool Phase_Tick(PhaseRuntime_t *rt, const PhaseConfig_t *cfg);

/**
 * Apply an extension or shortening.
 * extensionSeconds > 0 adds green time; < 0 shortens it.
 * Clamped so total does not go below minGreenTime.
 */
void Phase_ApplyExtension(PhaseRuntime_t *rt, const PhaseConfig_t *cfg,
                          int8_t extensionSeconds);

/**
 * Return true if the phase has been running at least minGreenTime.
 */
bool Phase_MinTimeElapsed(const PhaseRuntime_t *rt,
                          const PhaseConfig_t  *cfg);

/**
 * Return true if the phase has reached or exceeded maxGreenTime
 * (accounting for any extensions/shortenings).
 */
bool Phase_MaxTimeElapsed(const PhaseRuntime_t *rt,
                          const PhaseConfig_t  *cfg);
