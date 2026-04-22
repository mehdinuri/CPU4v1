/* Tests/Unit/Test_MockSafetyRelayAdapter.c
 *
 * Verifies the hexagonal port+mock pattern end-to-end: domain code
 * talks only to ISafetyRelayPort_t, the mock adapter records state,
 * and a test can assert on it. Proves the MP/App/ layout compiles and
 * links against Unity on the host target.
 */

#include "unity.h"

#include "Adapters/Mock/MockSafetyRelayAdapter.h"
#include "Ports/ISafetyRelayPort.h"

static MockSafetyRelayAdapterCtx_t g_ctx;
static ISafetyRelayPort_t g_port;

void setUp(void)
{
  MockSafetyRelayAdapterInit(&g_ctx);
  g_port = MockSafetyRelayAdapterCreatePort(&g_ctx);
}

void tearDown(void)
{
}

void test_init_sets_relay_open(void)
{
  SafetyRelayState_t commanded;
  SafetyRelayState_t actual;

  TEST_ASSERT_EQUAL_UINT8(1U,
                          SafetyRelayGetCommandedState(&g_port, &commanded));
  TEST_ASSERT_EQUAL_UINT8(1U,
                          SafetyRelayGetActualState(&g_port, &actual));
  TEST_ASSERT_EQUAL(SAFETY_RELAY_STATE_OPEN, commanded);
  TEST_ASSERT_EQUAL(SAFETY_RELAY_STATE_OPEN, actual);
  TEST_ASSERT_EQUAL_UINT8(1U, g_ctx.lastRelayDrive);
  TEST_ASSERT_EQUAL_UINT8(1U, g_ctx.lastTriacDrive);
  TEST_ASSERT_EQUAL_UINT32(0U, g_ctx.transitionCount);
}

void test_set_state_closes_relay(void)
{
  SafetyRelayState_t commanded;

  TEST_ASSERT_EQUAL_UINT8(1U,
                          SafetyRelaySetState(&g_port,
                                              SAFETY_RELAY_STATE_CLOSED));
  TEST_ASSERT_EQUAL_UINT8(1U,
                          SafetyRelayGetCommandedState(&g_port, &commanded));
  TEST_ASSERT_EQUAL(SAFETY_RELAY_STATE_CLOSED, commanded);
  TEST_ASSERT_EQUAL_UINT8(0U, g_ctx.lastRelayDrive);
  TEST_ASSERT_EQUAL_UINT8(0U, g_ctx.lastTriacDrive);
  TEST_ASSERT_EQUAL_UINT32(1U, g_ctx.transitionCount);
}

void test_set_state_same_value_does_not_count_transition(void)
{
  (void) SafetyRelaySetState(&g_port, SAFETY_RELAY_STATE_CLOSED);
  (void) SafetyRelaySetState(&g_port, SAFETY_RELAY_STATE_CLOSED);
  (void) SafetyRelaySetState(&g_port, SAFETY_RELAY_STATE_CLOSED);

  TEST_ASSERT_EQUAL_UINT32(1U, g_ctx.transitionCount);
}

void test_set_state_counts_each_distinct_transition(void)
{
  (void) SafetyRelaySetState(&g_port, SAFETY_RELAY_STATE_CLOSED);
  (void) SafetyRelaySetState(&g_port, SAFETY_RELAY_STATE_OPEN);
  (void) SafetyRelaySetState(&g_port, SAFETY_RELAY_STATE_CLOSED);
  (void) SafetyRelaySetState(&g_port, SAFETY_RELAY_STATE_OPEN);

  TEST_ASSERT_EQUAL_UINT32(4U, g_ctx.transitionCount);
}

void test_eco_topology_inverts_relay_drive_but_preserves_state_semantics(void)
{
  SafetyRelayState_t actual;

  TEST_ASSERT_EQUAL_UINT8(1U, g_ctx.lastRelayDrive);
  TEST_ASSERT_EQUAL_UINT8(1U, g_ctx.lastTriacDrive);

  (void) SafetyRelaySetState(&g_port, SAFETY_RELAY_STATE_CLOSED);

  TEST_ASSERT_EQUAL_UINT8(0U, g_ctx.lastRelayDrive);
  TEST_ASSERT_EQUAL_UINT8(0U, g_ctx.lastTriacDrive);
  TEST_ASSERT_EQUAL_UINT8(1U,
                          SafetyRelayGetActualState(&g_port, &actual));
  TEST_ASSERT_EQUAL(SAFETY_RELAY_STATE_CLOSED, actual);
}

void test_null_port_returns_zero(void)
{
  SafetyRelayState_t s;

  TEST_ASSERT_EQUAL_UINT8(0U,
                          SafetyRelaySetState(NULL, SAFETY_RELAY_STATE_CLOSED));
  TEST_ASSERT_EQUAL_UINT8(0U,
                          SafetyRelayGetCommandedState(NULL, &s));
  TEST_ASSERT_EQUAL_UINT8(0U,
                          SafetyRelayGetActualState(NULL, &s));
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_init_sets_relay_open);
  RUN_TEST(test_set_state_closes_relay);
  RUN_TEST(test_set_state_same_value_does_not_count_transition);
  RUN_TEST(test_set_state_counts_each_distinct_transition);
  RUN_TEST(test_eco_topology_inverts_relay_drive_but_preserves_state_semantics);
  RUN_TEST(test_null_port_returns_zero);

  return UNITY_END();
}
