/* App/Domain/Services/RelayControlService.h */
#ifndef RELAY_CONTROL_SERVICE_H
#define RELAY_CONTROL_SERVICE_H

#include <stdint.h>

typedef struct
{
  uint8_t userOutputPowerEnabled;
  uint8_t effectivePermitOutputPower;
  uint8_t relayDrive;
  uint8_t relayTopology;
  uint8_t safetyAction;
  uint32_t changeSequence;
} RelayControlService_t;

void RelayControlServiceInit(RelayControlService_t *service);
uint8_t RelayControlServiceSetUserOutputPowerEnabled(
  RelayControlService_t *service,
  uint8_t enabled);
uint8_t RelayControlServiceGetUserOutputPowerEnabled(
  const RelayControlService_t *service);
void RelayControlServiceSetAppliedState(RelayControlService_t *service,
                                        uint8_t effectivePermitOutputPower,
                                        uint8_t relayDrive,
                                        uint8_t relayTopology,
                                        uint8_t safetyAction);
uint8_t RelayControlServiceGetEffectivePermitOutputPower(
  const RelayControlService_t *service);
uint8_t RelayControlServiceGetRelayDrive(const RelayControlService_t *service);
uint8_t RelayControlServiceGetRelayTopology(const RelayControlService_t *service);
uint8_t RelayControlServiceGetSafetyAction(const RelayControlService_t *service);
uint32_t RelayControlServiceGetChangeSequence(
  const RelayControlService_t *service);

#endif /* RELAY_CONTROL_SERVICE_H */
