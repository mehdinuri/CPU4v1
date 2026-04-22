/* App/Domain/Malfunction/SafetyDecisionService.c */

#include "Malfunction/SafetyDecisionService.h"

#include <stddef.h>
#include <string.h>

static void RefreshRelayState(SafetyDecisionService_t *service)
{
  SafetyRelayState_t state;

  if (service == NULL)
  {
    return;
  }

  service->localPermitOutputPower =
    (uint8_t) ((service->configReady != 0U)
               && (service->requiredSsmHealthy != 0U)
               && (service->latchedAction != SAFETY_ACTION_DARK));
  state = (service->localPermitOutputPower != 0U) ? SAFETY_RELAY_STATE_CLOSED
                                                  : SAFETY_RELAY_STATE_OPEN;
  service->commandedRelayState = state;
  (void) SafetyRelaySetState(service->relayPort, state);
}

static SafetyAction_t DefaultActionForCode(FaultCode_t code)
{
  switch (code)
  {
      case FAULT_CODE_CONFLICT_GREEN_GREEN:
      case FAULT_CODE_CONFLICT_YELLOW_GREEN:
      case FAULT_CODE_CONFLICT_YELLOW_YELLOW:
      case FAULT_CODE_DUAL_INDICATION:
      case FAULT_CODE_DARK_CHANNEL:
      case FAULT_CODE_RED_FAIL:
      case FAULT_CODE_MIN_YELLOW_SHORT:
      case FAULT_CODE_CLEARANCE_SHORT:
      case FAULT_CODE_MODULE_CP_MISSING:
      case FAULT_CODE_MODULE_PSM_MISSING:
      case FAULT_CODE_MODULE_SSM_MISSING:
      case FAULT_CODE_MP_CONFIG_INVALID:
      {
        return SAFETY_ACTION_DARK;
      }

      case FAULT_CODE_PSM_LINE_VOLTAGE_LOW:
      case FAULT_CODE_PSM_LINE_VOLTAGE_HIGH:
      case FAULT_CODE_PSM_FREQUENCY_LOW:
      case FAULT_CODE_PSM_FREQUENCY_HIGH:
      {
        return SAFETY_ACTION_FLASH;
      }

      case FAULT_CODE_SIGNAL_SEQUENCE:
      case FAULT_CODE_LAMP_OPEN_CIRCUIT:
      case FAULT_CODE_LAMP_SHORT_CIRCUIT:
      case FAULT_CODE_LAMP_DRIVEN_EXTERNALLY:
      case FAULT_CODE_LAMP_ALL_BROKEN:
      case FAULT_CODE_LAMP_WORKING_COUNT_CHANGE:
      case FAULT_CODE_VOLTAGE_SENSOR_FAILURE:
      case FAULT_CODE_PSM_RAIL_24V_FAIL:
      case FAULT_CODE_PSM_RAIL_5V_FAIL:
      case FAULT_CODE_MP_WATCHDOG:
      case FAULT_CODE_MP_BATTERY_LOW:
      case FAULT_CODE_MP_TEMPERATURE_HIGH:
      case FAULT_CODE_MP_RELAY_FEEDBACK_MISMATCH:
      case FAULT_CODE_NONE:
      case FAULT_CODE_COUNT:
      default:
      {
        return SAFETY_ACTION_NONE;
      }
  } /* switch */
} /* DefaultActionForCode */

void SafetyDecisionServiceInit(SafetyDecisionService_t *service,
                               ISafetyRelayPort_t *relayPort)
{
  if (service == NULL)
  {
    return;
  }

  (void) memset(service, 0, sizeof(*service));
  service->relayPort = relayPort;
  service->latchedAction = SAFETY_ACTION_NONE;
  service->errorSeverityDropsRelay = 0U;
  service->commandedRelayState = SAFETY_RELAY_STATE_OPEN;
  RefreshRelayState(service);
}

void SafetyDecisionServiceConfigure(SafetyDecisionService_t *service,
                                    uint8_t errorSeverityDropsRelay)
{
  if (service == NULL)
  {
    return;
  }

  service->errorSeverityDropsRelay = (errorSeverityDropsRelay != 0U)
                                     ? 1U : 0U;
}

void SafetyDecisionServiceSetConfigReady(SafetyDecisionService_t *service,
                                         uint8_t configReady)
{
  if (service == NULL)
  {
    return;
  }

  service->configReady = (uint8_t) (configReady != 0U);
  RefreshRelayState(service);
}

void SafetyDecisionServiceSetRequiredSsmHealthy(SafetyDecisionService_t *service,
                                                uint8_t requiredSsmHealthy)
{
  if (service == NULL)
  {
    return;
  }

  service->requiredSsmHealthy = (uint8_t) (requiredSsmHealthy != 0U);
  RefreshRelayState(service);
}

void SafetyDecisionServiceOnFault(SafetyDecisionService_t *service,
                                  const FaultEvent_t *event)
{
  if ((service == NULL) || (event == NULL))
  {
    return;
  }

  SafetyAction_t requested = DefaultActionForCode(event->code);

  if (requested == SAFETY_ACTION_NONE)
  {
    return;
  }

  if ((event->severity < FAULT_SEVERITY_CRITICAL)
      && (service->errorSeverityDropsRelay == 0U)
      && (requested != SAFETY_ACTION_DARK))
  {
    return;
  }

  if (requested > service->latchedAction)
  {
    service->latchedAction = requested;
    service->lastLatchedCode = event->code;
    service->latchedTicks = event->timestampTicks;
    RefreshRelayState(service);
  }
}

void SafetyDecisionServiceReset(SafetyDecisionService_t *service)
{
  if (service == NULL)
  {
    return;
  }

  service->latchedAction = SAFETY_ACTION_NONE;
  service->lastLatchedCode = FAULT_CODE_NONE;
  service->latchedTicks = 0U;
  RefreshRelayState(service);
}

SafetyAction_t SafetyDecisionServiceGetLatchedAction(
  const SafetyDecisionService_t *service)
{
  if (service == NULL)
  {
    return SAFETY_ACTION_NONE;
  }

  return service->latchedAction;
}

FaultCode_t SafetyDecisionServiceGetLastLatchedCode(
  const SafetyDecisionService_t *service)
{
  if (service == NULL)
  {
    return FAULT_CODE_NONE;
  }

  return service->lastLatchedCode;
}

uint8_t SafetyDecisionServiceGetLocalPermitOutputPower(
  const SafetyDecisionService_t *service)
{
  if (service == NULL)
  {
    return 0U;
  }

  return service->localPermitOutputPower;
}

SafetyRelayState_t SafetyDecisionServiceGetCommandedRelayState(
  const SafetyDecisionService_t *service)
{
  if (service == NULL)
  {
    return SAFETY_RELAY_STATE_OPEN;
  }

  return service->commandedRelayState;
}
