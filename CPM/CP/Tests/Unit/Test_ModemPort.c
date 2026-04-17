/*
 * Tests/Unit/Test_ModemPort.c
 *
 * Unit tests for the IModemPort_t contract using the mock adapter.
 * Verifies that all inline dispatch helpers call through correctly and
 * that the mock's inspection fields capture what was passed.
 */
#include "unity.h"

#include <string.h>

#include "MockModemAdapter.h"

static MockModemAdapterCtx_t s_ctx;
static IModemPort_t s_port;

void setUp(void)
{
  MockModemAdapterInit(&s_ctx);
  s_port = MockModemAdapterCreatePort(&s_ctx);
}

void tearDown(void)
{
}

/* ------------------------------------------------------------------ */
/* InjectResponse / HandleResponse                                     */
/* ------------------------------------------------------------------ */

void test_handle_response_returns_injected_state(void)
{
  ModemInfo_t info;
  uint8_t result;

  memset(&info, 0, sizeof(info));
  MockModemAdapterInjectResponse(&s_ctx, 7U);

  result = ModemHandleResponse(&s_port, 0U, "OK", 1U, &info);

  TEST_ASSERT_EQUAL_UINT8(7U, result);
}

void test_inject_response_updates_bNextState(void)
{
  MockModemAdapterInjectResponse(&s_ctx, MODEM_STATE_CONNECTED);
  TEST_ASSERT_EQUAL_UINT8(MODEM_STATE_CONNECTED, s_ctx.bNextState);
}

void test_handle_response_returns_failed_sentinel(void)
{
  ModemInfo_t info;
  uint8_t result;

  memset(&info, 0, sizeof(info));
  MockModemAdapterInjectResponse(&s_ctx, MODEM_STATE_FAILED);

  result = ModemHandleResponse(&s_port, 3U, "", 0U, &info);

  TEST_ASSERT_EQUAL_UINT8(MODEM_STATE_FAILED, result);
}

/* ------------------------------------------------------------------ */
/* PrepareCommand                                                       */
/* ------------------------------------------------------------------ */

void test_prepare_command_increments_cmd_count(void)
{
  char outBuf[64];

  s_ctx.bPrepareResult = 1U;

  ModemPrepareCommand(&s_port, 0U, "apn", "host", 1234U, outBuf, 64U);
  TEST_ASSERT_EQUAL_UINT8(1U, s_ctx.cmdCount);

  ModemPrepareCommand(&s_port, 1U, "apn", "host", 1234U, outBuf, 64U);
  TEST_ASSERT_EQUAL_UINT8(2U, s_ctx.cmdCount);
}

void test_prepare_command_returns_configured_result(void)
{
  char outBuf[64];
  uint8_t result;

  s_ctx.bPrepareResult = 0U;
  result = ModemPrepareCommand(&s_port, 0U, "apn", "host", 1234U,
                               outBuf, 64U);
  TEST_ASSERT_EQUAL_UINT8(0U, result);

  s_ctx.bPrepareResult = 1U;
  result = ModemPrepareCommand(&s_port, 0U, "apn", "host", 1234U,
                               outBuf, 64U);
  TEST_ASSERT_EQUAL_UINT8(1U, result);
}

/* ------------------------------------------------------------------ */
/* IsDisconnected                                                       */
/* ------------------------------------------------------------------ */

void test_is_disconnected_returns_false_by_default(void)
{
  uint8_t result = ModemIsDisconnected(&s_port, "NO CARRIER", 10U);

  TEST_ASSERT_EQUAL_UINT8(0U, result);
}

void test_is_disconnected_returns_configured_value(void)
{
  s_ctx.bDisconnected = 1U;
  TEST_ASSERT_EQUAL_UINT8(1U,
                          ModemIsDisconnected(&s_port, "NORMAL", 6U));

  s_ctx.bDisconnected = 0U;
  TEST_ASSERT_EQUAL_UINT8(0U,
                          ModemIsDisconnected(&s_port, "NO CARRIER", 10U));
}

/* ------------------------------------------------------------------ */
/* GetGreetingType                                                      */
/* ------------------------------------------------------------------ */

void test_get_greeting_type_defaults_to_imei(void)
{
  TEST_ASSERT_EQUAL_UINT8(0U, ModemGetGreetingType(&s_port));
}

void test_get_greeting_type_returns_configured_value(void)
{
  s_ctx.bGreetingType = MODEM_GREETING_USR_MAC;
  TEST_ASSERT_EQUAL_UINT8(MODEM_GREETING_USR_MAC,
                          ModemGetGreetingType(&s_port));

  s_ctx.bGreetingType = MODEM_GREETING_ETH_MAC;
  TEST_ASSERT_EQUAL_UINT8(MODEM_GREETING_ETH_MAC,
                          ModemGetGreetingType(&s_port));
}

/* ------------------------------------------------------------------ */
/* GetInitialState / GetBaudRate / GetStateLabel                        */
/* ------------------------------------------------------------------ */

void test_get_initial_state_returns_zero(void)
{
  TEST_ASSERT_EQUAL_UINT8(0U, ModemGetInitialState(&s_port));
}

void test_get_baud_rate_returns_zero_for_mock(void)
{
  TEST_ASSERT_EQUAL_UINT32(0U, ModemGetBaudRate(&s_port));
}

void test_get_state_label_returns_non_null(void)
{
  const char *label = ModemGetStateLabel(&s_port, 0U);

  TEST_ASSERT_NOT_NULL(label);
}

/* ------------------------------------------------------------------ */
/* GetWaitParams smoke test                                             */
/* ------------------------------------------------------------------ */

void test_get_wait_params_fills_output_fields(void)
{
  const char *kw = NULL;
  char sc = 'X';
  uint32_t tms = 0U;
  uint8_t mr = 0U;

  ModemGetWaitParams(&s_port, 0U, &kw, &sc, &tms, &mr);

  TEST_ASSERT_NOT_NULL(kw);
  TEST_ASSERT_GREATER_THAN_UINT32(0U, tms);
  TEST_ASSERT_GREATER_THAN_UINT8(0U, mr);
}

/* ------------------------------------------------------------------ */
/* ModemInfo_t sentinel constants                                       */
/* ------------------------------------------------------------------ */

void test_sentinel_values_do_not_overlap_valid_states(void)
{
  TEST_ASSERT_NOT_EQUAL(MODEM_STATE_CONNECTED, MODEM_STATE_FAILED);
  /* 0xFE and 0xFF should be outside any adapter's normal state range */
  TEST_ASSERT_GREATER_THAN_UINT8(31U, MODEM_STATE_CONNECTED);
  TEST_ASSERT_GREATER_THAN_UINT8(31U, MODEM_STATE_FAILED);
}

/* ------------------------------------------------------------------ */

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_handle_response_returns_injected_state);
  RUN_TEST(test_inject_response_updates_bNextState);
  RUN_TEST(test_handle_response_returns_failed_sentinel);
  RUN_TEST(test_prepare_command_increments_cmd_count);
  RUN_TEST(test_prepare_command_returns_configured_result);
  RUN_TEST(test_is_disconnected_returns_false_by_default);
  RUN_TEST(test_is_disconnected_returns_configured_value);
  RUN_TEST(test_get_greeting_type_defaults_to_imei);
  RUN_TEST(test_get_greeting_type_returns_configured_value);
  RUN_TEST(test_get_initial_state_returns_zero);
  RUN_TEST(test_get_baud_rate_returns_zero_for_mock);
  RUN_TEST(test_get_state_label_returns_non_null);
  RUN_TEST(test_get_wait_params_fills_output_fields);
  RUN_TEST(test_sentinel_values_do_not_overlap_valid_states);

  return UNITY_END();
}
