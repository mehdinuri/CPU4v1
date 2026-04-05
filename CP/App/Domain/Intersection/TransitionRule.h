#pragma once

/*
 * App/Domain/Intersection/TransitionRule.h
 *
 * RPN (Reverse Polish Notation) transition rule evaluator.
 *
 * Rules are compiled by the MCT desktop tool into an operation pool
 * stored in flash. Each operation node has an operator and two operand
 * references. Operands read from phase runtime, Detector runtime, or
 * counters. The evaluator traverses the pool and returns a boolean result.
 */
#include "Types.h"

/**
 * Evaluate a transition rule and return its boolean result.
 *
 * @param rule       The rule to evaluate
 * @param ops        Operation pool for this signal Program
 * @param opCount    Number of valid operations in the pool
 * @param phases     Phase runtime array (for elapsed time / min-time checks)
 * @param Detectors  Detector runtime array (for demand / occupancy checks)
 * @param counters   Counter runtime array
 * @return           true if the rule condition is satisfied
 */
bool TransitionRule_Evaluate(const RuleConfig_t      *rule,
                             const OperationConfig_t *ops,
                             uint16_t opCount,
                             const PhaseRuntime_t    *phases,
                             const DetectorRuntime_t *Detectors,
                             const CounterRuntime_t  *counters);

/**
 * Select the highest-priority transition whose condition is currently true.
 *
 * @param currentState  The controller's current state
 * @param transitions   Transition pool for this signal Program
 * @param transCount    Number of transitions in the pool
 * @param rules         Rule pool
 * @param ops           Operation pool
 * @param opCount       Operation pool size
 * @param phases        Phase runtimes
 * @param Detectors     Detector runtimes
 * @param counters      Counter runtimes
 * @param outTransition Populated with the winning transition (if any)
 * @return              true if a transition was selected
 */
bool TransitionRule_SelectBest(ControllerState_t currentState,
                               const TransitionConfig_t *transitions,
                               uint8_t transCount,
                               const RuleConfig_t       *rules,
                               const OperationConfig_t  *ops,
                               uint16_t opCount,
                               const PhaseRuntime_t     *phases,
                               const DetectorRuntime_t  *Detectors,
                               const CounterRuntime_t   *counters,
                               TransitionConfig_t       *outTransition);
