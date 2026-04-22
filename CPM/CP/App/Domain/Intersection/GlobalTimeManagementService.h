/* App/Domain/Intersection/GlobalTimeManagementService.h
 *
 * Canonical 1201/1202 time management runtime: clock conversion, day-plan
 * selection, and timebase-action application.
 */
#ifndef GLOBAL_TIME_MANAGEMENT_SERVICE_H
#define GLOBAL_TIME_MANAGEMENT_SERVICE_H

#include <stdint.h>

#include "Domain/Intersection/IntersectionEngine.h"
#include "Ports/IRealtimeClockPort.h"

typedef struct
{
  IntersectionEngine_t *engine;
  IRealtimeClockPort_t *rtcPort;
  uint32_t lastObservedLocalTimeSeconds;
  uint8_t lastObservedLocalTimeValid;
  uint8_t scheduleStatus;
  uint8_t dayPlanStatus;
  uint8_t dayPlanEventStatus;
  uint8_t actionStatus;
  uint8_t lastAppliedActionNumber;
} GlobalTimeManagementService_t;

void GlobalTimeManagementServiceInit(GlobalTimeManagementService_t *service);
void GlobalTimeManagementServiceBind(GlobalTimeManagementService_t *service,
                                     IntersectionEngine_t *engine,
                                     IRealtimeClockPort_t *rtcPort);
void GlobalTimeManagementServiceReset(GlobalTimeManagementService_t *service);
void GlobalTimeManagementServiceStep(GlobalTimeManagementService_t *service);
void GlobalTimeManagementServiceHandleCommittedConfig(
  GlobalTimeManagementService_t *service,
  const IntersectionGlobalTimeManagementConfig_t *globalTimeManagementConfig);

uint8_t GlobalTimeManagementServiceGetScheduleStatus(
  GlobalTimeManagementService_t *service,
  uint8_t *scheduleStatus);
uint8_t GlobalTimeManagementServiceGetDayPlanStatus(
  GlobalTimeManagementService_t *service,
  uint8_t *dayPlanStatus);
uint8_t GlobalTimeManagementServiceGetControllerLocalTime(
  GlobalTimeManagementService_t *service,
  uint32_t *controllerLocalTimeSeconds);
uint8_t GlobalTimeManagementServiceGetGlobalTime(
  GlobalTimeManagementService_t *service,
  uint32_t *globalTimeSeconds);
uint8_t GlobalTimeManagementServiceGetGlobalTimeWithMilliseconds(
  GlobalTimeManagementService_t *service,
  uint32_t *globalTimeSeconds,
  uint16_t *globalTimeMilliseconds);
uint8_t GlobalTimeManagementServiceGetGlobalLocalTimeDifferential(
  GlobalTimeManagementService_t *service,
  int32_t *globalLocalTimeDifferentialSeconds);
uint8_t GlobalTimeManagementServiceSetGlobalTime(
  GlobalTimeManagementService_t *service,
  uint32_t globalTimeSeconds);
uint8_t GlobalTimeManagementServiceComputeStandardTimeZone(
  const IntersectionGlobalTimeManagementConfig_t *globalTimeManagementConfig,
  uint32_t globalTimeSeconds,
  int32_t desiredDifferentialSeconds,
  int32_t *standardTimeZoneSeconds);

#endif /* GLOBAL_TIME_MANAGEMENT_SERVICE_H */
