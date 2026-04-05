#pragma once

/*
 * App/Domain/Intersection/Sequence.h
 *
 * Pre-timed Sequence execution engine.
 *
 * A Sequence is a fixed list of up to SEQUENCE_STEPS_MAX steps, each with
 * a duration and a per-SG signal assignment. The engine advances steps on
 * elapsed time and drives signal outputs through ISignalOutputPort.
 */
#include "Types.h"
#include "Ports/ISignalOutputPort.h"

/**
 * Reset a Sequence runtime to step 0, not running.
 */
void Sequence_Reset(SequenceRuntime_t *rt);

/**
 * Advance the Sequence by one 100 ms tick.
 * Applies step signals to signalOut when a step boundary is crossed.
 *
 * @param seqIdx    Sequence index (for log/trace)
 * @param rt        Mutable runtime state
 * @param cfg       Immutable Sequence configuration
 * @param sgConfigs Signal group configs (for output index lookup)
 * @param sgCount   Number of configured signal groups
 * @param signalOut Port to emit lamp state changes
 * @return          true if the Sequence completed its last step
 */
bool Sequence_Tick(uint8_t seqIdx,
                   SequenceRuntime_t          *rt,
                   const SequenceConfig_t     *cfg,
                   const SignalGroupConfig_t  *sgConfigs,
                   uint8_t sgCount,
                   ISignalOutputPort_t        *signalOut);

/**
 * Apply the current step's signal assignments to all signal outputs.
 * Called once when a step begins.
 */
void Sequence_ApplyStep(uint8_t step,
                        const SequenceConfig_t    *cfg,
                        const SignalGroupConfig_t *sgConfigs,
                        uint8_t sgCount,
                        ISignalOutputPort_t       *signalOut);
