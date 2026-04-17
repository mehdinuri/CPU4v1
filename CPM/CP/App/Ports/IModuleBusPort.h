/* App/Ports/IModuleBusPort.h
 *
 * Snapshot-oriented port for the inter-module CAN/FDCAN field bus.
 * The domain consumes the latest aggregated input state without
 * depending on transport details or ISR callbacks.
 */
#ifndef IMODULE_BUS_PORT_H
#define IMODULE_BUS_PORT_H

#include <stdint.h>

#include "Domain/Intersection/IntersectionConfig.h"

#define MODULE_BUS_PROTOCOL_VERSION 1U

#define MODULE_BUS_SNAPSHOT_VALID_DETECTORS         0x01U
#define MODULE_BUS_SNAPSHOT_VALID_PEDS              0x02U
#define MODULE_BUS_SNAPSHOT_VALID_PREEMPT_INPUTS    0x04U
#define MODULE_BUS_SNAPSHOT_VALID_PREEMPT_CONTROLS  0x08U
#define MODULE_BUS_SNAPSHOT_VALID_MMU_STATUS        0x10U
#define MODULE_BUS_SNAPSHOT_VALID_LOAD_SWITCH       0x20U
#define MODULE_BUS_SNAPSHOT_VALID_DETECTOR_DIAGNOSTICS 0x40U

#define MODULE_BUS_MMU_STATUS_FORCE_ALL_RED         0x01U
#define MODULE_BUS_SOURCE_STATUS_FAULT              0x01U

typedef enum
{
  MODULE_BUS_DETECTOR_CLASS_VEHICLE = 1,
  MODULE_BUS_DETECTOR_CLASS_PEDESTRIAN = 2
} ModuleBusDetectorClass_t;

typedef struct
{
  uint8_t protocolVersion;
  uint8_t validMask;
  uint8_t healthMask;
  uint8_t staleMask;
  uint8_t contextFaultMask;
  uint8_t sequenceFaultMask;
  uint16_t detectorInputs;
  uint8_t pedInputs;
  uint8_t preemptInputs;
  uint8_t preemptControls;
  uint8_t mmuStatus;
  uint8_t sequence;
  uint16_t configEpoch;
  uint16_t loadSwitchReds;
  uint16_t loadSwitchYellows;
  uint16_t loadSwitchGreens;
  uint8_t vehicleDetectorAlarms[INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX];
  uint8_t vehicleDetectorReportedAlarms[
    INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX];
  uint8_t pedestrianDetectorAlarms[INTERSECTION_PED_INPUT_COUNT_MAX];
} ModuleBusSnapshot_t;

typedef struct
{
  void *ctx;

  uint8_t (*ReadSnapshot)(void *ctx, ModuleBusSnapshot_t *snapshot);
  uint8_t (*SetConfigEpoch)(void *ctx, uint16_t configEpoch);
  uint8_t (*CommandDetectorReset)(void *ctx,
                                  ModuleBusDetectorClass_t detectorClass,
                                  uint8_t detectorNumber);
} IModuleBusPort_t;

static inline uint8_t ModuleBusReadSnapshot(IModuleBusPort_t *p,
                                            ModuleBusSnapshot_t *snapshot)
{
  return p->ReadSnapshot(p->ctx, snapshot);
}

static inline uint8_t ModuleBusSetConfigEpoch(IModuleBusPort_t *p,
                                              uint16_t configEpoch)
{
  if ((p == NULL) || (p->SetConfigEpoch == NULL))
  {
    return 1U;
  }

  return p->SetConfigEpoch(p->ctx, configEpoch);
}

static inline uint8_t ModuleBusCommandDetectorReset(IModuleBusPort_t *p,
                                                    ModuleBusDetectorClass_t
                                                    detectorClass,
                                                    uint8_t detectorNumber)
{
  if ((p == NULL) || (p->CommandDetectorReset == NULL))
  {
    return 0U;
  }

  return p->CommandDetectorReset(p->ctx, detectorClass, detectorNumber);
}

static inline uint8_t ModuleBusSnapshotSourceReady(
  const ModuleBusSnapshot_t *snapshot,
  uint8_t sourceMask)
{
  if (snapshot == NULL)
  {
    return 0U;
  }

  return (uint8_t) (((snapshot->validMask & sourceMask) != 0U)
                    && ((snapshot->healthMask & sourceMask) != 0U)
                    && ((snapshot->staleMask & sourceMask) == 0U)
                    && ((snapshot->contextFaultMask & sourceMask) == 0U)
                    && ((snapshot->sequenceFaultMask & sourceMask) == 0U));
}

static inline uint8_t ModuleBusSnapshotDetectorInputActive(
  const ModuleBusSnapshot_t *snapshot,
  uint8_t
  detectorNumber)
{
  uint16_t mask;

  if ((snapshot == NULL) || (detectorNumber == 0U)
      || (detectorNumber > INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX))
  {
    return 0U;
  }

  mask = (uint16_t) (1U << (uint16_t) (detectorNumber - 1U));

  return (uint8_t) ((snapshot->detectorInputs & mask) != 0U);
}

static inline uint8_t ModuleBusSnapshotPedInputActive(
  const ModuleBusSnapshot_t *snapshot,
  uint8_t pedInputNumber)
{
  uint8_t mask;

  if ((snapshot == NULL) || (pedInputNumber == 0U)
      || (pedInputNumber > INTERSECTION_PED_INPUT_COUNT_MAX))
  {
    return 0U;
  }

  mask = (uint8_t) (1U << (uint8_t) (pedInputNumber - 1U));

  return (uint8_t) ((snapshot->pedInputs & mask) != 0U);
}

static inline uint8_t ModuleBusSnapshotPreemptInputActive(
  const ModuleBusSnapshot_t *snapshot,
  uint8_t preemptNumber)
{
  uint8_t mask;

  if ((snapshot == NULL) || (preemptNumber == 0U)
      || (preemptNumber > INTERSECTION_PREEMPT_COUNT_MAX))
  {
    return 0U;
  }

  mask = (uint8_t) (1U << (uint8_t) (preemptNumber - 1U));

  return (uint8_t) ((snapshot->preemptInputs & mask) != 0U);
}

static inline uint8_t ModuleBusSnapshotPreemptControlActive(
  const ModuleBusSnapshot_t *snapshot,
  uint8_t
  preemptNumber)
{
  uint8_t mask;

  if ((snapshot == NULL) || (preemptNumber == 0U)
      || (preemptNumber > INTERSECTION_PREEMPT_COUNT_MAX))
  {
    return 0U;
  }

  mask = (uint8_t) (1U << (uint8_t) (preemptNumber - 1U));

  return (uint8_t) ((snapshot->preemptControls & mask) != 0U);
}

static inline uint8_t ModuleBusSnapshotMmuForcesAllRed(
  const ModuleBusSnapshot_t *snapshot)
{
  if (snapshot == NULL)
  {
    return 0U;
  }

  return (uint8_t) ((snapshot->mmuStatus & MODULE_BUS_MMU_STATUS_FORCE_ALL_RED)
                    != 0U);
}

static inline uint8_t ModuleBusSnapshotLoadSwitchShowsRed(
  const ModuleBusSnapshot_t *snapshot,
  uint8_t channelNumber)
{
  uint16_t mask;

  if ((snapshot == NULL) || (channelNumber == 0U)
      || (channelNumber > INTERSECTION_CHANNEL_COUNT_MAX))
  {
    return 0U;
  }

  mask = (uint16_t) (1U << (uint16_t) (channelNumber - 1U));

  return (uint8_t) ((snapshot->loadSwitchReds & mask) != 0U);
}

static inline uint8_t ModuleBusSnapshotLoadSwitchShowsYellow(
  const ModuleBusSnapshot_t *snapshot,
  uint8_t
  channelNumber)
{
  uint16_t mask;

  if ((snapshot == NULL) || (channelNumber == 0U)
      || (channelNumber > INTERSECTION_CHANNEL_COUNT_MAX))
  {
    return 0U;
  }

  mask = (uint16_t) (1U << (uint16_t) (channelNumber - 1U));

  return (uint8_t) ((snapshot->loadSwitchYellows & mask) != 0U);
}

static inline uint8_t ModuleBusSnapshotLoadSwitchShowsGreen(
  const ModuleBusSnapshot_t *snapshot,
  uint8_t
  channelNumber)
{
  uint16_t mask;

  if ((snapshot == NULL) || (channelNumber == 0U)
      || (channelNumber > INTERSECTION_CHANNEL_COUNT_MAX))
  {
    return 0U;
  }

  mask = (uint16_t) (1U << (uint16_t) (channelNumber - 1U));

  return (uint8_t) ((snapshot->loadSwitchGreens & mask) != 0U);
}

static inline uint8_t ModuleBusSnapshotVehicleDetectorAlarmBits(
  const ModuleBusSnapshot_t *snapshot,
  uint8_t detectorNumber)
{
  if ((snapshot == NULL) || (detectorNumber == 0U)
      || (detectorNumber > INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX))
  {
    return 0U;
  }

  return snapshot->vehicleDetectorAlarms[detectorNumber - 1U];
}

static inline uint8_t ModuleBusSnapshotVehicleDetectorReportedAlarmBits(
  const ModuleBusSnapshot_t *snapshot,
  uint8_t detectorNumber)
{
  if ((snapshot == NULL) || (detectorNumber == 0U)
      || (detectorNumber > INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX))
  {
    return 0U;
  }

  return snapshot->vehicleDetectorReportedAlarms[detectorNumber - 1U];
}

static inline uint8_t ModuleBusSnapshotPedestrianDetectorAlarmBits(
  const ModuleBusSnapshot_t *snapshot,
  uint8_t detectorNumber)
{
  if ((snapshot == NULL) || (detectorNumber == 0U)
      || (detectorNumber > INTERSECTION_PED_INPUT_COUNT_MAX))
  {
    return 0U;
  }

  return snapshot->pedestrianDetectorAlarms[detectorNumber - 1U];
}

#endif /* IMODULE_BUS_PORT_H */
