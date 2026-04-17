/* App/Domain/Intersection/IntersectionController.h
 *
 * Thin domain coordinator that bridges field inputs into the timing engine
 * and dispatches the resulting output image.
 */
#ifndef INTERSECTION_CONTROLLER_H
#define INTERSECTION_CONTROLLER_H

#include <stdint.h>

#include "Domain/Intersection/IntersectionActivationService.h"
#include "Domain/Intersection/IntersectionEngine.h"
#include "Domain/Intersection/IntersectionOutputDispatcher.h"
#include "Ports/IModuleBusPort.h"
#include "Ports/IUnitInputPort.h"

typedef struct IntersectionController
{
  IntersectionEngine_t            *engine;
  IntersectionOutputDispatcher_t  *outputDispatcher;
  IntersectionActivationService_t *activationService;
  IModuleBusPort_t                *moduleBusPort;
  IUnitInputPort_t                *unitInputPort;
  IMmuPort_t                      *mmuPort;
  uint16_t expectedModuleBusConfigEpoch;
  ModuleBusSnapshot_t lastSnapshot;
  uint8_t lastSnapshotValid;
  uint8_t lastStepOk;
} IntersectionController_t;

void IntersectionControllerInit(IntersectionController_t *controller);
void IntersectionControllerBind(IntersectionController_t *controller,
                                IntersectionEngine_t *engine,
                                IntersectionOutputDispatcher_t *outputDispatcher,
                                IntersectionActivationService_t *activationService,
                                IModuleBusPort_t *moduleBusPort,
                                IUnitInputPort_t *unitInputPort,
                                IMmuPort_t *mmuPort);
void IntersectionControllerSetExpectedModuleBusConfigEpoch(
  IntersectionController_t *controller,
  uint16_t configEpoch);
uint8_t IntersectionControllerStep(IntersectionController_t *controller);
uint8_t IntersectionControllerGetLastSnapshot(
  const IntersectionController_t *controller,
  ModuleBusSnapshot_t *snapshot);

#endif /* INTERSECTION_CONTROLLER_H */
