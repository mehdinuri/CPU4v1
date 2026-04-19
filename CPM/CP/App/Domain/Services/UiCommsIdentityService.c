/* App/Domain/Services/UiCommsIdentityService.c */
#include "UiCommsIdentityService.h"

#include <string.h>

void UiCommsIdentityServiceInit(UiCommsIdentityService_t *service)
{
  if (service != NULL)
  {
    (void) memset(service, 0, sizeof(*service));
  }
}

void UiCommsIdentityServiceBind(UiCommsIdentityService_t *service,
                                ICommsStatusPort_t *commsStatusPort)
{
  if (service != NULL)
  {
    service->commsStatusPort = commsStatusPort;
  }
}

uint8_t UiCommsIdentityServiceRefresh(UiCommsIdentityService_t *service)
{
  CommsStatusSnapshot_t snapshot;

  if ((service == NULL) || (service->commsStatusPort == NULL))
  {
    return 0U;
  }

  (void) memset(&snapshot, 0, sizeof(snapshot));
  if (CommsStatusReadSnapshot(service->commsStatusPort, &snapshot) == 0U)
  {
    return 0U;
  }

  if ((service->valid == 0U)
      || (memcmp(&service->snapshot, &snapshot, sizeof(snapshot)) != 0))
  {
    service->snapshot = snapshot;
    service->updateSequence++;
    service->valid = 1U;
  }

  return 1U;
}

uint8_t UiCommsIdentityServiceGetSnapshot(
  const UiCommsIdentityService_t *service,
  CommsStatusSnapshot_t *snapshot)
{
  if ((service == NULL) || (snapshot == NULL) || (service->valid == 0U))
  {
    return 0U;
  }

  *snapshot = service->snapshot;
  return 1U;
}

uint32_t UiCommsIdentityServiceGetUpdateSequence(
  const UiCommsIdentityService_t *service)
{
  return (service == NULL) ? 0U : service->updateSequence;
}
