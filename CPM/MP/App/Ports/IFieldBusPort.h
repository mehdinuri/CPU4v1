/* App/Ports/IFieldBusPort.h
 *
 * Port interface for the shared field bus (FDCAN1). MP is a read-only
 * witness on this bus: it observes the same commanded load-switch
 * image CP broadcasts to SSMs (0x040/0x041) plus raw SSM telemetry
 * (0x050..0x057) and PSM telemetry (0x05A/0x05B).
 *
 * The adapter decodes wire frames into a FieldBusSnapshot_t; the
 * domain pulls the latest snapshot per MalfunctionEngine tick, so
 * engine code is trivially mockable.
 */
#ifndef I_FIELD_BUS_PORT_H
#define I_FIELD_BUS_PORT_H

#include <stddef.h>
#include <stdint.h>

#define FIELD_BUS_LOAD_SWITCH_BITS 96U
#define FIELD_BUS_LOAD_SWITCH_BYTES ((FIELD_BUS_LOAD_SWITCH_BITS + 7U) / 8U)
#define FIELD_BUS_SSM_COUNT 8U
#define FIELD_BUS_PSM_COUNT 2U
#define FIELD_BUS_SSM_OUTPUTS_PER_MODULE 12U
#define FIELD_BUS_SSM_CURRENT_GROUPS_PER_MODULE 4U

typedef struct
{
  uint8_t bits[FIELD_BUS_LOAD_SWITCH_BYTES];
} FieldBusLoadSwitchImage_t;

typedef struct
{
  uint16_t voltagePresenceBits;
  uint16_t currentByGroupDeciAmps[FIELD_BUS_SSM_CURRENT_GROUPS_PER_MODULE];
  uint8_t alive;
} FieldBusSsmTelemetry_t;

typedef struct
{
  uint16_t lineVoltageAdcCounts;
  uint16_t rail24V1AdcCounts;
  uint16_t rail24V2AdcCounts;
  uint16_t rail5V1AdcCounts;
  uint16_t rail5V2AdcCounts;
  uint16_t lineFrequencyRaw;
  uint8_t isolatedVoltage;
  uint8_t alive;
} FieldBusPsmTelemetry_t;

typedef struct
{
  FieldBusLoadSwitchImage_t commandedLoadSwitches;
  uint16_t cpuImageSequence;
  uint8_t cpAlive;
} FieldBusCpuImage_t;

typedef struct
{
  FieldBusCpuImage_t cpu;
  FieldBusSsmTelemetry_t ssm[FIELD_BUS_SSM_COUNT];
  FieldBusPsmTelemetry_t psm[FIELD_BUS_PSM_COUNT];
  uint32_t snapshotTicks;
} FieldBusSnapshot_t;

typedef struct
{
  void *ctx;

  uint8_t (*ReadSnapshot)(void *ctx, FieldBusSnapshot_t *snapshot);
} IFieldBusPort_t;

static inline uint8_t FieldBusReadSnapshot(const IFieldBusPort_t *port,
                                           FieldBusSnapshot_t *snapshot)
{
  if ((port == NULL) || (port->ReadSnapshot == NULL))
  {
    return 0U;
  }

  return port->ReadSnapshot(port->ctx, snapshot);
}

#endif /* I_FIELD_BUS_PORT_H */
