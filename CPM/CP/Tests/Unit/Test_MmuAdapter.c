/*
 * Tests/Unit/Test_MmuAdapter.c
 *
 * CP-side MMU seam coverage for relay authority and output filtering.
 */
#include "unity.h"

#include <string.h>

#include "Adapters/Mock/MockRelayAdapter.h"
#include "Adapters/STM32/MmuAdapter.h"

static MmuAdapterCtx_t s_ctx;
static MockRelayAdapterCtx_t s_relayCtx;
static IRelayPort_t s_relayPort;
static RelayControlService_t s_relayControlService;

static void FillRequestedImage(OutputDriverImage_t *image,
                               OutputDriverAspect_t aspect)
{
  uint8_t channelIndex;

  memset(image, 0, sizeof(*image));
  for (channelIndex = 0U; channelIndex < INTERSECTION_CHANNEL_COUNT_MAX;
       channelIndex++)
  {
    image->channels[channelIndex] = aspect;
  }
}

void setUp(void)
{
  MockRelayAdapterInit(&s_relayCtx);
  s_relayPort = MockRelayAdapterCreatePort(&s_relayCtx);
  MmuAdapterInit(&s_ctx);
  MmuAdapterBindRelayPort(&s_ctx, &s_relayPort);
  RelayControlServiceInit(&s_relayControlService);
  MmuAdapterBindRelayControlService(&s_ctx, &s_relayControlService);
}

void tearDown(void)
{
}

void test_mmu_adapter_requires_config_and_required_ssm_before_permitting_output_power(
  void)
{
  TEST_ASSERT_EQUAL_UINT8(0U, MmuAdapterGetPermitOutputPower(&s_ctx));
  TEST_ASSERT_EQUAL_UINT8(0U, MmuAdapterGetLastRelayDrive(&s_ctx));
  TEST_ASSERT_EQUAL_UINT8(0U, s_relayCtx.relayState);

  MmuAdapterSetSafetyAction(&s_ctx, MMU_CONTROL_ACTION_FLASH);

  TEST_ASSERT_EQUAL_UINT8(0U, MmuAdapterGetPermitOutputPower(&s_ctx));
  TEST_ASSERT_EQUAL_UINT8(0U, MmuAdapterGetLastRelayDrive(&s_ctx));
  TEST_ASSERT_EQUAL_UINT8(0U, s_relayCtx.relayState);

  MmuAdapterSetConfigReady(&s_ctx, 1U);

  TEST_ASSERT_EQUAL_UINT8(0U, MmuAdapterGetPermitOutputPower(&s_ctx));
  TEST_ASSERT_EQUAL_UINT8(0U, MmuAdapterGetLastRelayDrive(&s_ctx));
  TEST_ASSERT_EQUAL_UINT8(0U, s_relayCtx.relayState);

  MmuAdapterSetRequiredSsmHealthy(&s_ctx, 1U);

  TEST_ASSERT_EQUAL_UINT8(1U, MmuAdapterGetPermitOutputPower(&s_ctx));
  TEST_ASSERT_EQUAL_UINT8(1U, MmuAdapterGetLastRelayDrive(&s_ctx));
  TEST_ASSERT_EQUAL_UINT8(1U, s_relayCtx.relayState);

  MmuAdapterSetSafetyAction(&s_ctx, MMU_CONTROL_ACTION_DARK);

  TEST_ASSERT_EQUAL_UINT8(0U, MmuAdapterGetPermitOutputPower(&s_ctx));
  TEST_ASSERT_EQUAL_UINT8(0U, MmuAdapterGetLastRelayDrive(&s_ctx));
  TEST_ASSERT_EQUAL_UINT8(0U, s_relayCtx.relayState);
}

void test_mmu_adapter_inverts_relay_drive_for_eco_topology(void)
{
  MmuAdapterSetRelayTopology(&s_ctx, MMU_RELAY_TOPOLOGY_ECO_ACTIVE_HIGH_TRIP);

  TEST_ASSERT_EQUAL_UINT8(0U, MmuAdapterGetPermitOutputPower(&s_ctx));
  TEST_ASSERT_EQUAL_UINT8(1U, MmuAdapterGetLastRelayDrive(&s_ctx));
  TEST_ASSERT_EQUAL_UINT8(1U, s_relayCtx.relayState);

  MmuAdapterSetConfigReady(&s_ctx, 1U);
  MmuAdapterSetRequiredSsmHealthy(&s_ctx, 1U);
  MmuAdapterSetSafetyAction(&s_ctx, MMU_CONTROL_ACTION_NORMAL);

  TEST_ASSERT_EQUAL_UINT8(1U, MmuAdapterGetPermitOutputPower(&s_ctx));
  TEST_ASSERT_EQUAL_UINT8(0U, MmuAdapterGetLastRelayDrive(&s_ctx));
  TEST_ASSERT_EQUAL_UINT8(0U, s_relayCtx.relayState);
}

void test_mmu_adapter_force_all_red_filters_without_dropping_output_power(void)
{
  IMmuPort_t mmuPort;
  OutputDriverImage_t requested;
  OutputDriverImage_t approved;
  uint8_t channelIndex;

  mmuPort = MmuAdapterCreatePort(&s_ctx);
  MmuAdapterSetConfigReady(&s_ctx, 1U);
  MmuAdapterSetRequiredSsmHealthy(&s_ctx, 1U);
  MmuAdapterSetSafetyAction(&s_ctx, MMU_CONTROL_ACTION_FLASH);
  MmuAdapterSetForceAllRed(&s_ctx, 1U);
  FillRequestedImage(&requested, OUTPUT_DRIVER_ASPECT_GREEN);

  TEST_ASSERT_TRUE(MmuFilterOutputImage(&mmuPort, &requested, &approved));
  TEST_ASSERT_EQUAL_UINT8(1U, MmuAdapterGetPermitOutputPower(&s_ctx));
  TEST_ASSERT_EQUAL_UINT8(1U, s_relayCtx.relayState);

  for (channelIndex = 0U; channelIndex < INTERSECTION_CHANNEL_COUNT_MAX;
       channelIndex++)
  {
    TEST_ASSERT_EQUAL(OUTPUT_DRIVER_ASPECT_RED, approved.channels[channelIndex]);
  }

  MmuAdapterSetForceAllRed(&s_ctx, 0U);
  MmuAdapterSetSafetyAction(&s_ctx, MMU_CONTROL_ACTION_DARK);

  TEST_ASSERT_TRUE(MmuFilterOutputImage(&mmuPort, &requested, &approved));
  TEST_ASSERT_EQUAL_UINT8(0U, MmuAdapterGetPermitOutputPower(&s_ctx));
  TEST_ASSERT_EQUAL_UINT8(0U, s_relayCtx.relayState);

  for (channelIndex = 0U; channelIndex < INTERSECTION_CHANNEL_COUNT_MAX;
       channelIndex++)
  {
    TEST_ASSERT_EQUAL(OUTPUT_DRIVER_ASPECT_DARK,
                      approved.channels[channelIndex]);
  }
}

void test_mmu_adapter_user_power_disable_drops_relay_permit(void)
{
  MmuAdapterSetRelayTopology(&s_ctx, MMU_RELAY_TOPOLOGY_ECO_ACTIVE_HIGH_TRIP);
  MmuAdapterSetConfigReady(&s_ctx, 1U);
  MmuAdapterSetRequiredSsmHealthy(&s_ctx, 1U);
  MmuAdapterSetSafetyAction(&s_ctx, MMU_CONTROL_ACTION_NORMAL);

  TEST_ASSERT_EQUAL_UINT8(1U, MmuAdapterGetPermitOutputPower(&s_ctx));
  TEST_ASSERT_EQUAL_UINT8(0U, MmuAdapterGetLastRelayDrive(&s_ctx));
  TEST_ASSERT_EQUAL_UINT8(0U, s_relayCtx.relayState);

  TEST_ASSERT_TRUE(RelayControlServiceSetUserOutputPowerEnabled(
    &s_relayControlService,
    0U));
  MmuAdapterSetConfigReady(&s_ctx, 1U);

  TEST_ASSERT_EQUAL_UINT8(0U, MmuAdapterGetPermitOutputPower(&s_ctx));
  TEST_ASSERT_EQUAL_UINT8(1U, MmuAdapterGetLastRelayDrive(&s_ctx));
  TEST_ASSERT_EQUAL_UINT8(1U, s_relayCtx.relayState);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(
    test_mmu_adapter_requires_config_and_required_ssm_before_permitting_output_power);
  RUN_TEST(test_mmu_adapter_inverts_relay_drive_for_eco_topology);
  RUN_TEST(test_mmu_adapter_force_all_red_filters_without_dropping_output_power);
  RUN_TEST(test_mmu_adapter_user_power_disable_drops_relay_permit);
  return UNITY_END();
}
