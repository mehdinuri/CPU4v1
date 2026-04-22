/*
 * Tests/Unit/Test_VoltageRMS.c
 *
 * Unit tests for App/Domain/VoltageRMS.c
 */
#include "unity.h"
#include "Domain/VoltageRMS.h"

#include <stddef.h>

void setUp(void)
{
}

void tearDown(void)
{
}

/* ---------------------------------------------------------------------------
 * VoltageRMS_ComputeAC
 * ---------------------------------------------------------------------------*/

void test_compute_ac_dc_only_returns_zero(void)
{
  /* Constant-valued window has no AC component → RMS should be 0. */
  uint16_t samples[8] = { 2048U, 2048U, 2048U, 2048U,
                          2048U, 2048U, 2048U, 2048U };

  float rms = VoltageRMS_ComputeAC(samples, 8U, 3.3f, 4095.0f, 1.0f);

  TEST_ASSERT_FLOAT_WITHIN(1e-6f, 0.0f, rms);
}

void test_compute_ac_square_wave_raw_rms(void)
{
  /* Mean = 2048, deviations = ±100 → raw RMS = 100.
   * Scaled by (vref / adc_max) * scaling with all = 1.0 → result = 100. */
  uint16_t samples[4] = { 1948U, 2148U, 1948U, 2148U };

  float rms = VoltageRMS_ComputeAC(samples, 4U, 4095.0f, 4095.0f, 1.0f);

  TEST_ASSERT_FLOAT_WITHIN(1e-3f, 100.0f, rms);
}

void test_compute_ac_square_wave_with_scaling(void)
{
  /* Same ±100 raw RMS; apply PSM grid chain.
   * expected = 100 * 3.3 / 4095 * 276.89 ≈ 22.319 */
  uint16_t samples[4] = { 1948U, 2148U, 1948U, 2148U };

  float rms = VoltageRMS_ComputeAC(samples, 4U, 3.3f, 4095.0f, 276.89f);

  TEST_ASSERT_FLOAT_WITHIN(0.01f, 22.319f, rms);
}

void test_compute_ac_null_samples_returns_zero(void)
{
  float rms = VoltageRMS_ComputeAC(NULL, 4U, 3.3f, 4095.0f, 1.0f);

  TEST_ASSERT_FLOAT_WITHIN(1e-6f, 0.0f, rms);
}

void test_compute_ac_zero_count_returns_zero(void)
{
  uint16_t samples[1] = { 1000U };

  float rms = VoltageRMS_ComputeAC(samples, 0U, 3.3f, 4095.0f, 1.0f);

  TEST_ASSERT_FLOAT_WITHIN(1e-6f, 0.0f, rms);
}

void test_compute_ac_zero_adc_max_returns_zero(void)
{
  uint16_t samples[4] = { 100U, 200U, 100U, 200U };

  float rms = VoltageRMS_ComputeAC(samples, 4U, 3.3f, 0.0f, 1.0f);

  TEST_ASSERT_FLOAT_WITHIN(1e-6f, 0.0f, rms);
}

void test_compute_ac_single_sample_returns_zero(void)
{
  /* One sample can have no AC content (mean = sample). */
  uint16_t samples[1] = { 1234U };

  float rms = VoltageRMS_ComputeAC(samples, 1U, 3.3f, 4095.0f, 1.0f);

  TEST_ASSERT_FLOAT_WITHIN(1e-6f, 0.0f, rms);
}

/* ---------------------------------------------------------------------------
 * VoltageRMS_ConvertDC
 * ---------------------------------------------------------------------------*/

void test_convert_dc_zero_sample_returns_zero(void)
{
  float v = VoltageRMS_ConvertDC(0U, 3.3f, 4095.0f, 10.0f);

  TEST_ASSERT_FLOAT_WITHIN(1e-6f, 0.0f, v);
}

void test_convert_dc_scaling_applied(void)
{
  /* 2048 * 3.3/4095 * 2.05 ≈ 3.384 */
  float v = VoltageRMS_ConvertDC(2048U, 3.3f, 4095.0f, 2.05f);

  TEST_ASSERT_FLOAT_WITHIN(0.01f, 3.384f, v);
}

void test_convert_dc_regulator_vin(void)
{
  /* PSM Vin chain: scaling 11.29, example sample 2048 → ~18.63 V */
  float v = VoltageRMS_ConvertDC(2048U, 3.38f, 4095.0f, 11.29f);

  TEST_ASSERT_FLOAT_WITHIN(0.01f, 19.081f, v);
}

void test_convert_dc_zero_adc_max_returns_zero(void)
{
  float v = VoltageRMS_ConvertDC(1000U, 3.3f, 0.0f, 10.0f);

  TEST_ASSERT_FLOAT_WITHIN(1e-6f, 0.0f, v);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_compute_ac_dc_only_returns_zero);
  RUN_TEST(test_compute_ac_square_wave_raw_rms);
  RUN_TEST(test_compute_ac_square_wave_with_scaling);
  RUN_TEST(test_compute_ac_null_samples_returns_zero);
  RUN_TEST(test_compute_ac_zero_count_returns_zero);
  RUN_TEST(test_compute_ac_zero_adc_max_returns_zero);
  RUN_TEST(test_compute_ac_single_sample_returns_zero);
  RUN_TEST(test_convert_dc_zero_sample_returns_zero);
  RUN_TEST(test_convert_dc_scaling_applied);
  RUN_TEST(test_convert_dc_regulator_vin);
  RUN_TEST(test_convert_dc_zero_adc_max_returns_zero);

  return UNITY_END();
}
