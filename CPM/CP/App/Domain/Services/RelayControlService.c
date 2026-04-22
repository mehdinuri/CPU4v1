/* App/Domain/Services/RelayControlService.c */
#include "RelayControlService.h"

#include <string.h>

static uint8_t ComputeEffectivePermit(const RelayControlService_t *service)
{
  if (service == NULL)
  {
    return 0U;
  }

  return (uint8_t) ((service->localPermitOutputPower != 0U)
                    && (service->peerPermitValid != 0U)
                    && (service->peerPermitOutputPower != 0U));
}

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

void RelayControlServiceSetLocalState(RelayControlService_t *service,
                                      uint8_t localPermitOutputPower,
                                      uint8_t relayDrive,
                                      uint8_t relayTopology,
                                      uint8_t safetyAction)
{
  uint8_t effectivePermitOutputPower;

  if (service == NULL)
  {
    return;
  }

  effectivePermitOutputPower = ComputeEffectivePermit(service);
  if ((service->localPermitOutputPower != localPermitOutputPower)
      || (service->effectivePermitOutputPower != effectivePermitOutputPower)
      || (service->relayDrive != relayDrive)
      || (service->relayTopology != relayTopology)
      || (service->safetyAction != safetyAction))
  {
    service->localPermitOutputPower = localPermitOutputPower;
    service->effectivePermitOutputPower = ComputeEffectivePermit(service);
    service->relayDrive = relayDrive;
    service->relayTopology = relayTopology;
    service->safetyAction = safetyAction;
    service->changeSequence++;
  }
}

void RelayControlServiceSetPeerState(RelayControlService_t *service,
                                     uint8_t peerPermitValid,
                                     uint8_t peerPermitOutputPower)
{
  uint8_t effectivePermitOutputPower;

  if (service == NULL)
  {
    return;
  }

  peerPermitValid = (uint8_t) (peerPermitValid != 0U);
  peerPermitOutputPower = (uint8_t) (peerPermitOutputPower != 0U);
  effectivePermitOutputPower = ComputeEffectivePermit(service);

  if ((service->peerPermitValid != peerPermitValid)
      || (service->peerPermitOutputPower != peerPermitOutputPower)
      || (service->effectivePermitOutputPower != effectivePermitOutputPower))
  {
    service->peerPermitValid = peerPermitValid;
    service->peerPermitOutputPower = peerPermitOutputPower;
    service->effectivePermitOutputPower = effectivePermitOutputPower;
    service->effectivePermitOutputPower = ComputeEffectivePermit(service);
    service->changeSequence++;
  }
}

uint8_t RelayControlServiceGetLocalPermitOutputPower(
  const RelayControlService_t *service)
{
  return (service == NULL) ? 0U : service->localPermitOutputPower;
}

uint8_t RelayControlServiceGetPeerPermitOutputPower(
  const RelayControlService_t *service)
{
  return (service == NULL) ? 0U : service->peerPermitOutputPower;
}

uint8_t RelayControlServiceGetPeerPermitValid(
  const RelayControlService_t *service)
{
  return (service == NULL) ? 0U : service->peerPermitValid;
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
