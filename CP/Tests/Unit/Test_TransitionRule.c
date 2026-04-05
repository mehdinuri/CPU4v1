/*
 * Tests/unit/Test_TransitionRule.c
 *
 * Unit tests for App/Domain/Intersection/TransitionRule.c
 *
 * Operand encoding (from TransitionRule.c):
 *   0–31  → phase elapsed seconds (operand = phase index)
 *   32–63 → Detector demand count (operand = 32 + det index)
 *   64–75 → counter value (operand = 64 + counter index)
 *   ≥128  → literal value (operand - 128)
 */
#include "unity.h"
#include "Domain/Intersection/TransitionRule.h"

void setUp(void)
{
}

void tearDown(void)
{
}

/* --- helpers ----------------------------------------------------------- */

static PhaseRuntime_t phases[PHASES_MAX];
static DetectorRuntime_t Detectors[DETECTORS_MAX];
static CounterRuntime_t counters[COUNTERS_MAX];

static void reset_operands(void)
{
  int i;

  for (i = 0; i < PHASES_MAX;    i++)
  {
    phases[i].elapsedSeconds = 0U;
  }

  for (i = 0; i < DETECTORS_MAX; i++)
  {
    Detectors[i].demandCountInPeriod = 0U;
  }

  for (i = 0; i < COUNTERS_MAX;  i++)
  {
    counters[i].value = 0U;
  }
}

/* Build a single-op rule: (a op b) */
static bool eval_single_op(Operator_t op, uint8_t operandA, uint8_t operandB)
{
  OperationConfig_t ops[2];

  ops[0].op = op;
  ops[0].operandA = operandA;
  ops[0].operandB = operandB;
  ops[1].op = OPR_NONE;

  RuleConfig_t rule;

  rule.operationStart = 0U;

  return TransitionRule_Evaluate(&rule, ops, 2U, phases, Detectors, counters);
}

/* Literal helpers: encode literal n as operand 128+n (n must be 0..127) */
#define LIT(n) ((uint8_t) (128U + (uint8_t) (n)))
#define PHASE_OP(idx)    ((uint8_t) (idx))
#define DET_OP(idx)      ((uint8_t) (32U + (uint8_t) (idx)))
#define COUNTER_OP(idx)  ((uint8_t) (64U + (uint8_t) (idx)))

/* --- tests -------------------------------------------------------------- */

void test_literal_equal_true(void)
{
  reset_operands();
  TEST_ASSERT_TRUE(eval_single_op(OPR_EQUAL, LIT(5), LIT(5)));
}

void test_literal_equal_false(void)
{
  reset_operands();
  TEST_ASSERT_FALSE(eval_single_op(OPR_EQUAL, LIT(5), LIT(6)));
}

void test_literal_greater(void)
{
  reset_operands();
  TEST_ASSERT_TRUE(eval_single_op(OPR_GREATER, LIT(10), LIT(5)));
  TEST_ASSERT_FALSE(eval_single_op(OPR_GREATER, LIT(5), LIT(10)));
}

void test_literal_less_equal(void)
{
  reset_operands();
  TEST_ASSERT_TRUE(eval_single_op(OPR_LESS_EQUAL, LIT(5), LIT(5)));
  TEST_ASSERT_TRUE(eval_single_op(OPR_LESS_EQUAL, LIT(4), LIT(5)));
  TEST_ASSERT_FALSE(eval_single_op(OPR_LESS_EQUAL, LIT(6), LIT(5)));
}

void Test_Phase_elapsed_operand(void)
{
  reset_operands();
  phases[0].elapsedSeconds = 15U;

  /* phase[0].elapsed >= 10 → true */
  TEST_ASSERT_TRUE(eval_single_op(OPR_GREATER_EQUAL, PHASE_OP(0), LIT(10)));
  /* phase[0].elapsed >= 20 → false */
  TEST_ASSERT_FALSE(eval_single_op(OPR_GREATER_EQUAL, PHASE_OP(0), LIT(20)));
}

void Test_Detector_demand_operand(void)
{
  reset_operands();
  Detectors[1].demandCountInPeriod = 3U;

  /* Detector[1].demand > 0 → true */
  TEST_ASSERT_TRUE(eval_single_op(OPR_GREATER, DET_OP(1), LIT(0)));
  /* Detector[1].demand > 5 → false */
  TEST_ASSERT_FALSE(eval_single_op(OPR_GREATER, DET_OP(1), LIT(5)));
}

void test_counter_operand(void)
{
  reset_operands();
  counters[2].value = 42U;

  TEST_ASSERT_TRUE(eval_single_op(OPR_EQUAL, COUNTER_OP(2), LIT(42)));
  TEST_ASSERT_FALSE(eval_single_op(OPR_EQUAL, COUNTER_OP(2), LIT(0)));
}

void test_and_operator(void)
{
  reset_operands();
  /* 1 AND 1 → true */
  TEST_ASSERT_TRUE(eval_single_op(OPR_AND, LIT(1), LIT(1)));
  /* 1 AND 0 → false */
  TEST_ASSERT_FALSE(eval_single_op(OPR_AND, LIT(1), LIT(0)));
  /* 0 AND 0 → false */
  TEST_ASSERT_FALSE(eval_single_op(OPR_AND, LIT(0), LIT(0)));
}

void test_or_operator(void)
{
  reset_operands();
  TEST_ASSERT_TRUE(eval_single_op(OPR_OR, LIT(1), LIT(0)));
  TEST_ASSERT_TRUE(eval_single_op(OPR_OR, LIT(0), LIT(1)));
  TEST_ASSERT_FALSE(eval_single_op(OPR_OR, LIT(0), LIT(0)));
}

void test_div_by_zero_returns_zero(void)
{
  reset_operands();
  /* 10 / 0 should not crash and should return 0 */
  TEST_ASSERT_FALSE(eval_single_op(OPR_EQUAL, LIT(0), LIT(127)));
  /* More importantly: just call divide-by-zero path */
  OperationConfig_t ops[1];

  ops[0].op = OPR_DIV;
  ops[0].operandA = LIT(10);
  ops[0].operandB = LIT(0);
  RuleConfig_t rule;

  rule.operationStart = 0U;
  /* Should return 0 (false) without crash */
  bool result = TransitionRule_Evaluate(&rule,
                                        ops,
                                        1U,
                                        phases,
                                        Detectors,
                                        counters);

  TEST_ASSERT_FALSE(result);
}

void test_empty_rule_returns_false(void)
{
  reset_operands();
  OperationConfig_t ops[1];

  ops[0].op = OPR_NONE;
  RuleConfig_t rule;

  rule.operationStart = 0U;
  TEST_ASSERT_FALSE(TransitionRule_Evaluate(&rule,
                                            ops,
                                            1U,
                                            phases,
                                            Detectors,
                                            counters));
}

void test_select_best_picks_highest_priority(void)
{
  reset_operands();
  phases[0].elapsedSeconds = 20U;

  /* Two transitions both from CTRL_STATE_PHASE, both with phase[0] >= 10 */
  TransitionConfig_t transitions[2];

  transitions[0].fromState = (uint8_t) CTRL_STATE_PHASE;
  transitions[0].ruleIndex = 0U;
  transitions[0].priority = 5U;
  transitions[0].param1 = 0U;
  transitions[0].param2 = 0U;

  transitions[1].fromState = (uint8_t) CTRL_STATE_PHASE;
  transitions[1].ruleIndex = 0U;
  transitions[1].priority = 10U;       /* Higher — should win */
  transitions[1].param1 = 1U;
  transitions[1].param2 = 0U;

  OperationConfig_t ops[2];

  ops[0].op = OPR_GREATER_EQUAL;
  ops[0].operandA = PHASE_OP(0);
  ops[0].operandB = LIT(10);
  ops[1].op = OPR_NONE;

  RuleConfig_t rules[1];

  rules[0].operationStart = 0U;

  TransitionConfig_t out;
  bool fired = TransitionRule_SelectBest(
    CTRL_STATE_PHASE,
    transitions, 2U,
    rules, ops, 2U,
    phases, Detectors, counters,
    &out);

  TEST_ASSERT_TRUE(fired);
  TEST_ASSERT_EQUAL_UINT8(10U, out.priority);
  TEST_ASSERT_EQUAL_UINT8(1U, out.param1);
} /* test_select_best_picks_highest_priority */

void test_select_best_skips_wrong_from_state(void)
{
  reset_operands();
  phases[0].elapsedSeconds = 20U;

  TransitionConfig_t transitions[1];

  transitions[0].fromState = (uint8_t) CTRL_STATE_ALL_RED;   /* Not PHASE */
  transitions[0].ruleIndex = 0U;
  transitions[0].priority = 10U;
  transitions[0].param1 = 0U;
  transitions[0].param2 = 0U;

  OperationConfig_t ops[2];

  ops[0].op = OPR_GREATER_EQUAL;
  ops[0].operandA = PHASE_OP(0);
  ops[0].operandB = LIT(10);
  ops[1].op = OPR_NONE;

  RuleConfig_t rules[1];

  rules[0].operationStart = 0U;

  TransitionConfig_t out;
  bool fired = TransitionRule_SelectBest(
    CTRL_STATE_PHASE,              /* current state is PHASE */
    transitions, 1U,
    rules, ops, 2U,
    phases, Detectors, counters,
    &out);

  TEST_ASSERT_FALSE(fired);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_literal_equal_true);
  RUN_TEST(test_literal_equal_false);
  RUN_TEST(test_literal_greater);
  RUN_TEST(test_literal_less_equal);
  RUN_TEST(Test_Phase_elapsed_operand);
  RUN_TEST(Test_Detector_demand_operand);
  RUN_TEST(test_counter_operand);
  RUN_TEST(test_and_operator);
  RUN_TEST(test_or_operator);
  RUN_TEST(test_div_by_zero_returns_zero);
  RUN_TEST(test_empty_rule_returns_false);
  RUN_TEST(test_select_best_picks_highest_priority);
  RUN_TEST(test_select_best_skips_wrong_from_state);

  return UNITY_END();
}
