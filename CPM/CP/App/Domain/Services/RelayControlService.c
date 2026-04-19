/* App/Domain/Services/RelayControlService.c */
#include "RelayControlService.h"

#include <string.h>

void RelayControlServiceInit(RelayControlService_t *service)
{
  if (service != NULL)
  {
    (void) memset(service, 0, sizeof(*service));
    service->userOutputPowerEnabled = 1U;
  }
}

uint8_t RelayControlServiceSetUserOutputPowerEnabled(
  RelayControlService_t *service,
  uint8_t enabled)
{
  uint8_t normalized;

  if (service == NULL)
  {
    return 0U;
  }

  normalized = (uint8_t) (enabled != 0U);
  if (service->userOutputPowerEnabled != normalized)
  {
    service->userOutputPowerEnabled = normalized;
    service->changeSequence++;
  }

  return 1U;
}

uint8_t RelayControlServiceGetUserOutputPowerEnabled(
  const RelayControlService_t *service)
{
  return (service == NULL) ? 0U : service->userOutputPowerEnabled;
}

void RelayControlServiceSetAppliedState(RelayControlService_t *service,
                                        uint8_t effectivePermitOutputPower,
                                        uint8_t relayDrive,
                                        uint8_t relayTopology,
                                        uint8_t safetyAction)
{
  if (service == NULL)
  {
    return;
  }

  if ((service->effectivePermitOutputPower != effectivePermitOutputPower)
      || (service->relayDrive != relayDrive)
      || (service->relayTopology != relayTopology)
      || (service->safetyAction != safetyAction))
  {
    service->effectivePermitOutputPower = effectivePermitOutputPower;
    service->relayDrive = relayDrive;
    service->relayTopology = relayTopology;
    service->safetyAction = safetyAction;
    service->changeSequence++;
  }
}

uint8_t RelayControlServiceGetEffectivePermitOutputPower(
  const RelayControlService_t *service)
{
  return (service == NULL) ? 0U : service->effectivePermitOutputPower;
}

uint8_t RelayControlServiceGetRelayDrive(const RelayControlService_t *service)
{
  return (service == NULL) ? 0U : service->relayDrive;
}

uint8_t RelayControlServiceGetRelayTopology(const RelayControlService_t *service)
{
  return (service == NULL) ? 0U : service->relayTopology;
}

uint8_t RelayControlServiceGetSafetyAction(const RelayControlService_t *service)
{
  return (service == NULL) ? 0U : service->safetyAction;
}

uint32_t RelayControlServiceGetChangeSequence(
  const RelayControlService_t *service)
{
  return (service == NULL) ? 0U : service->changeSequence;
}
