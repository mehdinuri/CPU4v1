/* App/Ports/IUnitClockPort.h
 *
 * Port interface for unit clock source availability and runtime status.
 */
#ifndef I_UNIT_CLOCK_PORT_H
#define I_UNIT_CLOCK_PORT_H

#include <stddef.h>
#include <stdint.h>

#define UNIT_CLOCK_NON_SEQUENTIAL_DELTA_LENGTH 5U

typedef enum
{
  UNIT_CLOCK_SOURCE_OTHER = 1,
  UNIT_CLOCK_SOURCE_LINE_SYNC = 2,
  UNIT_CLOCK_SOURCE_RTC_SQWR = 3,
  UNIT_CLOCK_SOURCE_CRYSTAL = 4,
  UNIT_CLOCK_SOURCE_GNSS = 5,
  UNIT_CLOCK_SOURCE_NTP = 6
} UnitClockSource_t;

typedef enum
{
  UNIT_CLOCK_SOURCE_STATUS_NOT_ACTIVE = 1,
  UNIT_CLOCK_SOURCE_STATUS_ACTIVE = 2,
  UNIT_CLOCK_SOURCE_STATUS_DATA_ERROR = 3,
  UNIT_CLOCK_SOURCE_STATUS_DATA_TIMEOUT_ERROR = 4,
  UNIT_CLOCK_SOURCE_STATUS_PENDING_UPDATE = 5,
  UNIT_CLOCK_SOURCE_STATUS_NON_SEQUENTIAL = 6
} UnitClockSourceStatus_t;

typedef enum
{
  UNIT_CLOCK_NON_SEQUENTIAL_SOURCE_UNKNOWN = 1,
  UNIT_CLOCK_NON_SEQUENTIAL_SOURCE_DST_CHANGE = 2,
  UNIT_CLOCK_NON_SEQUENTIAL_SOURCE_MANAGEMENT_STATION = 3,
  UNIT_CLOCK_NON_SEQUENTIAL_SOURCE_LOCAL_USER = 4,
  UNIT_CLOCK_NON_SEQUENTIAL_SOURCE_TIME_SOURCE_CHANGE = 5,
  UNIT_CLOCK_NON_SEQUENTIAL_SOURCE_TIME_SOURCE_DISCONTINUITY = 6
} UnitClockNonSequentialSource_t;

typedef struct
{
  uint8_t bytes[UNIT_CLOCK_NON_SEQUENTIAL_DELTA_LENGTH];
  uint8_t length;
} UnitClockNonSequentialDelta_t;

typedef struct
{
  void *ctx;
  uint8_t (*GetSourceCount)(void *ctx);
  uint8_t (*GetSourceAvailable)(void *ctx,
                                uint8_t sourceIndex,
                                uint8_t *sourceAvailable);
  uint8_t (*GetCommandedSource)(void *ctx, uint8_t *commandedSource);
  uint8_t (*GetCurrentSource)(void *ctx, uint8_t *currentSource);
  uint8_t (*GetCurrentStatus)(void *ctx, uint8_t *currentStatus);
  uint8_t (*GetNonSequentialSource)(void *ctx,
                                    uint8_t *nonSequentialSource);
  uint8_t (*GetNonSequentialChange)(void *ctx,
                                    uint32_t *nonSequentialChangeSeconds);
  uint8_t (*GetNonSequentialDelta)(void *ctx,
                                   UnitClockNonSequentialDelta_t *delta);
  void (*AcknowledgeCurrentStatusRead)(void *ctx);
} IUnitClockPort_t;

static inline uint8_t UnitClockPortGetSourceCount(const IUnitClockPort_t *port)
{
  if ((port == NULL) || (port->GetSourceCount == NULL))
  {
    return 0U;
  }

  return port->GetSourceCount(port->ctx);
}

static inline uint8_t UnitClockPortGetSourceAvailable(
  const IUnitClockPort_t *port,
  uint8_t sourceIndex,
  uint8_t *sourceAvailable)
{
  if ((port == NULL) || (port->GetSourceAvailable == NULL))
  {
    return 0U;
  }

  return port->GetSourceAvailable(port->ctx, sourceIndex, sourceAvailable);
}

static inline uint8_t UnitClockPortGetCommandedSource(
  const IUnitClockPort_t *port,
  uint8_t *commandedSource)
{
  if ((port == NULL) || (port->GetCommandedSource == NULL))
  {
    return 0U;
  }

  return port->GetCommandedSource(port->ctx, commandedSource);
}

static inline uint8_t UnitClockPortGetCurrentSource(
  const IUnitClockPort_t *port,
  uint8_t *currentSource)
{
  if ((port == NULL) || (port->GetCurrentSource == NULL))
  {
    return 0U;
  }

  return port->GetCurrentSource(port->ctx, currentSource);
}

static inline uint8_t UnitClockPortGetCurrentStatus(
  const IUnitClockPort_t *port,
  uint8_t *currentStatus)
{
  if ((port == NULL) || (port->GetCurrentStatus == NULL))
  {
    return 0U;
  }

  return port->GetCurrentStatus(port->ctx, currentStatus);
}

static inline uint8_t UnitClockPortGetNonSequentialSource(
  const IUnitClockPort_t *port,
  uint8_t *nonSequentialSource)
{
  if ((port == NULL) || (port->GetNonSequentialSource == NULL))
  {
    return 0U;
  }

  return port->GetNonSequentialSource(port->ctx, nonSequentialSource);
}

static inline uint8_t UnitClockPortGetNonSequentialChange(
  const IUnitClockPort_t *port,
  uint32_t *nonSequentialChangeSeconds)
{
  if ((port == NULL) || (port->GetNonSequentialChange == NULL))
  {
    return 0U;
  }

  return port->GetNonSequentialChange(port->ctx,
                                      nonSequentialChangeSeconds);
}

static inline uint8_t UnitClockPortGetNonSequentialDelta(
  const IUnitClockPort_t *port,
  UnitClockNonSequentialDelta_t *delta)
{
  if ((port == NULL) || (port->GetNonSequentialDelta == NULL))
  {
    return 0U;
  }

  return port->GetNonSequentialDelta(port->ctx, delta);
}

static inline void UnitClockPortAcknowledgeCurrentStatusRead(
  const IUnitClockPort_t *port)
{
  if ((port != NULL) && (port->AcknowledgeCurrentStatusRead != NULL))
  {
    port->AcknowledgeCurrentStatusRead(port->ctx);
  }
}

#endif /* I_UNIT_CLOCK_PORT_H */
