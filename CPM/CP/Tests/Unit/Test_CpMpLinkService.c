/*
 * Tests/Unit/Test_CpMpLinkService.c
 *
 * CP-side private control-link coverage: outbound heartbeat/config frames and
 * inbound MP safety / unit-alarm projection.
 */
#include "unity.h"

#include <string.h>

#include "Domain/Intersection/CpMpLinkService.h"
#include "Domain/Intersection/IntersectionController.h"

typedef struct
{
  ControlBusFrame_t txFrames[64];
  uint8_t txCount;
  ControlBusRxCallback_t rxCallback;
  void *rxCallbackCtx;
} FakeControlBusCtx_t;

typedef struct
{
  uint8_t forceAllRed;
  MmuControlAction_t safetyAction;
} FakeMmuCtx_t;

typedef struct
{
  uint8_t count;
  uint8_t eventCode;
  uint8_t eventParam;
  uint16_t eventShortParam;
  uint32_t eventLongParam;
} FakeLogCtx_t;

static CpMpLinkService_t s_service;
static ConfigurationService_t s_configurationService;
static IntersectionController_t s_controller;
static RelayControlService_t s_relayControlService;
static FakeControlBusCtx_t s_controlBusCtx;
static FakeMmuCtx_t s_mmuCtx;
static FakeLogCtx_t s_logCtx;
static IControlBusPort_t s_controlBusPort;
static IMmuPort_t s_mmuPort;
static ILogEventPort_t s_logPort;

static uint8_t FakeControlBusSendFrame(void *ctx, const ControlBusFrame_t *frame)
{
  FakeControlBusCtx_t *bus = (FakeControlBusCtx_t *) ctx;

  if ((bus == NULL) || (frame == NULL) || (bus->txCount >= 64U))
  {
    return 0U;
  }

  bus->txFrames[bus->txCount++] = *frame;

  return 1U;
}

static uint8_t FakeControlBusRegisterRx(void *ctx,
                                        ControlBusRxCallback_t cb,
                                        void *cbCtx)
{
  FakeControlBusCtx_t *bus = (FakeControlBusCtx_t *) ctx;

  if (bus == NULL)
  {
    return 0U;
  }

  bus->rxCallback = cb;
  bus->rxCallbackCtx = cbCtx;

  return 1U;
}

static void FakeControlBusInjectRx(const ControlBusFrame_t *frame)
{
  if ((frame != NULL) && (s_controlBusCtx.rxCallback != NULL))
  {
    s_controlBusCtx.rxCallback(s_controlBusCtx.rxCallbackCtx, frame);
  }
}

static uint8_t FakeMmuSetForceAllRed(void *ctx, uint8_t forceAllRed)
{
  FakeMmuCtx_t *mmu = (FakeMmuCtx_t *) ctx;

  if (mmu == NULL)
  {
    return 0U;
  }

  mmu->forceAllRed = (uint8_t) (forceAllRed != 0U);

  return 1U;
}

static uint8_t FakeMmuSetSafetyAction(void *ctx, MmuControlAction_t action)
{
  FakeMmuCtx_t *mmu = (FakeMmuCtx_t *) ctx;

  if (mmu == NULL)
  {
    return 0U;
  }

  mmu->safetyAction = action;

  return 1U;
}

static uint8_t FakeMmuFilter(void *ctx,
                             const OutputDriverImage_t *requested,
                             OutputDriverImage_t *approved)
{
  FakeMmuCtx_t *mmu = (FakeMmuCtx_t *) ctx;
  uint8_t channelIndex;

  if ((mmu == NULL) || (requested == NULL) || (approved == NULL))
  {
    return 0U;
  }

  *approved = *requested;

  if (mmu->forceAllRed != 0U)
  {
    for (channelIndex = 0U; channelIndex < INTERSECTION_CHANNEL_COUNT_MAX;
         channelIndex++)
    {
      approved->channels[channelIndex] = OUTPUT_DRIVER_ASPECT_RED;
    }
  }
  else if (mmu->safetyAction == MMU_CONTROL_ACTION_DARK)
  {
    for (channelIndex = 0U; channelIndex < INTERSECTION_CHANNEL_COUNT_MAX;
         channelIndex++)
    {
      approved->channels[channelIndex] = OUTPUT_DRIVER_ASPECT_DARK;
    }
  }

  return 1U;
}

static uint8_t FakeLogAppend(void *ctx,
                             uint8_t eventCode,
                             uint8_t eventParam,
                             uint16_t eventShortParam,
                             uint32_t eventLongParam)
{
  FakeLogCtx_t *logCtx = (FakeLogCtx_t *) ctx;

  if (logCtx == NULL)
  {
    return 0U;
  }

  logCtx->count++;
  logCtx->eventCode = eventCode;
  logCtx->eventParam = eventParam;
  logCtx->eventShortParam = eventShortParam;
  logCtx->eventLongParam = eventLongParam;

  return 1U;
}

static void ReplicateSequencePlansFromBase(IntersectionConfig_t *config)
{
  uint8_t sequenceIndex;
  uint8_t ringIndex;

  for (sequenceIndex = 1U;
       sequenceIndex < INTERSECTION_SEQUENCE_COUNT_MAX;
       sequenceIndex++)
  {
    for (ringIndex = 0U; ringIndex < INTERSECTION_RING_COUNT_MAX; ringIndex++)
    {
      config->sequencePlans[sequenceIndex][ringIndex] = config->rings[ringIndex];
    }
  }
}

static IntersectionConfig_t MakeConfig(void)
{
  IntersectionConfig_t config;
  uint8_t phaseIndex;
  uint8_t detectorIndex;

  IntersectionConfigInitDefaults(&config);
  config.phaseCount = 4U;
  config.ringCount = 2U;
  config.barrierCount = 2U;

  for (phaseIndex = 0U; phaseIndex < config.phaseCount; ++phaseIndex)
  {
    config.phases[phaseIndex].phaseOptions = PHASE_OPTIONS_ENABLED;
    config.phases[phaseIndex].ring = (phaseIndex < 2U) ? 0U : 1U;
    config.phases[phaseIndex].minGreenDs = 50U;
    config.phases[phaseIndex].yellowChangeDs = 30U;
    config.phases[phaseIndex].redClearDs = 20U;
    config.phases[phaseIndex].walkSeconds = 0U;
    config.phases[phaseIndex].pedClearSeconds = 0U;
  }

  config.phases[0].concurrency.length = 1U;
  config.phases[0].concurrency.values[0] = 3U;
  config.rings[0].phaseCount = 2U;
  config.rings[0].barrierPhaseCount = 1U;
  config.rings[0].phaseOrder[0] = 0U;
  config.rings[0].phaseOrder[1] = 1U;
  config.rings[1].phaseCount = 2U;
  config.rings[1].barrierPhaseCount = 1U;
  config.rings[1].phaseOrder[0] = 2U;
  config.rings[1].phaseOrder[1] = 3U;
  ReplicateSequencePlansFromBase(&config);
  config.vehicleDetectors[0].callPhase = 2U;
  config.vehicleDetectors[1].callPhase = 1U;
  config.pedestrianDetectors[0].callPhase = 1U;
  config.pedestrianDetectors[1].callPhase = 2U;

  for (detectorIndex = config.phaseCount;
       detectorIndex < INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX;
       detectorIndex++)
  {
    config.vehicleDetectors[detectorIndex].callPhase = 0U;
  }

  for (detectorIndex = config.phaseCount;
       detectorIndex < INTERSECTION_PED_INPUT_COUNT_MAX;
       detectorIndex++)
  {
    config.pedestrianDetectors[detectorIndex].callPhase = 0U;
  }

  config.inputMapping.preemptInputs[0] = 2U;
  config.inputMapping.preemptControls[0] = 1U;
  config.channels[0].controlType =
    INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_VEHICLE;
  config.channels[0].controlSource = 1U;
  config.channels[0].flashMask = 0x04U;
  config.channels[1].controlType = INTERSECTION_CHANNEL_CONTROL_TYPE_OVERLAP;
  config.channels[1].controlSource = 1U;
  config.channels[2].controlType =
    INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_VEHICLE;
  config.channels[2].controlSource = 2U;
  config.overlaps[0].type = (uint8_t) INTERSECTION_OVERLAP_TYPE_NORMAL;
  config.overlaps[0].includedPhases.length = 1U;
  config.overlaps[0].includedPhases.values[0] = 3U;
  config.overlaps[0].trailYellowDs = 4U;
  config.overlaps[0].trailRedDs = 6U;
  config.ioMap.outputs[0].deviceType = (uint8_t) INTERSECTION_IO_MAP_DEVICE_FIO;
  config.ioMap.outputs[0].devicePin = 1U;
  config.ioMap.outputs[0].function =
    (uint8_t) INTERSECTION_IO_MAP_OUTPUT_FUNCTION_CHANNEL_GREEN;
  config.ioMap.outputs[0].functionIndex = 1U;
  config.unit.startUpFlashSeconds = 5U;
  config.unit.startUpFlashMode =
    INTERSECTION_UNIT_STARTUP_FLASH_MODE_ALL_RED_CONTROLLER_FLASH;
  config.unit.failureFlashPeriodDs =
    INTERSECTION_UNIT_FAILURE_FLASH_PERIOD_2000MS_DS;

  return config;
}

static void DrainConfigTransferToIdle(void)
{
  uint8_t stepCount;

  for (stepCount = 0U; stepCount < 32U; ++stepCount)
  {
    CpMpLinkServiceStep(&s_service);
    if (s_service.txState == CPMP_TX_STATE_IDLE)
    {
      return;
    }
  }

  TEST_FAIL_MESSAGE("config transfer did not reach idle");
}

void setUp(void)
{
  IntersectionConfig_t config = MakeConfig();

  memset(&s_service, 0, sizeof(s_service));
  memset(&s_configurationService, 0, sizeof(s_configurationService));
  RelayControlServiceInit(&s_relayControlService);
  memset(&s_controlBusCtx, 0, sizeof(s_controlBusCtx));
  memset(&s_mmuCtx, 0, sizeof(s_mmuCtx));
  memset(&s_logCtx, 0, sizeof(s_logCtx));

  s_configurationService.activeConfig = config;
  s_configurationService.activeGeneration = 7U;

  s_mmuPort.ctx = &s_mmuCtx;
  s_mmuPort.SetForceAllRed = FakeMmuSetForceAllRed;
  s_mmuPort.SetSafetyAction = FakeMmuSetSafetyAction;
  s_mmuPort.FilterOutputImage = FakeMmuFilter;

  IntersectionControllerInit(&s_controller);
  s_controller.mmuPort = &s_mmuPort;

  s_controlBusPort.ctx = &s_controlBusCtx;
  s_controlBusPort.SendFrame = FakeControlBusSendFrame;
  s_controlBusPort.RegisterRxCallback = FakeControlBusRegisterRx;

  s_logPort.ctx = &s_logCtx;
  s_logPort.Append = FakeLogAppend;

  CpMpLinkServiceInit(&s_service,
                      &s_controlBusPort,
                      &s_logPort,
                      &s_configurationService,
                      &s_controller,
                      &s_relayControlService);
}

void tearDown(void)
{
}

void test_link_service_sends_heartbeat_and_config_transfer_frames(void)
{
  RelayControlServiceSetLocalState(&s_relayControlService,
                                   1U,
                                   0U,
                                   1U,
                                   MMU_CONTROL_ACTION_NORMAL);
  TEST_ASSERT_EQUAL_UINT8(0U, s_controlBusCtx.txCount);

  CpMpLinkServiceStep(&s_service);
  TEST_ASSERT_EQUAL_UINT8(2U, s_controlBusCtx.txCount);
  TEST_ASSERT_EQUAL_UINT16(CPMP_FRAME_ID_CP_HEARTBEAT,
                           s_controlBusCtx.txFrames[0].standardId);
  TEST_ASSERT_EQUAL_UINT8(1U, s_controlBusCtx.txFrames[0].data[12]);
  TEST_ASSERT_EQUAL_UINT16(CPMP_FRAME_ID_CP_CFG_BEGIN,
                           s_controlBusCtx.txFrames[1].standardId);

  CpMpLinkServiceStep(&s_service);
  TEST_ASSERT_EQUAL_UINT8(4U, s_controlBusCtx.txCount);
  TEST_ASSERT_EQUAL_UINT16(CPMP_FRAME_ID_CP_HEARTBEAT,
                           s_controlBusCtx.txFrames[2].standardId);
  TEST_ASSERT_EQUAL_UINT16(CPMP_FRAME_ID_CP_CFG_CHUNK,
                           s_controlBusCtx.txFrames[3].standardId);
  TEST_ASSERT_EQUAL_UINT8(CPMP_PROTOCOL_VERSION,
                          s_controlBusCtx.txFrames[3].data[0]);
}

void test_link_service_builds_explicit_channel_monitoring_profile(void)
{
  TEST_ASSERT_EQUAL_UINT32((uint32_t) (1UL << 2U),
                           s_service.configImage.channelConflictMask[0]);
  TEST_ASSERT_EQUAL_UINT32((uint32_t) (1UL << 2U),
                           s_service.configImage.channelConflictMask[1]);
  TEST_ASSERT_EQUAL_UINT32((uint32_t) ((1UL << 0U) | (1UL << 1U)),
                           s_service.configImage.channelConflictMask[2]);
  TEST_ASSERT_EQUAL_UINT8(30U, s_service.configImage.channelMinYellowDs[0]);
  TEST_ASSERT_EQUAL_UINT8(4U, s_service.configImage.channelMinYellowDs[1]);
  TEST_ASSERT_EQUAL_UINT8(20U, s_service.configImage.channelRedClearDs[0]);
  TEST_ASSERT_EQUAL_UINT8(6U, s_service.configImage.channelRedClearDs[1]);
  TEST_ASSERT_EQUAL_UINT8(0x04U, s_service.configImage.channelFlashMask[0]);
  TEST_ASSERT_EQUAL_UINT8(INTERSECTION_UNIT_FAILURE_FLASH_PERIOD_2000MS_DS,
                          s_service.configImage.failureFlashPeriodDs);
}

void test_link_service_applies_mp_safety_and_caches_fault_status(void)
{
  ControlBusFrame_t frame;
  uint16_t activeSetId = ConfigurationServiceGetActiveSetId(&s_configurationService);
  CpMpFaultStatusImage_t faultStatus;

  memset(&frame, 0, sizeof(frame));
  frame.standardId = CPMP_FRAME_ID_MP_SAFETY;
  frame.length = 2U;
  frame.data[0] = CPMP_PROTOCOL_VERSION;
  frame.data[1] = CPMP_SAFETY_ACTION_DARK;
  FakeControlBusInjectRx(&frame);

  CpMpLinkServiceStep(&s_service);
  TEST_ASSERT_EQUAL(MMU_CONTROL_ACTION_FLASH, s_mmuCtx.safetyAction);

  frame.standardId = CPMP_FRAME_ID_MP_HEARTBEAT;
  frame.length = 10U;
  frame.data[0] = CPMP_PROTOCOL_VERSION;
  frame.data[1] = CPMP_CONFIG_STATE_APPLIED;
  frame.data[2] = CPMP_SAFETY_ACTION_DARK;
  frame.data[4] = (uint8_t) (activeSetId & 0xFFU);
  frame.data[5] = (uint8_t) ((activeSetId >> 8U) & 0xFFU);
  frame.data[6] = 7U;
  frame.data[7] = 0U;
  frame.data[8] = 0U;
  frame.data[9] = 0U;
  FakeControlBusInjectRx(&frame);

  CpMpLinkServiceStep(&s_service);
  TEST_ASSERT_EQUAL(MMU_CONTROL_ACTION_DARK, s_mmuCtx.safetyAction);

  frame.standardId = CPMP_FRAME_ID_MP_FAULTS;
  frame.length = 48U;
  frame.data[0] = CPMP_PROTOCOL_VERSION;
  frame.data[1] = 0x78U;
  frame.data[2] = 0x56U;
  frame.data[3] = 0x34U;
  frame.data[4] = 0x12U;
  frame.data[5] = 0xEFU;
  frame.data[6] = 0xCDU;
  frame.data[7] = 0xABU;
  frame.data[8] = 0x90U;
  frame.data[9] = 0x01U;
  frame.data[17] = 0x02U;
  frame.data[45] = CPMP_SAFETY_ACTION_DARK;
  frame.data[46] = 0x2AU;
  frame.data[47] = CPMP_CONFIG_STATE_APPLIED;
  FakeControlBusInjectRx(&frame);

  TEST_ASSERT_TRUE(CpMpLinkServiceGetFaultStatus(&s_service, &faultStatus));
  TEST_ASSERT_EQUAL_HEX32(0x12345678UL, faultStatus.sequence);
  TEST_ASSERT_EQUAL_HEX32(0x90ABCDEFUL, faultStatus.globalFlags);
  TEST_ASSERT_EQUAL_HEX16(CPMP_FAULT_CHANNEL_FLAG_CONFLICT,
                          faultStatus.channelFlags[0]);
  TEST_ASSERT_EQUAL_HEX16(CPMP_FAULT_CHANNEL_FLAG_RED_FAIL,
                          faultStatus.channelFlags[1]);
  TEST_ASSERT_EQUAL_UINT8(CPMP_SAFETY_ACTION_DARK, faultStatus.safetyAction);
  TEST_ASSERT_EQUAL_UINT8(0x2AU, faultStatus.safetyReasonCode);
  TEST_ASSERT_EQUAL_UINT8(CPMP_CONFIG_STATE_APPLIED, faultStatus.configState);
  TEST_ASSERT_EQUAL_UINT8(0x2AU,
                          CpMpLinkServiceGetLastSafetyReasonCode(&s_service));
}

void test_link_service_acks_and_logs_mp_fault_events_once(void)
{
  ControlBusFrame_t frame;

  memset(&frame, 0, sizeof(frame));
  frame.standardId = CPMP_FRAME_ID_MP_EVENT;
  frame.length = 17U;
  frame.data[0] = CPMP_PROTOCOL_VERSION;
  frame.data[1] = 3U;
  frame.data[5] = 9U;
  frame.data[9] = 0xAAU;
  frame.data[10] = 0xBBU;
  frame.data[13] = 0x34U;
  frame.data[14] = 0x12U;
  frame.data[15] = 42U;
  frame.data[16] = 3U;

  FakeControlBusInjectRx(&frame);
  TEST_ASSERT_EQUAL_UINT8(1U, s_logCtx.count);
  TEST_ASSERT_EQUAL_UINT8(120U, s_logCtx.eventCode);
  TEST_ASSERT_EQUAL_UINT8((uint8_t) ((3U << 6U) | 42U), s_logCtx.eventParam);
  TEST_ASSERT_EQUAL_UINT16(0x1234U, s_logCtx.eventShortParam);
  TEST_ASSERT_EQUAL_UINT32(0x0000BBAAUL, s_logCtx.eventLongParam);
  TEST_ASSERT_EQUAL_UINT16(CPMP_FRAME_ID_CP_EVENT_ACK,
                           s_controlBusCtx.txFrames[0].standardId);

  FakeControlBusInjectRx(&frame);
  TEST_ASSERT_EQUAL_UINT8(1U, s_logCtx.count);
  TEST_ASSERT_EQUAL_UINT8(2U, s_controlBusCtx.txCount);
}

void test_link_service_keeps_flash_until_mp_reports_matching_config(void)
{
  ControlBusFrame_t frame;
  uint16_t activeSetId = ConfigurationServiceGetActiveSetId(&s_configurationService);

  memset(&frame, 0, sizeof(frame));
  frame.standardId = CPMP_FRAME_ID_MP_HEARTBEAT;
  frame.length = 10U;
  frame.data[0] = CPMP_PROTOCOL_VERSION;
  frame.data[1] = CPMP_CONFIG_STATE_APPLIED;
  frame.data[2] = CPMP_SAFETY_ACTION_NORMAL;
  frame.data[4] = 1U;
  frame.data[5] = 0U;
  frame.data[6] = 7U;
  frame.data[7] = 0U;
  frame.data[8] = 0U;
  frame.data[9] = 0U;
  FakeControlBusInjectRx(&frame);

  CpMpLinkServiceStep(&s_service);
  TEST_ASSERT_EQUAL(MMU_CONTROL_ACTION_FLASH, s_mmuCtx.safetyAction);

  frame.data[4] = (uint8_t) (activeSetId & 0xFFU);
  frame.data[5] = (uint8_t) ((activeSetId >> 8U) & 0xFFU);
  FakeControlBusInjectRx(&frame);

  CpMpLinkServiceStep(&s_service);
  TEST_ASSERT_EQUAL(MMU_CONTROL_ACTION_NORMAL, s_mmuCtx.safetyAction);
}

void test_link_service_reports_peer_and_authority_state(void)
{
  ControlBusFrame_t frame;
  uint16_t activeSetId = ConfigurationServiceGetActiveSetId(&s_configurationService);

  RelayControlServiceSetLocalState(&s_relayControlService,
                                   1U,
                                   0U,
                                   1U,
                                   MMU_CONTROL_ACTION_NORMAL);
  TEST_ASSERT_FALSE(CpMpLinkServicePeerHealthy(&s_service));
  TEST_ASSERT_FALSE(CpMpLinkServiceAuthorityReady(&s_service));
  TEST_ASSERT_EQUAL_UINT8(CPMP_SAFETY_ACTION_FLASH,
                          CpMpLinkServiceGetEffectiveSafetyAction(&s_service));

  memset(&frame, 0, sizeof(frame));
  frame.standardId = CPMP_FRAME_ID_MP_HEARTBEAT;
  frame.length = 10U;
  frame.data[0] = CPMP_PROTOCOL_VERSION;
  frame.data[1] = CPMP_CONFIG_STATE_APPLIED;
  frame.data[2] = CPMP_SAFETY_ACTION_NORMAL;
  frame.data[3] = 1U;
  frame.data[4] = (uint8_t) (activeSetId & 0xFFU);
  frame.data[5] = (uint8_t) ((activeSetId >> 8U) & 0xFFU);
  frame.data[6] = 7U;
  FakeControlBusInjectRx(&frame);

  CpMpLinkServiceStep(&s_service);

  TEST_ASSERT_TRUE(CpMpLinkServicePeerHealthy(&s_service));
  TEST_ASSERT_TRUE(CpMpLinkServiceAuthorityReady(&s_service));
  TEST_ASSERT_EQUAL_UINT8(CPMP_SAFETY_ACTION_NORMAL,
                          CpMpLinkServiceGetEffectiveSafetyAction(&s_service));
  TEST_ASSERT_EQUAL_UINT8(1U,
                          RelayControlServiceGetPeerPermitValid(
                            &s_relayControlService));
  TEST_ASSERT_EQUAL_UINT8(1U,
                          RelayControlServiceGetPeerPermitOutputPower(
                            &s_relayControlService));
  TEST_ASSERT_EQUAL_UINT8(1U,
                          RelayControlServiceGetEffectivePermitOutputPower(
                            &s_relayControlService));
}

void test_link_service_falls_back_to_flash_on_mp_timeout(void)
{
  ControlBusFrame_t frame;
  uint16_t activeSetId = ConfigurationServiceGetActiveSetId(&s_configurationService);
  uint8_t i;

  RelayControlServiceSetLocalState(&s_relayControlService,
                                   1U,
                                   0U,
                                   1U,
                                   MMU_CONTROL_ACTION_NORMAL);
  memset(&frame, 0, sizeof(frame));
  frame.standardId = CPMP_FRAME_ID_MP_HEARTBEAT;
  frame.length = 10U;
  frame.data[0] = CPMP_PROTOCOL_VERSION;
  frame.data[1] = CPMP_CONFIG_STATE_APPLIED;
  frame.data[2] = CPMP_SAFETY_ACTION_NORMAL;
  frame.data[3] = 1U;
  frame.data[4] = (uint8_t) (activeSetId & 0xFFU);
  frame.data[5] = (uint8_t) ((activeSetId >> 8U) & 0xFFU);
  frame.data[6] = 7U;
  frame.data[7] = 0U;
  frame.data[8] = 0U;
  frame.data[9] = 0U;
  FakeControlBusInjectRx(&frame);

  CpMpLinkServiceStep(&s_service);
  TEST_ASSERT_EQUAL(MMU_CONTROL_ACTION_NORMAL, s_mmuCtx.safetyAction);
  TEST_ASSERT_EQUAL_UINT8(1U,
                          RelayControlServiceGetEffectivePermitOutputPower(
                            &s_relayControlService));

  for (i = 0U; i <= CPMP_PEER_TIMEOUT_TICKS; ++i)
  {
    CpMpLinkServiceStep(&s_service);
  }

  TEST_ASSERT_EQUAL(MMU_CONTROL_ACTION_FLASH, s_mmuCtx.safetyAction);
  TEST_ASSERT_EQUAL_UINT8(1U,
                          RelayControlServiceGetLocalPermitOutputPower(
                            &s_relayControlService));
  TEST_ASSERT_EQUAL_UINT8(0U,
                          RelayControlServiceGetPeerPermitValid(
                            &s_relayControlService));
  TEST_ASSERT_EQUAL_UINT8(0U,
                          RelayControlServiceGetEffectivePermitOutputPower(
                            &s_relayControlService));
}

void test_link_service_restarts_config_transfer_until_mp_matches_active_config(void)
{
  uint8_t txCountAfterFirstTransfer;

  DrainConfigTransferToIdle();
  txCountAfterFirstTransfer = s_controlBusCtx.txCount;

  CpMpLinkServiceStep(&s_service);
  TEST_ASSERT_EQUAL_UINT8((uint8_t) (txCountAfterFirstTransfer + 2U),
                          s_controlBusCtx.txCount);
  TEST_ASSERT_EQUAL_UINT16(CPMP_FRAME_ID_CP_HEARTBEAT,
                           s_controlBusCtx.txFrames[txCountAfterFirstTransfer]
                             .standardId);
  TEST_ASSERT_EQUAL_UINT16(
    CPMP_FRAME_ID_CP_CFG_BEGIN,
    s_controlBusCtx.txFrames[txCountAfterFirstTransfer + 1U].standardId);
}

void test_link_service_restarts_config_transfer_after_mp_reports_invalid_config(void)
{
  ControlBusFrame_t frame;
  uint16_t activeSetId = ConfigurationServiceGetActiveSetId(
    &s_configurationService);

  memset(&frame, 0, sizeof(frame));
  frame.standardId = CPMP_FRAME_ID_MP_HEARTBEAT;
  frame.length = 10U;
  frame.data[0] = CPMP_PROTOCOL_VERSION;
  frame.data[1] = CPMP_CONFIG_STATE_APPLIED;
  frame.data[2] = CPMP_SAFETY_ACTION_NORMAL;
  frame.data[4] = (uint8_t) (activeSetId & 0xFFU);
  frame.data[5] = (uint8_t) ((activeSetId >> 8U) & 0xFFU);
  frame.data[6] = 7U;
  FakeControlBusInjectRx(&frame);

  DrainConfigTransferToIdle();
  s_controlBusCtx.txCount = 0U;
  FakeControlBusInjectRx(&frame);

  CpMpLinkServiceStep(&s_service);
  TEST_ASSERT_EQUAL_UINT8(1U, s_controlBusCtx.txCount);
  TEST_ASSERT_EQUAL_UINT16(CPMP_FRAME_ID_CP_HEARTBEAT,
                           s_controlBusCtx.txFrames[0].standardId);

  memset(&frame, 0, sizeof(frame));
  frame.standardId = CPMP_FRAME_ID_MP_HEARTBEAT;
  frame.length = 10U;
  frame.data[0] = CPMP_PROTOCOL_VERSION;
  frame.data[1] = CPMP_CONFIG_STATE_INVALID;
  frame.data[2] = CPMP_SAFETY_ACTION_FLASH;
  frame.data[4] = (uint8_t) (activeSetId & 0xFFU);
  frame.data[5] = (uint8_t) ((activeSetId >> 8U) & 0xFFU);
  frame.data[6] = 7U;
  FakeControlBusInjectRx(&frame);

  s_controlBusCtx.txCount = 0U;
  CpMpLinkServiceStep(&s_service);
  TEST_ASSERT_EQUAL_UINT8(2U, s_controlBusCtx.txCount);
  TEST_ASSERT_EQUAL_UINT16(CPMP_FRAME_ID_CP_HEARTBEAT,
                           s_controlBusCtx.txFrames[0].standardId);
  TEST_ASSERT_EQUAL_UINT16(CPMP_FRAME_ID_CP_CFG_BEGIN,
                           s_controlBusCtx.txFrames[1].standardId);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_link_service_sends_heartbeat_and_config_transfer_frames);
  RUN_TEST(test_link_service_builds_explicit_channel_monitoring_profile);
  RUN_TEST(test_link_service_applies_mp_safety_and_caches_fault_status);
  RUN_TEST(test_link_service_acks_and_logs_mp_fault_events_once);
  RUN_TEST(test_link_service_keeps_flash_until_mp_reports_matching_config);
  RUN_TEST(test_link_service_reports_peer_and_authority_state);
  RUN_TEST(test_link_service_falls_back_to_flash_on_mp_timeout);
  RUN_TEST(
    test_link_service_restarts_config_transfer_until_mp_matches_active_config);
  RUN_TEST(
    test_link_service_restarts_config_transfer_after_mp_reports_invalid_config);
  return UNITY_END();
}
