/* App/Domain/Services/UiCommsIdentityService.h */
#ifndef UI_COMMS_IDENTITY_SERVICE_H
#define UI_COMMS_IDENTITY_SERVICE_H

#include <stdint.h>

#include "Ports/ICommsStatusPort.h"

typedef struct
{
  ICommsStatusPort_t *commsStatusPort;
  CommsStatusSnapshot_t snapshot;
  uint32_t updateSequence;
  uint8_t valid;
} UiCommsIdentityService_t;

void UiCommsIdentityServiceInit(UiCommsIdentityService_t *service);
void UiCommsIdentityServiceBind(UiCommsIdentityService_t *service,
                                ICommsStatusPort_t *commsStatusPort);
uint8_t UiCommsIdentityServiceRefresh(UiCommsIdentityService_t *service);
uint8_t UiCommsIdentityServiceGetSnapshot(
  const UiCommsIdentityService_t *service,
  CommsStatusSnapshot_t *snapshot);
uint32_t UiCommsIdentityServiceGetUpdateSequence(
  const UiCommsIdentityService_t *service);

#endif /* UI_COMMS_IDENTITY_SERVICE_H */
