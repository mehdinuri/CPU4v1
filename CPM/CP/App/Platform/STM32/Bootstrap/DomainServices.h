/* App/Platform/STM32/Bootstrap/DomainServices.h
 *
 * Domain services owned and wired by MainApplication_Init().
 */
#ifndef DOMAIN_SERVICES_H
#define DOMAIN_SERVICES_H

#include "Domain/Intersection/IntersectionEngine.h"
#include "Domain/Intersection/IntersectionActivationService.h"
#include "Domain/Intersection/CpMpLinkService.h"
#include "Domain/Intersection/ConfigurationService.h"
#include "Domain/Intersection/GlobalTimeManagementService.h"
#include "Domain/Intersection/IntersectionController.h"
#include "Domain/Intersection/DetectorReportService.h"
#include "Domain/Intersection/IntersectionOutputDispatcher.h"
#include "Domain/Services/OutputTestService.h"
#include "Domain/Services/RelayControlService.h"
#include "Domain/Services/MmiEventLogService.h"
#include "Domain/Services/MmiLocalSettingsService.h"
#include "Domain/Services/MmiMaintenanceService.h"
#include "Domain/Services/MmiService.h"
#include "Domain/Services/MmiSnapshotCache.h"
#include "Domain/Services/UiCommsIdentityService.h"
#include "Domain/Services/UiDoorService.h"
#include "Domain/Services/UiLanguageService.h"
#include "Domain/Services/UiPowerService.h"
#include "Domain/Services/UserAuthService.h"
#include "Ports/ISystemResetPort.h"

extern ConfigurationService_t g_configurationService;
extern CpMpLinkService_t g_cpMpLinkService;
extern IntersectionEngine_t g_intersectionEngine;
extern IntersectionActivationService_t g_intersectionActivationService;
extern IntersectionController_t g_intersectionController;
extern DetectorReportService_t g_detectorReportService;
extern GlobalTimeManagementService_t g_globalTimeManagementService;
extern IntersectionOutputDispatcher_t g_intersectionOutputDispatcher;
extern UserAuthService_t g_userAuthService;
extern UiPowerService_t g_uiPowerService;
extern UiLanguageService_t g_uiLanguageService;
extern UiCommsIdentityService_t g_uiCommsIdentityService;
extern UiDoorService_t g_uiDoorService;
extern RelayControlService_t g_relayControlService;
extern OutputTestService_t g_outputTestService;
extern MmiEventLogService_t g_mmiEventLogService;
extern MmiLocalSettingsService_t g_mmiLocalSettingsService;
extern MmiMaintenanceService_t g_mmiMaintenanceService;
extern MmiService_t g_mmiService;
extern MmiSnapshotCache_t g_mmiSnapshotCache;
extern ISystemResetPort_t g_systemResetPort;

#endif /* DOMAIN_SERVICES_H */
