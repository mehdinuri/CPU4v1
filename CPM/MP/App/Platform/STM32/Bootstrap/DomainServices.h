/* App/Platform/STM32/Bootstrap/DomainServices.h
 *
 * Global domain service instances wired by MainApplication_Init().
 * Task bodies and the MalfunctionEngine read from these singletons.
 */
#ifndef DOMAIN_SERVICES_H
#define DOMAIN_SERVICES_H

#include "Alarm/UnitAlarmService.h"
#include "FaultMonitor/FaultMonitorService.h"
#include "Intersection/ConfigurationService.h"
#include "Malfunction/MalfunctionEngine.h"
#include "Malfunction/SafetyDecisionService.h"

extern ConfigurationService_t g_configurationService;
extern SafetyDecisionService_t g_safetyDecisionService;
extern FaultMonitorService_t g_faultMonitorService;
extern UnitAlarmService_t g_unitAlarmService;
extern MalfunctionEngine_t g_malfunctionEngine;

#endif /* DOMAIN_SERVICES_H */
