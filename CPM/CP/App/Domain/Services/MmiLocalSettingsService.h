/* App/Domain/Services/MmiLocalSettingsService.h
 *
 * Clean boundary for non-NTCIP settings exposed through MMI v2.
 */
#ifndef MMI_LOCAL_SETTINGS_SERVICE_H
#define MMI_LOCAL_SETTINGS_SERVICE_H

#include <stdint.h>

#include "Domain/Intersection/ConfigurationService.h"
#include "Domain/Intersection/GlobalTimeManagementService.h"
#include "Domain/Services/MmiProtocol.h"
#include "Domain/Services/UserAuthService.h"
#include "Ports/IBrokenInputSettingsPort.h"
#include "Ports/IGpsPort.h"
#include "Ports/IGpsTimeSyncPort.h"
#include "Ports/IModemConfigPort.h"
#include "Ports/IRealtimeClockPort.h"
#include "Ports/IUserSettingsPort.h"

typedef struct
{
  IModemConfigPort_t *modemConfigPort;
  IGpsPort_t *gpsPort;
  IRealtimeClockPort_t *rtcPort;
  IGpsTimeSyncPort_t *gpsTimeSyncPort;
  IUserSettingsPort_t *userSettingsPort;
  IBrokenInputSettingsPort_t *brokenInputSettingsPort;
  ConfigurationService_t *configurationService;
  GlobalTimeManagementService_t *globalTimeManagementService;
  UserAuthService_t *userAuthService;
} MmiLocalSettingsService_t;

void MmiLocalSettingsServiceInit(MmiLocalSettingsService_t *service);
void MmiLocalSettingsServiceBind(MmiLocalSettingsService_t *service,
                                 IModemConfigPort_t *modemConfigPort,
                                 IGpsPort_t *gpsPort,
                                 IRealtimeClockPort_t *rtcPort,
                                 IGpsTimeSyncPort_t *gpsTimeSyncPort,
                                 IUserSettingsPort_t *userSettingsPort,
                                 IBrokenInputSettingsPort_t *brokenInputSettingsPort,
                                 ConfigurationService_t *configurationService,
                                 GlobalTimeManagementService_t *globalTimeManagementService,
                                 UserAuthService_t *userAuthService);
MmiProtocolStatus_t MmiLocalSettingsServiceRead(
  const MmiLocalSettingsService_t *service,
  uint8_t resourceId,
  uint8_t *payload,
  uint16_t *payloadLength);
MmiProtocolStatus_t MmiLocalSettingsServiceWrite(
  const MmiLocalSettingsService_t *service,
  uint8_t resourceId,
  const uint8_t *payload,
  uint16_t payloadLength);

#endif /* MMI_LOCAL_SETTINGS_SERVICE_H */
