/*
 * Tests/Unit/Test_FrequencyCaptureFSM.c
 *
 * Unit tests for App/Domain/FrequencyCaptureFSM.c
 */
#include "unity.h"
#include "Domain/FrequencyCaptureFSM.h"

void setUp(void)
{
}

void tearDown(void)
{
}

/* ---------------------------------------------------------------------------
 * Test fixtures
 * ---------------------------------------------------------------------------*/

/* Matches tim.c constants except counterMax — scaled down for readable
 * wraparound arithmetic in tests. */
static const FrequencyCaptureFSMConfig_t cfg = {
  .clockFreqHz = 100000U,
  .counterMax = 9999U,
  .captureTimeoutMs = 100U,
  .targetFreqHz = 50U,
  .freqToleranceHz = 5U,
  .badReadingsBeforeSleep = 10U
};

/* ---------------------------------------------------------------------------
 * Init
 * ---------------------------------------------------------------------------*/

void test_init_leaves_unprimed(void)
{
  FrequencyCaptureFSMState_t s;

  FrequencyCaptureFSM_Init(&s, 1000U);

  TEST_ASSERT_EQUAL_UINT8(0U, s.primed);
  TEST_ASSERT_EQUAL_UINT8(0U, s.measuredFreqHz);
  TEST_ASSERT_EQUAL_UINT16(0U, s.badReadingsCount);
  TEST_ASSERT_EQUAL_UINT32(1000U, s.lastCaptureTimeMs);
}

/* ---------------------------------------------------------------------------
 * OnEdge
 * ---------------------------------------------------------------------------*/

void test_first_edge_primes_and_returns_zero(void)
{
  FrequencyCaptureFSMState_t s;

  FrequencyCaptureFSM_Init(&s, 0U);

  uint8_t published = FrequencyCaptureFSM_OnEdge(&s, &cfg, 5000U, 10U);

  TEST_ASSERT_EQUAL_UINT8(0U, published);
  TEST_ASSERT_EQUAL_UINT8(1U, s.primed);
  TEST_ASSERT_EQUAL_UINT32(5000U, s.prevCaptureValue);
  TEST_ASSERT_EQUAL_UINT32(10U, s.lastCaptureTimeMs);
}

void test_second_edge_publishes_50hz(void)
{
  /* clock=100000, diff=2000 → freq=50 */
  FrequencyCaptureFSMState_t s;

  FrequencyCaptureFSM_Init(&s, 0U);

  (void) FrequencyCaptureFSM_OnEdge(&s, &cfg, 1000U, 0U);
  uint8_t published = FrequencyCaptureFSM_OnEdge(&s, &cfg, 3000U, 20U);

  TEST_ASSERT_EQUAL_UINT8(1U, published);
  TEST_ASSERT_EQUAL_UINT8(50U, s.measuredFreqHz);
}

void test_wraparound_handled(void)
{
  /* prev=9000, curr=1000 across counter_max=9999.
   * diff = (9999 - 9000) + 1000 + 1 = 2000 → freq=50 */
  FrequencyCaptureFSMState_t s;

  FrequencyCaptureFSM_Init(&s, 0U);

  (void) FrequencyCaptureFSM_OnEdge(&s, &cfg, 9000U, 0U);
  uint8_t published = FrequencyCaptureFSM_OnEdge(&s, &cfg, 1000U, 20U);

  TEST_ASSERT_EQUAL_UINT8(1U, published);
  TEST_ASSERT_EQUAL_UINT8(50U, s.measuredFreqHz);
}

void test_same_capture_value_twice_forces_reprime(void)
{
  FrequencyCaptureFSMState_t s;

  FrequencyCaptureFSM_Init(&s, 0U);

  (void) FrequencyCaptureFSM_OnEdge(&s, &cfg, 5000U, 0U);
  uint8_t published = FrequencyCaptureFSM_OnEdge(&s, &cfg, 5000U, 10U);

  TEST_ASSERT_EQUAL_UINT8(1U, published);
  TEST_ASSERT_EQUAL_UINT8(0U, s.measuredFreqHz);
  TEST_ASSERT_EQUAL_UINT8(0U, s.primed);
}

void test_third_edge_uses_prev_as_baseline(void)
{
  /* After a successful pair, next edge should compute diff from the most
   * recent capture, not the original one. */
  FrequencyCaptureFSMState_t s;

  FrequencyCaptureFSM_Init(&s, 0U);

  (void) FrequencyCaptureFSM_OnEdge(&s, &cfg, 1000U, 0U);
  (void) FrequencyCaptureFSM_OnEdge(&s, &cfg, 3000U, 20U);
  uint8_t published = FrequencyCaptureFSM_OnEdge(&s, &cfg, 4000U, 30U);

  /* third - second = 1000 → freq = 100 */
  TEST_ASSERT_EQUAL_UINT8(1U, published);
  TEST_ASSERT_EQUAL_UINT8(100U, s.measuredFreqHz);
}

/* ---------------------------------------------------------------------------
 * Evaluate
 * ---------------------------------------------------------------------------*/

void test_evaluate_unprimed_is_treated_as_timeout(void)
{
  FrequencyCaptureFSMState_t s;

  FrequencyCaptureFSM_Init(&s, 0U);

  FrequencyCaptureFSMVerdict_e v = FrequencyCaptureFSM_Evaluate(&s, &cfg, 1U);

  TEST_ASSERT_EQUAL_INT(FREQ_FSM_VERDICT_BAD, v);
  TEST_ASSERT_EQUAL_UINT16(1U, s.badReadingsCount);
}

void test_evaluate_in_tolerance_clears_strikes(void)
{
  FrequencyCaptureFSMState_t s;

  FrequencyCaptureFSM_Init(&s, 0U);
  (void) FrequencyCaptureFSM_OnEdge(&s, &cfg, 1000U, 0U);
  (void) FrequencyCaptureFSM_OnEdge(&s, &cfg, 3000U, 20U);
  s.badReadingsCount = 5U;

  FrequencyCaptureFSMVerdict_e v = FrequencyCaptureFSM_Evaluate(&s, &cfg, 30U);

  TEST_ASSERT_EQUAL_INT(FREQ_FSM_VERDICT_OK, v);
  TEST_ASSERT_EQUAL_UINT16(0U, s.badReadingsCount);
}

void test_evaluate_out_of_tolerance_increments_strikes(void)
{
  FrequencyCaptureFSMState_t s;

  FrequencyCaptureFSM_Init(&s, 0U);
  (void) FrequencyCaptureFSM_OnEdge(&s, &cfg, 1000U, 0U);
  /* diff=5000 → freq=20 (out of 45–55 band) */
  (void) FrequencyCaptureFSM_OnEdge(&s, &cfg, 6000U, 50U);

  FrequencyCaptureFSMVerdict_e v = FrequencyCaptureFSM_Evaluate(&s, &cfg, 60U);

  TEST_ASSERT_EQUAL_INT(FREQ_FSM_VERDICT_BAD, v);
  TEST_ASSERT_EQUAL_UINT16(1U, s.badReadingsCount);
}

void test_evaluate_threshold_reached_returns_flash(void)
{
  FrequencyCaptureFSMState_t s;

  FrequencyCaptureFSM_Init(&s, 0U);
  (void) FrequencyCaptureFSM_OnEdge(&s, &cfg, 1000U, 0U);
  (void) FrequencyCaptureFSM_OnEdge(&s, &cfg, 6000U, 50U); /* freq 20, bad */
  s.badReadingsCount = 9U; /* one away from threshold */

  FrequencyCaptureFSMVerdict_e v = FrequencyCaptureFSM_Evaluate(&s, &cfg, 60U);

  TEST_ASSERT_EQUAL_INT(FREQ_FSM_VERDICT_ENTER_FLASH, v);
  TEST_ASSERT_EQUAL_UINT16(10U, s.badReadingsCount);
}

void test_evaluate_timeout_resets_freq_and_prime(void)
{
  FrequencyCaptureFSMState_t s;

  FrequencyCaptureFSM_Init(&s, 0U);
  (void) FrequencyCaptureFSM_OnEdge(&s, &cfg, 1000U, 0U);
  (void) FrequencyCaptureFSM_OnEdge(&s, &cfg, 3000U, 20U);
  /* Freq was 50 (OK). Now jump far enough past timeout window. */
  FrequencyCaptureFSMVerdict_e v = FrequencyCaptureFSM_Evaluate(&s, &cfg, 200U);

  TEST_ASSERT_EQUAL_INT(FREQ_FSM_VERDICT_BAD, v);
  TEST_ASSERT_EQUAL_UINT8(0U, s.measuredFreqHz);
  TEST_ASSERT_EQUAL_UINT8(0U, s.primed);
}

void test_evaluate_boundary_inclusive_low(void)
{
  /* 45 Hz is exactly target − tolerance → should be OK */
  FrequencyCaptureFSMState_t s;

  FrequencyCaptureFSM_Init(&s, 0U);
  s.measuredFreqHz = 45U;
  s.primed = 1U;
  s.lastCaptureTimeMs = 50U;

  FrequencyCaptureFSMVerdict_e v = FrequencyCaptureFSM_Evaluate(&s, &cfg, 60U);

  TEST_ASSERT_EQUAL_INT(FREQ_FSM_VERDICT_OK, v);
}

void test_evaluate_boundary_inclusive_high(void)
{
  /* 55 Hz is exactly target + tolerance → should be OK */
  FrequencyCaptureFSMState_t s;

  FrequencyCaptureFSM_Init(&s, 0U);
  s.measuredFreqHz = 55U;
  s.primed = 1U;
  s.lastCaptureTimeMs = 50U;

  FrequencyCaptureFSMVerdict_e v = FrequencyCaptureFSM_Evaluate(&s, &cfg, 60U);

  TEST_ASSERT_EQUAL_INT(FREQ_FSM_VERDICT_OK, v);
}

void test_evaluate_tolerance_exceeds_target_does_not_underflow(void)
{
  /* Regression: if tolerance > target, the old uint8 math wrapped so
   * lo became ~251 and every reading looked out-of-band.  The fixed
   * arithmetic clamps lo at 0 and treats any 0..hi reading as OK. */
  const FrequencyCaptureFSMConfig_t cfgWide = {
    .clockFreqHz = 100000U,
    .counterMax = 9999U,
    .captureTimeoutMs = 100U,
    .targetFreqHz = 5U,
    .freqToleranceHz = 10U,
    .badReadingsBeforeSleep = 10U
  };

  FrequencyCaptureFSMState_t s;

  FrequencyCaptureFSM_Init(&s, 0U);
  s.measuredFreqHz = 3U;      /* within the [0, 15] band */
  s.primed = 1U;
  s.lastCaptureTimeMs = 50U;

  FrequencyCaptureFSMVerdict_e v = FrequencyCaptureFSM_Evaluate(&s,
                                                                &cfgWide,
                                                                60U);

  TEST_ASSERT_EQUAL_INT(FREQ_FSM_VERDICT_OK, v);
  TEST_ASSERT_EQUAL_UINT16(0U, s.badReadingsCount);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_init_leaves_unprimed);
  RUN_TEST(test_first_edge_primes_and_returns_zero);
  RUN_TEST(test_second_edge_publishes_50hz);
  RUN_TEST(test_wraparound_handled);
  RUN_TEST(test_same_capture_value_twice_forces_reprime);
  RUN_TEST(test_third_edge_uses_prev_as_baseline);
  RUN_TEST(test_evaluate_unprimed_is_treated_as_timeout);
  RUN_TEST(test_evaluate_in_tolerance_clears_strikes);
  RUN_TEST(test_evaluate_out_of_tolerance_increments_strikes);
  RUN_TEST(test_evaluate_threshold_reached_returns_flash);
  RUN_TEST(test_evaluate_timeout_resets_freq_and_prime);
  RUN_TEST(test_evaluate_boundary_inclusive_low);
  RUN_TEST(test_evaluate_boundary_inclusive_high);
  RUN_TEST(test_evaluate_tolerance_exceeds_target_does_not_underflow);

  return UNITY_END();
}
