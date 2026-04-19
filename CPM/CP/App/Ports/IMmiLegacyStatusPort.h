/* App/Ports/IMmiLegacyStatusPort.h
 *
 * Compatibility port for the remaining legacy MMI read-models. This keeps the
 * old wire contract available while the MMI task migrates off legacy globals.
 */
#ifndef IMMI_LEGACY_STATUS_PORT_H
#define IMMI_LEGACY_STATUS_PORT_H

#include <stddef.h>
#include <stdint.h>

typedef struct
{
  uint16_t psmVoltageTenths[2];
  uint8_t psmFrequency[2];
} MmiLegacyMeasurement_t;

typedef struct
{
  uint8_t timeSource;
  uint8_t second;
  uint8_t minute;
  uint8_t hour;
  uint8_t day;
  uint8_t month;
  uint8_t year;
} MmiLegacyTime_t;

typedef struct
{
  uint8_t state;
  uint8_t arg1;
  uint8_t arg2;
  uint8_t arg3;
  uint8_t arg4;
  uint8_t arg5;
  uint8_t arg6;
  uint8_t arg7;
  uint8_t arg8;
} MmiLegacyWorkmode_t;

typedef struct
{
  uint8_t gpsModemConnected;
  uint8_t gpsAntennaConnected;
  uint8_t gprsModemConnected;
  uint8_t gprsCenterConnected;
  uint8_t relayClosed;
  uint8_t lastDigitalInputDemand;
  uint8_t lastLoopDetectorDemand;
  uint8_t gpsModeType;
} MmiLegacyModuleStatus_t;

typedef struct
{
  uint8_t setNumber;
  uint8_t signalingMode;
  uint8_t signalingModeSource;
  uint8_t param1;
  uint8_t param2;
} MmiLegacyErrorRecord_t;

typedef struct
{
  uint8_t modemType;
  uint8_t state;
  uint8_t subState;
  uint8_t signalQuality;
} MmiLegacyGprsLog_t;

typedef struct
{
  void *ctx;

  uint8_t (*ReadMeasurement)(void *ctx, MmiLegacyMeasurement_t *measurement);
  uint8_t (*ReadTime)(void *ctx, MmiLegacyTime_t *timeValue);
  uint8_t (*ReadWorkmode)(void *ctx, MmiLegacyWorkmode_t *workmode);
  uint8_t (*ReadModuleStatus)(void *ctx, MmiLegacyModuleStatus_t *moduleStatus);
  uint8_t (*GetSetTotal)(void *ctx);
  uint8_t (*ReadErrorRecord)(void *ctx,
                             uint8_t setIndex,
                             MmiLegacyErrorRecord_t *record);
  uint8_t (*ReadSignalsBlock)(void *ctx,
                              uint8_t blockIndex,
                              uint16_t words[4]);
  uint8_t (*ReadInputs)(void *ctx,
                        uint32_t *loopDemands,
                        uint32_t *digitalDemands);
  uint8_t (*ReadGprsLog)(void *ctx, MmiLegacyGprsLog_t *logState);
  const char *(*GetGprsImei)(void *ctx);
  const char *(*GetUsrMac)(void *ctx);
  const char *(*GetEthernetMac)(void *ctx);
  const char *(*GetGsmOperator)(void *ctx);
} IMmiLegacyStatusPort_t;

static inline uint8_t MmiLegacyStatusReadMeasurement(
  IMmiLegacyStatusPort_t *port,
  MmiLegacyMeasurement_t *measurement)
{
  return ((port == NULL) || (port->ReadMeasurement == NULL)) ? 0U
         : port->ReadMeasurement(port->ctx, measurement);
}

static inline uint8_t MmiLegacyStatusReadTime(IMmiLegacyStatusPort_t *port,
                                              MmiLegacyTime_t *timeValue)
{
  return ((port == NULL) || (port->ReadTime == NULL)) ? 0U
         : port->ReadTime(port->ctx, timeValue);
}

static inline uint8_t MmiLegacyStatusReadWorkmode(
  IMmiLegacyStatusPort_t *port,
  MmiLegacyWorkmode_t *workmode)
{
  return ((port == NULL) || (port->ReadWorkmode == NULL)) ? 0U
         : port->ReadWorkmode(port->ctx, workmode);
}

static inline uint8_t MmiLegacyStatusReadModuleStatus(
  IMmiLegacyStatusPort_t *port,
  MmiLegacyModuleStatus_t *moduleStatus)
{
  return ((port == NULL) || (port->ReadModuleStatus == NULL)) ? 0U
         : port->ReadModuleStatus(port->ctx, moduleStatus);
}

static inline uint8_t MmiLegacyStatusGetSetTotal(IMmiLegacyStatusPort_t *port)
{
  return ((port == NULL) || (port->GetSetTotal == NULL)) ? 0U
         : port->GetSetTotal(port->ctx);
}

static inline uint8_t MmiLegacyStatusReadErrorRecord(
  IMmiLegacyStatusPort_t *port,
  uint8_t setIndex,
  MmiLegacyErrorRecord_t *record)
{
  return ((port == NULL) || (port->ReadErrorRecord == NULL)) ? 0U
         : port->ReadErrorRecord(port->ctx, setIndex, record);
}

static inline uint8_t MmiLegacyStatusReadSignalsBlock(
  IMmiLegacyStatusPort_t *port,
  uint8_t blockIndex,
  uint16_t words[4])
{
  return ((port == NULL) || (port->ReadSignalsBlock == NULL)) ? 0U
         : port->ReadSignalsBlock(port->ctx, blockIndex, words);
}

static inline uint8_t MmiLegacyStatusReadInputs(IMmiLegacyStatusPort_t *port,
                                                uint32_t *loopDemands,
                                                uint32_t *digitalDemands)
{
  return ((port == NULL) || (port->ReadInputs == NULL)) ? 0U
         : port->ReadInputs(port->ctx, loopDemands, digitalDemands);
}

static inline uint8_t MmiLegacyStatusReadGprsLog(
  IMmiLegacyStatusPort_t *port,
  MmiLegacyGprsLog_t *logState)
{
  return ((port == NULL) || (port->ReadGprsLog == NULL)) ? 0U
         : port->ReadGprsLog(port->ctx, logState);
}

static inline const char *MmiLegacyStatusGetGprsImei(
  IMmiLegacyStatusPort_t *port)
{
  return ((port == NULL) || (port->GetGprsImei == NULL)) ? NULL
         : port->GetGprsImei(port->ctx);
}

static inline const char *MmiLegacyStatusGetUsrMac(IMmiLegacyStatusPort_t *port)
{
  return ((port == NULL) || (port->GetUsrMac == NULL)) ? NULL
         : port->GetUsrMac(port->ctx);
}

static inline const char *MmiLegacyStatusGetEthernetMac(
  IMmiLegacyStatusPort_t *port)
{
  return ((port == NULL) || (port->GetEthernetMac == NULL)) ? NULL
         : port->GetEthernetMac(port->ctx);
}

static inline const char *MmiLegacyStatusGetGsmOperator(
  IMmiLegacyStatusPort_t *port)
{
  return ((port == NULL) || (port->GetGsmOperator == NULL)) ? NULL
         : port->GetGsmOperator(port->ctx);
}

#endif /* IMMI_LEGACY_STATUS_PORT_H */
