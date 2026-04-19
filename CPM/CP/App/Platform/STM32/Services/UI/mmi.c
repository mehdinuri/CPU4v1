/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  include files */
#include "mmi.h"

#include <string.h>

#include "DomainServices.h"
#include "LegacyCanTx.h"
#include "MLM.h"
/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  members */

/*  os members */

/*  private members */
tSMMIRuntime SMMIRuntime;

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Method declaration */
enum
{
  MMI_LEGACY_ACK = 0x06U,
  MMI_LEGACY_NAK = 0x15U,
  MMI_LEGACY_IMEI_LEN = 15U,
  MMI_LEGACY_MAC_LEN = 12U,
  MMI_LEGACY_VERSION_ARG0 = 4U,
  MMI_LEGACY_VERSION_ARG1 = 4U,
  MMI_LEGACY_VERSION_ARG2 = 0U,
  MMI_LEGACY_VERSION_ARG3 = 2U,
  MMI_LEGACY_VERSION_ARG4 = (uint8_t) 'T',
  MMI_LEGACY_EVENT_DOOR_OPEN = 64U,
  MMI_LEGACY_EVENT_DOOR_CLOSED = 65U,
  MMI_LEGACY_USER_SETTINGS_CHANGED = 240U,
  MMI_LEGACY_BROKEN_INPUT_SET = 240U
};

static void TransmitLegacyFrame(uint8_t dataLength,
                                uint16_t standardId,
                                const void *data)
{
  uint8_t txBuffer[8];

  (void) memset(&txBuffer[0], 0, sizeof(txBuffer));
  if ((data != NULL) && (dataLength > 0U))
  {
    (void) memcpy(&txBuffer[0], data, dataLength);
  }

  CANTxRequest(dataLength,
               LEGACY_CAN_ID_TYPE_STD,
               standardId,
               &txBuffer[0]);
}

static void TransmitLegacyAck(uint16_t standardId, uint8_t accepted)
{
  const uint8_t response = (accepted != FALSE) ? MMI_LEGACY_ACK
                           : MMI_LEGACY_NAK;

  TransmitLegacyFrame(sizeof(response), standardId, &response);
}

static uint8_t ReadLocalFlags(MmiLocalUserFlagsV2_t *settings)
{
  uint8_t payload[sizeof(MmiLocalUserFlagsV2_t)];
  uint16_t payloadLength = 0U;

  if (settings == NULL)
  {
    return FALSE;
  }

  if (MmiLocalSettingsServiceRead(&g_mmiLocalSettingsService,
                                  MMI_PROTOCOL_V2_LOCAL_RESOURCE_USER_FLAGS,
                                  &payload[0],
                                  &payloadLength) != MMI_PROTOCOL_V2_STATUS_OK)
  {
    return FALSE;
  }

  if (payloadLength != sizeof(*settings))
  {
    return FALSE;
  }

  memcpy(settings, &payload[0], sizeof(*settings));
  return TRUE;
}

static uint8_t WriteLocalFlags(const MmiLocalUserFlagsV2_t *settings)
{
  if (settings == NULL)
  {
    return FALSE;
  }

  return (uint8_t) (MmiLocalSettingsServiceWrite(
                      &g_mmiLocalSettingsService,
                      MMI_PROTOCOL_V2_LOCAL_RESOURCE_USER_FLAGS,
                      (const uint8_t *) settings,
                      sizeof(*settings)) == MMI_PROTOCOL_V2_STATUS_OK);
}

static uint8_t ReadLocalBrokenInputSettings(
  MmiLocalBrokenInputSettingsV2_t *settings)
{
  uint8_t payload[sizeof(MmiLocalBrokenInputSettingsV2_t)];
  uint16_t payloadLength = 0U;

  if (settings == NULL)
  {
    return FALSE;
  }

  if (MmiLocalSettingsServiceRead(&g_mmiLocalSettingsService,
                                  MMI_PROTOCOL_V2_LOCAL_RESOURCE_BROKEN_INPUT,
                                  &payload[0],
                                  &payloadLength) != MMI_PROTOCOL_V2_STATUS_OK)
  {
    return FALSE;
  }

  if (payloadLength != sizeof(*settings))
  {
    return FALSE;
  }

  memcpy(settings, &payload[0], sizeof(*settings));
  return TRUE;
}

static uint8_t WriteLocalBrokenInputSettings(
  const MmiLocalBrokenInputSettingsV2_t *settings)
{
  if (settings == NULL)
  {
    return FALSE;
  }

  return (uint8_t) (MmiLocalSettingsServiceWrite(
                      &g_mmiLocalSettingsService,
                      MMI_PROTOCOL_V2_LOCAL_RESOURCE_BROKEN_INPUT,
                      (const uint8_t *) settings,
                      sizeof(*settings)) == MMI_PROTOCOL_V2_STATUS_OK);
}

static uint8_t ReadLocalGpsSettings(MmiLocalGpsSettingsV2_t *settings)
{
  uint8_t payload[sizeof(MmiLocalGpsSettingsV2_t)];
  uint16_t payloadLength = 0U;

  if (settings == NULL)
  {
    return FALSE;
  }

  if (MmiLocalSettingsServiceRead(&g_mmiLocalSettingsService,
                                  MMI_PROTOCOL_V2_LOCAL_RESOURCE_GPS,
                                  &payload[0],
                                  &payloadLength) != MMI_PROTOCOL_V2_STATUS_OK)
  {
    return FALSE;
  }

  if (payloadLength != sizeof(*settings))
  {
    return FALSE;
  }

  memcpy(settings, &payload[0], sizeof(*settings));
  return TRUE;
}

static uint8_t WriteLocalGpsSettings(const MmiLocalGpsSettingsV2_t *settings)
{
  if (settings == NULL)
  {
    return FALSE;
  }

  return (uint8_t) (MmiLocalSettingsServiceWrite(
                      &g_mmiLocalSettingsService,
                      MMI_PROTOCOL_V2_LOCAL_RESOURCE_GPS,
                      (const uint8_t *) settings,
                      sizeof(*settings)) == MMI_PROTOCOL_V2_STATUS_OK);
}

static uint8_t ReadLocalModemSettings(MmiLocalModemSettingsV2_t *settings)
{
  uint8_t payload[sizeof(MmiLocalModemSettingsV2_t)];
  uint16_t payloadLength = 0U;

  if (settings == NULL)
  {
    return FALSE;
  }

  if (MmiLocalSettingsServiceRead(&g_mmiLocalSettingsService,
                                  MMI_PROTOCOL_V2_LOCAL_RESOURCE_MODEM,
                                  &payload[0],
                                  &payloadLength) != MMI_PROTOCOL_V2_STATUS_OK)
  {
    return FALSE;
  }

  if (payloadLength != sizeof(*settings))
  {
    return FALSE;
  }

  memcpy(settings, &payload[0], sizeof(*settings));
  return TRUE;
}

static uint8_t WriteLocalModemSettings(const MmiLocalModemSettingsV2_t *settings)
{
  if (settings == NULL)
  {
    return FALSE;
  }

  return (uint8_t) (MmiLocalSettingsServiceWrite(
                      &g_mmiLocalSettingsService,
                      MMI_PROTOCOL_V2_LOCAL_RESOURCE_MODEM,
                      (const uint8_t *) settings,
                      sizeof(*settings)) == MMI_PROTOCOL_V2_STATUS_OK);
}

static uint8_t ReadLocalClockSettings(MmiLocalClockSettingsV2_t *settings)
{
  uint8_t payload[sizeof(MmiLocalClockSettingsV2_t)];
  uint16_t payloadLength = 0U;

  if (settings == NULL)
  {
    return FALSE;
  }

  if (MmiLocalSettingsServiceRead(&g_mmiLocalSettingsService,
                                  MMI_PROTOCOL_V2_LOCAL_RESOURCE_CLOCK_SETTINGS,
                                  &payload[0],
                                  &payloadLength) != MMI_PROTOCOL_V2_STATUS_OK)
  {
    return FALSE;
  }

  if (payloadLength != sizeof(*settings))
  {
    return FALSE;
  }

  memcpy(settings, &payload[0], sizeof(*settings));
  return TRUE;
}

static uint8_t WriteLocalClockSettings(
  const MmiLocalClockSettingsV2_t *settings)
{
  if (settings == NULL)
  {
    return FALSE;
  }

  return (uint8_t) (MmiLocalSettingsServiceWrite(
                      &g_mmiLocalSettingsService,
                      MMI_PROTOCOL_V2_LOCAL_RESOURCE_CLOCK_SETTINGS,
                      (const uint8_t *) settings,
                      sizeof(*settings)) == MMI_PROTOCOL_V2_STATUS_OK);
}

static uint8_t ReadLatestEventLogIndex(uint16_t *logIndex)
{
  if (logIndex == NULL)
  {
    return FALSE;
  }

  return MmiEventLogServiceGetLatestIndex(&g_mmiEventLogService, logIndex);
}

static uint8_t ReadEventLogRecord(uint16_t logIndex,
                                  MmiEventRecordV2_t *record)
{
  if (record == NULL)
  {
    return FALSE;
  }

  return MmiEventLogServiceReadRecord(&g_mmiEventLogService,
                                      logIndex,
                                      record);
}

static uint8_t ReadLatestDoorLogIndex(uint8_t closedState, uint16_t *logIndex)
{
  if (logIndex == NULL)
  {
    return FALSE;
  }

  return MmiEventLogServiceFindLatestByEventCode(
    &g_mmiEventLogService,
    (closedState != FALSE) ? MMI_LEGACY_EVENT_DOOR_CLOSED
    : MMI_LEGACY_EVENT_DOOR_OPEN,
    logIndex);
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  methods */
void OpenMMI(void)
{
  TransmitLegacyFrame(0U, CAN_MMI_STD_ID_OPEN_MMI, NULL);
}

void CloseMMI(void)
{
  TransmitLegacyFrame(0U, CAN_MMI_STD_ID_CLOSE_MMI, NULL);
}

void StreamPSMMeasurements(void)
{
  tSMMIMeasurement SMMIMeasurement;
  MmiLegacyMeasurement_t measurement;

  (void) memset(&SMMIMeasurement, 0, sizeof(SMMIMeasurement));
  (void) memset(&measurement, 0, sizeof(measurement));
  if (MmiLegacyStatusReadMeasurement(&g_mmiLegacyStatusPort, &measurement)
      != FALSE)
  {
    SMMIMeasurement.psm1Voltage = measurement.psmVoltageTenths[0];
    SMMIMeasurement.psm2Voltage = measurement.psmVoltageTenths[1];
    SMMIMeasurement.psm1Frequency = measurement.psmFrequency[0];
    SMMIMeasurement.psm2Frequency = measurement.psmFrequency[1];
  }

  TransmitLegacyFrame(sizeof(tSMMIMeasurement),
                      CAN_MMI_RUNTIME_MEASUREMENT_ANSWER_STD_ID,
                      &SMMIMeasurement);
}

void StreamDateTime(void)
{
  tSMMITime SMMITime;
  MmiLegacyTime_t timeValue;

  (void) memset(&SMMITime, 0, sizeof(SMMITime));
  (void) memset(&timeValue, 0, sizeof(timeValue));
  if (MmiLegacyStatusReadTime(&g_mmiLegacyStatusPort, &timeValue) != FALSE)
  {
    SMMITime.bTimeSource = timeValue.timeSource;
    SMMITime.bSeconds = timeValue.second;
    SMMITime.bMinutes = timeValue.minute;
    SMMITime.bHours = timeValue.hour;
    SMMITime.bDay = timeValue.day;
    SMMITime.bMonth = timeValue.month;
    SMMITime.bYear = timeValue.year;
  }

  TransmitLegacyFrame(sizeof(tSMMITime),
                      CAN_MMI_RUNTIME_TIME_ANSWER_STD_ID,
                      &SMMITime);
}

void StreamGPRSImei(void)
{
  const char *imei = MmiLegacyStatusGetGprsImei(&g_mmiLegacyStatusPort);
  uint8_t bDataLen = FDCAN_DATA_MAX_LEN;
  uint8_t txBuffer[8];

  (void) memset(&txBuffer[0], 0, sizeof(txBuffer));
  if (imei != NULL)
  {
    (void) memcpy(&txBuffer[0], imei, bDataLen);
  }
  TransmitLegacyFrame(bDataLen, CAN_MMI_STD_ID_GPRS_MODEM_IMEI_PART1, &txBuffer[0]);

  bDataLen = MMI_LEGACY_IMEI_LEN % FDCAN_DATA_MAX_LEN;
  (void) memset(&txBuffer[0], 0, sizeof(txBuffer));
  if (imei != NULL)
  {
    (void) memcpy(&txBuffer[0], &imei[FDCAN_DATA_MAX_LEN], bDataLen);
  }
  TransmitLegacyFrame(bDataLen, CAN_MMI_STD_ID_GPRS_MODEM_IMEI_PART2, &txBuffer[0]);
}

void StreamUSRMAC(void)
{
  const char *mac = MmiLegacyStatusGetUsrMac(&g_mmiLegacyStatusPort);
  uint8_t bDataLen = FDCAN_DATA_MAX_LEN;
  uint8_t txBuffer[8];

  (void) memset(&txBuffer[0], 0, sizeof(txBuffer));
  if (mac != NULL)
  {
    (void) memcpy(&txBuffer[0], mac, bDataLen);
  }
  TransmitLegacyFrame(bDataLen, CAN_MMI_STD_ID_USR_MAC_PART1, &txBuffer[0]);

  bDataLen = MMI_LEGACY_MAC_LEN % FDCAN_DATA_MAX_LEN;
  (void) memset(&txBuffer[0], 0, sizeof(txBuffer));
  if (mac != NULL)
  {
    (void) memcpy(&txBuffer[0], &mac[FDCAN_DATA_MAX_LEN], bDataLen);
  }
  TransmitLegacyFrame(bDataLen, CAN_MMI_STD_ID_USR_MAC_PART2, &txBuffer[0]);
}

void StreamEthernetMAC(void)
{
  const char *mac = MmiLegacyStatusGetEthernetMac(&g_mmiLegacyStatusPort);
  uint8_t bDataLen = FDCAN_DATA_MAX_LEN;
  uint8_t txBuffer[8];

  (void) memset(&txBuffer[0], 0, sizeof(txBuffer));
  if (mac != NULL)
  {
    (void) memcpy(&txBuffer[0], mac, bDataLen);
  }
  TransmitLegacyFrame(bDataLen, CAN_MMI_STD_ID_USR_MAC_PART1, &txBuffer[0]);

  bDataLen = MMI_LEGACY_MAC_LEN % FDCAN_DATA_MAX_LEN;
  (void) memset(&txBuffer[0], 0, sizeof(txBuffer));
  if (mac != NULL)
  {
    (void) memcpy(&txBuffer[0], &mac[FDCAN_DATA_MAX_LEN], bDataLen);
  }
  TransmitLegacyFrame(bDataLen, CAN_MMI_STD_ID_USR_MAC_PART2, &txBuffer[0]);
}

void StreamGsmOperator(void)
{
  const char *operatorName = MmiLegacyStatusGetGsmOperator(&g_mmiLegacyStatusPort);
  uint8_t txBuffer[8];

  (void) memset(&txBuffer[0], 0, sizeof(txBuffer));
  if (operatorName != NULL)
  {
    (void) memcpy(&txBuffer[0], operatorName, FDCAN_DATA_MAX_LEN);
  }
  TransmitLegacyFrame(FDCAN_DATA_MAX_LEN,
                      CAN_MMI_STD_ID_GPRS_GSM_OPERATOR,
                      &txBuffer[0]);
}

void StreamGPRSState(void)
{
  tSMMIGprsLog SMMIGprsLog;
  MmiLegacyGprsLog_t gprsLog;

  memset(&SMMIGprsLog, 0, sizeof(SMMIGprsLog));
  (void) memset(&gprsLog, 0, sizeof(gprsLog));
  if (MmiLegacyStatusReadGprsLog(&g_mmiLegacyStatusPort, &gprsLog) != FALSE)
  {
    SMMIGprsLog.bModem = gprsLog.modemType;
    SMMIGprsLog.bState = gprsLog.state;
    SMMIGprsLog.bSubState = gprsLog.subState;
    SMMIGprsLog.bSignalQuality = gprsLog.signalQuality;
  }

  TransmitLegacyFrame(sizeof(tSMMIGprsLog),
                      CAN_MMI_GET_GPRS_MODEM_LOG_ANSWER_STD_ID,
                      &SMMIGprsLog);
}

void StreamOperationRuntime(void)
{
  tSMMIWorkmode SMMIWorkmode;
  MmiLegacyWorkmode_t workmode;

  (void) memset(&SMMIWorkmode, 0, sizeof(SMMIWorkmode));
  (void) memset(&workmode, 0, sizeof(workmode));
  if (MmiLegacyStatusReadWorkmode(&g_mmiLegacyStatusPort, &workmode) != FALSE)
  {
    SMMIWorkmode.bState = workmode.state;
    SMMIWorkmode.bArg1 = workmode.arg1;
    SMMIWorkmode.bArg2 = workmode.arg2;
    SMMIWorkmode.bArg3 = workmode.arg3;
    SMMIWorkmode.bArg4 = workmode.arg4;
    SMMIWorkmode.bArg5 = workmode.arg5;
    SMMIWorkmode.bArg6 = workmode.arg6;
    SMMIWorkmode.bArg7 = workmode.arg7;
    SMMIWorkmode.bArg8 = workmode.arg8;
  }

  TransmitLegacyFrame(sizeof(tSMMIWorkmode),
                      CAN_MMI_RUNTIME_WORKMODE_ANSWER_STD_ID,
                      &SMMIWorkmode);
} /* StreamOperationRuntime */

void StreamModuleRuntime(void)
{
  tSMMIModule SMMIModule;
  MmiLegacyModuleStatus_t moduleStatus;

  (void) memset(&SMMIModule, 0, sizeof(SMMIModule));
  (void) memset(&moduleStatus, 0, sizeof(moduleStatus));
  if (MmiLegacyStatusReadModuleStatus(&g_mmiLegacyStatusPort,
                                      &moduleStatus) != FALSE)
  {
    SMMIModule.fGPSModemConnected = moduleStatus.gpsModemConnected;
    SMMIModule.fGPSAntennaConnected = moduleStatus.gpsAntennaConnected;
    SMMIModule.fGPRSModemConnected = moduleStatus.gprsModemConnected;
    SMMIModule.fGPRSCenterConnected = moduleStatus.gprsCenterConnected;
    SMMIModule.fIsRelayClosed = moduleStatus.relayClosed;
    SMMIModule.bLastDigitalInputDemand = moduleStatus.lastDigitalInputDemand;
    SMMIModule.bLastLoopDedectorDemand = moduleStatus.lastLoopDetectorDemand;
    SMMIModule.bGPSModeType = moduleStatus.gpsModeType;
  }

  TransmitLegacyFrame(sizeof(tSMMIModule),
                      CAN_MMI_MODULE_STATUS_ANSWER_STD_ID,
                      &SMMIModule);
}

void StreamGateStateChanged(uint8_t fState, tpSLogRecord pSLog)
{
  tSMMICabinetDoorStateChange SMMICabinetDoorStateChange;

  SMMICabinetDoorStateChange.fState = fState;

  /*  Cabinet Door Closed */
  SMMICabinetDoorStateChange.bEvent =
    (fState != FALSE) ? MMI_LEGACY_EVENT_DOOR_CLOSED : MMI_LEGACY_EVENT_DOOR_OPEN;
  SMMICabinetDoorStateChange.bSeconds = pSLog->bSeconds;
  SMMICabinetDoorStateChange.bMinutes = pSLog->bMinutes;
  SMMICabinetDoorStateChange.bHours = pSLog->bHours;
  SMMICabinetDoorStateChange.bDay = pSLog->bMonthDay;
  SMMICabinetDoorStateChange.bMonth = pSLog->bMonth;
  SMMICabinetDoorStateChange.bYear = (uint8_t) (pSLog->sYear % 100U);

  TransmitLegacyFrame(sizeof(tSMMICabinetDoorStateChange),
                      CAN_MMI_STD_ID_CABINET_DOOR_STATE_CHANGE,
                      &SMMICabinetDoorStateChange);
}

void StreamErrorRuntime(void)
{
  tSMMIError SMMIError;
  uint8_t bSetNo = 0;
  uint8_t bSetTotal = MmiLegacyStatusGetSetTotal(&g_mmiLegacyStatusPort);

  memset(&SMMIError, 0, sizeof(tSMMIError));

  for (bSetNo = 0; bSetNo < bSetTotal; bSetNo++)
  {
    MmiLegacyErrorRecord_t errorRecord;

    (void) memset(&errorRecord, 0, sizeof(errorRecord));
    if (MmiLegacyStatusReadErrorRecord(&g_mmiLegacyStatusPort,
                                       bSetNo,
                                       &errorRecord) == FALSE)
    {
      continue;
    }

    SMMIError.bSetNo = errorRecord.setNumber;
    SMMIError.bSignalingMode = errorRecord.signalingMode;
    SMMIError.bSigModeSource = errorRecord.signalingModeSource;
    SMMIError.bParam1 = errorRecord.param1;
    SMMIError.bParam2 = errorRecord.param2;

    TransmitLegacyFrame(sizeof(SMMIError),
                        CAN_MMI_ERROR_ANSWER_STD_ID,
                        &SMMIError);
  }
}

void StreamSignals(void)
{
  tSMMISGSignals SMMISGSignals;
  uint8_t blockIndex;
  uint16_t signalWords[CAN_MMI_MAX_SSM_PER_MSG];

  if (SMMIRuntime.SFlags.fSignalStream)
  {
    for (blockIndex = 0U; blockIndex < 2U; ++blockIndex)
    {
      (void) memset(&SMMISGSignals, 0, sizeof(SMMISGSignals));
      (void) memset(&signalWords[0], 0, sizeof(signalWords));
      if (MmiLegacyStatusReadSignalsBlock(&g_mmiLegacyStatusPort,
                                          blockIndex,
                                          &signalWords[0])
          == FALSE)
      {
        continue;
      }
      (void) memcpy(&SMMISGSignals.SMMISSMSignals[0],
                    &signalWords[0],
                    sizeof(signalWords));

      TransmitLegacyFrame(sizeof(tSMMISGSignals),
                          CAN_MMI_GET_SIGNALS_ANSWER_STD_ID,
                          &SMMISGSignals);
    }
  }
} /* StreamSignals */

void StreamInputs(void)
{
  tSMMIInputs SMMIInputs;
  uint32_t loopDemands = 0U;
  uint32_t digitalDemands = 0U;

  if (SMMIRuntime.SFlags.fInputStream)
  {
    memset(&SMMIInputs, 0, sizeof(SMMIInputs));
    (void) MmiLegacyStatusReadInputs(&g_mmiLegacyStatusPort,
                                     &loopDemands,
                                     &digitalDemands);
    SMMIInputs.lLoopDemands = loopDemands;
    SMMIInputs.lDigitalInputDemands = digitalDemands;

    TransmitLegacyFrame(sizeof(SMMIInputs),
                        CAN_MMI_GET_INPUTS_ANSWER_STD_ID,
                        &SMMIInputs);
  }
}

void StreamSOTest(void)
{
  tSMMISOStream1 SMMISOStream1;
  tSMMISOStream2 SMMISOStream2;
  MmiMaintenanceOutputTestStatus_t status;

  if (SMMIRuntime.SFlags.fSOTestStream)
  {
    (void) memset(&SMMISOStream1, 0, sizeof(SMMISOStream1));
    (void) memset(&SMMISOStream2, 0, sizeof(SMMISOStream2));
    (void) memset(&status, 0, sizeof(status));
    if (MmiMaintenanceServiceReadOutputTestStatus(&g_mmiMaintenanceService,
                                                  &status) == FALSE)
    {
      return;
    }

    SMMISOStream1.bSONo = status.outputNumber;
    SMMISOStream1.sPowerNet = status.powerNet;
    SMMISOStream1.sPower = status.power;
    SMMISOStream1.bState = status.state;
    SMMISOStream1.sNet = status.net;

    SMMISOStream2.bSONo = status.outputNumber;
    SMMISOStream2.sNow = status.currentNow;
    SMMISOStream2.sMax = status.currentMax;
    SMMISOStream2.sMin = status.currentMin;

    TransmitLegacyFrame(sizeof(tSMMISOStream1),
                        CAN_MMI_SO_TEST_STREAM_1_STD_ID,
                        &SMMISOStream1);
    TransmitLegacyFrame(sizeof(tSMMISOStream2),
                        CAN_MMI_SO_TEST_STREAM_2_STD_ID,
                        &SMMISOStream2);
  }
} /* StreamSOTest */

void ParseMMIRequest(tSFDCANRxMsg *pcanMMIRequest)
{
  tSFDCANTxMsg canMMIAnswer;

  memset(&canMMIAnswer, 0, sizeof(tSFDCANTxMsg));

  switch (pcanMMIRequest->RxHeader.Identifier)
  {
      case CAN_MMI_STD_ID_LAST_MMI_CLOSE_LOG_REQUEST:
      case CAN_MMI_STD_ID_LAST_MMI_OPEN_LOG_REQUEST:
      {
        uint16_t sLogIndex = MMI_PROTOCOL_V2_EVENT_CURSOR_NONE;
        MmiEventRecordV2_t logRecord;
        tSMMICabinetDoorStateChange SMMIGateState;

        memset(&logRecord, 0, sizeof(logRecord));
        if (ReadLatestDoorLogIndex(
              (uint8_t) (pcanMMIRequest->RxHeader.Identifier
                         == CAN_MMI_STD_ID_LAST_MMI_CLOSE_LOG_REQUEST),
              &sLogIndex)
            && (sLogIndex != MMI_PROTOCOL_V2_EVENT_CURSOR_NONE)
            && ReadEventLogRecord(sLogIndex, &logRecord))
        {
          SMMIGateState.fState = (logRecord.eventCode
                                  == MMI_LEGACY_EVENT_DOOR_CLOSED) ? TRUE
                                 : FALSE;
          SMMIGateState.bEvent = logRecord.eventCode;
          SMMIGateState.bSeconds = logRecord.second;
          SMMIGateState.bMinutes = logRecord.minute;
          SMMIGateState.bHours = logRecord.hour;
          SMMIGateState.bDay = logRecord.day;
          SMMIGateState.bMonth = logRecord.month;
          SMMIGateState.bYear = (uint8_t) (logRecord.year % 100U);

          TransmitLegacyFrame(sizeof(tSMMICabinetDoorStateChange),
                              (pcanMMIRequest->RxHeader.Identifier
                               == CAN_MMI_STD_ID_LAST_MMI_CLOSE_LOG_REQUEST)
                              ? CAN_MMI_STD_ID_LAST_MMI_CLOSE_LOG_ANSWER
                              : CAN_MMI_STD_ID_LAST_MMI_OPEN_LOG_ANSWER,
                              &SMMIGateState);
        }

        break;
      }

      case CAN_MMI_VERSTON_REQUEST_STD_ID:
      {
        tSMMIVersion SMMIVersion;

        SMMIVersion.bArg0 = MMI_LEGACY_VERSION_ARG0;
        SMMIVersion.bArg1 = MMI_LEGACY_VERSION_ARG1;
        SMMIVersion.bArg2 = MMI_LEGACY_VERSION_ARG2;
        SMMIVersion.bArg3 = MMI_LEGACY_VERSION_ARG3;
        SMMIVersion.bArg4 = MMI_LEGACY_VERSION_ARG4;

        TransmitLegacyFrame(sizeof(tSMMIVersion),
                            CAN_MMI_VERSTON_ANSWER_STD_ID,
                            &SMMIVersion);
        break;
      }

      case CAN_MMI_RUNTIME_MEASUREMENT_REQUEST_STD_ID:
      {
        StreamPSMMeasurements();
        break;
      }

      case CAN_MMI_RUNTIME_WORKMODE_REQUEST_STD_ID:
      {
        StreamOperationRuntime();
        break;
      }

      case CAN_MMI_RUNTIME_TIME_REQUEST_STD_ID:
      {
        StreamDateTime();
        break;
      }

      case CAN_MMI_MODULE_STATUS_REQUEST_STD_ID:
      {
        tSMMIModule SMMIModule;
        MmiLegacyModuleStatus_t moduleStatus;

        memset(&SMMIModule, 0, sizeof(SMMIModule));
        memset(&moduleStatus, 0, sizeof(moduleStatus));
        if (MmiLegacyStatusReadModuleStatus(&g_mmiLegacyStatusPort,
                                            &moduleStatus) != FALSE)
        {
          SMMIModule.fGPSModemConnected = moduleStatus.gpsModemConnected;
          SMMIModule.fGPSAntennaConnected = moduleStatus.gpsAntennaConnected;
          SMMIModule.fGPRSModemConnected = moduleStatus.gprsModemConnected;
          SMMIModule.fGPRSCenterConnected = moduleStatus.gprsCenterConnected;
          SMMIModule.fIsRelayClosed = moduleStatus.relayClosed;
          SMMIModule.bLastDigitalInputDemand =
            moduleStatus.lastDigitalInputDemand;
          SMMIModule.bLastLoopDedectorDemand =
            moduleStatus.lastLoopDetectorDemand;
          SMMIModule.bGPSModeType = moduleStatus.gpsModeType;
        }

        TransmitLegacyFrame(sizeof(tSMMIModule),
                            CAN_MMI_MODULE_STATUS_ANSWER_STD_ID,
                            &SMMIModule);
        break;
      }

      case CAN_MMI_ERROR_REQUEST_STD_ID:
      {
        tSMMIError SMMIError;

        uint8_t bSetNo = 0;
        uint8_t bSetTotal = MmiLegacyStatusGetSetTotal(&g_mmiLegacyStatusPort);

        memset(&SMMIError, 0, sizeof(tSMMIError));

        for (bSetNo = 0; bSetNo < bSetTotal; bSetNo++)
        {
          MmiLegacyErrorRecord_t errorRecord;

          memset(&errorRecord, 0, sizeof(errorRecord));
          if (MmiLegacyStatusReadErrorRecord(&g_mmiLegacyStatusPort,
                                             bSetNo,
                                             &errorRecord))
          {
            SMMIError.bSetNo = errorRecord.setNumber;
            SMMIError.bSignalingMode = errorRecord.signalingMode;
            SMMIError.bSigModeSource = errorRecord.signalingModeSource;
            SMMIError.bParam1 = errorRecord.param1;
            SMMIError.bParam2 = errorRecord.param2;

            TransmitLegacyFrame(sizeof(SMMIError),
                                CAN_MMI_ERROR_ANSWER_STD_ID,
                                &SMMIError);
            break;
          }
        }

        break;
      }

      case CAN_MMI_CHANGE_MODE_REQUEST_STD_ID:
      {
        tSMMIChangeMode SMMIChangeMode;

        memcpy(&SMMIChangeMode,
               pcanMMIRequest->Data,
               pcanMMIRequest->RxHeader.DataLength);
        (void) MmiMaintenanceServiceRequestModeControl(
          &g_mmiMaintenanceService,
          SMMIChangeMode.bRequestedMode);

        break;
      }

      case CAN_MMI_SET_TIME_REQUEST_STD_ID:
      {
        tSMMISetTime SMMISetTime;
        MmiLocalClockSettingsV2_t clockSettings;

        memset(&clockSettings, 0, sizeof(clockSettings));
        memset(&SMMISetTime, 0, sizeof(SMMISetTime));

        memcpy(&SMMISetTime,
               pcanMMIRequest->Data,
               pcanMMIRequest->RxHeader.DataLength);

        if (ReadLocalClockSettings(&clockSettings))
        {
          clockSettings.second = SMMISetTime.bSecond;
          clockSettings.minute = SMMISetTime.bMinute;
          clockSettings.hour = SMMISetTime.bHour;
          clockSettings.day = SMMISetTime.bDay;
          clockSettings.month = SMMISetTime.bMonth;
          clockSettings.year = SMMISetTime.bYear;
          (void) WriteLocalClockSettings(&clockSettings);
        }

        break;
      }

      case CAN_MMI_GET_LOG_REQUEST_STD_ID:
      {
        uint8_t fValidLogSent = FALSE;
        tSMMIGetLogRequest SMMIGetLogRequest;
        tSMMILogTime SMMILogTime;
        tSMMILogContent SMMILogContent;
        MmiEventRecordV2_t logRecord;

        memset(&SMMIGetLogRequest, 0, sizeof(tSMMIGetLogRequest));
        memset(&SMMILogTime, 0, sizeof(tSMMILogTime));
        memset(&SMMILogContent, 0, sizeof(tSMMILogContent));
        memset(&logRecord, 0, sizeof(logRecord));

        memcpy(&SMMIGetLogRequest,
               pcanMMIRequest->Data,
               pcanMMIRequest->RxHeader.DataLength);

        if (MmiEventLogServiceCanReadFromIndex(&g_mmiEventLogService,
                                               SMMIGetLogRequest.sLogIndex)
            && ReadEventLogRecord(SMMIGetLogRequest.sLogIndex, &logRecord))
        {
          SMMILogContent.bLog = logRecord.eventCode;
          SMMILogContent.bParam = logRecord.eventParam;
          SMMILogContent.sParam = logRecord.eventShortParam;
          SMMILogContent.lParam = logRecord.eventLongParam;

          SMMILogTime.bSecond = logRecord.second;
          SMMILogTime.bMinute = logRecord.minute;
          SMMILogTime.bHour = logRecord.hour;
          SMMILogTime.bDay = logRecord.day;
          SMMILogTime.bMonth = logRecord.month;
          SMMILogTime.bYear = (uint8_t) (logRecord.year % 100U);

          TransmitLegacyFrame(sizeof(tSMMILogTime),
                              CAN_MMI_GET_LOG_ANSWER_DATE_TIME_STD_ID,
                              &SMMILogTime);
          TransmitLegacyFrame(sizeof(tSMMILogContent),
                              CAN_MMI_GET_LOG_ANSWER_CONTENT_STD_ID,
                              &SMMILogContent);

          fValidLogSent = TRUE;
        }

        if (!fValidLogSent)
        {
          TransmitLegacyFrame(sizeof(tSMMILogTime),
                              CAN_MMI_GET_LOG_ANSWER_DATE_TIME_STD_ID,
                              &SMMILogTime);
          TransmitLegacyFrame(sizeof(tSMMILogContent),
                              CAN_MMI_GET_LOG_ANSWER_CONTENT_STD_ID,
                              &SMMILogContent);
        }

        break;
      }

      case CAN_MMI_GET_LAST_LOG_INDEX_REQUEST_STD_ID:
      {
        tSMMILastLogIndex SMMILastLogIndex;
        uint16_t latestLogIndex = MMI_PROTOCOL_V2_EVENT_CURSOR_NONE;

        if ((ReadLatestEventLogIndex(&latestLogIndex) != FALSE)
            && (latestLogIndex != MMI_PROTOCOL_V2_EVENT_CURSOR_NONE))
        {
          SMMILastLogIndex.sMMILastLogIndex = latestLogIndex;
        }
        else
        {
          SMMILastLogIndex.sMMILastLogIndex = CAN_MMI_LOG_INDEX_MAX_VALUE;
        }

        TransmitLegacyFrame(sizeof(tSMMILastLogIndex),
                            CAN_MMI_GET_LAST_LOG_INDEX_ANSWER_STD_ID,
                            &SMMILastLogIndex);

        break;
      }

      case CAN_MMI_GET_GPRS_MODEM_LOG_REQUEST_STD_ID:
      {
        StreamGPRSState();
        break;
      }

      case CAN_MMI_SET_RELAY_STATE_REQUEST_STD_ID:
      {
        tSMMIRelayState SMMIRelayState;

        memcpy(&SMMIRelayState,
               pcanMMIRequest->Data,
               pcanMMIRequest->RxHeader.DataLength);
        (void) MmiMaintenanceServiceRequestRelayState(
          &g_mmiMaintenanceService,
          SMMIRelayState.bRelayStateRequest);
        break;
      }

      case CAN_MMI_SET_GPS_PORT_REQUEST_STD_ID:
      {
        tSMMIGpsSettingsPort SMMIGpsSettingsPort;
        MmiLocalGpsSettingsV2_t gpsSettings;

        memcpy(&SMMIGpsSettingsPort,
               pcanMMIRequest->Data,
               pcanMMIRequest->RxHeader.DataLength);

        if (ReadLocalGpsSettings(&gpsSettings)
            && (gpsSettings.gpsPortType
                != SMMIGpsSettingsPort.bGpsPortRequest))
        {
          gpsSettings.gpsPortType = SMMIGpsSettingsPort.bGpsPortRequest;
          if (WriteLocalGpsSettings(&gpsSettings))
          {
            SystemResetPortRequest(&g_systemResetPort);
          }
        }

        break;
      }

      case CAN_MMI_SET_GPS_BAUD_RATE_REQUEST_STD_ID:
      {
        tSMMIGpsSettingsBaudRateIndex SMMIGpsSettingsBaudRateIndex;
        MmiLocalGpsSettingsV2_t gpsSettings;

        memcpy(&SMMIGpsSettingsBaudRateIndex,
               pcanMMIRequest->Data,
               pcanMMIRequest->RxHeader.DataLength);

        if (ReadLocalGpsSettings(&gpsSettings)
            && (gpsSettings.gpsBaudRateIndex
                != SMMIGpsSettingsBaudRateIndex.bGpsBaudRateIndexRequest))
        {
          gpsSettings.gpsBaudRateIndex =
            SMMIGpsSettingsBaudRateIndex.bGpsBaudRateIndexRequest;
          if (WriteLocalGpsSettings(&gpsSettings))
          {
            tSMMIGpsSettingsBaudRateIndex SMMIBaudRateAnswer;

            TransmitLegacyAck(CAN_MMI_SET_GPS_BAUD_RATE_ANSWER_STD_ID, TRUE);

            SMMIBaudRateAnswer.bGpsBaudRateIndexRequest =
              gpsSettings.gpsBaudRateIndex;

            TransmitLegacyFrame(sizeof(tSMMIGpsSettingsBaudRateIndex),
                                CAN_MMI_GET_GPS_BAUD_RATE_ANSWER_STD_ID,
                                &SMMIBaudRateAnswer);

            SystemResetPortRequest(&g_systemResetPort);
          }
          else
          {
            TransmitLegacyAck(CAN_MMI_SET_GPS_BAUD_RATE_ANSWER_STD_ID, FALSE);
          }
        }

        break;
      }

      case CAN_MMI_GET_GPS_BAUD_RATE_REQUEST_STD_ID:
      {
        tSMMIGpsSettingsBaudRateIndex SMMIGpsSettingsBaudRateIndex;
        MmiLocalGpsSettingsV2_t gpsSettings;

        memset(&SMMIGpsSettingsBaudRateIndex, 0, sizeof(SMMIGpsSettingsBaudRateIndex));
        if (ReadLocalGpsSettings(&gpsSettings))
        {
          SMMIGpsSettingsBaudRateIndex.bGpsBaudRateIndexRequest =
            gpsSettings.gpsBaudRateIndex;
        }

        TransmitLegacyFrame(sizeof(tSMMIGpsSettingsBaudRateIndex),
                            CAN_MMI_GET_GPS_BAUD_RATE_ANSWER_STD_ID,
                            &SMMIGpsSettingsBaudRateIndex);
        break;
      }

      /* H&D Commented */

      /*
       *  case CAN_MMI_SET_HEATER_SETTINGS_REQUEST_STD_ID:
       *  {
       *  tSMMIHeater SMMIHeater;
       *  tSHeaterLampDim SHeater;
       *  memcpy(&SMMIHeater, pcanMMIRequest->Data,
       *  pcanMMIRequest->RxHeader.DataLength);
       *
       *  SHeater.fLogicLevel = SMMIHeater.bLogicLevel;
       *  SHeater.fState      = SMMIHeater.bState;
       *
       *  HeaterInfoSave(&SHeater);
       *  }
       *  break;
       *
       *  case CAN_MMI_SET_DIMMING_SETTINGS_REQUEST_STD_ID:
       *  {
       *  tSMMIDimming SMMIDimming;
       *  tSHeaterLampDim SDimm;
       *  memcpy(&SMMIDimming, pcanMMIRequest->Data,
       *  pcanMMIRequest->RxHeader.DataLength);
       *
       *  SDimm.fLogicLevel = SMMIDimming.bLogicLevel;
       *  SDimm.fState      = SMMIDimming.bState;
       *
       *  LampDimmingInfoSave(&SDimm);
       *  }
       *  break;
       */

      case CAN_MMI_SET_USER_SETTINGS_REQUEST_STD_ID:
      {
        tSMMIUserSettings SMMIUserSettings;
        MmiLocalUserFlagsV2_t settings;

        memcpy(&SMMIUserSettings,
               pcanMMIRequest->Data,
               pcanMMIRequest->RxHeader.DataLength);

        if (ReadLocalFlags(&settings))
        {
          settings.configFlag = SMMIUserSettings.fConfigFlag;
          (void) WriteLocalFlags(&settings);
        }

        break;
      }

      case CAN_MMI_GET_USER_SETTINGS_REQUEST_STD_ID:
      {
        tSMMIUserSettings SMMIUserSettings;
        MmiLocalUserFlagsV2_t settings;

        memset(&SMMIUserSettings, 0, sizeof(tSMMIUserSettings));
        if (ReadLocalFlags(&settings))
        {
          SMMIUserSettings.fSettingsChanged = MMI_LEGACY_USER_SETTINGS_CHANGED;
          SMMIUserSettings.fConfigFlag = settings.configFlag;
          SMMIUserSettings.fLogFlag = settings.logFlag;
          SMMIUserSettings.fTrafficCountsFlag = settings.trafficCountsFlag;
        }

        TransmitLegacyFrame(sizeof(tSMMIUserSettings),
                            CAN_MMI_GET_USER_SETTINGS_ANSWER_STD_ID,
                            &SMMIUserSettings);
        break;
      }

      case CAN_MMI_SET_USER_SETTINGS_PART2_REQUEST_STD_ID:
      {
        tSMMIUserSettingsPart2 SMMIUserSettingsPart2;
        MmiLocalUserFlagsV2_t settings;

        memcpy(&SMMIUserSettingsPart2,
               pcanMMIRequest->Data,
               pcanMMIRequest->RxHeader.DataLength);
        if (ReadLocalFlags(&settings))
        {
          settings.standbyInfoFlag = SMMIUserSettingsPart2.fStandbyInfoFlag;
          (void) WriteLocalFlags(&settings);
        }

        break;
      }

      case CAN_MMI_GET_USER_SETTINGS_PART2_REQUEST_STD_ID:
      {
        tSMMIUserSettingsPart2 SMMIUserSettingsPart2;
        MmiLocalUserFlagsV2_t settings;

        memset(&SMMIUserSettingsPart2, 0, sizeof(tSMMIUserSettingsPart2));
        if (ReadLocalFlags(&settings))
        {
          SMMIUserSettingsPart2.fStandbyInfoFlag =
            settings.standbyInfoFlag;
        }

        TransmitLegacyFrame(sizeof(tSMMIUserSettingsPart2),
                            CAN_MMI_GET_USER_SETTINGS_PART2_ANSWER_STD_ID,
                            &SMMIUserSettingsPart2);
        break;
      }

      case CAN_MMI_SET_BROKEN_INPUT_SETTINGS_REQUEST_STD_ID:
      {
        tSMMIBrokenInputSettings SMMISettings;
        MmiLocalBrokenInputSettingsV2_t settings;

        memcpy(&SMMISettings,
               pcanMMIRequest->Data,
               pcanMMIRequest->RxHeader.DataLength);
        memset(&settings, 0, sizeof(settings));
        settings.loopInputFlag = SMMISettings.fLoopInputFlag;
        settings.digitalInputFlag = SMMISettings.fDigitalInputFlag;

        if (WriteLocalBrokenInputSettings(&settings))
        {
          TransmitLegacyAck(CAN_MMI_SET_BROKEN_INPUT_SETTINGS_ANSWER_STD_ID,
                            TRUE);
        }
        else
        {
          TransmitLegacyAck(CAN_MMI_SET_BROKEN_INPUT_SETTINGS_ANSWER_STD_ID,
                            FALSE);
        }
        break;
      }

      case CAN_MMI_GET_BROKEN_INPUT_SETTINGS_REQUEST_STD_ID:
      {
        tSMMIBrokenInputSettings SMMISettings;
        MmiLocalBrokenInputSettingsV2_t settings;

        memset(&SMMISettings, 0, sizeof(SMMISettings));
        if (ReadLocalBrokenInputSettings(&settings))
        {
          SMMISettings.fAlreadySet = MMI_LEGACY_BROKEN_INPUT_SET;
          SMMISettings.fDigitalInputFlag = settings.digitalInputFlag;
          SMMISettings.fLoopInputFlag = settings.loopInputFlag;
        }

        TransmitLegacyFrame(sizeof(SMMISettings),
                            CAN_MMI_GET_BROKEN_INPUT_SETTINGS_ANSWER_STD_ID,
                            &SMMISettings);
        break;
      }

      case CAN_MMI_SET_DAYLIGHT_SAVING_TIME_SETTINGS_REQUEST_STD_ID:
      {
        tSDaylightSavingTimeSettings SDaylightSavingTimeSettings;
        MmiLocalClockSettingsV2_t clockSettings;

        memset(&SDaylightSavingTimeSettings, 0,
               sizeof(tSDaylightSavingTimeSettings));
        memset(&clockSettings, 0, sizeof(clockSettings));

        memcpy(&SDaylightSavingTimeSettings,
               pcanMMIRequest->Data,
               pcanMMIRequest->RxHeader.DataLength);

        if (ReadLocalClockSettings(&clockSettings))
        {
          clockSettings.daylightSavingEnabled =
            SDaylightSavingTimeSettings.fDaylightSavingTimeFlag;
          (void) WriteLocalClockSettings(&clockSettings);
        }

        break;
      }

      case CAN_MMI_GET_DAYLIGHT_SAVING_TIME_SETTINGS_REQUEST_STD_ID:
      {
        tSDaylightSavingTimeSettings SDaylightSavingTimeSettings;
        MmiLocalClockSettingsV2_t clockSettings;

        memset(&SDaylightSavingTimeSettings, 0,
               sizeof(tSDaylightSavingTimeSettings));
        memset(&clockSettings, 0, sizeof(clockSettings));
        if (ReadLocalClockSettings(&clockSettings))
        {
          SDaylightSavingTimeSettings.fDaylightSavingTimeFlag =
            clockSettings.daylightSavingEnabled;
        }

        TransmitLegacyFrame(sizeof(tSDaylightSavingTimeSettings),
                            CAN_MMI_GET_DAYLIGHT_SAVING_TIME_SETTINGS_ANSWER_STD_ID,
                            &SDaylightSavingTimeSettings);
        break;
      }

      case CAN_MMI_GET_ADMIN_USER_INFO_REQUEST_STD_ID:
      {
        MmiLocalAdminInfoV2_t adminInfo;
        uint16_t adminInfoLength = 0U;
        uint16_t sAdminUsername = 0U;
        uint16_t sAdminPassword = 0xFFFFU;

        memset(&adminInfo, 0, sizeof(adminInfo));
        (void) MmiLocalSettingsServiceRead(&g_mmiLocalSettingsService,
                                           MMI_PROTOCOL_V2_LOCAL_RESOURCE_ADMIN,
                                           (uint8_t *) &adminInfo,
                                           &adminInfoLength);
        sAdminUsername = adminInfo.adminUsername;

        memcpy(&canMMIAnswer.Data[0], &sAdminUsername, 2);
        memcpy(&canMMIAnswer.Data[4], &sAdminPassword, 2);
        TransmitLegacyFrame(sizeof(canMMIAnswer.Data),
                            CAN_MMI_GET_ADMIN_USER_INFO_ANSWER_STD_ID,
                            &canMMIAnswer.Data[0]);
        break;
      }

      case CAN_MMI_SET_ADMIN_USER_INFO_REQUEST_STD_ID:
      {
        TransmitLegacyAck(CAN_MMI_SET_ADMIN_USER_INFO_ANSWER_STD_ID, FALSE);
        break;
      }

      case CAN_MMI_GET_SIGNALS_REQUEST_STD_ID:
      {
        StreamSignals();
        break;
      }

      case CAN_MMI_START_SIGNAL_STREAM_STD_ID:
      {
        SMMIRuntime.SFlags.fSignalStream = TRUE;
        break;
      }

      case CAN_MMI_STOP_SIGNAL_STREAM_STD_ID:
      {
        SMMIRuntime.SFlags.fSignalStream = FALSE;
        break;
      }

      case CAN_MMI_GET_INPUTS_REQUEST_STD_ID:
      {
        StreamInputs();
        break;
      }

      case CAN_MMI_START_INPUT_STREAM_STD_ID:
      {
        SMMIRuntime.SFlags.fInputStream = TRUE;
        break;
      }

      case CAN_MMI_STOP_INPUT_STREAM_STD_ID:
      {
        SMMIRuntime.SFlags.fInputStream = FALSE;
        break;
      }

      /* H&D Commented */

      /*
       *  case CAN_MMI_GET_HEATER_SETTINGS_REQUEST_STD_ID:
       *  {
       *  tSMMIHeater SMMIHeater;
       *  tSHeaterLampDim SHeater;
       *  HeaterInfoGet(&SHeater);
       *
       *  SMMIHeater.bLogicLevel  = SHeater.fLogicLevel;
       *  SMMIHeater.bState       = SHeater.fState;
       *
       *  canMMIAnswer.TxHeader.DataLength    = sizeof(tSMMIHeater);
       *  canMMIAnswer.TxHeader.Identifier  =
       *  CAN_MMI_GET_HEATER_SETTINGS_ANSWER_STD_ID; memcpy(canMMIAnswer.Data,
       *  &SMMIHeater, sizeof(tSMMIHeater));
       *  CANTxRequest(canMMIAnswer.TxHeader.DataLength, CAN_ID_TYPE_STD,
       *  canMMIAnswer.TxHeader.Identifier, (uint8_t*)canMMIAnswer.Data);
       *  }
       *  break;
       *
       *  case CAN_MMI_GET_DIMMING_SETTINGS_REQUEST_STD_ID:
       *  {
       *  tSMMIDimming SMMIDimming;
       *  tSHeaterLampDim SDimm;
       *  LampDimmingInfoRead();
       *  LampDimmingInfoGet(&SDimm);
       *
       *  SMMIDimming.bLogicLevel  = SDimm.fLogicLevel;
       *  SMMIDimming.bState        = SDimm.fState;
       *
       *  canMMIAnswer.TxHeader.DataLength    = sizeof(tSMMIDimming);
       *  canMMIAnswer.TxHeader.Identifier  =
       *  CAN_MMI_GET_DIMMING_SETTINGS_ANSWER_STD_ID; memcpy(canMMIAnswer.Data,
       *  &SMMIDimming, sizeof(tSMMIDimming));
       *  CANTxRequest(canMMIAnswer.TxHeader.DataLength, CAN_ID_TYPE_STD,
       *  canMMIAnswer.TxHeader.Identifier, (uint8_t*)canMMIAnswer.Data);
       *  }
       *  break;
       */

      case CAN_MMI_SET_GPRS_MODEM_REQUEST_STD_ID:
      {
        tSMMISetGprsModem SMMISetGprsModem;
        MmiLocalModemSettingsV2_t settings;

        memcpy(&SMMISetGprsModem,
               pcanMMIRequest->Data,
               pcanMMIRequest->RxHeader.DataLength);

        if (ReadLocalModemSettings(&settings)
            && (settings.modemType != SMMISetGprsModem.bModemType))
        {
          settings.modemType = SMMISetGprsModem.bModemType;
          if (WriteLocalModemSettings(&settings))
          {
            SystemResetPortRequest(&g_systemResetPort);
          }
        }

        break;
      }

      case CAN_MMI_IAP_MODE_REQUEST_STD_ID:
      {
        TransmitLegacyAck(CAN_MMI_IAP_MODE_ANSWER_STD_ID,
                          MmiMaintenanceServiceEnterIapMode(
                            &g_mmiMaintenanceService));
        break;
      }

      case CAN_MMI_FACTORY_DEFAULTS_REQUEST_STD_ID:
      {
        (void) MmiMaintenanceServiceFactoryReset(&g_mmiMaintenanceService);
        break;
      }

      case CAN_MMI_SO_TEST_START_STD_ID:
      {
        if (MmiMaintenanceServiceStartOutputTest(&g_mmiMaintenanceService)
            != FALSE)
        {
          SMMIRuntime.bSOTestSONo = 0U;
          SMMIRuntime.SFlags.fSOTestStream = TRUE;
        }
        break;
      }

      case CAN_MMI_SO_TEST_STOP_STD_ID:
      {
        if (MmiMaintenanceServiceStopOutputTest(&g_mmiMaintenanceService)
            != FALSE)
        {
          SMMIRuntime.SFlags.fSOTestStream = FALSE;
        }
        break;
      }

      case CAN_MMI_SO_TEST_CHANGE_STD_ID:
      {
        tSMMISOTest SMMISOTest;

        memset(&SMMISOTest, 0, sizeof(SMMISOTest));
        memcpy(&SMMISOTest,
               pcanMMIRequest->Data,
               pcanMMIRequest->RxHeader.DataLength);

        if (MmiMaintenanceServiceSelectOutputTest(&g_mmiMaintenanceService,
                                                  SMMISOTest.bSONo)
            != FALSE)
        {
          SMMIRuntime.bSOTestSONo = SMMISOTest.bSONo;
        }
        break;
      }
  } /* switch */
} /* ParseMMIRequest */

void MMIRequest(tpSFDCANRxMsg pSMMIReq)
{
  tpSFDCANRxMsg pSRxMsg =
    (tpSFDCANRxMsg) osMemoryPoolAlloc(MMIReqsMemPoolHandle,
                                      0);

  if (pSRxMsg != NULL)
  {
    memcpy(pSRxMsg, pSMMIReq, sizeof(tSFDCANRxMsg));

    if (osMessageQueuePut(MMIReqsQueHandle, &pSRxMsg, 0, 0) != osOK)
    {
      osMemoryPoolFree(MMIReqsMemPoolHandle, pSRxMsg);
    }
  }
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  tasks */
void MMITaskFunc(void *argument)
{
  UNUSED(argument);

  tpSFDCANRxMsg pSReq = NULL;

  while (FOREVER)
  {
    if (osMessageQueueGet(MMIReqsQueHandle, &pSReq, NULL,
                          osWaitForever) == osOK)
    {
      ParseMMIRequest(pSReq);
      osMemoryPoolFree(MMIReqsMemPoolHandle, pSReq);
    }
  }
}
