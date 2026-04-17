/* App/Adapters/STM32/DoorSensorAdapter.h
 *
 * IDoorSensorPort concrete implementation.
 * Reads DOOR_Pin (PC2) directly via HAL.  The GPIO pin is already
 * configured by MX_GPIO_Init so no additional init is required.
 */
#ifndef DOOR_SENSOR_ADAPTER_H
#define DOOR_SENSOR_ADAPTER_H

#include "Ports/IDoorSensorPort.h"

typedef struct
{
  uint8_t reserved; /* no mutable state needed */
} DoorSensorAdapterCtx_t;

/* No-op — pin configured by MX_GPIO_Init. Provided for symmetry. */
void DoorSensorAdapterInit(DoorSensorAdapterCtx_t *ctx);

/* Build an IDoorSensorPort_t wired to ctx. */
IDoorSensorPort_t DoorSensorAdapterCreatePort(DoorSensorAdapterCtx_t *ctx);

#endif /* DOOR_SENSOR_ADAPTER_H */
