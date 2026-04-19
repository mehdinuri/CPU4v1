/* Tests/Unit/Test_CpMpLinkService.c */

#include "unity.h"

#include <string.h>

#include "Adapters/Mock/MockControlBusAdapter.h"
#include "Adapters/Mock/MockSafetyRelayAdapter.h"
#include "FaultMonitor/FaultMonitorService.h"
#include "Intersection/CpMpLinkService.h"

static CpMpLinkService_t s_service;
static MockControlBusAdapterCtx_t s_controlBusCtx;
static IControlBusPort_t s_controlBusPort;
static ConfigurationService_t s_configurationService;
static MockSafetyRelayAdapterCtx_t s_relayCtx;
static ISafetyRelayPort_t s_relayPort;
static SafetyDecisionService_t s_safetyService;
static FaultMonitorService_t s_faultMonitorService;

static CpMpMmuConfigImageV1_t MakeConfigImage(void)
{
  CpMpMmuConfigImageV1_t image;

  (void) memset(&image, 0, sizeof(image));
  image.phaseCount = 4U;
  image.ringCount = 2U;
  image.channelCount = 4U;
  image.outputMapCount = 3U;
  image.startupFlashSeconds = 5U;
  image.startupFlashMode = 3U;
  image.channelControlType[0] = INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_VEHICLE;
  image.channelControlSource[0] = 1U;
  image.channelControlType[1] = INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_VEHICLE;
  image.channelControlSource[1] = 2U;
  image.phaseYellowChangeDs[0] = 30U;
  image.phaseRedClearDs[0] = 20U;
  image.phaseConcurrencyMask[0] = 0x04U;
  image.outputMap[0].outputIndex = 0U;
  image.outputMap[0].channelIndex = 0U;
  image.outputMap[0].color = CPMP_OUTPUT_COLOR_RED;
  image.outputMap[1].outputIndex = 1U;
  image.outputMap[1].channelIndex = 0U;
  image.outputMap[1].color = CPMP_OUTPUT_COLOR_YELLOW;
  image.outputMap[2].outputIndex = 2U;
  image.outputMap[2].channelIndex = 0U;
  image.outputMap[2].color = CPMP_OUTPUT_COLOR_GREEN;

  return image;
}

static void InjectHeartbeat(uint16_t setId, uint32_t generation)
{
  ControlBusFrame_t frame;

  (void) memset(&frame, 0, sizeof(frame));
  frame.standardId = CPMP_FRAME_ID_CP_HEARTBEAT;
  frame.length = 12U;
  frame.data[0] = CPMP_PROTOCOL_VERSION;
  frame.data[2] = (uint8_t) (setId & 0xFFU);
  frame.data[3] = (uint8_t) ((setId >> 8U) & 0xFFU);
  frame.data[8] = (uint8_t) (generation & 0xFFU);
  frame.data[9] = (uint8_t) ((generation >> 8U) & 0xFFU);
  frame.data[10] = (uint8_t) ((generation >> 16U) & 0xFFU);
  frame.data[11] = (uint8_t) ((generation >> 24U) & 0xFFU);
  MockControlBusAdapterInjectRxFrame(&s_controlBusCtx, &frame);
}

static void InjectConfigImage(uint16_t setId,
                              uint32_t generation,
                              const CpMpMmuConfigImageV1_t *image)
{
  ControlBusFrame_t frame;
  const uint8_t *bytes = (const uint8_t *) image;
  uint8_t totalChunks = (uint8_t) ((sizeof(*image)
                                    + CPMP_CONFIG_CHUNK_PAYLOAD_BYTES - 1U)
                                   / CPMP_CONFIG_CHUNK_PAYLOAD_BYTES);
  uint8_t chunkIndex;

  (void) memset(&frame, 0, sizeof(frame));
  frame.standardId = CPMP_FRAME_ID_CP_CFG_BEGIN;
  frame.length = 10U;
  frame.data[0] = CPMP_PROTOCOL_VERSION;
  frame.data[1] = totalChunks;
  frame.data[2] = (uint8_t) (setId & 0xFFU);
  frame.data[3] = (uint8_t) ((setId >> 8U) & 0xFFU);
  frame.data[4] = (uint8_t) (generation & 0xFFU);
  frame.data[5] = (uint8_t) ((generation >> 8U) & 0xFFU);
  frame.data[6] = (uint8_t) ((generation >> 16U) & 0xFFU);
  frame.data[7] = (uint8_t) ((generation >> 24U) & 0xFFU);
  frame.data[8] = (uint8_t) (sizeof(*image) & 0xFFU);
  frame.data[9] = (uint8_t) ((sizeof(*image) >> 8U) & 0xFFU);
  MockControlBusAdapterInjectRxFrame(&s_controlBusCtx, &frame);

  for (chunkIndex = 0U; chunkIndex < totalChunks; ++chunkIndex)
  {
    uint16_t offset = (uint16_t) chunkIndex * CPMP_CONFIG_CHUNK_PAYLOAD_BYTES;
    uint16_t remaining = (uint16_t) (sizeof(*image) - offset);
    uint8_t length;

    if (remaining > CPMP_CONFIG_CHUNK_PAYLOAD_BYTES)
    {
      length = CPMP_CONFIG_CHUNK_PAYLOAD_BYTES;
    }
    else
    {
      length = (uint8_t) remaining;
    }

    (void) memset(&frame, 0, sizeof(frame));
    frame.standardId = CPMP_FRAME_ID_CP_CFG_CHUNK;
    frame.length = (uint8_t) (4U + length);
    frame.data[0] = CPMP_PROTOCOL_VERSION;
    frame.data[1] = chunkIndex;
    frame.data[2] = length;
    (void) memcpy(&frame.data[4], &bytes[offset], length);
    MockControlBusAdapterInjectRxFrame(&s_controlBusCtx, &frame);
  }

  (void) memset(&frame, 0, sizeof(frame));
  frame.standardId = CPMP_FRAME_ID_CP_CFG_COMMIT;
  frame.length = 8U;
  frame.data[0] = CPMP_PROTOCOL_VERSION;
  frame.data[2] = (uint8_t) (setId & 0xFFU);
  frame.data[3] = (uint8_t) ((setId >> 8U) & 0xFFU);
  frame.data[4] = (uint8_t) (generation & 0xFFU);
  frame.data[5] = (uint8_t) ((generation >> 8U) & 0xFFU);
  frame.data[6] = (uint8_t) ((generation >> 16U) & 0xFFU);
  frame.data[7] = (uint8_t) ((generation >> 24U) & 0xFFU);
  MockControlBusAdapterInjectRxFrame(&s_controlBusCtx, &frame);
}

void setUp(void)
{
  MockControlBusAdapterInit(&s_controlBusCtx);
  s_controlBusPort = MockControlBusAdapterCreatePort(&s_controlBusCtx);
  ConfigurationServiceInit(&s_configurationService, NULL);
  MockSafetyRelayAdapterInit(&s_relayCtx);
  s_relayPort = MockSafetyRelayAdapterCreatePort(&s_relayCtx);
  SafetyDecisionServiceInit(&s_safetyService, &s_relayPort);
  SafetyDecisionServiceReset(&s_safetyService);
  FaultMonitorServiceInit(&s_faultMonitorService);
  CpMpLinkServiceInit(&s_service,
                      &s_controlBusPort,
                      &s_configurationService,
                      &s_safetyService,
                      &s_faultMonitorService);
}

void tearDown(void)
{
}

void test_link_service_applies_cp_config_and_reports_normal_when_matching(void)
{
  CpMpMmuConfigImageV1_t image = MakeConfigImage();
  const IntersectionConfig_t *config;

  InjectHeartbeat(3U, 9U);
  InjectConfigImage(3U, 9U, &image);

  CpMpLinkServiceStep(&s_service);

  TEST_ASSERT_EQUAL_UINT8(CPMP_CONFIG_STATE_APPLIED, s_service.configState);
  TEST_ASSERT_EQUAL_UINT8(CONFIG_STATE_VALID,
                          ConfigurationServiceGetState(&s_configurationService));

  config = ConfigurationServiceGetConfig(&s_configurationService);
  TEST_ASSERT_NOT_NULL(config);
  TEST_ASSERT_EQUAL_UINT8(4U, config->phaseCount);
  TEST_ASSERT_EQUAL_UINT8(INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_VEHICLE,
                          config->channels[0].controlType);
  TEST_ASSERT_EQUAL_UINT8(1U, config->channels[0].controlSource);

  TEST_ASSERT_TRUE(s_controlBusCtx.txCount >= 4U);
  TEST_ASSERT_EQUAL_UINT16(CPMP_FRAME_ID_MP_HEARTBEAT,
                           s_controlBusCtx.txBuffer[0].standardId);
  TEST_ASSERT_EQUAL_UINT8(CPMP_CONFIG_STATE_APPLIED,
                          s_controlBusCtx.txBuffer[0].data[1]);
  TEST_ASSERT_EQUAL_UINT8(CPMP_SAFETY_ACTION_NORMAL,
                          s_controlBusCtx.txBuffer[0].data[2]);
}

void test_link_service_reports_flash_until_cp_config_is_applied(void)
{
  CpMpLinkServiceStep(&s_service);

  TEST_ASSERT_EQUAL_UINT16(CPMP_FRAME_ID_MP_HEARTBEAT,
                           s_controlBusCtx.txBuffer[0].standardId);
  TEST_ASSERT_EQUAL_UINT8(CPMP_SAFETY_ACTION_FLASH,
                          s_controlBusCtx.txBuffer[0].data[2]);
}

void test_link_service_projects_fault_monitor_status_into_fault_frame(void)
{
  CpMpMmuConfigImageV1_t image = MakeConfigImage();
  FaultEvent_t event = {
    FAULT_CODE_CONFLICT_GREEN_GREEN, FAULT_SEVERITY_CRITICAL, 0U, 9U, 0U
  };

  InjectHeartbeat(1U, 5U);
  InjectConfigImage(1U, 5U, &image);
  FaultMonitorServiceOnFault(&s_faultMonitorService, &event);
  SafetyDecisionServiceOnFault(&s_safetyService, &event);

  CpMpLinkServiceStep(&s_service);

  TEST_ASSERT_EQUAL_UINT16(CPMP_FRAME_ID_MP_FAULTS,
                           s_controlBusCtx.txBuffer[3].standardId);
  TEST_ASSERT_EQUAL_UINT8(CPMP_PROTOCOL_VERSION,
                          s_controlBusCtx.txBuffer[3].data[0]);
  TEST_ASSERT_EQUAL_UINT32(1U,
                           (uint32_t) s_controlBusCtx.txBuffer[3].data[1]
                           | ((uint32_t) s_controlBusCtx.txBuffer[3].data[2] << 8U)
                           | ((uint32_t) s_controlBusCtx.txBuffer[3].data[3] << 16U)
                           | ((uint32_t) s_controlBusCtx.txBuffer[3].data[4] << 24U));
  TEST_ASSERT_EQUAL_UINT32(0U,
                           (uint32_t) s_controlBusCtx.txBuffer[3].data[5]
                           | ((uint32_t) s_controlBusCtx.txBuffer[3].data[6] << 8U)
                           | ((uint32_t) s_controlBusCtx.txBuffer[3].data[7] << 16U)
                           | ((uint32_t) s_controlBusCtx.txBuffer[3].data[8] << 24U));
  TEST_ASSERT_EQUAL_UINT16(CPMP_FAULT_CHANNEL_FLAG_CONFLICT,
                           (uint16_t) s_controlBusCtx.txBuffer[3].data[9]
                           | ((uint16_t) s_controlBusCtx.txBuffer[3].data[10] << 8U));
  TEST_ASSERT_EQUAL_UINT8(CPMP_SAFETY_ACTION_DARK,
                          s_controlBusCtx.txBuffer[3].data[41]);
  TEST_ASSERT_EQUAL_UINT8((uint8_t) FAULT_CODE_CONFLICT_GREEN_GREEN,
                          s_controlBusCtx.txBuffer[3].data[42]);
  TEST_ASSERT_EQUAL_UINT8(CPMP_CONFIG_STATE_APPLIED,
                          s_controlBusCtx.txBuffer[3].data[43]);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_link_service_applies_cp_config_and_reports_normal_when_matching);
  RUN_TEST(test_link_service_reports_flash_until_cp_config_is_applied);
  RUN_TEST(test_link_service_projects_fault_monitor_status_into_fault_frame);
  return UNITY_END();
}
