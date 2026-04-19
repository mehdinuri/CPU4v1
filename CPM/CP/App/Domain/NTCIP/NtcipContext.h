/* App/Domain/NTCIP/NtcipContext.h
 *
 * Shared application context bound into NTCIP object groups.
 */
#ifndef NTCIP_CONTEXT_H
#define NTCIP_CONTEXT_H

#include "Domain/Intersection/CpMpLinkService.h"
#include "Domain/Intersection/ConfigurationService.h"
#include "Domain/Intersection/DetectorReportService.h"
#include "Domain/Intersection/GlobalTimeManagementService.h"
#include "Domain/Intersection/IntersectionController.h"
#include "Domain/Intersection/IntersectionEngine.h"
#include "Domain/NTCIP/Core/NtcipDbTransactionService.h"
#include "Ports/IDoorSensorPort.h"
#include "Ports/IHeaterPort.h"
#include "Ports/IPowerMonitorPort.h"
#include "Ports/IUnitAlarmPort.h"
#include "Ports/IUnitClockPort.h"

typedef struct
{
  ConfigurationService_t *configurationService;
  IntersectionEngine_t *intersectionEngine;
  IntersectionController_t *intersectionController;
  NtcipDbTransactionService_t *dbTransactionService;
  DetectorReportService_t *detectorReportService;
  GlobalTimeManagementService_t *globalTimeManagementService;
  IDoorSensorPort_t *doorSensorPort;
  IHeaterPort_t *heaterPort;
  IPowerMonitorPort_t *powerMonitorPort;
  IUnitAlarmPort_t *unitAlarmPort;
  IUnitClockPort_t *unitClockPort;
  CpMpLinkService_t *cpMpLinkService;
} NtcipContext_t;

static inline void NtcipContextInit(NtcipContext_t *context,
                                    ConfigurationService_t *configurationService,
                                    IntersectionEngine_t *intersectionEngine,
                                    IntersectionController_t *
                                    intersectionController,
                                    NtcipDbTransactionService_t *
                                    dbTransactionService)
{
  context->configurationService = configurationService;
  context->intersectionEngine = intersectionEngine;
  context->intersectionController = intersectionController;
  context->dbTransactionService = dbTransactionService;
  context->detectorReportService = NULL;
  context->globalTimeManagementService = NULL;
  context->doorSensorPort = NULL;
  context->heaterPort = NULL;
  context->powerMonitorPort = NULL;
  context->unitAlarmPort = NULL;
  context->unitClockPort = NULL;
  context->cpMpLinkService = NULL;
}

static inline void NtcipContextBindDetectorReportService(
  NtcipContext_t *context,
  DetectorReportService_t *detectorReportService)
{
  if (context != NULL)
  {
    context->detectorReportService = detectorReportService;
  }
}

static inline void NtcipContextBindGlobalTimeManagementService(
  NtcipContext_t *context,
  GlobalTimeManagementService_t *globalTimeManagementService)
{
  if (context != NULL)
  {
    context->globalTimeManagementService = globalTimeManagementService;
  }
}

static inline void NtcipContextBindDoorSensorPort(NtcipContext_t *context,
                                                  IDoorSensorPort_t *
                                                  doorSensorPort)
{
  if (context != NULL)
  {
    context->doorSensorPort = doorSensorPort;
  }
}

static inline void NtcipContextBindHeaterPort(NtcipContext_t *context,
                                              IHeaterPort_t *heaterPort)
{
  if (context != NULL)
  {
    context->heaterPort = heaterPort;
  }
}

static inline void NtcipContextBindPowerMonitorPort(NtcipContext_t *context,
                                                    IPowerMonitorPort_t *
                                                    powerMonitorPort)
{
  if (context != NULL)
  {
    context->powerMonitorPort = powerMonitorPort;
  }
}

static inline void NtcipContextBindUnitClockPort(NtcipContext_t *context,
                                                 IUnitClockPort_t *unitClockPort)
{
  if (context != NULL)
  {
    context->unitClockPort = unitClockPort;
  }
}

static inline void NtcipContextBindUnitAlarmPort(NtcipContext_t *context,
                                                 IUnitAlarmPort_t *unitAlarmPort)
{
  if (context != NULL)
  {
    context->unitAlarmPort = unitAlarmPort;
  }
}

static inline void NtcipContextBindCpMpLinkService(
  NtcipContext_t *context,
  CpMpLinkService_t *cpMpLinkService)
{
  if (context != NULL)
  {
    context->cpMpLinkService = cpMpLinkService;
  }
}

#endif /* NTCIP_CONTEXT_H */
