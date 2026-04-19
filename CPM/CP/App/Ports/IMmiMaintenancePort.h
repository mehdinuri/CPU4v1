/* App/Ports/IMmiMaintenancePort.h
 *
 * Compatibility port for legacy MMI maintenance side effects.
 */
#ifndef IMMI_MAINTENANCE_PORT_H
#define IMMI_MAINTENANCE_PORT_H

#include <stddef.h>
#include <stdint.h>

typedef struct
{
  uint8_t outputNumber;
  uint8_t state;
  uint16_t powerNet;
  uint16_t power;
  uint16_t net;
  uint16_t currentNow;
  uint16_t currentMin;
  uint16_t currentMax;
} MmiMaintenanceOutputTestStatus_t;

typedef struct
{
  void *ctx;

  uint8_t (*RequestModeControl)(void *ctx, uint8_t requestedControl);
  uint8_t (*RequestRelayState)(void *ctx, uint8_t requestedState);
  uint8_t (*FactoryReset)(void *ctx);
  uint8_t (*EnterIapMode)(void *ctx);
  uint8_t (*StartOutputTest)(void *ctx);
  uint8_t (*StopOutputTest)(void *ctx);
  uint8_t (*SelectOutputTest)(void *ctx, uint8_t outputNumber);
  uint8_t (*ReadOutputTestStatus)(void *ctx,
                                  MmiMaintenanceOutputTestStatus_t *status);
} IMmiMaintenancePort_t;

static inline uint8_t MmiMaintenancePortRequestModeControl(
  IMmiMaintenancePort_t *port,
  uint8_t requestedControl)
{
  return ((port == NULL) || (port->RequestModeControl == NULL)) ? 0U
         : port->RequestModeControl(port->ctx, requestedControl);
}

static inline uint8_t MmiMaintenancePortRequestRelayState(
  IMmiMaintenancePort_t *port,
  uint8_t requestedState)
{
  return ((port == NULL) || (port->RequestRelayState == NULL)) ? 0U
         : port->RequestRelayState(port->ctx, requestedState);
}

static inline uint8_t MmiMaintenancePortFactoryReset(
  IMmiMaintenancePort_t *port)
{
  return ((port == NULL) || (port->FactoryReset == NULL)) ? 0U
         : port->FactoryReset(port->ctx);
}

static inline uint8_t MmiMaintenancePortEnterIapMode(
  IMmiMaintenancePort_t *port)
{
  return ((port == NULL) || (port->EnterIapMode == NULL)) ? 0U
         : port->EnterIapMode(port->ctx);
}

static inline uint8_t MmiMaintenancePortStartOutputTest(
  IMmiMaintenancePort_t *port)
{
  return ((port == NULL) || (port->StartOutputTest == NULL)) ? 0U
         : port->StartOutputTest(port->ctx);
}

static inline uint8_t MmiMaintenancePortStopOutputTest(
  IMmiMaintenancePort_t *port)
{
  return ((port == NULL) || (port->StopOutputTest == NULL)) ? 0U
         : port->StopOutputTest(port->ctx);
}

static inline uint8_t MmiMaintenancePortSelectOutputTest(
  IMmiMaintenancePort_t *port,
  uint8_t outputNumber)
{
  return ((port == NULL) || (port->SelectOutputTest == NULL)) ? 0U
         : port->SelectOutputTest(port->ctx, outputNumber);
}

static inline uint8_t MmiMaintenancePortReadOutputTestStatus(
  IMmiMaintenancePort_t *port,
  MmiMaintenanceOutputTestStatus_t *status)
{
  return ((port == NULL) || (port->ReadOutputTestStatus == NULL)) ? 0U
         : port->ReadOutputTestStatus(port->ctx, status);
}

#endif /* IMMI_MAINTENANCE_PORT_H */
