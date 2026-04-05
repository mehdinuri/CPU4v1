/*
 * App/Domain/Intersection/TransitionRule.c
 *
 * RPN transition rule evaluator.
 * Ported from legacy Tasks/Src/Program.c (rule/operation evaluation logic).
 *
 * The bytecode Format is:
 *   - Each OperationConfig_t has: op (operator), operandA, operandB
 *   - Operands 0–31 → phase runtime index
 *   - Operands 32–63 → Detector runtime index (operand - 32)
 *   - Operands 64–75 → counter index (operand - 64)
 *   - Operands ≥ 128 → literal value (operand - 128)
 *
 * The evaluator resolves each operand to a 32-bit integer value, then
 * applies the operator, and stores the result as the output of that node.
 * The result of the last operation in the range is the rule's boolean output.
 */
#include "Domain/Intersection/TransitionRule.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define OPERAND_PHASE_BASE    0U
#define OPERAND_PHASE_LIMIT   32U
#define OPERAND_DET_BASE      32U
#define OPERAND_DET_LIMIT     64U
#define OPERAND_COUNTER_BASE  64U
#define OPERAND_COUNTER_LIMIT 76U
#define OPERAND_LITERAL_BASE  128U

/* Resolve an operand index to its current 32-bit integer value */
static uint32_t resolve_operand(uint8_t operand,
                                const PhaseRuntime_t    *phases,
                                const DetectorRuntime_t *Detectors,
                                const CounterRuntime_t  *counters,
                                const uint32_t          *nodeResults,
                                uint8_t nodeCount)
{
  if (operand >= OPERAND_LITERAL_BASE)
  {
    return (uint32_t) (operand - OPERAND_LITERAL_BASE);
  }

  if ((operand >= OPERAND_COUNTER_BASE) && (operand < OPERAND_COUNTER_LIMIT) )
  {
    uint8_t idx = operand - OPERAND_COUNTER_BASE;

    return counters[idx].value;
  }

  if ((operand >= OPERAND_DET_BASE) && (operand < OPERAND_DET_LIMIT) )
  {
    uint8_t idx = operand - OPERAND_DET_BASE;

    return Detectors[idx].demandCountInPeriod;
  }

  if (operand < OPERAND_PHASE_LIMIT)
  {
    /* Phase elapsed seconds */
    return phases[operand].elapsedSeconds;
  }

  /* Operand references another operation node result */
  if (operand < nodeCount)
  {
    return nodeResults[operand];
  }

  return 0U;
}

/* Apply one RPN operation and return the boolean/integer result */
static uint32_t apply_op(Operator_t op, uint32_t a, uint32_t b)
{
  switch (op)
  {
      case OPR_EQUAL:
      { return (a == b) ? 1U : 0U; }

      case OPR_NOTEQUAL:
      { return (a != b) ? 1U : 0U; }

      case OPR_LESS:
      { return (a <  b) ? 1U : 0U; }

      case OPR_LESS_EQUAL:
      { return (a <= b) ? 1U : 0U; }

      case OPR_GREATER:
      { return (a >  b) ? 1U : 0U; }

      case OPR_GREATER_EQUAL:
      { return (a >= b) ? 1U : 0U; }

      case OPR_ADD:
      { return a + b; }

      case OPR_SUB:
      { return (a > b) ? (a - b) : 0U; }

      case OPR_MUL:
      { return a * b; }

      case OPR_DIV:
      { return (b != 0U) ? (a / b) : 0U; }

      case OPR_MODULO:
      { return (b != 0U) ? (a % b) : 0U; }

      case OPR_AND:
      { return (a && b) ? 1U : 0U; }

      case OPR_OR:
      { return (a || b) ? 1U : 0U; }

      default:
      { return 0U; }
  } /* switch */
} /* apply_op */

bool TransitionRule_Evaluate(const RuleConfig_t      *rule,
                             const OperationConfig_t *ops,
                             uint16_t opCount,
                             const PhaseRuntime_t    *phases,
                             const DetectorRuntime_t *Detectors,
                             const CounterRuntime_t  *counters)
{
  /* Node result cache — one entry per operation in the range */
  uint32_t results[256] = { 0U };
  uint16_t nodeCount = 0U;

  uint16_t start = rule->operationStart;

  /* Evaluate operations until we run out of pool or reach end of range.
   * Treat operation with OPR_NONE as the end sentinel. */
  for (uint16_t i = 0U; i < opCount && start + i < 256U; i++)
  {
    const OperationConfig_t *op = &ops[start + i];

    if (op->op == OPR_NONE)
    {
      break;
    }

    uint32_t a = resolve_operand(op->operandA, phases, Detectors,
                                 counters, results, nodeCount);
    uint32_t b = resolve_operand(op->operandB, phases, Detectors,
                                 counters, results, nodeCount);

    results[nodeCount] = apply_op(op->op, a, b);
    nodeCount++;
  }

  if (nodeCount == 0U)
  {
    return false;     /* Empty rule = not satisfied */
  }

  return results[nodeCount - 1U] != 0U;
}

bool TransitionRule_SelectBest(ControllerState_t currentState,
                               const TransitionConfig_t *transitions,
                               uint8_t transCount,
                               const RuleConfig_t       *rules,
                               const OperationConfig_t  *ops,
                               uint16_t opCount,
                               const PhaseRuntime_t     *phases,
                               const DetectorRuntime_t  *Detectors,
                               const CounterRuntime_t   *counters,
                               TransitionConfig_t       *outTransition)
{
  bool found = false;
  uint8_t bestPriority = 0U;

  for (uint8_t i = 0U; i < transCount; i++)
  {
    const TransitionConfig_t *t = &transitions[i];

    /* Check if this transition applies to the current state */
    if ((t->fromState != (uint8_t) CTRL_STATE_ANY)
        && (t->fromState != (uint8_t) currentState) )
    {
      continue;
    }

    /* Check rule condition */
    if (!TransitionRule_Evaluate(&rules[t->ruleIndex], ops, opCount,
                                 phases, Detectors, counters))
    {
      continue;
    }

    /* Keep highest priority */
    if (!found || (t->priority > bestPriority) )
    {
      *outTransition = *t;
      bestPriority = t->priority;
      found = true;
    }
  }

  return found;
}
