/* App/Ports/IPowerMonitorPort.h
 *
 * Port interface for cabinet power source and line-voltage monitoring used by
 * the NTCIP 1202 cabinetEnvironment subtree.
 */
#ifndef IPOWER_MONITOR_PORT_H
#define IPOWER_MONITOR_PORT_H

#include <stddef.h>
#include <stdint.h>

typedef struct
{
  void *ctx;

  uint8_t (*GetPrimarySource)(void *ctx, uint8_t *powerSource);
  uint8_t (*GetLineVoltageTenthsVrms)(void *ctx, uint16_t *lineVoltage);
} IPowerMonitorPort_t;

static inline uint8_t PowerMonitorGetPrimarySource(
  const IPowerMonitorPort_t *port,
  uint8_t *powerSource)
{
  if ((port == NULL) || (port->GetPrimarySource == NULL))
  {
    return 0U;
  }

  return port->GetPrimarySource(port->ctx, powerSource);
}

static inline uint8_t PowerMonitorGetLineVoltageTenthsVrms(
  const IPowerMonitorPort_t *port,
  uint16_t *lineVoltage)
{
  if ((port == NULL) || (port->GetLineVoltageTenthsVrms == NULL))
  {
    return 0U;
  }

  return port->GetLineVoltageTenthsVrms(port->ctx, lineVoltage);
}

#endif /* IPOWER_MONITOR_PORT_H */
