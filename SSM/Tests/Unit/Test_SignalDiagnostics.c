/*
 * Tests/Unit/Test_SignalDiagnostics.c
 */

#include <string.h>
#include "unity.h"
#include "Domain/SignalDiagnostics.h"

static SignalDiagnosticsState_t state;
static SignalOutputImage_t obs;
static CurrentMeasurementSnapshot_t snap;

void setUp(void)
{
  SignalDiagnostics_Reset(&state);
  memset(&obs, 0, sizeof(obs));
  memset(&snap, 0, sizeof(snap));
}

void tearDown(void)
{
}

void test_learning_phase_ignores_dark_group(void)
{
  /* Group 0 is dark (obs.channels[0..2] == 0) */
  snap.currentsMa[0] = 500;

  SignalDiagnostics_Step(&state, &obs, &snap);

  /* Baseline should still be 0 */
  TEST_ASSERT_EQUAL_UINT16(0, state.baselineCurrentMa[0][0]);
}

void test_learning_phase_ignores_low_current(void)
{
  /* Group 0 has Red ON (bit 0) */
  obs.channels[0] = 1;
  snap.currentsMa[0] = 10; /* below SIGNAL_DIAGNOSTICS_LEARN_MIN_MA */

  SignalDiagnostics_Step(&state, &obs, &snap);

  TEST_ASSERT_EQUAL_UINT16(0, state.baselineCurrentMa[0][1]);
}

void test_learning_phase_records_baseline(void)
{
  /* Group 0 has Red ON (bit 0) */
  obs.channels[0] = 1;
  snap.currentsMa[0] = 200;

  SignalDiagnostics_Step(&state, &obs, &snap);

  TEST_ASSERT_EQUAL_UINT16(200, state.baselineCurrentMa[0][1]);
}

void test_monitoring_phase_passes_with_sufficient_current(void)
{
  /* Learn baseline: 200mA for Red */
  obs.channels[0] = 1;
  snap.currentsMa[0] = 200;
  SignalDiagnostics_Step(&state, &obs, &snap);

  /* Next cycle: 180mA (90% of baseline) */
  snap.currentsMa[0] = 180;
  uint8_t fault = SignalDiagnostics_Step(&state, &obs, &snap);

  TEST_ASSERT_EQUAL_UINT8(0, fault);
  TEST_ASSERT_EQUAL_UINT8(0, state.lampOutFault[0]);
}

void test_monitoring_phase_fails_with_undercurrent(void)
{
  /* Learn baseline: 200mA for Red */
  obs.channels[0] = 1;
  snap.currentsMa[0] = 200;
  SignalDiagnostics_Step(&state, &obs, &snap);

  /* Next cycle: 80mA (40% of baseline, below 50% threshold) */
  snap.currentsMa[0] = 80;
  uint8_t fault = SignalDiagnostics_Step(&state, &obs, &snap);

  TEST_ASSERT_EQUAL_UINT8(1, fault);
  TEST_ASSERT_EQUAL_UINT8(1, state.lampOutFault[0]);
}

void test_multiple_combinations_learned_independently(void)
{
  /* Group 0: Learn Red (200mA) */
  obs.channels[0] = 1;
  snap.currentsMa[0] = 200;
  SignalDiagnostics_Step(&state, &obs, &snap);

  /* Group 0: Learn Yellow (300mA) */
  memset(&obs, 0, sizeof(obs));
  obs.channels[1] = 1;
  snap.currentsMa[0] = 300;
  SignalDiagnostics_Step(&state, &obs, &snap);

  TEST_ASSERT_EQUAL_UINT16(200, state.baselineCurrentMa[0][1]);
  TEST_ASSERT_EQUAL_UINT16(300, state.baselineCurrentMa[0][2]);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_learning_phase_ignores_dark_group);
  RUN_TEST(test_learning_phase_ignores_low_current);
  RUN_TEST(test_learning_phase_records_baseline);
  RUN_TEST(test_monitoring_phase_passes_with_sufficient_current);
  RUN_TEST(test_monitoring_phase_fails_with_undercurrent);
  RUN_TEST(test_multiple_combinations_learned_independently);

  return UNITY_END();
}
