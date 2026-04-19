#include "unity.h"

#include <string.h>

#include "Domain/Services/MmiMaintenanceService.h"

typedef struct
{
  uint8_t modeOk;
  uint8_t relayOk;
  uint8_t factoryResetOk;
  uint8_t iapOk;
  uint8_t startOk;
  uint8_t stopOk;
  uint8_t selectOk;
  uint8_t readStatusOk;
  uint8_t requestedMode;
  uint8_t requestedRelayState;
  uint8_t selectedOutput;
  uint8_t factoryResetCount;
  uint8_t iapCount;
  uint8_t startCount;
  uint8_t stopCount;
  MmiMaintenanceOutputTestStatus_t status;
} TestMaintenanceCtx_t;

typedef struct
{
  uint8_t resetOk;
  uint8_t resetCount;
  ModuleBusDetectorClass_t detectorClass;
  uint8_t detectorNumber;
} TestModuleBusCtx_t;

static uint8_t TestRequestModeControl(void *ctx, uint8_t requestedControl)
{
  TestMaintenanceCtx_t *maintenance = (TestMaintenanceCtx_t *) ctx;

  maintenance->requestedMode = requestedControl;
  return maintenance->modeOk;
}

static uint8_t TestRequestRelayState(void *ctx, uint8_t requestedState)
{
  TestMaintenanceCtx_t *maintenance = (TestMaintenanceCtx_t *) ctx;

  maintenance->requestedRelayState = requestedState;
  return maintenance->relayOk;
}

static uint8_t TestFactoryReset(void *ctx)
{
  TestMaintenanceCtx_t *maintenance = (TestMaintenanceCtx_t *) ctx;

  maintenance->factoryResetCount++;
  return maintenance->factoryResetOk;
}

static uint8_t TestEnterIapMode(void *ctx)
{
  TestMaintenanceCtx_t *maintenance = (TestMaintenanceCtx_t *) ctx;

  maintenance->iapCount++;
  return maintenance->iapOk;
}

static uint8_t TestStartOutputTest(void *ctx)
{
  TestMaintenanceCtx_t *maintenance = (TestMaintenanceCtx_t *) ctx;

  maintenance->startCount++;
  return maintenance->startOk;
}

static uint8_t TestStopOutputTest(void *ctx)
{
  TestMaintenanceCtx_t *maintenance = (TestMaintenanceCtx_t *) ctx;

  maintenance->stopCount++;
  return maintenance->stopOk;
}

static uint8_t TestSelectOutputTest(void *ctx, uint8_t outputNumber)
{
  TestMaintenanceCtx_t *maintenance = (TestMaintenanceCtx_t *) ctx;

  maintenance->selectedOutput = outputNumber;
  return maintenance->selectOk;
}

static uint8_t TestReadOutputTestStatus(void *ctx,
                                        MmiMaintenanceOutputTestStatus_t *status)
{
  TestMaintenanceCtx_t *maintenance = (TestMaintenanceCtx_t *) ctx;

  if ((maintenance->readStatusOk == 0U) || (status == NULL))
  {
    return 0U;
  }

  *status = maintenance->status;
  return 1U;
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
  TestMaintenanceCtx_t maintenanceCtx;
  IMmiMaintenancePort_t maintenancePort;
  MmiMaintenanceService_t service;
  MmiMaintenanceOutputTestStatus_t status;

  (void) memset(&maintenanceCtx, 0, sizeof(maintenanceCtx));
  maintenanceCtx.modeOk = 1U;
  maintenanceCtx.relayOk = 1U;
  maintenanceCtx.factoryResetOk = 1U;
  maintenanceCtx.iapOk = 1U;
  maintenanceCtx.startOk = 1U;
  maintenanceCtx.stopOk = 1U;
  maintenanceCtx.selectOk = 1U;
  maintenanceCtx.readStatusOk = 1U;
  maintenanceCtx.status.outputNumber = 9U;
  maintenanceCtx.status.currentNow = 123U;

  maintenancePort.ctx = &maintenanceCtx;
  maintenancePort.RequestModeControl = TestRequestModeControl;
  maintenancePort.RequestRelayState = TestRequestRelayState;
  maintenancePort.FactoryReset = TestFactoryReset;
  maintenancePort.EnterIapMode = TestEnterIapMode;
  maintenancePort.StartOutputTest = TestStartOutputTest;
  maintenancePort.StopOutputTest = TestStopOutputTest;
  maintenancePort.SelectOutputTest = TestSelectOutputTest;
  maintenancePort.ReadOutputTestStatus = TestReadOutputTestStatus;

  MmiMaintenanceServiceInit(&service);
  MmiMaintenanceServiceBind(&service, &maintenancePort, NULL, NULL);

  TEST_ASSERT_TRUE(MmiMaintenanceServiceRequestModeControl(&service, 4U));
  TEST_ASSERT_EQUAL_UINT8(4U, maintenanceCtx.requestedMode);

  TEST_ASSERT_TRUE(MmiMaintenanceServiceRequestRelayState(&service, 1U));
  TEST_ASSERT_EQUAL_UINT8(1U, maintenanceCtx.requestedRelayState);

  TEST_ASSERT_TRUE(MmiMaintenanceServiceFactoryReset(&service));
  TEST_ASSERT_EQUAL_UINT8(1U, maintenanceCtx.factoryResetCount);

  TEST_ASSERT_TRUE(MmiMaintenanceServiceEnterIapMode(&service));
  TEST_ASSERT_EQUAL_UINT8(1U, maintenanceCtx.iapCount);

  TEST_ASSERT_TRUE(MmiMaintenanceServiceStartOutputTest(&service));
  TEST_ASSERT_EQUAL_UINT8(1U, maintenanceCtx.startCount);

  TEST_ASSERT_TRUE(MmiMaintenanceServiceSelectOutputTest(&service, 7U));
  TEST_ASSERT_EQUAL_UINT8(7U, maintenanceCtx.selectedOutput);

  TEST_ASSERT_TRUE(MmiMaintenanceServiceStopOutputTest(&service));
  TEST_ASSERT_EQUAL_UINT8(1U, maintenanceCtx.stopCount);

  (void) memset(&status, 0, sizeof(status));
  TEST_ASSERT_TRUE(MmiMaintenanceServiceReadOutputTestStatus(&service, &status));
  TEST_ASSERT_EQUAL_UINT8(9U, status.outputNumber);
  TEST_ASSERT_EQUAL_UINT16(123U, status.currentNow);
}

void test_MmiMaintenanceServiceExecuteMapsDetectorRelayAndFactoryCommands(void)
{
  TestMaintenanceCtx_t maintenanceCtx;
  TestModuleBusCtx_t moduleBusCtx;
  IMmiMaintenancePort_t maintenancePort;
  IModuleBusPort_t moduleBusPort;
  MmiMaintenanceService_t service;
  MmiMaintenanceModeCommandV2_t modeCommand = { 3U };
  MmiMaintenanceDetectorResetCommandV2_t detectorReset = { 1U, 5U };
  MmiMaintenanceDetectorResetCommandV2_t badDetectorReset = { 9U, 1U };
  MmiMaintenanceRelayCommandV2_t relayCommand = { 0U };
  MmiMaintenanceOutputTestCommandV2_t outputSelect =
  {
    MMI_MAINTENANCE_OUTPUT_TEST_COMMAND_SELECT,
    11U
  };
  MmiMaintenanceFactoryResetCommandV2_t factoryReset = { 0xA5U, 0x5AU };
  MmiMaintenanceFactoryResetCommandV2_t badFactoryReset = { 0x00U, 0x00U };

  (void) memset(&maintenanceCtx, 0, sizeof(maintenanceCtx));
  (void) memset(&moduleBusCtx, 0, sizeof(moduleBusCtx));
  maintenanceCtx.modeOk = 1U;
  maintenanceCtx.relayOk = 1U;
  maintenanceCtx.selectOk = 1U;
  maintenanceCtx.factoryResetOk = 1U;
  moduleBusCtx.resetOk = 1U;

  maintenancePort.ctx = &maintenanceCtx;
  maintenancePort.RequestModeControl = TestRequestModeControl;
  maintenancePort.RequestRelayState = TestRequestRelayState;
  maintenancePort.FactoryReset = TestFactoryReset;
  maintenancePort.EnterIapMode = TestEnterIapMode;
  maintenancePort.StartOutputTest = TestStartOutputTest;
  maintenancePort.StopOutputTest = TestStopOutputTest;
  maintenancePort.SelectOutputTest = TestSelectOutputTest;
  maintenancePort.ReadOutputTestStatus = TestReadOutputTestStatus;

  moduleBusPort.ctx = &moduleBusCtx;
  moduleBusPort.ReadSnapshot = TestReadSnapshot;
  moduleBusPort.SetConfigEpoch = TestSetConfigEpoch;
  moduleBusPort.CommandDetectorReset = TestCommandDetectorReset;

  MmiMaintenanceServiceInit(&service);
  MmiMaintenanceServiceBind(&service, &maintenancePort, &moduleBusPort, NULL);

  TEST_ASSERT_EQUAL_UINT8(MMI_PROTOCOL_V2_STATUS_OK,
                          MmiMaintenanceServiceExecute(
                            &service,
                            MMI_PROTOCOL_V2_MAINTENANCE_RESOURCE_MODE_CONTROL,
                            (const uint8_t *) &modeCommand,
                            sizeof(modeCommand)));
  TEST_ASSERT_EQUAL_UINT8(3U, maintenanceCtx.requestedMode);

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
  TEST_ASSERT_EQUAL_UINT8(0U, maintenanceCtx.requestedRelayState);

  TEST_ASSERT_EQUAL_UINT8(MMI_PROTOCOL_V2_STATUS_OK,
                          MmiMaintenanceServiceExecute(
                            &service,
                            MMI_PROTOCOL_V2_MAINTENANCE_RESOURCE_OUTPUT_TEST,
                            (const uint8_t *) &outputSelect,
                            sizeof(outputSelect)));
  TEST_ASSERT_EQUAL_UINT8(11U, maintenanceCtx.selectedOutput);

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
  TEST_ASSERT_EQUAL_UINT8(1U, maintenanceCtx.factoryResetCount);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_MmiMaintenanceServiceDirectMethodsUseBoundPorts);
  RUN_TEST(test_MmiMaintenanceServiceExecuteMapsDetectorRelayAndFactoryCommands);
  return UNITY_END();
}
