/* App/Domain/Malfunction/SafetyDecisionService.h
 *
 * Single chokepoint mapping a FaultEvent_t stream into a relay action.
 * Domain policy: any critical fault drops the SSM safety relay; error
 * severity is logged but does not drop the relay unless configured;
 * warnings never drop the relay.
 *
 * The service also holds the current latched action so that a single
 * transient fault does not re-command the relay every tick.
 */
#ifndef SAFETY_DECISION_SERVICE_H
#define SAFETY_DECISION_SERVICE_H

#include <stdint.h>

#include "Malfunction/FaultCodes.h"
#include "Ports/ISafetyRelayPort.h"

typedef enum
{
  SAFETY_ACTION_NONE = 0U,
  SAFETY_ACTION_FLASH = 1U,
  SAFETY_ACTION_DARK = 2U
} SafetyAction_t;

typedef struct
{
  ISafetyRelayPort_t *relayPort;
  SafetyAction_t latchedAction;
  FaultCode_t lastLatchedCode;
  uint32_t latchedTicks;
  uint8_t errorSeverityDropsRelay;
} SafetyDecisionService_t;

void SafetyDecisionServiceInit(SafetyDecisionService_t *service,
                               ISafetyRelayPort_t *relayPort);
void SafetyDecisionServiceConfigure(SafetyDecisionService_t *service,
                                    uint8_t errorSeverityDropsRelay);
void SafetyDecisionServiceOnFault(SafetyDecisionService_t *service,
                                  const FaultEvent_t *event);
void SafetyDecisionServiceReset(SafetyDecisionService_t *service);
SafetyAction_t SafetyDecisionServiceGetLatchedAction(
  const SafetyDecisionService_t *service);
FaultCode_t SafetyDecisionServiceGetLastLatchedCode(
  const SafetyDecisionService_t *service);

#endif /* SAFETY_DECISION_SERVICE_H */
