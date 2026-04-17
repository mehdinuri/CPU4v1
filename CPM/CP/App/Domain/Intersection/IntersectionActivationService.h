/* App/Domain/Intersection/IntersectionActivationService.h
 *
 * Online activation service for staged committed controller plans.
 */
#ifndef INTERSECTION_ACTIVATION_SERVICE_H
#define INTERSECTION_ACTIVATION_SERVICE_H

#include <stdint.h>

#include "Domain/Intersection/IntersectionConfig.h"
#include "Ports/IModuleBusPort.h"

struct IntersectionController;

typedef enum
{
  INTERSECTION_ACTIVATION_STATE_IDLE = 0,
  INTERSECTION_ACTIVATION_STATE_STAGED,
  INTERSECTION_ACTIVATION_STATE_WAIT_SAFE_PRECONDITIONS,
  INTERSECTION_ACTIVATION_STATE_FORCE_ALL_RED,
  INTERSECTION_ACTIVATION_STATE_WAIT_ALL_RED_FEEDBACK,
  INTERSECTION_ACTIVATION_STATE_SWITCH_PLAN,
  INTERSECTION_ACTIVATION_STATE_RELEASE_STARTUP,
  INTERSECTION_ACTIVATION_STATE_FAILED
} IntersectionActivationState_t;

typedef enum
{
  INTERSECTION_ACTIVATION_ERROR_NONE = 0,
  INTERSECTION_ACTIVATION_ERROR_INVALID_ARGUMENT,
  INTERSECTION_ACTIVATION_ERROR_UNSUPPORTED_RUNTIME,
  INTERSECTION_ACTIVATION_ERROR_MMU_COMMAND,
  INTERSECTION_ACTIVATION_ERROR_ENGINE_LOAD
} IntersectionActivationError_t;

typedef struct
{
  uint8_t valid;
  uint16_t setId;
  uint32_t controlHash;
  IntersectionConfig_t config;
} IntersectionCompiledPlan_t;

typedef struct
{
  uint16_t committedSetId;
  uint16_t liveSetId;
  uint16_t pendingSetId;
  IntersectionActivationState_t state;
  IntersectionActivationError_t error;
} IntersectionActivationStatus_t;

typedef struct
{
  IntersectionCompiledPlan_t plans[2];
  uint8_t activePlanIndex;
  uint8_t stagedPlanIndex;
  uint8_t activePlanValid;
  uint8_t stagedPlanValid;
  uint8_t allRedStableTicks;
  IntersectionActivationStatus_t status;
} IntersectionActivationService_t;

void IntersectionActivationServiceInit(
  IntersectionActivationService_t *service);
uint8_t IntersectionActivationServiceLoadCommittedLivePlan(
  IntersectionActivationService_t *service,
  const IntersectionConfig_t *config,
  uint16_t setId);
uint8_t IntersectionActivationServiceStageCommittedConfig(
  IntersectionActivationService_t *service,
  const IntersectionConfig_t *config,
  uint16_t setId);
uint8_t IntersectionActivationServiceTick(
  IntersectionActivationService_t *service,
  struct IntersectionController *controller,
  const ModuleBusSnapshot_t *snapshot,
  uint8_t snapshotValid);
uint8_t IntersectionActivationServiceActivationInProgress(
  const IntersectionActivationService_t *service);
uint8_t IntersectionActivationServiceGetStatus(
  const IntersectionActivationService_t *service,
  IntersectionActivationStatus_t *status);

#endif /* INTERSECTION_ACTIVATION_SERVICE_H */
