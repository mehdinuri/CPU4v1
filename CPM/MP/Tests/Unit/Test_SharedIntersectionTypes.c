/* Tests/Unit/Test_SharedIntersectionTypes.c
 *
 * Smoke-tests that MP Host-Test can compile and link against the shared
 * intersection types vendored at Libs/Intersection/ and Libs/Ports/
 * (copied from CP, to be unified with CP later).
 *
 * This deliberately does not test semantics — IntersectionConfig.c is
 * CP's source of truth and has its own tests in CP. The purpose here
 * is just to guarantee the include path is wired and the types compile
 * under MP's host toolchain flags.
 */

#include <stddef.h>
#include <stdint.h>

#include "unity.h"

#include "Intersection/IntersectionConfig.h"
#include "Ports/IUnitClockPort.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_intersection_limits_are_sane(void)
{
  TEST_ASSERT_EQUAL_UINT8(8U, INTERSECTION_PHASE_COUNT_MAX);
  TEST_ASSERT_EQUAL_UINT8(2U, INTERSECTION_RING_COUNT_MAX);
  TEST_ASSERT_EQUAL_UINT8(2U, INTERSECTION_BARRIER_COUNT_MAX);
  TEST_ASSERT_EQUAL_UINT8(32U, INTERSECTION_CHANNEL_COUNT_MAX);
  TEST_ASSERT_EQUAL_UINT8(32U, INTERSECTION_OVERLAP_COUNT_MAX);
}

void test_channel_control_types_have_stable_values(void)
{
  TEST_ASSERT_EQUAL_INT(1, INTERSECTION_CHANNEL_CONTROL_TYPE_OTHER);
  TEST_ASSERT_EQUAL_INT(2, INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_VEHICLE);
  TEST_ASSERT_EQUAL_INT(3, INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_PEDESTRIAN);
  TEST_ASSERT_EQUAL_INT(4, INTERSECTION_CHANNEL_CONTROL_TYPE_OVERLAP);
}

void test_unit_clock_sources_have_stable_values(void)
{
  TEST_ASSERT_EQUAL_INT(2, UNIT_CLOCK_SOURCE_LINE_SYNC);
  TEST_ASSERT_EQUAL_INT(5, UNIT_CLOCK_SOURCE_GNSS);
}

void test_can_allocate_an_intersection_config(void)
{
  IntersectionConfig_t config;

  config.phaseCount = 4U;
  config.ringCount = 2U;
  config.barrierCount = 1U;
  TEST_ASSERT_EQUAL_UINT8(4U, config.phaseCount);
  TEST_ASSERT_EQUAL_UINT8(2U, config.ringCount);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_intersection_limits_are_sane);
  RUN_TEST(test_channel_control_types_have_stable_values);
  RUN_TEST(test_unit_clock_sources_have_stable_values);
  RUN_TEST(test_can_allocate_an_intersection_config);

  return UNITY_END();
}
