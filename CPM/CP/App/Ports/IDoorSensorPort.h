/* App/Ports/IDoorSensorPort.h
 *
 * Port interface for the cabinet door tamper sensor (input, PC2).
 * Maps to the NTCIP 1201 controllerStatus door alarm bit.
 * Returns 1 when the door is open, 0 when closed.
 */
#ifndef IDOOR_SENSOR_PORT_H
#define IDOOR_SENSOR_PORT_H

#include <stdint.h>

typedef struct
{
  void    *ctx;

  uint8_t (*IsOpen)(void *ctx); /* returns 1 if door is open */
} IDoorSensorPort_t;

static inline uint8_t DoorSensorIsOpen(IDoorSensorPort_t *p)
{
  return p->IsOpen(p->ctx);
}

#endif /* IDOOR_SENSOR_PORT_H */
