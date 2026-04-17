/* App/Domain/Intersection/IntersectionOutputDispatcher.h
 *
 * Dispatches the engine output intent image through the MMU/output-driver
 * seam.
 */
#ifndef INTERSECTION_OUTPUT_DISPATCHER_H
#define INTERSECTION_OUTPUT_DISPATCHER_H

#include <stdint.h>

#include "Domain/Intersection/IntersectionEngine.h"
#include "Ports/IMmuPort.h"
#include "Ports/IOutputDriverPort.h"

typedef struct
{
  const IntersectionEngine_t *engine;
  IMmuPort_t                 *mmuPort;
  IOutputDriverPort_t        *outputDriverPort;
  OutputDriverImage_t lastRequestedImage;
  OutputDriverImage_t lastAppliedImage;
  uint8_t lastDispatchOk;
} IntersectionOutputDispatcher_t;

void IntersectionOutputDispatcherInit(
  IntersectionOutputDispatcher_t *dispatcher);
void IntersectionOutputDispatcherBind(
  IntersectionOutputDispatcher_t *dispatcher,
  const IntersectionEngine_t *engine,
  IMmuPort_t *mmuPort,
  IOutputDriverPort_t *outputDriverPort);
uint8_t IntersectionOutputDispatcherDispatch(
  IntersectionOutputDispatcher_t *dispatcher);
uint8_t IntersectionOutputDispatcherGetLastRequestedImage(
  const IntersectionOutputDispatcher_t *dispatcher,
  OutputDriverImage_t *
  image);
uint8_t IntersectionOutputDispatcherGetLastAppliedImage(
  const IntersectionOutputDispatcher_t *dispatcher,
  OutputDriverImage_t *
  image);

#endif /* INTERSECTION_OUTPUT_DISPATCHER_H */
