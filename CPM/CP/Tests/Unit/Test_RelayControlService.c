/* Tests/Unit/Test_RelayControlService.c */
#include "unity.h"

#include "Domain/Services/RelayControlService.h"

static RelayControlService_t s_service;

void setUp(void)
{
  RelayControlServiceInit(&s_service);
}

void tearDown(void)
{
}

void test_relay_control_service_initializes_with_user_enabled_but_no_permit(void)
{
  TEST_ASSERT_EQUAL_UINT8(1U,
                          RelayControlServiceGetUserOutputPowerEnabled(
                            &s_service));
  TEST_ASSERT_EQUAL_UINT8(0U,
                          RelayControlServiceGetLocalPermitOutputPower(
                            &s_service));
  TEST_ASSERT_EQUAL_UINT8(0U,
                          RelayControlServiceGetPeerPermitValid(&s_service));
  TEST_ASSERT_EQUAL_UINT8(0U,
                          RelayControlServiceGetEffectivePermitOutputPower(
                            &s_service));
}

void test_relay_control_service_requires_valid_peer_vote_for_effective_permit(void)
{
  RelayControlServiceSetLocalState(&s_service, 1U, 0U, 1U, 0U);
  TEST_ASSERT_EQUAL_UINT8(0U,
                          RelayControlServiceGetEffectivePermitOutputPower(
                            &s_service));

  RelayControlServiceSetPeerState(&s_service, 1U, 1U);
  TEST_ASSERT_EQUAL_UINT8(1U,
                          RelayControlServiceGetEffectivePermitOutputPower(
                            &s_service));

  RelayControlServiceSetPeerState(&s_service, 0U, 0U);
  TEST_ASSERT_EQUAL_UINT8(1U,
                          RelayControlServiceGetLocalPermitOutputPower(
                            &s_service));
  TEST_ASSERT_EQUAL_UINT8(0U,
                          RelayControlServiceGetPeerPermitValid(&s_service));
  TEST_ASSERT_EQUAL_UINT8(0U,
                          RelayControlServiceGetEffectivePermitOutputPower(
                            &s_service));
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_relay_control_service_initializes_with_user_enabled_but_no_permit);
  RUN_TEST(
    test_relay_control_service_requires_valid_peer_vote_for_effective_permit);
  return UNITY_END();
}
