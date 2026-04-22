/*
 * Tests/Unit/Test_FieldBusTxAdapter.c
 *
 * Host-side coverage for legacy-compatible field-bus flash-config frames.
 */
#include "unity.h"

#include <string.h>

#include "Adapters/STM32/FieldBusTxAdapter.h"
#include "Adapters/STM32/LegacyFieldCanIds.h"
#include "Domain/Intersection/ConfigurationService.h"
#include "MockConfigRepositoryAdapter.h"

typedef struct
{
  uint16_t identifier;
  uint8_t data[8];
  uint8_t length;
} TxFrame_t;

static MockConfigRepositoryAdapterCtx_t s_repoCtx;
static IConfigRepositoryPort_t s_repoPort;
static ConfigurationService_t s_configurationService;
static FieldBusTxAdapterCtx_t s_adapterCtx;
static TxFrame_t s_frames[16];
static uint8_t s_frameCount;

uint8_t FieldCanQueueTxSendStandard(uint16_t identifier,
                                    const uint8_t *data,
                                    uint8_t length)
{
  if ((s_frameCount >= (uint8_t) (sizeof(s_frames) / sizeof(s_frames[0])))
      || ((data == NULL) && (length != 0U)))
  {
    return 0U;
  }

  s_frames[s_frameCount].identifier = identifier;
  s_frames[s_frameCount].length = length;
  (void) memset(s_frames[s_frameCount].data, 0, sizeof(s_frames[s_frameCount].data));
  if ((data != NULL) && (length != 0U))
  {
    (void) memcpy(s_frames[s_frameCount].data, data, length);
  }
  s_frameCount++;

  return 1U;
}

static void CommitFlashConfig(uint8_t flashMask, uint8_t failureFlashPeriodDs)
{
  IntersectionIoMapConfig_t ioMap;

  TEST_ASSERT_TRUE(ConfigurationServiceCreateTransaction(&s_configurationService));
  TEST_ASSERT_TRUE(ConfigurationServiceGetCandidateIoMapConfig(
    &s_configurationService,
    &ioMap));
  ioMap.outputs[0].deviceType = (uint8_t) INTERSECTION_IO_MAP_DEVICE_FIO;
  ioMap.outputs[0].deviceAddr = 0U;
  ioMap.outputs[0].devicePin = 1U;
  ioMap.outputs[0].function =
    (uint8_t) INTERSECTION_IO_MAP_OUTPUT_FUNCTION_CHANNEL_RED;
  ioMap.outputs[0].functionIndex = 1U;
  TEST_ASSERT_TRUE(ConfigurationServiceSetIoMapConfig(&s_configurationService,
                                                      &ioMap));
  TEST_ASSERT_TRUE(ConfigurationServiceSetChannelFlashMask(&s_configurationService,
                                                           0U,
                                                           flashMask));
  TEST_ASSERT_TRUE(
    ConfigurationServiceSetUnitFailureFlashPeriodDs(&s_configurationService,
                                                    failureFlashPeriodDs));
  TEST_ASSERT_TRUE(ConfigurationServiceVerify(&s_configurationService));
  TEST_ASSERT_TRUE(ConfigurationServiceCommit(&s_configurationService));
}

void setUp(void)
{
  MockConfigRepositoryAdapterInit(&s_repoCtx);
  s_repoPort = MockConfigRepositoryAdapterCreatePort(&s_repoCtx);
  ConfigurationServiceInit(&s_configurationService, &s_repoPort);
  FieldBusTxAdapterInit(&s_adapterCtx, &s_configurationService, NULL, 0U);
  s_frameCount = 0U;
}

void tearDown(void)
{
}

void test_flash_config_frames_keep_legacy_ids_and_use_committed_period(void)
{
  OutputDriverImage_t image;
  IOutputDriverPort_t port = FieldBusTxAdapterCreatePort(&s_adapterCtx);

  CommitFlashConfig(0x04U, INTERSECTION_UNIT_FAILURE_FLASH_PERIOD_2000MS_DS);
  (void) memset(&image, 0, sizeof(image));
  image.channels[0] = OUTPUT_DRIVER_ASPECT_DARK;
  TEST_ASSERT_TRUE(OutputDriverApply(&port, &image));

  FieldBusTxAdapterStep();

  TEST_ASSERT_EQUAL_UINT8(6U, s_frameCount);
  TEST_ASSERT_EQUAL_UINT16(LEGACY_FIELD_CAN_ID_CPU_FLASH_SIGNALS0,
                           s_frames[4].identifier);
  TEST_ASSERT_EQUAL_UINT8(8U, s_frames[4].length);
  TEST_ASSERT_BITS_HIGH(0x01U, s_frames[4].data[0]);
  TEST_ASSERT_EQUAL_UINT8(INTERSECTION_UNIT_FAILURE_FLASH_PERIOD_2000MS_DS,
                          s_frames[4].data[6]);
  TEST_ASSERT_EQUAL_UINT8(0U, s_frames[4].data[7]);
  TEST_ASSERT_EQUAL_UINT16(LEGACY_FIELD_CAN_ID_CPU_FLASH_SIGNALS1,
                           s_frames[5].identifier);
  TEST_ASSERT_EQUAL_UINT8(INTERSECTION_UNIT_FAILURE_FLASH_PERIOD_2000MS_DS,
                          s_frames[5].data[6]);
  TEST_ASSERT_EQUAL_UINT8(0U, s_frames[5].data[7]);
}

void test_config_epoch_change_forces_flash_config_resend_with_new_period(void)
{
  OutputDriverImage_t image;
  IOutputDriverPort_t port = FieldBusTxAdapterCreatePort(&s_adapterCtx);

  CommitFlashConfig(0x04U, INTERSECTION_UNIT_FAILURE_FLASH_PERIOD_500MS_DS);
  (void) memset(&image, 0, sizeof(image));
  image.channels[0] = OUTPUT_DRIVER_ASPECT_DARK;
  TEST_ASSERT_TRUE(OutputDriverApply(&port, &image));

  FieldBusTxAdapterStep();
  TEST_ASSERT_EQUAL_UINT8(INTERSECTION_UNIT_FAILURE_FLASH_PERIOD_500MS_DS,
                          s_frames[4].data[6]);

  CommitFlashConfig(0x04U, INTERSECTION_UNIT_FAILURE_FLASH_PERIOD_4000MS_DS);
  FieldBusTxAdapterSetConfigEpoch(&s_adapterCtx, 1U);
  s_frameCount = 0U;

  FieldBusTxAdapterStep();

  TEST_ASSERT_EQUAL_UINT16(LEGACY_FIELD_CAN_ID_CPU_FLASH_SIGNALS0,
                           s_frames[4].identifier);
  TEST_ASSERT_EQUAL_UINT8(INTERSECTION_UNIT_FAILURE_FLASH_PERIOD_4000MS_DS,
                          s_frames[4].data[6]);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_flash_config_frames_keep_legacy_ids_and_use_committed_period);
  RUN_TEST(test_config_epoch_change_forces_flash_config_resend_with_new_period);
  return UNITY_END();
}
