/* App/Platform/STM32/Bootstrap/DomainServices.h
 *
 * Domain services owned and wired by MainApplication_Init().
 */
#ifndef DOMAIN_SERVICES_H
#define DOMAIN_SERVICES_H

#include "Domain/Intersection/IntersectionEngine.h"
#include "Domain/Intersection/IntersectionActivationService.h"
#include "Domain/Intersection/ConfigurationService.h"
#include "Domain/Intersection/GlobalTimeManagementService.h"
#include "Domain/Intersection/IntersectionController.h"
#include "Domain/Intersection/DetectorReportService.h"
#include "Domain/Intersection/IntersectionOutputDispatcher.h"

extern ConfigurationService_t g_configurationService;
extern IntersectionEngine_t g_intersectionEngine;
extern IntersectionActivationService_t g_intersectionActivationService;
extern IntersectionController_t g_intersectionController;
extern DetectorReportService_t g_detectorReportService;
extern GlobalTimeManagementService_t g_globalTimeManagementService;
extern IntersectionOutputDispatcher_t g_intersectionOutputDispatcher;

#endif /* DOMAIN_SERVICES_H */
