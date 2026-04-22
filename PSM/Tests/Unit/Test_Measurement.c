/*
 * Tests/Unit/Test_Measurement.c
 *
 * Unit tests for App/Domain/Measurement.c
 * Tests cover voltage scaling, range checks, and offset calibration.
 */
#include "unity.h"
#include "Domain/Measurement.h"

/* Unity boilerplate */
void setUp(void)
{
}

void tearDown(void)
{
}

/* ---------------------------------------------------------------------------
 * Helpers
 * ---------------------------------------------------------------------------*/
static MeasurementOffset_t make_offset(uint8_t op, uint8_t val)
{
  MeasurementOffset_t off;

  off.operation = op;
  off.value = val;

  return off;
}

/* ---------------------------------------------------------------------------
 * Measurement_ScaleNetVoltage
 * ---------------------------------------------------------------------------*/

void test_scale_net_voltage_no_offset(void)
{
  MeasurementOffset_t off = make_offset(OFFSET_OPERATION_NONE, 0U);

  /* 220.0 V / 0.73029 ≈ 301 */
  uint16_t result = Measurement_ScaleNetVoltage(220.0f, &off);

  TEST_ASSERT_EQUAL_UINT16(301U, result);
}

void test_scale_net_voltage_sum_offset(void)
{
  MeasurementOffset_t off = make_offset(OFFSET_OPERATION_SUM, 10U);

  /* (220.0 + 10) / 0.73029 = 314.95… → truncates to 314 */
  uint16_t result = Measurement_ScaleNetVoltage(220.0f, &off);

  TEST_ASSERT_EQUAL_UINT16(314U, result);
}

void test_scale_net_voltage_subtract_offset(void)
{
  MeasurementOffset_t off = make_offset(OFFSET_OPERATION_SUBTRACT, 10U);

  /* (220.0 - 10) / 0.73029 ≈ 287 */
  uint16_t result = Measurement_ScaleNetVoltage(220.0f, &off);

  TEST_ASSERT_EQUAL_UINT16(287U, result);
}

void test_scale_net_voltage_unknown_operation_treated_as_none(void)
{
  MeasurementOffset_t off = make_offset(0xFFU, 5U);

  /* Unknown op → offset ignored → 220.0 / 0.73029 ≈ 301 */
  uint16_t result = Measurement_ScaleNetVoltage(220.0f, &off);

  TEST_ASSERT_EQUAL_UINT16(301U, result);
}

/* ---------------------------------------------------------------------------
 * SafeFloatToU16 — clamp behaviour on out-of-range inputs (fix 1.2)
 * ---------------------------------------------------------------------------*/

void test_scale_net_voltage_negative_result_clamps_to_zero(void)
{
  /* 50 V − 100 offset = −50 V; would be UB without clamp → must return 0 */
  MeasurementOffset_t off = make_offset(OFFSET_OPERATION_SUBTRACT, 100U);
  uint16_t result = Measurement_ScaleNetVoltage(50.0f, &off);

  TEST_ASSERT_EQUAL_UINT16(0U, result);
}

void test_scale_reg_voltage_large_value_clamps_to_max(void)
{
  /* 10000 V * 10.0 = 100000 → exceeds uint16_t max → clamp to 65535 */
  uint16_t result = Measurement_ScaleRegVoltage(10000.0f,
                                                MEASUREMENT_CP_REG_VIN_COEFFICIENT);

  TEST_ASSERT_EQUAL_UINT16(65535U, result);
}

/* ---------------------------------------------------------------------------
 * Measurement_ScaleRegVoltage
 * ---------------------------------------------------------------------------*/

void test_scale_reg_voltage_vin(void)
{
  /* 24.0 V * 10.0 = 240 */
  uint16_t result = Measurement_ScaleRegVoltage(24.0f,
                                                MEASUREMENT_CP_REG_VIN_COEFFICIENT);

  TEST_ASSERT_EQUAL_UINT16(240U, result);
}

void test_scale_reg_voltage_vout(void)
{
  /* 5.7 V * 10.0 = 57 */
  uint16_t result = Measurement_ScaleRegVoltage(5.7f,
                                                MEASUREMENT_CP_REG_VOUT_COEFFICIENT);

  TEST_ASSERT_EQUAL_UINT16(57U, result);
}

/* ---------------------------------------------------------------------------
 * Measurement_DCIsOK
 * ---------------------------------------------------------------------------*/

void test_dc_is_ok_valid_range(void)
{
  /* Vin = 24.0 V → 240, Vout = 5.7 V → 57 — both in range */
  TEST_ASSERT_EQUAL_UINT8(1U, Measurement_DCIsOK(240U, 57U));
}

void test_dc_is_ok_vin_too_low(void)
{
  /* Vin = 21.9 V → 219 < 220 */
  TEST_ASSERT_EQUAL_UINT8(0U, Measurement_DCIsOK(219U, 57U));
}

void test_dc_is_ok_vin_too_high(void)
{
  /* Vin = 26.1 V → 261 > 260 */
  TEST_ASSERT_EQUAL_UINT8(0U, Measurement_DCIsOK(261U, 57U));
}

void test_dc_is_ok_vout_too_low(void)
{
  /* Vout = 4.6 V → 46 < 47 */
  TEST_ASSERT_EQUAL_UINT8(0U, Measurement_DCIsOK(240U, 46U));
}

void test_dc_is_ok_vout_too_high(void)
{
  /* Vout = 6.8 V → 68 > 67 */
  TEST_ASSERT_EQUAL_UINT8(0U, Measurement_DCIsOK(240U, 68U));
}

/* ---------------------------------------------------------------------------
 * Measurement_ACIsOK
 * ---------------------------------------------------------------------------*/

void test_ac_is_ok_valid_range(void)
{
  /* 215 V / 0.73029 ≈ 294 — inside [226, 363] */
  TEST_ASSERT_EQUAL_UINT8(1U, Measurement_ACIsOK(294U));
}

void test_ac_is_ok_too_low(void)
{
  /* below 226 */
  TEST_ASSERT_EQUAL_UINT8(0U, Measurement_ACIsOK(200U));
}

void test_ac_is_ok_too_high(void)
{
  /* above 363 */
  TEST_ASSERT_EQUAL_UINT8(0U, Measurement_ACIsOK(400U));
}

void test_ac_is_ok_boundary_min(void)
{
  TEST_ASSERT_EQUAL_UINT8(1U, Measurement_ACIsOK(226U));
}

void test_ac_is_ok_boundary_max(void)
{
  TEST_ASSERT_EQUAL_UINT8(1U, Measurement_ACIsOK(363U));
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_scale_net_voltage_no_offset);
  RUN_TEST(test_scale_net_voltage_sum_offset);
  RUN_TEST(test_scale_net_voltage_subtract_offset);
  RUN_TEST(test_scale_net_voltage_unknown_operation_treated_as_none);
  RUN_TEST(test_scale_net_voltage_negative_result_clamps_to_zero);
  RUN_TEST(test_scale_reg_voltage_large_value_clamps_to_max);
  RUN_TEST(test_scale_reg_voltage_vin);
  RUN_TEST(test_scale_reg_voltage_vout);
  RUN_TEST(test_dc_is_ok_valid_range);
  RUN_TEST(test_dc_is_ok_vin_too_low);
  RUN_TEST(test_dc_is_ok_vin_too_high);
  RUN_TEST(test_dc_is_ok_vout_too_low);
  RUN_TEST(test_dc_is_ok_vout_too_high);
  RUN_TEST(test_ac_is_ok_valid_range);
  RUN_TEST(test_ac_is_ok_too_low);
  RUN_TEST(test_ac_is_ok_too_high);
  RUN_TEST(test_ac_is_ok_boundary_min);
  RUN_TEST(test_ac_is_ok_boundary_max);

  return UNITY_END();
}
