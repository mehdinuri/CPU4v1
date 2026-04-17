/* App/Adapters/Mock/MockDoorSensorAdapter.h
 *
 * IDoorSensorPort in-memory test double.
 * Set doorOpen = 1 to simulate an open door.
 */
#ifndef MOCK_DOOR_SENSOR_ADAPTER_H
#define MOCK_DOOR_SENSOR_ADAPTER_H

#include "Ports/IDoorSensorPort.h"

typedef struct
{
  uint8_t doorOpen;
  uint32_t readCount;
} MockDoorSensorAdapterCtx_t;

void MockDoorSensorAdapterInit(MockDoorSensorAdapterCtx_t *ctx);
IDoorSensorPort_t MockDoorSensorAdapterCreatePort(
  MockDoorSensorAdapterCtx_t *ctx);

#endif /* MOCK_DOOR_SENSOR_ADAPTER_H */
