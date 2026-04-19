#include "unity.h"

#include <string.h>

#include "Domain/Services/MmiMaintenanceService.h"

typedef struct
{
  uint8_t modeOk;
  uint8_t factoryResetOk;
  uint8_t requestedMode;
  uint8_t factoryResetCount;
} TestControlCtx_t;

typedef struct
{
  uint8_t resetOk;
  uint8_t resetCount;
  ModuleBusDetectorClass_t detectorClass;
  uint8_t detectorNumber;
} TestModuleBusCtx_t;

static uint8_t TestRequestModeControl(void *ctx, uint8_t requestedControl)
{
  TestControlCtx_t *maintenance = (TestControlCtx_t *) ctx;

  maintenance->requestedMode = requestedControl;
  return maintenance->modeOk;
}

static uint8_t TestFactoryReset(void *ctx)
{
  TestControlCtx_t *maintenance = (TestControlCtx_t *) ctx;

  maintenance->factoryResetCount++;
  return maintenance->factoryResetOk;
}

static uint8_t TestReadSnapshot(void *ctx, ModuleBusSnapshot_t *snapshot)
{
  (void) ctx;
  (void) snapshot;
  return 0U;
}

static uint8_t TestSetConfigEpoch(void *ctx, uint16_t configEpoch)
{
  (void) ctx;
  (void) configEpoch;
  return 1U;
}

static uint8_t TestCommandDetectorReset(void *ctx,
                                        ModuleBusDetectorClass_t detectorClass,
                                        uint8_t detectorNumber)
{
  TestModuleBusCtx_t *moduleBus = (TestModuleBusCtx_t *) ctx;

  moduleBus->resetCount++;
  moduleBus->detectorClass = detectorClass;
  moduleBus->detectorNumber = detectorNumber;
  return moduleBus->resetOk;
}

void setUp(void)
{
}

void tearDown(void)
{
}

void test_MmiMaintenanceServiceDirectMethodsUseBoundPorts(void)
{
  TestControlCtx_t controlCtx;
  IControllerModeControlPort_t controllerModePort;
  IFactoryResetPort_t factoryResetPort;
  RelayControlService_t relayControlService;
  OutputTestService_t outputTestService;
  MmiMaintenanceService_t service;
  MmiMaintenanceOutputTestStatus_t status;

  (void) memset(&controlCtx, 0, sizeof(controlCtx));
  controlCtx.modeOk = 1U;
  controlCtx.factoryResetOk = 1U;

  controllerModePort.ctx = &controlCtx;
  controllerModePort.RequestModeControl = TestRequestModeControl;
  factoryResetPort.ctx = &controlCtx;
  factoryResetPort.RequestFactoryReset = TestFactoryReset;

  MmiMaintenanceServiceInit(&service);
  MmiMaintenanceServiceBind(&service,
                            &controllerModePort,
                            NULL,
                            NULL,
                            &factoryResetPort);
  RelayControlServiceInit(&relayControlService);
  MmiMaintenanceServiceBindRelayControlService(&service, &relayControlService);
  OutputTestServiceInit(&outputTestService);
  MmiMaintenanceServiceBindOutputTestService(&service, &outputTestService);

  TEST_ASSERT_TRUE(MmiMaintenanceServiceRequestModeControl(&service, 4U));
  TEST_ASSERT_EQUAL_UINT8(4U, controlCtx.requestedMode);

  TEST_ASSERT_TRUE(MmiMaintenanceServiceRequestRelayState(&service, 1U));
  TEST_ASSERT_TRUE(RelayControlServiceGetUserOutputPowerEnabled(
    &relayControlService));

  TEST_ASSERT_TRUE(MmiMaintenanceServiceFactoryReset(&service));
  TEST_ASSERT_EQUAL_UINT8(1U, controlCtx.factoryResetCount);

  TEST_ASSERT_TRUE(MmiMaintenanceServiceStartOutputTest(&service));
  TEST_ASSERT_TRUE(OutputTestServiceIsEnabled(&outputTestService));

  TEST_ASSERT_TRUE(MmiMaintenanceServiceSelectOutputTest(&service, 7U));
  TEST_ASSERT_EQUAL_UINT16((uint16_t) (1U << 6U),
                           OutputTestServiceGetForcedMask(&outputTestService));

  (void) memset(&status, 0, sizeof(status));
  TEST_ASSERT_TRUE(MmiMaintenanceServiceReadOutputTestStatus(&service, &status));
  TEST_ASSERT_EQUAL_UINT8(7U, status.outputNumber);
  TEST_ASSERT_EQUAL_UINT8(OUTPUT_DRIVER_ASPECT_GREEN, status.state);

  TEST_ASSERT_TRUE(MmiMaintenanceServiceStopOutputTest(&service));
  TEST_ASSERT_FALSE(OutputTestServiceIsEnabled(&outputTestService));
}

void test_MmiMaintenanceServiceExecuteMapsDetectorRelayAndFactoryCommands(void)
{
  TestControlCtx_t controlCtx;
  TestModuleBusCtx_t moduleBusCtx;
  IControllerModeControlPort_t controllerModePort;
  IFactoryResetPort_t factoryResetPort;
  IModuleBusPort_t moduleBusPort;
  RelayControlService_t relayControlService;
  OutputTestService_t outputTestService;
  MmiMaintenanceService_t service;
  MmiMaintenanceModeCommandV2_t modeCommand = { 3U };
  MmiMaintenanceDetectorResetCommandV2_t detectorReset = { 1U, 5U };
  MmiMaintenanceDetectorResetCommandV2_t badDetectorReset = { 9U, 1U };
  MmiMaintenanceRelayCommandV2_t relayCommand = { 0U };
  MmiMaintenanceOutputTestCommandV2_t outputSelect =
  {
    MMI_MAINTENANCE_OUTPUT_TEST_COMMAND_SELECT,
    11U,
    0U,
    0U
  };
  MmiMaintenanceFactoryResetCommandV2_t factoryReset = { 0xA5U, 0x5AU };
  MmiMaintenanceFactoryResetCommandV2_t badFactoryReset = { 0x00U, 0x00U };

  (void) memset(&controlCtx, 0, sizeof(controlCtx));
  (void) memset(&moduleBusCtx, 0, sizeof(moduleBusCtx));
  controlCtx.modeOk = 1U;
  controlCtx.factoryResetOk = 1U;
  moduleBusCtx.resetOk = 1U;

  controllerModePort.ctx = &controlCtx;
  controllerModePort.RequestModeControl = TestRequestModeControl;
  factoryResetPort.ctx = &controlCtx;
  factoryResetPort.RequestFactoryReset = TestFactoryReset;

  moduleBusPort.ctx = &moduleBusCtx;
  moduleBusPort.ReadSnapshot = TestReadSnapshot;
  moduleBusPort.SetConfigEpoch = TestSetConfigEpoch;
  moduleBusPort.CommandDetectorReset = TestCommandDetectorReset;

  MmiMaintenanceServiceInit(&service);
  MmiMaintenanceServiceBind(&service,
                            &controllerModePort,
                            &moduleBusPort,
                            NULL,
                            &factoryResetPort);
  RelayControlServiceInit(&relayControlService);
  MmiMaintenanceServiceBindRelayControlService(&service, &relayControlService);
  OutputTestServiceInit(&outputTestService);
  MmiMaintenanceServiceBindOutputTestService(&service, &outputTestService);

  TEST_ASSERT_EQUAL_UINT8(MMI_PROTOCOL_V2_STATUS_OK,
                          MmiMaintenanceServiceExecute(
                            &service,
                            MMI_PROTOCOL_V2_MAINTENANCE_RESOURCE_MODE_CONTROL,
                            (const uint8_t *) &modeCommand,
                            sizeof(modeCommand)));
  TEST_ASSERT_EQUAL_UINT8(3U, controlCtx.requestedMode);

  TEST_ASSERT_EQUAL_UINT8(MMI_PROTOCOL_V2_STATUS_OK,
                          MmiMaintenanceServiceExecute(
                            &service,
                            MMI_PROTOCOL_V2_MAINTENANCE_RESOURCE_DETECTOR_RESET,
                            (const uint8_t *) &detectorReset,
                            sizeof(detectorReset)));
  TEST_ASSERT_EQUAL_UINT8(1U, moduleBusCtx.resetCount);
  TEST_ASSERT_EQUAL_UINT32(MODULE_BUS_DETECTOR_CLASS_VEHICLE,
                           moduleBusCtx.detectorClass);
  TEST_ASSERT_EQUAL_UINT8(5U, moduleBusCtx.detectorNumber);

  TEST_ASSERT_EQUAL_UINT8(MMI_PROTOCOL_V2_STATUS_INVALID_VALUE,
                          MmiMaintenanceServiceExecute(
                            &service,
                            MMI_PROTOCOL_V2_MAINTENANCE_RESOURCE_DETECTOR_RESET,
                            (const uint8_t *) &badDetectorReset,
                            sizeof(badDetectorReset)));

  TEST_ASSERT_EQUAL_UINT8(MMI_PROTOCOL_V2_STATUS_OK,
                          MmiMaintenanceServiceExecute(
                            &service,
                            MMI_PROTOCOL_V2_MAINTENANCE_RESOURCE_RELAY_COMMAND,
                            (const uint8_t *) &relayCommand,
                            sizeof(relayCommand)));
  TEST_ASSERT_FALSE(RelayControlServiceGetUserOutputPowerEnabled(
    &relayControlService));

  TEST_ASSERT_EQUAL_UINT8(MMI_PROTOCOL_V2_STATUS_OK,
                          MmiMaintenanceServiceExecute(
                            &service,
                            MMI_PROTOCOL_V2_MAINTENANCE_RESOURCE_OUTPUT_TEST,
                            (const uint8_t *) &outputSelect,
                            sizeof(outputSelect)));
  TEST_ASSERT_EQUAL_UINT16((uint16_t) (1U << 10U),
                           OutputTestServiceGetForcedMask(&outputTestService));

  TEST_ASSERT_EQUAL_UINT8(MMI_PROTOCOL_V2_STATUS_INVALID_VALUE,
                          MmiMaintenanceServiceExecute(
                            &service,
                            MMI_PROTOCOL_V2_MAINTENANCE_RESOURCE_FACTORY_RESET,
                            (const uint8_t *) &badFactoryReset,
                            sizeof(badFactoryReset)));
  TEST_ASSERT_EQUAL_UINT8(MMI_PROTOCOL_V2_STATUS_OK,
                          MmiMaintenanceServiceExecute(
                            &service,
                            MMI_PROTOCOL_V2_MAINTENANCE_RESOURCE_FACTORY_RESET,
                            (const uint8_t *) &factoryReset,
                            sizeof(factoryReset)));
  TEST_ASSERT_EQUAL_UINT8(1U, controlCtx.factoryResetCount);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_MmiMaintenanceServiceDirectMethodsUseBoundPorts);
  RUN_TEST(test_MmiMaintenanceServiceExecuteMapsDetectorRelayAndFactoryCommands);
  return UNITY_END();
}
