/* App/Domain/Intersection/IntersectionController.c
 *
 * Domain-level controller loop orchestration. This keeps the RTOS task thin
 * and keeps transport snapshots separate from engine internals.
 */
#include "IntersectionController.h"

#include <string.h>

static uint8_t ChannelFeedbackMatchesAspect(OutputDriverAspect_t expectedAspect,
                                            uint8_t feedbackRed,
                                            uint8_t feedbackYellow,
                                            uint8_t feedbackGreen)
{
  switch (expectedAspect)
  {
      case OUTPUT_DRIVER_ASPECT_RED:
      {
        return (uint8_t) ((feedbackRed != 0U) && (feedbackYellow == 0U)
                          && (feedbackGreen == 0U));
      }

      case OUTPUT_DRIVER_ASPECT_YELLOW:
      {
        return (uint8_t) ((feedbackRed == 0U) && (feedbackYellow != 0U)
                          && (feedbackGreen == 0U));
      }

      case OUTPUT_DRIVER_ASPECT_GREEN:
      {
        return (uint8_t) ((feedbackRed == 0U) && (feedbackYellow == 0U)
                          && (feedbackGreen != 0U));
      }

      case OUTPUT_DRIVER_ASPECT_DARK:
      {
        return (uint8_t) ((feedbackRed == 0U) && (feedbackYellow == 0U)
                          && (feedbackGreen == 0U));
      }

      case OUTPUT_DRIVER_ASPECT_FLASH_RED:
      case OUTPUT_DRIVER_ASPECT_FLASH_YELLOW:
      case OUTPUT_DRIVER_ASPECT_FLASH_GREEN:
      default:
      {
        return 1U;
      }
  }
}

static uint8_t OutputFeedbackMatchesDispatcher(
  const IntersectionOutputDispatcher_t *dispatcher,
  const ModuleBusSnapshot_t *
  snapshot)
{
  OutputDriverImage_t appliedImage;
  uint8_t channelNumber;

  if ((dispatcher == NULL) || (snapshot == NULL)
      || (dispatcher->lastDispatchOk == 0U)
      || (IntersectionOutputDispatcherGetLastAppliedImage(dispatcher,
                                                          &appliedImage) == 0U))
  {
    return 1U;
  }

  for (channelNumber = 1U; channelNumber <= INTERSECTION_CHANNEL_COUNT_MAX;
       ++channelNumber)
  {
    if (ChannelFeedbackMatchesAspect(
          appliedImage.channels[channelNumber - 1U],
          ModuleBusSnapshotLoadSwitchShowsRed(snapshot, channelNumber),
          ModuleBusSnapshotLoadSwitchShowsYellow(snapshot, channelNumber),
          ModuleBusSnapshotLoadSwitchShowsGreen(snapshot, channelNumber))
        == 0U)
    {
      return 0U;
    }
  }

  return 1U;
}

static uint8_t SnapshotMatchesControllerContract(
  const IntersectionController_t *controller,
  const ModuleBusSnapshot_t *snapshot)
{
  if ((controller == NULL) || (snapshot == NULL))
  {
    return 0U;
  }

  if (snapshot->protocolVersion != MODULE_BUS_PROTOCOL_VERSION)
  {
    return 0U;
  }

  if ((controller->expectedModuleBusConfigEpoch != 0U)
      && (snapshot->configEpoch != controller->expectedModuleBusConfigEpoch))
  {
    return 0U;
  }

  return 1U;
}

static const IntersectionConfig_t *GetControllerConfig(
  const IntersectionController_t *controller)
{
  if ((controller == NULL) || (controller->engine == NULL))
  {
    return NULL;
  }

  return IntersectionEngineGetConfig(controller->engine);
}

static void ApplyLocalUnitInputs(IntersectionController_t *controller)
{
  uint8_t dimmingInputActive = 0U;
  uint8_t interconnectCommand = 0U;
  uint8_t interconnectInputsValid = 1U;

  if ((controller == NULL) || (controller->engine == NULL))
  {
    return;
  }

  if (controller->unitInputPort != NULL)
  {
    dimmingInputActive = UnitInputGetDimmingInputActive(
      controller->unitInputPort);
    interconnectCommand = UnitInputGetInterconnectCommand(
      controller->unitInputPort);
    interconnectInputsValid = UnitInputGetInterconnectInputsValid(
      controller->unitInputPort);
  }

  (void) IntersectionEngineSetLocalInterconnectInputsValid(controller->engine,
                                                           interconnectInputsValid);
  (void) IntersectionEngineSetLocalInterconnectCommand(controller->engine,
                                                       interconnectCommand);
  (void) IntersectionEngineSetLocalDimmingInput(controller->engine,
                                                dimmingInputActive);
}

static uint8_t ApplyModuleBusSnapshot(IntersectionController_t *controller,
                                      const ModuleBusSnapshot_t *snapshot)
{
  const IntersectionConfig_t *config;
  uint8_t snapshotContractOk;
  uint8_t detectorNumber;
  uint8_t pedDetectorNumber;
  uint8_t preemptNumber;
  uint8_t mmuFlashActive;
  uint8_t detectorSourceReady;
  uint8_t pedSourceReady;
  uint8_t preemptInputReady;
  uint8_t preemptControlReady;
  uint8_t mmuSourceReady;
  uint8_t loadSwitchSourceReady;

  if ((controller == NULL) || (controller->engine == NULL)
      || (snapshot == NULL))
  {
    return 0U;
  }

  config = GetControllerConfig(controller);
  snapshotContractOk = SnapshotMatchesControllerContract(controller, snapshot);
  detectorSourceReady = ModuleBusSnapshotSourceReady(
    snapshot,
    MODULE_BUS_SNAPSHOT_VALID_DETECTORS);
  pedSourceReady = ModuleBusSnapshotSourceReady(snapshot,
                                                MODULE_BUS_SNAPSHOT_VALID_PEDS);
  preemptInputReady = ModuleBusSnapshotSourceReady(
    snapshot,
    MODULE_BUS_SNAPSHOT_VALID_PREEMPT_INPUTS);
  preemptControlReady = ModuleBusSnapshotSourceReady(
    snapshot,
    MODULE_BUS_SNAPSHOT_VALID_PREEMPT_CONTROLS);
  mmuSourceReady = ModuleBusSnapshotSourceReady(snapshot,
                                                MODULE_BUS_SNAPSHOT_VALID_MMU_STATUS);
  loadSwitchSourceReady = ModuleBusSnapshotSourceReady(
    snapshot,
    MODULE_BUS_SNAPSHOT_VALID_LOAD_SWITCH);

  if (snapshotContractOk == 0U)
  {
    detectorSourceReady = 0U;
    pedSourceReady = 0U;
    preemptInputReady = 0U;
    preemptControlReady = 0U;
    mmuSourceReady = 0U;
    loadSwitchSourceReady = 0U;
  }

  for (detectorNumber = 1U;
       detectorNumber <= INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX;
       ++detectorNumber)
  {
    (void) IntersectionEngineSetVehicleDetectorInput(
      controller->engine,
      detectorNumber,
      (uint8_t) ((detectorSourceReady != 0U)
                 && (ModuleBusSnapshotDetectorInputActive(snapshot,
                                                         detectorNumber)
                     != 0U)));
  }

  for (pedDetectorNumber = 1U;
       pedDetectorNumber <= INTERSECTION_PED_INPUT_COUNT_MAX;
       ++pedDetectorNumber)
  {
    (void) IntersectionEngineSetPedestrianDetectorInput(
      controller->engine,
      pedDetectorNumber,
      (uint8_t) ((pedSourceReady != 0U)
                 && (ModuleBusSnapshotPedInputActive(snapshot,
                                                    pedDetectorNumber)
                     != 0U)));
  }

  for (preemptNumber = 1U; preemptNumber <= INTERSECTION_PREEMPT_COUNT_MAX;
       ++preemptNumber)
  {
    uint8_t preemptInputActive = 0U;
    uint8_t preemptControlActive = 0U;

    if (config != NULL)
    {
      uint8_t inputSource = config->inputMapping.preemptInputs[preemptNumber
                                                               - 1U];
      uint8_t controlSource =
        config->inputMapping.preemptControls[preemptNumber - 1U];

      if ((preemptInputReady != 0U) && (inputSource != 0U))
      {
        preemptInputActive = ModuleBusSnapshotPreemptInputActive(snapshot,
                                                                 inputSource);
      }

      if ((preemptControlReady != 0U) && (controlSource != 0U))
      {
        preemptControlActive = ModuleBusSnapshotPreemptControlActive(
          snapshot,
          controlSource);
      }
    }

    (void) IntersectionEngineSetPreemptInput(
      controller->engine,
      preemptNumber,
      preemptInputActive);
    (void) IntersectionEngineSetPreemptControlState(
      controller->engine,
      preemptNumber,
      preemptControlActive);
  }

  mmuFlashActive = 0U;

  if ((snapshot->validMask & MODULE_BUS_SNAPSHOT_VALID_MMU_STATUS) != 0U)
  {
    if ((snapshotContractOk != 0U) && (mmuSourceReady != 0U))
    {
      mmuFlashActive = ModuleBusSnapshotMmuForcesAllRed(snapshot);
    }
    else
    {
      mmuFlashActive = 1U;
    }
  }

  if (((snapshot->validMask & MODULE_BUS_SNAPSHOT_VALID_LOAD_SWITCH) != 0U)
      && (mmuFlashActive == 0U))
  {
    if ((snapshotContractOk == 0U) || (loadSwitchSourceReady == 0U))
    {
      mmuFlashActive = 1U;
    }
    else if (OutputFeedbackMatchesDispatcher(controller->outputDispatcher,
                                             snapshot) == 0U)
    {
      mmuFlashActive = 1U;
    }
  }

  (void) IntersectionEngineSetMmuFlashControl(controller->engine,
                                              mmuFlashActive);
  return MmuSetForceAllRed(controller->mmuPort, mmuFlashActive);
} /* ApplyModuleBusSnapshot */

void IntersectionControllerInit(IntersectionController_t *controller)
{
  if (controller == NULL)
  {
    return;
  }

  memset(controller, 0, sizeof(*controller));
}

void IntersectionControllerBind(IntersectionController_t *controller,
                                IntersectionEngine_t *engine,
                                IntersectionOutputDispatcher_t *outputDispatcher,
                                IntersectionActivationService_t *activationService,
                                IModuleBusPort_t *moduleBusPort,
                                IUnitInputPort_t *unitInputPort,
                                IMmuPort_t *mmuPort)
{
  if (controller == NULL)
  {
    return;
  }

  controller->engine = engine;
  controller->outputDispatcher = outputDispatcher;
  controller->activationService = activationService;
  controller->moduleBusPort = moduleBusPort;
  controller->unitInputPort = unitInputPort;
  controller->mmuPort = mmuPort;
  controller->expectedModuleBusConfigEpoch = 0U;
  controller->lastStepOk = 0U;
}

void IntersectionControllerSetExpectedModuleBusConfigEpoch(
  IntersectionController_t *controller,
  uint16_t configEpoch)
{
  if (controller == NULL)
  {
    return;
  }

  controller->expectedModuleBusConfigEpoch = configEpoch;
}

uint8_t IntersectionControllerStep(IntersectionController_t *controller)
{
  ModuleBusSnapshot_t snapshot;
  uint8_t snapshotValid = 0U;
  uint8_t activationHandled;
  uint8_t snapshotApplyOk = 1U;

  if ((controller == NULL) || (controller->engine == NULL)
      || (controller->outputDispatcher == NULL))
  {
    return 0U;
  }

  ApplyLocalUnitInputs(controller);

  if ((controller->moduleBusPort != NULL)
      && (ModuleBusReadSnapshot(controller->moduleBusPort, &snapshot) != 0U))
  {
    controller->lastSnapshot = snapshot;
    controller->lastSnapshotValid = 1U;
    snapshotValid = 1U;
    snapshotApplyOk = ApplyModuleBusSnapshot(controller, &snapshot);
    if (snapshotApplyOk == 0U)
    {
      controller->lastStepOk = 0U;

      return 0U;
    }
  }

  activationHandled = 0U;

  if (controller->activationService != NULL)
  {
    activationHandled = IntersectionActivationServiceTick(
      controller->activationService,
      controller,
      snapshotValid != 0U ? &snapshot : NULL,
      snapshotValid);
  }

  if (activationHandled != 0U)
  {
    controller->lastStepOk =
      IntersectionOutputDispatcherDispatch(controller->outputDispatcher);

    return controller->lastStepOk;
  }

  IntersectionEngineTick(controller->engine);
  controller->lastStepOk =
    IntersectionOutputDispatcherDispatch(controller->outputDispatcher);

  return controller->lastStepOk;
}

uint8_t IntersectionControllerGetLastSnapshot(
  const IntersectionController_t *controller,
  ModuleBusSnapshot_t *snapshot)
{
  if ((controller == NULL) || (snapshot == NULL)
      || (controller->lastSnapshotValid == 0U))
  {
    return 0U;
  }

  *snapshot = controller->lastSnapshot;

  return 1U;
}
