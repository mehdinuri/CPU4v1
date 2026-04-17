/* App/Ports/ISystemMonitorPort.h
 *
 * Port interface for MP self-monitoring: battery voltage, enclosure
 * temperature, charger state. Read by ModuleHealthMonitor to detect
 * MP-local faults that don't involve the field bus.
 */
#ifndef I_SYSTEM_MONITOR_PORT_H
#define I_SYSTEM_MONITOR_PORT_H

#include <stddef.h>
#include <stdint.h>

typedef struct
{
  void *ctx;

  uint8_t (*GetBatteryVoltageMilliVolts)(void *ctx,
                                         uint16_t *milliVolts);
  uint8_t (*GetThermistorDegC)(void *ctx, int16_t *degCelsius);
  uint8_t (*GetChargerActive)(void *ctx, uint8_t *active);
  uint8_t (*SetChargerEnable)(void *ctx, uint8_t enable);
} ISystemMonitorPort_t;

static inline uint8_t SystemMonitorGetBatteryVoltageMilliVolts(
  const ISystemMonitorPort_t *port,
  uint16_t *
  milliVolts)
{
  if ((port == NULL) || (port->GetBatteryVoltageMilliVolts == NULL))
  {
    return 0U;
  }

  return port->GetBatteryVoltageMilliVolts(port->ctx, milliVolts);
}

static inline uint8_t SystemMonitorGetThermistorDegC(
  const ISystemMonitorPort_t *port,
  int16_t *degCelsius)
{
  if ((port == NULL) || (port->GetThermistorDegC == NULL))
  {
    return 0U;
  }

  return port->GetThermistorDegC(port->ctx, degCelsius);
}

static inline uint8_t SystemMonitorGetChargerActive(
  const ISystemMonitorPort_t *port,
  uint8_t *active)
{
  if ((port == NULL) || (port->GetChargerActive == NULL))
  {
    return 0U;
  }

  return port->GetChargerActive(port->ctx, active);
}

static inline uint8_t SystemMonitorSetChargerEnable(ISystemMonitorPort_t *port,
                                                    uint8_t enable)
{
  if ((port == NULL) || (port->SetChargerEnable == NULL))
  {
    return 0U;
  }

  return port->SetChargerEnable(port->ctx, enable);
}

#endif /* I_SYSTEM_MONITOR_PORT_H */
