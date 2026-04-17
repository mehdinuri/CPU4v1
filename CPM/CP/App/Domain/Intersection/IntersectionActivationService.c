/* App/Domain/Intersection/IntersectionActivationService.c
 *
 * Stages committed controller plans and activates them online through a
 * controlled all-red soft reload.
 */
#include "IntersectionActivationService.h"

#include <stddef.h>
#include <string.h>

#include "Domain/Intersection/IntersectionController.h"
#include "Domain/Intersection/IntersectionEngine.h"

#define INTERSECTION_ACTIVATION_ALL_RED_HOLD_TICKS 30U

static uint32_t ComputeConfigHash(const IntersectionConfig_t *config)
{
  const uint8_t *bytes = (const uint8_t *) config;
  uint32_t hash = 2166136261UL;
  size_t index;

  if (config == NULL)
  {
    return 0U;
  }

  for (index = 0U; index < sizeof(*config); ++index)
  {
    hash ^= (uint32_t) bytes[index];
    hash *= 16777619UL;
  }

  return hash;
}

static uint8_t CompilePlan(const IntersectionConfig_t *config,
                           uint16_t setId,
                           IntersectionCompiledPlan_t *plan)
{
  IntersectionConfigErrorInfo_t errorInfo;

  if ((config == NULL) || (plan == NULL))
  {
    return 0U;
  }

  if ((IntersectionConfigValidate(config, &errorInfo) == 0U)
      || (IntersectionConfigValidateRuntimeSupport(config, &errorInfo) == 0U))
  {
    return 0U;
  }

  memset(plan, 0, sizeof(*plan));
  plan->config = *config;
  plan->setId = setId;
  plan->controlHash = ComputeConfigHash(config);
  plan->valid = 1U;

  return 1U;
}

static uint8_t PlansAreControlEquivalent(
  const IntersectionCompiledPlan_t *left,
  const IntersectionCompiledPlan_t *right)
{
  if ((left == NULL) || (right == NULL) || (left->valid == 0U)
      || (right->valid == 0U))
  {
    return 0U;
  }

  return (uint8_t) (left->controlHash == right->controlHash);
}

static uint8_t LoadSwitchFeedbackAllRed(
  const IntersectionCompiledPlan_t *plan,
  const ModuleBusSnapshot_t *snapshot)
{
  uint8_t channelIndex;

  if ((plan == NULL) || (snapshot == NULL))
  {
    return 0U;
  }

  for (channelIndex = 0U; channelIndex < INTERSECTION_CHANNEL_COUNT_MAX;
       ++channelIndex)
  {
    const IntersectionChannelConfig_t *channel = &plan->config.channels[channelIndex];
    uint8_t channelNumber = (uint8_t) (channelIndex + 1U);

    if (channel->controlSource == 0U)
    {
      continue;
    }

    if ((ModuleBusSnapshotLoadSwitchShowsRed(snapshot, channelNumber) == 0U)
        || (ModuleBusSnapshotLoadSwitchShowsYellow(snapshot, channelNumber)
            != 0U)
        || (ModuleBusSnapshotLoadSwitchShowsGreen(snapshot, channelNumber) != 0U))
    {
      return 0U;
    }
  }

  return 1U;
}

static uint8_t SafeToStartActivation(const IntersectionController_t *controller)
{
  const IntersectionRuntime_t *runtime;

  if ((controller == NULL) || (controller->engine == NULL))
  {
    return 0U;
  }

  runtime = IntersectionEngineGetRuntime(controller->engine);

  if (runtime == NULL)
  {
    return 0U;
  }

  return (uint8_t) ((runtime->mode != INTERSECTION_CONTROL_MODE_PREEMPT)
                    && (runtime->unitControlStatus
                        != INTERSECTION_UNIT_CONTROL_STATUS_REMOTE_MANUAL_CONTROL));
}

static uint8_t PublishLiveSetId(IntersectionController_t *controller,
                                uint16_t setId)
{
  if (controller == NULL)
  {
    return 0U;
  }

  IntersectionControllerSetExpectedModuleBusConfigEpoch(controller, setId);

  if (ModuleBusSetConfigEpoch(controller->moduleBusPort, setId) == 0U)
  {
    return 0U;
  }

  if (OutputDriverSetConfigEpoch(
        controller->outputDispatcher != NULL
          ? controller->outputDispatcher->outputDriverPort
          : NULL,
        setId) == 0U)
  {
    return 0U;
  }

  return 1U;
}

void IntersectionActivationServiceInit(IntersectionActivationService_t *service)
{
  if (service != NULL)
  {
    memset(service, 0, sizeof(*service));
  }
}

uint8_t IntersectionActivationServiceLoadCommittedLivePlan(
  IntersectionActivationService_t *service,
  const IntersectionConfig_t *config,
  uint16_t setId)
{
  if ((service == NULL) || (CompilePlan(config, setId, &service->plans[0]) == 0U))
  {
    if (service != NULL)
    {
      service->status.error = INTERSECTION_ACTIVATION_ERROR_UNSUPPORTED_RUNTIME;
      service->status.state = INTERSECTION_ACTIVATION_STATE_FAILED;
    }

    return 0U;
  }

  memset(&service->plans[1], 0, sizeof(service->plans[1]));
  service->activePlanIndex = 0U;
  service->stagedPlanIndex = 1U;
  service->activePlanValid = 1U;
  service->stagedPlanValid = 0U;
  service->allRedStableTicks = 0U;
  service->status.committedSetId = setId;
  service->status.liveSetId = setId;
  service->status.pendingSetId = 0U;
  service->status.state = INTERSECTION_ACTIVATION_STATE_IDLE;
  service->status.error = INTERSECTION_ACTIVATION_ERROR_NONE;

  return 1U;
}

uint8_t IntersectionActivationServiceStageCommittedConfig(
  IntersectionActivationService_t *service,
  const IntersectionConfig_t *config,
  uint16_t setId)
{
  uint8_t slotIndex;

  if (service == NULL)
  {
    return 0U;
  }

  service->status.committedSetId = setId;
  slotIndex = (uint8_t) (service->activePlanIndex ^ 1U);

  if (CompilePlan(config, setId, &service->plans[slotIndex]) == 0U)
  {
    service->stagedPlanValid = 0U;
    service->status.pendingSetId = 0U;
    service->status.state = INTERSECTION_ACTIVATION_STATE_FAILED;
    service->status.error = INTERSECTION_ACTIVATION_ERROR_UNSUPPORTED_RUNTIME;

    return 0U;
  }

  if ((service->activePlanValid != 0U)
      && (PlansAreControlEquivalent(&service->plans[service->activePlanIndex],
                                    &service->plans[slotIndex]) != 0U))
  {
    service->stagedPlanValid = 0U;
    service->status.pendingSetId = 0U;
    service->status.state = INTERSECTION_ACTIVATION_STATE_IDLE;
    service->status.error = INTERSECTION_ACTIVATION_ERROR_NONE;

    return 1U;
  }

  service->stagedPlanIndex = slotIndex;
  service->stagedPlanValid = 1U;
  service->allRedStableTicks = 0U;
  service->status.pendingSetId = setId;
  service->status.state = INTERSECTION_ACTIVATION_STATE_STAGED;
  service->status.error = INTERSECTION_ACTIVATION_ERROR_NONE;

  return 1U;
}

uint8_t IntersectionActivationServiceTick(
  IntersectionActivationService_t *service,
  IntersectionController_t *controller,
  const ModuleBusSnapshot_t *snapshot,
  uint8_t snapshotValid)
{
  if ((service == NULL) || (controller == NULL) || (controller->engine == NULL)
      || (controller->outputDispatcher == NULL))
  {
    return 0U;
  }

  switch (service->status.state)
  {
      case INTERSECTION_ACTIVATION_STATE_IDLE:
      {
        return 0U;
      }

      case INTERSECTION_ACTIVATION_STATE_STAGED:
      {
        service->allRedStableTicks = 0U;
        service->status.state =
          INTERSECTION_ACTIVATION_STATE_WAIT_SAFE_PRECONDITIONS;

        return 1U;
      }

      case INTERSECTION_ACTIVATION_STATE_WAIT_SAFE_PRECONDITIONS:
      {
        if (SafeToStartActivation(controller) == 0U)
        {
          return 0U;
        }

        if (MmuSetForceAllRed(controller->mmuPort, 1U) == 0U)
        {
          service->status.state = INTERSECTION_ACTIVATION_STATE_FAILED;
          service->status.error = INTERSECTION_ACTIVATION_ERROR_MMU_COMMAND;

          return 0U;
        }

        service->allRedStableTicks = 0U;
        service->status.state =
          INTERSECTION_ACTIVATION_STATE_WAIT_ALL_RED_FEEDBACK;

        return 1U;
      }

      case INTERSECTION_ACTIVATION_STATE_FORCE_ALL_RED:
      case INTERSECTION_ACTIVATION_STATE_WAIT_ALL_RED_FEEDBACK:
      {
        if (MmuSetForceAllRed(controller->mmuPort, 1U) == 0U)
        {
          service->status.state = INTERSECTION_ACTIVATION_STATE_FAILED;
          service->status.error = INTERSECTION_ACTIVATION_ERROR_MMU_COMMAND;

          return 0U;
        }

        service->status.state =
          INTERSECTION_ACTIVATION_STATE_WAIT_ALL_RED_FEEDBACK;

        if ((service->activePlanValid != 0U) && (snapshotValid != 0U)
            && (snapshot != NULL)
            && (ModuleBusSnapshotSourceReady(
                  snapshot,
                  MODULE_BUS_SNAPSHOT_VALID_LOAD_SWITCH) != 0U)
            && (LoadSwitchFeedbackAllRed(
                  &service->plans[service->activePlanIndex],
                  snapshot) != 0U))
        {
          if (service->allRedStableTicks < 0xFFU)
          {
            service->allRedStableTicks++;
          }
        }
        else
        {
          service->allRedStableTicks = 0U;
        }

        if (service->allRedStableTicks
            >= INTERSECTION_ACTIVATION_ALL_RED_HOLD_TICKS)
        {
          service->status.state = INTERSECTION_ACTIVATION_STATE_SWITCH_PLAN;
        }

        return 1U;
      }

      case INTERSECTION_ACTIVATION_STATE_SWITCH_PLAN:
      {
        const IntersectionCompiledPlan_t *plan;

        if (service->stagedPlanValid == 0U)
        {
          service->status.state = INTERSECTION_ACTIVATION_STATE_FAILED;
          service->status.error = INTERSECTION_ACTIVATION_ERROR_UNSUPPORTED_RUNTIME;

          return 0U;
        }

        plan = &service->plans[service->stagedPlanIndex];

        if (IntersectionEngineLoadConfig(controller->engine, &plan->config) == 0U)
        {
          service->status.state = INTERSECTION_ACTIVATION_STATE_FAILED;
          service->status.error = INTERSECTION_ACTIVATION_ERROR_ENGINE_LOAD;

          return 0U;
        }

        if ((PublishLiveSetId(controller, plan->setId) == 0U)
            || (MmuSetForceAllRed(controller->mmuPort, 0U) == 0U))
        {
          service->status.state = INTERSECTION_ACTIVATION_STATE_FAILED;
          service->status.error = INTERSECTION_ACTIVATION_ERROR_MMU_COMMAND;

          return 0U;
        }

        service->activePlanIndex = service->stagedPlanIndex;
        service->activePlanValid = 1U;
        service->stagedPlanValid = 0U;
        service->allRedStableTicks = 0U;
        service->status.liveSetId = plan->setId;
        service->status.pendingSetId = 0U;
        service->status.state = INTERSECTION_ACTIVATION_STATE_RELEASE_STARTUP;
        service->status.error = INTERSECTION_ACTIVATION_ERROR_NONE;

        return 1U;
      }

      case INTERSECTION_ACTIVATION_STATE_RELEASE_STARTUP:
      {
        service->status.state = INTERSECTION_ACTIVATION_STATE_IDLE;
        return 0U;
      }

      case INTERSECTION_ACTIVATION_STATE_FAILED:
      default:
      {
        return 0U;
      }
  }
}

uint8_t IntersectionActivationServiceActivationInProgress(
  const IntersectionActivationService_t *service)
{
  if (service == NULL)
  {
    return 0U;
  }

  return (uint8_t) ((service->status.state != INTERSECTION_ACTIVATION_STATE_IDLE)
                    && (service->status.state
                        != INTERSECTION_ACTIVATION_STATE_FAILED));
}

uint8_t IntersectionActivationServiceGetStatus(
  const IntersectionActivationService_t *service,
  IntersectionActivationStatus_t *status)
{
  if ((service == NULL) || (status == NULL))
  {
    return 0U;
  }

  *status = service->status;

  return 1U;
}
