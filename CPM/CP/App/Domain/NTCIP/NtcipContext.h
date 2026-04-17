/* App/Domain/NTCIP/NtcipContext.h
 *
 * Shared application context bound into NTCIP object groups.
 */
#ifndef NTCIP_CONTEXT_H
#define NTCIP_CONTEXT_H

#include "Domain/Intersection/ConfigurationService.h"
#include "Domain/Intersection/DetectorReportService.h"
#include "Domain/Intersection/IntersectionController.h"
#include "Domain/Intersection/IntersectionEngine.h"
#include "Domain/NTCIP/Core/NtcipDbTransactionService.h"
#include "Ports/IUnitAlarmPort.h"
#include "Ports/IUnitClockPort.h"

typedef struct
{
  ConfigurationService_t *configurationService;
  IntersectionEngine_t *intersectionEngine;
  IntersectionController_t *intersectionController;
  NtcipDbTransactionService_t *dbTransactionService;
  DetectorReportService_t *detectorReportService;
  IUnitAlarmPort_t *unitAlarmPort;
  IUnitClockPort_t *unitClockPort;
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
  context->unitAlarmPort = NULL;
  context->unitClockPort = NULL;
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

#endif /* NTCIP_CONTEXT_H */
