/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  include files */
#include "mmi.h"

#include <string.h>

#include "CanMsgParser.h"
#include "MCSAsynch.h"
#include "MSM.h"
#include "cpmpcomm.h"
#include "data.h"
#include "gps.h"
#include "lcd.h"
#include "main.h"
#include "MCS.h"
#include "MLM.h"
#include "program.h"
#include "time.h"
/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  members */

/*  os members */

/*  private members */
tSMMIRuntime SMMIRuntime;

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Method declaration */

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  methods */
void OpenMMI(void)
{
  tSFDCANTxMsg canMMIAnswer;

  canMMIAnswer.TxHeader.DataLength = 0;
  canMMIAnswer.TxHeader.Identifier = CAN_MMI_STD_ID_OPEN_MMI;
  CANTxRequest(canMMIAnswer.TxHeader.DataLength,
               CAN_ID_TYPE_STD,
               canMMIAnswer.TxHeader.Identifier,
               (uint8_t *) canMMIAnswer.Data);
}

void CloseMMI(void)
{
  tSFDCANTxMsg canMMIAnswer;

  canMMIAnswer.TxHeader.DataLength = 0;
  canMMIAnswer.TxHeader.Identifier = CAN_MMI_STD_ID_CLOSE_MMI;
  CANTxRequest(canMMIAnswer.TxHeader.DataLength,
               CAN_ID_TYPE_STD,
               canMMIAnswer.TxHeader.Identifier,
               (uint8_t *) canMMIAnswer.Data);
}

void StreamPSMMeasurements(void)
{
  tSFDCANTxMsg canMMIAnswer;
  tSMMIMeasurement SMMIMeasurement;

  SMMIMeasurement.psm1Voltage = (GetPowerSupplyNet(0) * 0.73029);
  SMMIMeasurement.psm2Voltage = (GetPowerSupplyNet(1) * 0.73029);
  SMMIMeasurement.psm1Frequency = GetPowerSupplyFreq(0);
  SMMIMeasurement.psm2Frequency = GetPowerSupplyFreq(1);

  canMMIAnswer.TxHeader.DataLength = sizeof(tSMMIMeasurement);
  canMMIAnswer.TxHeader.Identifier = CAN_MMI_RUNTIME_MEASUREMENT_ANSWER_STD_ID;
  memcpy(canMMIAnswer.Data, &SMMIMeasurement, sizeof(tSMMIMeasurement));

  CANTxRequest(canMMIAnswer.TxHeader.DataLength,
               CAN_ID_TYPE_STD,
               canMMIAnswer.TxHeader.Identifier,
               (uint8_t *) canMMIAnswer.Data);
}

void StreamDateTime(void)
{
  tSFDCANTxMsg canMMIAnswer;
  tSMMITime SMMITime;
  tSTime STimeNow;

  TimeGet(&STimeNow);

  SMMITime.bTimeSource = TimeSourceGet();
  SMMITime.bSeconds = STimeNow.SCurrentTime.Seconds;
  SMMITime.bMinutes = STimeNow.SCurrentTime.Minutes;
  SMMITime.bHours = STimeNow.SCurrentTime.Hours;
  SMMITime.bDay = STimeNow.SCurrentDate.Date;
  SMMITime.bMonth = STimeNow.SCurrentDate.Month;
  SMMITime.bYear = STimeNow.SCurrentDate.Year;

  canMMIAnswer.TxHeader.DataLength = sizeof(tSMMITime);
  canMMIAnswer.TxHeader.Identifier = CAN_MMI_RUNTIME_TIME_ANSWER_STD_ID;

  memcpy(canMMIAnswer.Data, &SMMITime, sizeof(tSMMITime));
  CANTxRequest(canMMIAnswer.TxHeader.DataLength,
               CAN_ID_TYPE_STD,
               canMMIAnswer.TxHeader.Identifier,
               (uint8_t *) canMMIAnswer.Data);
}

void StreamGPRSImei(void)
{
  tSFDCANTxMsg canMMIAnswer;
  uint8_t bDataLen = FDCAN_DATA_MAX_LEN;

  memcpy(canMMIAnswer.Data, MCSGetGprsModemIMEI(), bDataLen);
  canMMIAnswer.TxHeader.DataLength = bDataLen;
  canMMIAnswer.TxHeader.Identifier = CAN_MMI_STD_ID_GPRS_MODEM_IMEI_PART1;
  CANTxRequest(canMMIAnswer.TxHeader.DataLength,
               CAN_ID_TYPE_STD,
               canMMIAnswer.TxHeader.Identifier,
               (uint8_t *) canMMIAnswer.Data);

  bDataLen = MCS_MAX_IMEI_LEN % FDCAN_DATA_MAX_LEN;
  memcpy(canMMIAnswer.Data, &MCSGetGprsModemIMEI()[FDCAN_DATA_MAX_LEN],
         bDataLen);
  canMMIAnswer.TxHeader.DataLength = bDataLen;
  canMMIAnswer.TxHeader.Identifier = CAN_MMI_STD_ID_GPRS_MODEM_IMEI_PART2;
  CANTxRequest(canMMIAnswer.TxHeader.DataLength,
               CAN_ID_TYPE_STD,
               canMMIAnswer.TxHeader.Identifier,
               (uint8_t *) canMMIAnswer.Data);
}

void StreamUSRMAC(void)
{
  tSFDCANTxMsg canMMIAnswer;
  uint8_t bDataLen = FDCAN_DATA_MAX_LEN;

  memcpy(canMMIAnswer.Data, MCSGetUSRModuleMAC(), bDataLen);
  canMMIAnswer.TxHeader.DataLength = bDataLen;
  canMMIAnswer.TxHeader.Identifier = CAN_MMI_STD_ID_USR_MAC_PART1;
  CANTxRequest(canMMIAnswer.TxHeader.DataLength,
               CAN_ID_TYPE_STD,
               canMMIAnswer.TxHeader.Identifier,
               (uint8_t *) canMMIAnswer.Data);

  bDataLen = MCS_MAX_MAC_LEN % FDCAN_DATA_MAX_LEN;
  memcpy(canMMIAnswer.Data, &MCSGetUSRModuleMAC()[FDCAN_DATA_MAX_LEN],
         bDataLen);
  canMMIAnswer.TxHeader.DataLength = bDataLen;
  canMMIAnswer.TxHeader.Identifier = CAN_MMI_STD_ID_USR_MAC_PART2;
  CANTxRequest(canMMIAnswer.TxHeader.DataLength,
               CAN_ID_TYPE_STD,
               canMMIAnswer.TxHeader.Identifier,
               (uint8_t *) canMMIAnswer.Data);
}

void StreamEthernetMAC(void)
{
  tSFDCANTxMsg canMMIAnswer;
  uint8_t bDataLen = FDCAN_DATA_MAX_LEN;

  memcpy(canMMIAnswer.Data, MCSGetRuntimeEthernetMAC(), bDataLen);
  canMMIAnswer.TxHeader.DataLength = bDataLen;
  canMMIAnswer.TxHeader.Identifier = CAN_MMI_STD_ID_USR_MAC_PART1;
  CANTxRequest(canMMIAnswer.TxHeader.DataLength,
               CAN_ID_TYPE_STD,
               canMMIAnswer.TxHeader.Identifier,
               (uint8_t *) canMMIAnswer.Data);

  bDataLen = MCS_MAX_MAC_LEN % FDCAN_DATA_MAX_LEN;
  memcpy(canMMIAnswer.Data,
         &MCSGetRuntimeEthernetMAC()[FDCAN_DATA_MAX_LEN],
         bDataLen);
  canMMIAnswer.TxHeader.DataLength = bDataLen;
  canMMIAnswer.TxHeader.Identifier = CAN_MMI_STD_ID_USR_MAC_PART2;
  CANTxRequest(canMMIAnswer.TxHeader.DataLength,
               CAN_ID_TYPE_STD,
               canMMIAnswer.TxHeader.Identifier,
               (uint8_t *) canMMIAnswer.Data);
}

void StreamGsmOperator(void)
{
  tSFDCANTxMsg canMMIAnswer;

  memcpy(canMMIAnswer.Data, MCSGetGprsGsmOperator(), FDCAN_DATA_MAX_LEN);
  canMMIAnswer.TxHeader.DataLength = FDCAN_DATA_MAX_LEN;
  canMMIAnswer.TxHeader.Identifier = CAN_MMI_STD_ID_GPRS_GSM_OPERATOR;
  CANTxRequest(canMMIAnswer.TxHeader.DataLength,
               CAN_ID_TYPE_STD,
               canMMIAnswer.TxHeader.Identifier,
               (uint8_t *) canMMIAnswer.Data);
}

void StreamGPRSState(void)
{
  tSFDCANTxMsg canMMIAnswer;
  tSMMIGprsLog SMMIGprsLog;

  memset(&SMMIGprsLog, 0, sizeof(SMMIGprsLog));

  SMMIGprsLog.bModem = MCSGetModemType();
  SMMIGprsLog.bState = MCSGetGPRSState();
  SMMIGprsLog.bSignalQuality = MCSGetGprsSignalQuality() / 6;

  canMMIAnswer.TxHeader.DataLength = sizeof(tSMMIGprsLog);
  canMMIAnswer.TxHeader.Identifier = CAN_MMI_GET_GPRS_MODEM_LOG_ANSWER_STD_ID;

  memcpy(canMMIAnswer.Data, &SMMIGprsLog, sizeof(tSMMIGprsLog));
  CANTxRequest(canMMIAnswer.TxHeader.DataLength,
               CAN_ID_TYPE_STD,
               canMMIAnswer.TxHeader.Identifier,
               (uint8_t *) canMMIAnswer.Data);
}

void StreamOperationRuntime(void)
{
  tSFDCANTxMsg canMMIAnswer;
  tSMMIWorkmode SMMIWorkmode;

  if (CPMPStateGet() == PACKET_TYPE_CP_DEFAULT)
  {
    SMMIWorkmode.bState = StateCurrentGet();
  }
  else
  {
    SMMIWorkmode.bState = STATES_PROGRAM_LOAD;
  }

  switch (SMMIWorkmode.bState)
  {
      case STATES_SEQ:
      {
        SMMIWorkmode.bArg1 = SeqCurrentGet();
        SMMIWorkmode.bArg2 = SeqTotalGet();
        SMMIWorkmode.bArg3 = SeqCurrentStepGet() + 1;
        SMMIWorkmode.bArg4 = SeqCurStepNumTotalGet();
        SMMIWorkmode.bArg5 = SeqCurrentStepCurrentDurationGet();
        SMMIWorkmode.bArg6 = SeqCurrentStepDurationGet();
        SMMIWorkmode.bArg7 = SeqDurCurGet();
        SMMIWorkmode.bArg8 = SeqDurGet(SeqCurrentGet() - 1)
                             + SeqTotalExtDurGet();
        break;
      }

      case STATES_PHASE:
      {
        SMMIWorkmode.bArg1 = ProgramCurrentNoGet();
        SMMIWorkmode.bArg2 = PhaseTotalGet();
        SMMIWorkmode.bArg3 = PhaseMinDurationGet(ProgramCurrentNoGet() - 1);
        SMMIWorkmode.bArg4 = WorkPlanPhaseDurGet(ProgramCurrentNoGet() - 1);
        SMMIWorkmode.bArg5 = PhaseElapsedDurGet(ProgramCurrentNoGet() - 1);
        break;
      }

      case STATES_PHASE_TRANSITION:
      {
        SMMIWorkmode.bArg1 = ProgramCurrentNoGet();
        SMMIWorkmode.bArg2 = ProgramTargetNoGet();
        break;
      }

      case STATES_PROGRAM_LOAD:
      {
        SMMIWorkmode.bArg1 = ProgramLoadingStatusGet();
        break;
      }
  }

  canMMIAnswer.TxHeader.DataLength = sizeof(tSMMIWorkmode);
  canMMIAnswer.TxHeader.Identifier = CAN_MMI_RUNTIME_WORKMODE_ANSWER_STD_ID;
  memcpy(canMMIAnswer.Data, &SMMIWorkmode, sizeof(tSMMIWorkmode));
  CANTxRequest(canMMIAnswer.TxHeader.DataLength,
               CAN_ID_TYPE_STD,
               canMMIAnswer.TxHeader.Identifier,
               (uint8_t *) canMMIAnswer.Data);
} /* StreamOperationRuntime */

void StreamModuleRuntime(void)
{
  tSFDCANTxMsg canMMIAnswer;
  tSMMIModule SMMIModule;

  SMMIModule.fGPSModemConnected = GpsModemAliveGet();
  SMMIModule.fGPSAntennaConnected = GpsAntStatusGet();
  SMMIModule.fGPRSModemConnected = MCSAsynchConnectedGet();
  SMMIModule.fGPRSCenterConnected = MCSAsynchConnectedGet();
  SMMIModule.fIsRelayClosed = !GetPowerRelay();
  SMMIModule.bLastDigitalInputDemand = GetLastInputDemandIssued();
  SMMIModule.bLastLoopDedectorDemand = GetLastDetectorDemandIssued();
  SMMIModule.bGPSModeType = GpsPortGet();

  canMMIAnswer.TxHeader.DataLength = sizeof(tSMMIModule);
  canMMIAnswer.TxHeader.Identifier = CAN_MMI_MODULE_STATUS_ANSWER_STD_ID;
  memcpy(canMMIAnswer.Data, &SMMIModule, sizeof(tSMMIModule));
  CANTxRequest(canMMIAnswer.TxHeader.DataLength,
               CAN_ID_TYPE_STD,
               canMMIAnswer.TxHeader.Identifier,
               (uint8_t *) canMMIAnswer.Data);
}

void StreamGateStateChanged(uint8_t fState, tpSLogRecord pSLog)
{
  tSFDCANTxMsg canMMIAnswer;
  tSMMICabinetDoorStateChange SMMICabinetDoorStateChange;

  SMMICabinetDoorStateChange.fState = fState;

  /*  Cabinet Door Closed */
  SMMICabinetDoorStateChange.bEvent =
    (fState) ? EVENT_DOOR_CLOSED : EVENT_DOOR_OPEN;
  SMMICabinetDoorStateChange.bSeconds = pSLog->bSeconds;
  SMMICabinetDoorStateChange.bMinutes = pSLog->bMinutes;
  SMMICabinetDoorStateChange.bHours = pSLog->bHours;
  SMMICabinetDoorStateChange.bDay = pSLog->bMonthDay;
  SMMICabinetDoorStateChange.bMonth = pSLog->bMonth;
  SMMICabinetDoorStateChange.bYear = pSLog->sYear - (TimeCenturyGet() * 100);

  canMMIAnswer.TxHeader.DataLength = sizeof(tSMMICabinetDoorStateChange);
  canMMIAnswer.TxHeader.Identifier = CAN_MMI_STD_ID_CABINET_DOOR_STATE_CHANGE;

  memcpy(canMMIAnswer.Data, &SMMICabinetDoorStateChange,
         sizeof(tSMMICabinetDoorStateChange));
  CANTxRequest(canMMIAnswer.TxHeader.DataLength,
               CAN_ID_TYPE_STD,
               canMMIAnswer.TxHeader.Identifier,
               (uint8_t *) canMMIAnswer.Data);
}

void StreamErrorRuntime(void)
{
  tSFDCANTxMsg canMMIAnswer;
  tSMMIError SMMIError;

  uint8_t bSetNo = 0;
  uint8_t bSetTotal = SetTotalGet();

  memset(&SMMIError, 0, sizeof(tSMMIError));

  for (bSetNo = 0; bSetNo < bSetTotal; bSetNo++)
  {
    tSSetRuntime SSetRuntime;

    SetRuntimeGet(bSetNo, &SSetRuntime);

    SMMIError.bSetNo = (bSetNo + 1);
    SMMIError.bSignalingMode = SSetRuntime.bSignalingMode;
    SMMIError.bSigModeSource = SSetRuntime.bSigModeSource;
    SMMIError.bParam1 = SSetRuntime.bParam1;
    SMMIError.bParam2 = SSetRuntime.bParam2;

    canMMIAnswer.TxHeader.DataLength = sizeof(SMMIError);
    canMMIAnswer.TxHeader.Identifier = CAN_MMI_ERROR_ANSWER_STD_ID;

    memcpy(canMMIAnswer.Data, &SMMIError, sizeof(SMMIError));
    CANTxRequest(canMMIAnswer.TxHeader.DataLength,
                 CAN_ID_TYPE_STD,
                 canMMIAnswer.TxHeader.Identifier,
                 (uint8_t *) canMMIAnswer.Data);
  }
}

void StreamSignals(void)
{
  tSFDCANTxMsg canMMIAnswer;
  uint8_t bSGNo = 0, bSONo = 0, bSignal = 0, bVoltages = 0, bCurrentSG = 0,
          bCurrentSO = 0, bSOType = 0;
  uint16_t sPeriod = 0, sSSMNo = 0;
  tSMMISGSignals SMMISGSignals;

  if (SMMIRuntime.SFlags.fSignalStream)
  {
    memset(&SMMISGSignals, 0, sizeof(SMMISGSignals));
    for (sSSMNo = 0; sSSMNo < MODULES_SSM_MAX / 2; sSSMNo++)
    {
      SMMISGSignals.SMMISSMSignals[sSSMNo % CAN_MMI_MAX_SSM_PER_MSG] = (sSSMNo
                                                                        + 1) <<
                                                                       12;
      for (bSGNo = 0; bSGNo < 4; bSGNo++)
      {
        bCurrentSG = (sSSMNo * 4) + bSGNo;
        for (bSONo = 0; bSONo < 3; bSONo++)
        {
          bCurrentSO = (bCurrentSG * 3) + bSONo;
          bSOType = GetSOType(bCurrentSO);
          bSignal = SGSignalGet(bCurrentSG);
          if (bSOType)
          {
            bVoltages = SignalVoltagesGet(bSignal);
            sPeriod = SubSignalHasFlash(bSignal, bSOType);

            if ((sPeriod == 0) || ((sPeriod > 0) && FlashOnGet(sPeriod)))
            {
              if (bVoltages & bSOType)
              {
                SMMISGSignals.SMMISSMSignals[sSSMNo
                                             % CAN_MMI_MAX_SSM_PER_MSG] |=
                  laValue2Bit[11
                              -
                              (
                                bCurrentSO % 12)];
              }
            }
          }
        }
      }
    }

    canMMIAnswer.TxHeader.DataLength = sizeof(tSMMISGSignals);
    canMMIAnswer.TxHeader.Identifier = CAN_MMI_GET_SIGNALS_ANSWER_STD_ID;
    memcpy(canMMIAnswer.Data, &SMMISGSignals, sizeof(tSMMISGSignals));
    CANTxRequest(canMMIAnswer.TxHeader.DataLength,
                 CAN_ID_TYPE_STD,
                 canMMIAnswer.TxHeader.Identifier,
                 (uint8_t *) canMMIAnswer.Data);

    memset(&SMMISGSignals, 0, sizeof(SMMISGSignals));
    for (sSSMNo = 4; sSSMNo < MODULES_SSM_MAX; sSSMNo++)
    {
      SMMISGSignals.SMMISSMSignals[sSSMNo % CAN_MMI_MAX_SSM_PER_MSG] = (sSSMNo
                                                                        + 1) <<
                                                                       12;
      for (bSGNo = 0; bSGNo < 4; bSGNo++)
      {
        bCurrentSG = (sSSMNo * 4) + bSGNo;
        for (bSONo = 0; bSONo < 3; bSONo++)
        {
          bCurrentSO = (bCurrentSG * 3) + bSONo;
          bSOType = GetSOType(bCurrentSO);
          bSignal = SGSignalGet(bCurrentSG);
          if (bSOType)
          {
            bVoltages = SignalVoltagesGet(bSignal);
            sPeriod = SubSignalHasFlash(bSignal,
                                        bSOType);

            if ((sPeriod == 0) || ((sPeriod > 0) && FlashOnGet(sPeriod)))
            {
              if (bVoltages & bSOType)
              {
                SMMISGSignals.SMMISSMSignals[sSSMNo
                                             % CAN_MMI_MAX_SSM_PER_MSG] |=
                  laValue2Bit[11
                              -
                              (
                                bCurrentSO % 12)];
              }
            }
          }
        }
      }
    }

    canMMIAnswer.TxHeader.DataLength = sizeof(tSMMISGSignals);
    canMMIAnswer.TxHeader.Identifier = CAN_MMI_GET_SIGNALS_ANSWER_STD_ID;
    memcpy(canMMIAnswer.Data, &SMMISGSignals, sizeof(tSMMISGSignals));
    CANTxRequest(canMMIAnswer.TxHeader.DataLength,
                 CAN_ID_TYPE_STD,
                 canMMIAnswer.TxHeader.Identifier,
                 (uint8_t *) canMMIAnswer.Data);
  }
} /* StreamSignals */

void StreamInputs(void)
{
  tSFDCANTxMsg canMMIAnswer;
  tSMMIInputs SMMIInputs;
  uint8_t bModuleIndex = 0;
  uint8_t bitIndex = 0;

  if (SMMIRuntime.SFlags.fInputStream)
  {
    memset(&SMMIInputs, 0, sizeof(SMMIInputs));
    /* input levels */
    for (bModuleIndex = 0; bModuleIndex < MODULES_IO_MAX; bModuleIndex++)
    {
      for (bitIndex = 0; bitIndex < 16; bitIndex++)
      {
        if (!GetBitValue(SaCanDetectorIOInputs[bModuleIndex].sLoopEmptyStates,
                         bitIndex))
        {
          SetBitValue(SMMIInputs.lLoopDemands,
                      ((!bModuleIndex) ? bitIndex : (bitIndex + 16)));
        }

        if (!GetBitValue(SaCanDigitalIOInputs[bModuleIndex].sInputStates,
                         bitIndex))
        {
          SetBitValue(SMMIInputs.lDigitalInputDemands,
                      ((!bModuleIndex) ? bitIndex : (bitIndex + 16)));
        }
      }
    }

    canMMIAnswer.TxHeader.DataLength = sizeof(SMMIInputs);
    canMMIAnswer.TxHeader.Identifier = CAN_MMI_GET_INPUTS_ANSWER_STD_ID;
    memcpy(canMMIAnswer.Data, &SMMIInputs, sizeof(SMMIInputs));
    CANTxRequest(canMMIAnswer.TxHeader.DataLength,
                 CAN_ID_TYPE_STD,
                 canMMIAnswer.TxHeader.Identifier,
                 (uint8_t *) canMMIAnswer.Data);
  }
}

void StreamSOTest(void)
{
  tSFDCANTxMsg canMMIAnswer;
  tSMMISOStream1 SMMISOStream1;
  tSMMISOStream2 SMMISOStream2;

  if (SMMIRuntime.SFlags.fSOTestStream)
  {
    SMMISOStream1.bSONo = SMMIRuntime.bSOTestSONo + 1;
    SMMISOStream1.sPowerNet = GetSOPowerRecordNet(SMMIRuntime.bSOTestSONo);
    SMMISOStream1.sPower = GetSOPower(SMMIRuntime.bSOTestSONo);
    SMMISOStream1.bState = 0;
    SMMISOStream1.sNet = 0;

    SMMISOStream2.bSONo = SMMIRuntime.bSOTestSONo + 1;
    SMMISOStream2.sNow = GetCurrentMeasurement((SMMIRuntime.bSOTestSONo
                                                /
                                                SIGNAL_OUTPUTS_PER_CURRENT_GROUP),
                                               CURRENT_NOW);
    SMMISOStream2.sMax = GetCurrentMeasurement((SMMIRuntime.bSOTestSONo
                                                /
                                                SIGNAL_OUTPUTS_PER_CURRENT_GROUP),
                                               CURRENT_MAX);
    SMMISOStream2.sMin = GetCurrentMeasurement((SMMIRuntime.bSOTestSONo
                                                /
                                                SIGNAL_OUTPUTS_PER_CURRENT_GROUP),
                                               CURRENT_MIN);

    canMMIAnswer.TxHeader.DataLength = sizeof(tSMMISOStream1);
    canMMIAnswer.TxHeader.Identifier = CAN_MMI_SO_TEST_STREAM_1_STD_ID;
    memcpy(canMMIAnswer.Data, &SMMISOStream1, sizeof(tSMMISOStream1));
    CANTxRequest(canMMIAnswer.TxHeader.DataLength,
                 CAN_ID_TYPE_STD,
                 canMMIAnswer.TxHeader.Identifier,
                 (uint8_t *) canMMIAnswer.Data);

    canMMIAnswer.TxHeader.DataLength = sizeof(tSMMISOStream2);
    canMMIAnswer.TxHeader.Identifier = CAN_MMI_SO_TEST_STREAM_2_STD_ID;
    memcpy(canMMIAnswer.Data, &SMMISOStream2, sizeof(tSMMISOStream2));
    CANTxRequest(canMMIAnswer.TxHeader.DataLength,
                 CAN_ID_TYPE_STD,
                 canMMIAnswer.TxHeader.Identifier,
                 (uint8_t *) canMMIAnswer.Data);
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
        uint16_t sLogIndex = (pcanMMIRequest->RxHeader.Identifier
                              == CAN_MMI_STD_ID_LAST_MMI_CLOSE_LOG_REQUEST)
                             ? GetGateClosedLogRecordIndex()
                             : GetGateOpenLogRecordIndex();
        tSLogRecord sLog;
        tSMMICabinetDoorStateChange SMMIGateState;

        memset(&sLog, 0, sizeof(tSLogRecord));
        if (LogRequest(LOG_REQ_READ_FROM, &sLog, 0, 0, 0, 0, sLogIndex))
        {
          SMMIGateState.fState = (sLog.SEvent.bEvent
                                  == EVENT_DOOR_CLOSED) ? TRUE : FALSE;
          SMMIGateState.bEvent = sLog.SEvent.bEvent;
          SMMIGateState.bSeconds = sLog.bSeconds;
          SMMIGateState.bMinutes = sLog.bMinutes;
          SMMIGateState.bHours = sLog.bHours;
          SMMIGateState.bDay = sLog.bMonthDay;
          SMMIGateState.bMonth = sLog.bMonth;
          SMMIGateState.bYear = sLog.sYear - (TimeCenturyGet() * 100);

          canMMIAnswer.TxHeader.DataLength =
            sizeof(tSMMICabinetDoorStateChange);
          canMMIAnswer.TxHeader.Identifier =
            (pcanMMIRequest->RxHeader.Identifier
             ==
             CAN_MMI_STD_ID_LAST_MMI_CLOSE_LOG_REQUEST)
            ? CAN_MMI_STD_ID_LAST_MMI_CLOSE_LOG_ANSWER
            :
            CAN_MMI_STD_ID_LAST_MMI_OPEN_LOG_ANSWER;
          memcpy(canMMIAnswer.Data, &SMMIGateState,
                 sizeof(tSMMICabinetDoorStateChange));
          CANTxRequest(canMMIAnswer.TxHeader.DataLength,
                       CAN_ID_TYPE_STD,
                       canMMIAnswer.TxHeader.Identifier,
                       (uint8_t *) canMMIAnswer.Data);
        }

        break;
      }

      case CAN_MMI_VERSTON_REQUEST_STD_ID:
      {
        tSMMIVersion SMMIVersion;

        SMMIVersion.bArg0 = MAESTRO_VERSION_ARG0;
        SMMIVersion.bArg1 = MAESTRO_VERSION_ARG1;
        SMMIVersion.bArg2 = MAESTRO_VERSION_ARG2;
        SMMIVersion.bArg3 = MAESTRO_VERSION_ARG3;
        SMMIVersion.bArg4 = MAESTRO_VERSION_ARG4;

        canMMIAnswer.TxHeader.DataLength = sizeof(tSMMIVersion);
        canMMIAnswer.TxHeader.Identifier = CAN_MMI_VERSTON_ANSWER_STD_ID;
        memcpy(canMMIAnswer.Data, &SMMIVersion, sizeof(tSMMIVersion));
        CANTxRequest(canMMIAnswer.TxHeader.DataLength,
                     CAN_ID_TYPE_STD,
                     canMMIAnswer.TxHeader.Identifier,
                     (uint8_t *) canMMIAnswer.Data);
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

        SMMIModule.fGPSModemConnected = GpsModemAliveGet();
        SMMIModule.fGPSAntennaConnected = GpsAntStatusGet();
        SMMIModule.fGPRSModemConnected = MCSAsynchConnectedGet();
        SMMIModule.fGPRSCenterConnected = MCSAsynchConnectedGet();
        SMMIModule.fIsRelayClosed = GetPowerRelay();
        SMMIModule.bLastDigitalInputDemand = GetLastInputDemandIssued();
        SMMIModule.bLastLoopDedectorDemand = GetLastDetectorDemandIssued();
        SMMIModule.bGPSModeType = GpsPortGet();

        canMMIAnswer.TxHeader.DataLength = sizeof(tSMMIModule);
        canMMIAnswer.TxHeader.Identifier = CAN_MMI_MODULE_STATUS_ANSWER_STD_ID;
        memcpy(canMMIAnswer.Data, &SMMIModule, sizeof(tSMMIModule));
        CANTxRequest(canMMIAnswer.TxHeader.DataLength,
                     CAN_ID_TYPE_STD,
                     canMMIAnswer.TxHeader.Identifier,
                     (uint8_t *) canMMIAnswer.Data);
        break;
      }

      case CAN_MMI_ERROR_REQUEST_STD_ID:
      {
        tSMMIError SMMIError;

        uint8_t bSetNo = 0;
        uint8_t bSetTotal = SetTotalGet();

        memset(&SMMIError, 0, sizeof(tSMMIError));

        for (bSetNo = 0; bSetNo < bSetTotal; bSetNo++)
        {
          if (SetSigModeIsEmergent(bSetNo))
          {
            tSSetRuntime SSetRuntime;

            SetRuntimeGet(bSetNo, &SSetRuntime);

            SMMIError.bSetNo = (bSetNo + 1);
            SMMIError.bSignalingMode = SSetRuntime.bSignalingMode;
            SMMIError.bSigModeSource = SSetRuntime.bSigModeSource;
            SMMIError.bParam1 = SSetRuntime.bParam1;
            SMMIError.bParam2 = SSetRuntime.bParam2;

            canMMIAnswer.TxHeader.DataLength = sizeof(SMMIError);
            canMMIAnswer.TxHeader.Identifier = CAN_MMI_ERROR_ANSWER_STD_ID;
            memcpy(canMMIAnswer.Data, &SMMIError, sizeof(SMMIError));
            CANTxRequest(canMMIAnswer.TxHeader.DataLength,
                         CAN_ID_TYPE_STD,
                         canMMIAnswer.TxHeader.Identifier,
                         (uint8_t *) canMMIAnswer.Data);
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
        switch (SMMIChangeMode.bRequestedMode)
        {
            case LCD_USER_REQUEST_ALL_RED:
            {
              UserStateReqSet(STATES_CLOSED);
              break;
            }

            case LCD_USER_REQUEST_DARK:
            {
              UserStateReqSet(STATES_NO_CONTROL);
              break;
            }

            case LCD_USER_REQUEST_FLASH:
            {
              UserStateReqSet(STATES_FLASH);
              break;
            }

            case LCD_USER_REQUEST_PLAN_RETURN:
            {
              UserStateReqFree();
              break;
            }
        }

        break;
      }

      case CAN_MMI_SET_TIME_REQUEST_STD_ID:
      {
        tSMMISetTime SMMISetTime;
        tSTime SCurrMMITime;

        memset(&SCurrMMITime, 0, sizeof(SCurrMMITime));
        memset(&SMMISetTime, 0, sizeof(SMMISetTime));

        memcpy(&SMMISetTime,
               pcanMMIRequest->Data,
               pcanMMIRequest->RxHeader.DataLength);

        SCurrMMITime.bCentury = TIME_CURRENT_CENTURY - 1;
        SCurrMMITime.SCurrentTime.Seconds = SMMISetTime.bSecond;
        SCurrMMITime.SCurrentTime.Minutes = SMMISetTime.bMinute;
        SCurrMMITime.SCurrentTime.Hours = SMMISetTime.bHour;
        SCurrMMITime.SCurrentDate.Date = SMMISetTime.bDay;
        SCurrMMITime.SCurrentDate.Month = SMMISetTime.bMonth;
        SCurrMMITime.SCurrentDate.Year = SMMISetTime.bYear;
        SCurrMMITime.SCurrentDate.WeekDay =
          TimeWeekDayOfYearCalc(SCurrMMITime.SCurrentDate.Month,
                                SCurrMMITime.
                                SCurrentDate.Date,
                                TimeFullYearCalc
                                  (&SCurrMMITime));

        if (!GpsModemAliveGet() || !GpsRTCInitialUpdateDoneGet())     /* Set time only if GPS is detached */
        {
          if (TimeIsValid(&SCurrMMITime))
          {
            TimeSet(&SCurrMMITime);
          }
        }

        break;
      }

      case CAN_MMI_GET_LOG_REQUEST_STD_ID:
      {
        uint8_t fValidLogSent = FALSE;
        tSMMIGetLogRequest SMMIGetLogRequest;
        tSMMILogTime SMMILogTime;
        tSMMILogContent SMMILogContent;

        memset(&SMMIGetLogRequest, 0, sizeof(tSMMIGetLogRequest));
        memset(&SMMILogTime, 0, sizeof(tSMMILogTime));
        memset(&SMMILogContent, 0, sizeof(tSMMILogContent));

        memcpy(&SMMIGetLogRequest,
               pcanMMIRequest->Data,
               pcanMMIRequest->RxHeader.DataLength);

        if (LogIndexIsValid(SMMIGetLogRequest.sLogIndex))
        {
          if (LogEventNew(SMMIGetLogRequest.sLogIndex) != LOG_NO_NEW_LOG)
          {
            tSLogRecord SMMILog;

            if (LogRequest(LOG_REQ_READ_NEXT,
                           &SMMILog,
                           0,
                           0,
                           0,
                           0,
                           SMMIGetLogRequest.sLogIndex))
            {
              SMMILogContent.bLog = SMMILog.SEvent.bEvent;
              SMMILogContent.bParam = SMMILog.SEvent.bParam;
              SMMILogContent.sParam = SMMILog.SEvent.sParam;
              SMMILogContent.lParam = SMMILog.SEvent.lParam;

              SMMILogTime.bSecond = SMMILog.bSeconds;
              SMMILogTime.bMinute = SMMILog.bMinutes;
              SMMILogTime.bHour = SMMILog.bHours;
              SMMILogTime.bDay = SMMILog.bMonthDay;
              SMMILogTime.bMonth = SMMILog.bMonth;
              SMMILogTime.bYear = SMMILog.sYear - (TimeCenturyGet() * 100);

              canMMIAnswer.TxHeader.DataLength = sizeof(tSMMILogTime);
              canMMIAnswer.TxHeader.Identifier =
                CAN_MMI_GET_LOG_ANSWER_DATE_TIME_STD_ID;
              memcpy(canMMIAnswer.Data, &SMMILogTime, sizeof(tSMMILogTime));

              CANTxRequest(canMMIAnswer.TxHeader.DataLength,
                           CAN_ID_TYPE_STD,
                           canMMIAnswer.TxHeader.Identifier,
                           (uint8_t *) canMMIAnswer.Data);

              canMMIAnswer.TxHeader.DataLength = sizeof(tSMMILogContent);
              canMMIAnswer.TxHeader.Identifier =
                CAN_MMI_GET_LOG_ANSWER_CONTENT_STD_ID;
              memcpy(canMMIAnswer.Data, &SMMILogContent,
                     sizeof(tSMMILogContent));

              CANTxRequest(canMMIAnswer.TxHeader.DataLength,
                           CAN_ID_TYPE_STD,
                           canMMIAnswer.TxHeader.Identifier,
                           (uint8_t *) canMMIAnswer.Data);

              fValidLogSent = TRUE;
            }
          }
        }

        if (!fValidLogSent)
        {
          canMMIAnswer.TxHeader.DataLength = sizeof(tSMMILogTime);
          canMMIAnswer.TxHeader.Identifier =
            CAN_MMI_GET_LOG_ANSWER_DATE_TIME_STD_ID;
          memcpy(canMMIAnswer.Data, &SMMILogTime, sizeof(tSMMILogTime));

          CANTxRequest(canMMIAnswer.TxHeader.DataLength,
                       CAN_ID_TYPE_STD,
                       canMMIAnswer.TxHeader.Identifier,
                       (uint8_t *) canMMIAnswer.Data);

          canMMIAnswer.TxHeader.DataLength = sizeof(tSMMILogContent);
          canMMIAnswer.TxHeader.Identifier =
            CAN_MMI_GET_LOG_ANSWER_CONTENT_STD_ID;
          memcpy(canMMIAnswer.Data, &SMMILogContent, sizeof(tSMMILogContent));

          CANTxRequest(canMMIAnswer.TxHeader.DataLength,
                       CAN_ID_TYPE_STD,
                       canMMIAnswer.TxHeader.Identifier,
                       (uint8_t *) canMMIAnswer.Data);
        }

        break;
      }

      case CAN_MMI_GET_LAST_LOG_INDEX_REQUEST_STD_ID:
      {
        tSMMILastLogIndex SMMILastLogIndex;

        if (LogExists())
        {
          SMMILastLogIndex.sMMILastLogIndex =
            LogEventNew(CAN_MMI_LOG_INDEX_MAX_VALUE);

          canMMIAnswer.TxHeader.DataLength = sizeof(tSMMILastLogIndex);
          canMMIAnswer.TxHeader.Identifier =
            CAN_MMI_GET_LAST_LOG_INDEX_ANSWER_STD_ID;
          memcpy(canMMIAnswer.Data, &SMMILastLogIndex,
                 sizeof(tSMMILastLogIndex));

          CANTxRequest(canMMIAnswer.TxHeader.DataLength,
                       CAN_ID_TYPE_STD,
                       canMMIAnswer.TxHeader.Identifier,
                       (uint8_t *) canMMIAnswer.Data);
        }
        else
        {
          SMMILastLogIndex.sMMILastLogIndex = CAN_MMI_LOG_INDEX_MAX_VALUE;

          canMMIAnswer.TxHeader.DataLength = sizeof(tSMMILastLogIndex);
          canMMIAnswer.TxHeader.Identifier =
            CAN_MMI_GET_LAST_LOG_INDEX_ANSWER_STD_ID;
          memcpy(canMMIAnswer.Data, &SMMILastLogIndex,
                 sizeof(tSMMILastLogIndex));

          CANTxRequest(canMMIAnswer.TxHeader.DataLength,
                       CAN_ID_TYPE_STD,
                       canMMIAnswer.TxHeader.Identifier,
                       (uint8_t *) canMMIAnswer.Data);
        }

        break;
      }

      case CAN_MMI_GET_GPRS_MODEM_LOG_REQUEST_STD_ID:
      {
        tSMMIGprsLog SMMIGprsLog;

        memset(&SMMIGprsLog, 0, sizeof(tSMMIGprsLog));
        break;
      }

      case CAN_MMI_SET_RELAY_STATE_REQUEST_STD_ID:
      {
        tSMMIRelayState SMMIRelayState;

        memcpy(&SMMIRelayState,
               pcanMMIRequest->Data,
               pcanMMIRequest->RxHeader.DataLength);
        SetLCDPowerRelay(SMMIRelayState.bRelayStateRequest);
        SetLCDPowerRelayRequest(TRUE);
        break;
      }

      case CAN_MMI_SET_GPS_PORT_REQUEST_STD_ID:
      {
        tSMMIGpsSettingsPort SMMIGpsSettingsPort;

        memcpy(&SMMIGpsSettingsPort,
               pcanMMIRequest->Data,
               pcanMMIRequest->RxHeader.DataLength);

        if (GpsPortGet() != SMMIGpsSettingsPort.bGpsPortRequest)
        {
          GpsPortSet(SMMIGpsSettingsPort.bGpsPortRequest);
          if (GpsPortWrite())
          {
            SecureSystemReset();
          }
        }

        break;
      }

      case CAN_MMI_SET_GPS_BAUD_RATE_REQUEST_STD_ID:
      {
        uint8_t bAckNack;
        tSMMIGpsSettingsBaudRateIndex SMMIGpsSettingsBaudRateIndex;

        memcpy(&SMMIGpsSettingsBaudRateIndex,
               pcanMMIRequest->Data,
               pcanMMIRequest->RxHeader.DataLength);

        if (GpsBaudRateIndexGet()
            != SMMIGpsSettingsBaudRateIndex.bGpsBaudRateIndexRequest)
        {
          GpsBaudRateIndexSet(
            SMMIGpsSettingsBaudRateIndex.bGpsBaudRateIndexRequest);
          if (GpsBaudRateIndexWrite())
          {
            tSMMIGpsSettingsBaudRateIndex SMMIGpsSettingsBaudRateIndex;

            bAckNack = MCS_ASYNCH_MSG_ACK;

            canMMIAnswer.TxHeader.DataLength = sizeof(bAckNack);
            canMMIAnswer.TxHeader.Identifier =
              CAN_MMI_SET_GPS_BAUD_RATE_ANSWER_STD_ID;
            memcpy(canMMIAnswer.Data, &bAckNack, sizeof(bAckNack));
            CANTxRequest(canMMIAnswer.TxHeader.DataLength,
                         CAN_ID_TYPE_STD,
                         canMMIAnswer.TxHeader.Identifier,
                         (uint8_t *) canMMIAnswer.Data);

            SMMIGpsSettingsBaudRateIndex.bGpsBaudRateIndexRequest =
              GpsBaudRateIndexGet();

            canMMIAnswer.TxHeader.DataLength =
              sizeof(tSMMIGpsSettingsBaudRateIndex);
            canMMIAnswer.TxHeader.Identifier =
              CAN_MMI_GET_GPS_BAUD_RATE_ANSWER_STD_ID;
            memcpy(canMMIAnswer.Data, &SMMIGpsSettingsBaudRateIndex,
                   sizeof(tSMMIGpsSettingsBaudRateIndex));
            CANTxRequest(canMMIAnswer.TxHeader.DataLength,
                         CAN_ID_TYPE_STD,
                         canMMIAnswer.TxHeader.Identifier,
                         (uint8_t *) canMMIAnswer.Data);

            SecureSystemReset();
          }
          else
          {
            bAckNack = MCS_ASYNCH_MSG_NAK;
            canMMIAnswer.TxHeader.DataLength = sizeof(bAckNack);
            canMMIAnswer.TxHeader.Identifier =
              CAN_MMI_SET_GPS_BAUD_RATE_ANSWER_STD_ID;
            memcpy(canMMIAnswer.Data, &bAckNack, sizeof(bAckNack));
            CANTxRequest(canMMIAnswer.TxHeader.DataLength,
                         CAN_ID_TYPE_STD,
                         canMMIAnswer.TxHeader.Identifier,
                         (uint8_t *) canMMIAnswer.Data);
          }
        }

        break;
      }

      case CAN_MMI_GET_GPS_BAUD_RATE_REQUEST_STD_ID:
      {
        tSMMIGpsSettingsBaudRateIndex SMMIGpsSettingsBaudRateIndex;

        SMMIGpsSettingsBaudRateIndex.bGpsBaudRateIndexRequest =
          GpsBaudRateIndexGet();

        canMMIAnswer.TxHeader.DataLength =
          sizeof(tSMMIGpsSettingsBaudRateIndex);
        canMMIAnswer.TxHeader.Identifier =
          CAN_MMI_GET_GPS_BAUD_RATE_ANSWER_STD_ID;
        memcpy(canMMIAnswer.Data, &SMMIGpsSettingsBaudRateIndex,
               sizeof(tSMMIGpsSettingsBaudRateIndex));
        CANTxRequest(canMMIAnswer.TxHeader.DataLength,
                     CAN_ID_TYPE_STD,
                     canMMIAnswer.TxHeader.Identifier,
                     (uint8_t *) canMMIAnswer.Data);
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

        tSUserSettings SLUserSettings;

        UserSettingsRead();
        UserSettingsGet(&SLUserSettings);

        memcpy(&SMMIUserSettings,
               pcanMMIRequest->Data,
               pcanMMIRequest->RxHeader.DataLength);
        SLUserSettings.fSettingsChanged = USER_SETTINGS_CHANGE_CONTROL_VLAUE;
        SLUserSettings.fConfigFlag = SMMIUserSettings.fConfigFlag;

        UserSettingsSet(&SLUserSettings);
        if (UserSettingsSave())
        {
          UserSettingsRead();
        }

        break;
      }

      case CAN_MMI_GET_USER_SETTINGS_REQUEST_STD_ID:
      {
        tSMMIUserSettings SMMIUserSettings;
        tSUserSettings SLUserSettings;

        memset(&SMMIUserSettings, 0, sizeof(tSMMIUserSettings));
        memset(&SLUserSettings, 0, sizeof(tSUserSettings));

        UserSettingsRead();
        UserSettingsGet(&SLUserSettings);

        SMMIUserSettings.fSettingsChanged = SLUserSettings.fSettingsChanged;
        SMMIUserSettings.fConfigFlag = SLUserSettings.fConfigFlag;
        SMMIUserSettings.fLogFlag = SLUserSettings.fLogFlag;
        SMMIUserSettings.fTrafficCountsFlag = SLUserSettings.fTrafficCountsFlag;

        canMMIAnswer.TxHeader.DataLength = sizeof(tSMMIUserSettings);
        canMMIAnswer.TxHeader.Identifier =
          CAN_MMI_GET_USER_SETTINGS_ANSWER_STD_ID;
        memcpy(canMMIAnswer.Data, &SMMIUserSettings, sizeof(tSMMIUserSettings));
        CANTxRequest(canMMIAnswer.TxHeader.DataLength,
                     CAN_ID_TYPE_STD,
                     canMMIAnswer.TxHeader.Identifier,
                     (uint8_t *) canMMIAnswer.Data);
        break;
      }

      case CAN_MMI_SET_USER_SETTINGS_PART2_REQUEST_STD_ID:
      {
        tSMMIUserSettingsPart2 SMMIUserSettingsPart2;

        tSUserSettings SLUserSettings;

        UserSettingsRead();
        UserSettingsGet(&SLUserSettings);

        memcpy(&SMMIUserSettingsPart2,
               pcanMMIRequest->Data,
               pcanMMIRequest->RxHeader.DataLength);
        SLUserSettings.fSettingsChanged = USER_SETTINGS_CHANGE_CONTROL_VLAUE;
        SLUserSettings.fStandbyInfoFlag =
          SMMIUserSettingsPart2.fStandbyInfoFlag;

        UserSettingsSet(&SLUserSettings);
        if (UserSettingsSave())
        {
          UserSettingsRead();
        }

        break;
      }

      case CAN_MMI_GET_USER_SETTINGS_PART2_REQUEST_STD_ID:
      {
        tSMMIUserSettingsPart2 SMMIUserSettingsPart2;
        tSUserSettings SLUserSettings;

        memset(&SMMIUserSettingsPart2, 0, sizeof(tSMMIUserSettingsPart2));
        memset(&SLUserSettings, 0, sizeof(tSUserSettings));

        UserSettingsRead();
        UserSettingsGet(&SLUserSettings);

        SMMIUserSettingsPart2.fStandbyInfoFlag =
          SLUserSettings.fStandbyInfoFlag;

        canMMIAnswer.TxHeader.DataLength = sizeof(tSMMIUserSettingsPart2);
        canMMIAnswer.TxHeader.Identifier =
          CAN_MMI_GET_USER_SETTINGS_PART2_ANSWER_STD_ID;
        memcpy(canMMIAnswer.Data, &SMMIUserSettingsPart2,
               sizeof(tSMMIUserSettingsPart2));
        CANTxRequest(canMMIAnswer.TxHeader.DataLength,
                     CAN_ID_TYPE_STD,
                     canMMIAnswer.TxHeader.Identifier,
                     (uint8_t *) canMMIAnswer.Data);
        break;
      }

      case CAN_MMI_SET_BROKEN_INPUT_SETTINGS_REQUEST_STD_ID:
      {
        uint8_t bAckNack = MCS_ASYNCH_MSG_NAK;
        tSBrokenInputSettings SSettings;

        BrokenInputSettingsRead();
        BrokenInputSettingsGet(&SSettings);

        memcpy(&SSettings,
               pcanMMIRequest->Data,
               pcanMMIRequest->RxHeader.DataLength);
        SSettings.fAlreadySet = BROKEN_INPUT_SETTINGS_SET_CONTROL_VLAUE;

        BrokenInputSettingsSet(&SSettings);
        if (BrokenInputSettingsSave())
        {
          bAckNack = MCS_ASYNCH_MSG_ACK;
        }

        BrokenInputSettingsRead();

        canMMIAnswer.TxHeader.DataLength = sizeof(bAckNack);
        canMMIAnswer.TxHeader.Identifier =
          CAN_MMI_SET_BROKEN_INPUT_SETTINGS_ANSWER_STD_ID;
        memcpy(canMMIAnswer.Data, &bAckNack, sizeof(bAckNack));
        CANTxRequest(canMMIAnswer.TxHeader.DataLength,
                     CAN_ID_TYPE_STD,
                     canMMIAnswer.TxHeader.Identifier,
                     (uint8_t *) canMMIAnswer.Data);
        break;
      }

      case CAN_MMI_GET_BROKEN_INPUT_SETTINGS_REQUEST_STD_ID:
      {
        tSBrokenInputSettings SSettings;
        tSMMIBrokenInputSettings SMMISettings;

        memset(&SSettings, 0, sizeof(SSettings));
        memset(&SMMISettings, 0, sizeof(SMMISettings));

        BrokenInputSettingsRead();
        BrokenInputSettingsGet(&SSettings);

        SMMISettings.fAlreadySet = SSettings.fAlreadySet;
        SMMISettings.fDigitalInputFlag = SSettings.SFlags.fDigitalBusy;
        SMMISettings.fLoopInputFlag = SSettings.SFlags.fLoopBusy;

        canMMIAnswer.TxHeader.DataLength = sizeof(SSettings);
        canMMIAnswer.TxHeader.Identifier =
          CAN_MMI_GET_BROKEN_INPUT_SETTINGS_ANSWER_STD_ID;
        memcpy(canMMIAnswer.Data, &SMMISettings, sizeof(SMMISettings));
        CANTxRequest(canMMIAnswer.TxHeader.DataLength,
                     CAN_ID_TYPE_STD,
                     canMMIAnswer.TxHeader.Identifier,
                     (uint8_t *) canMMIAnswer.Data);
        break;
      }

      case CAN_MMI_SET_DAYLIGHT_SAVING_TIME_SETTINGS_REQUEST_STD_ID:
      {
        uint8_t bMMIDSTFlag;
        tSDaylightSavingTimeSettings SDaylightSavingTimeSettings;

        memset(&SDaylightSavingTimeSettings, 0,
               sizeof(tSDaylightSavingTimeSettings));
        bMMIDSTFlag = 0;

        ReadDaylightSavingTimeFlag();
        GetDaylightSavingTimeFlag(&bMMIDSTFlag);

        memcpy(&SDaylightSavingTimeSettings,
               pcanMMIRequest->Data,
               pcanMMIRequest->RxHeader.DataLength);

        SetDaylightSavingTimeFlag(
          SDaylightSavingTimeSettings.fDaylightSavingTimeFlag);
        if (WriteDaylightSavingTimeFlag())
        {
          ReadDaylightSavingTimeFlag();
        }

        if (GpsModemAliveGet())
        {
          if (bMMIDSTFlag
              != SDaylightSavingTimeSettings.fDaylightSavingTimeFlag)
          {
            GpsRTCInitialUpdateDoneSet(FALSE);
          }
        }

        break;
      }

      case CAN_MMI_GET_DAYLIGHT_SAVING_TIME_SETTINGS_REQUEST_STD_ID:
      {
        tSDaylightSavingTimeSettings SDaylightSavingTimeSettings;
        uint8_t bMMIDSTFlag;

        memset(&SDaylightSavingTimeSettings, 0,
               sizeof(tSDaylightSavingTimeSettings));
        bMMIDSTFlag = 0;

        ReadDaylightSavingTimeFlag();
        GetDaylightSavingTimeFlag(&bMMIDSTFlag);

        SDaylightSavingTimeSettings.fDaylightSavingTimeFlag = bMMIDSTFlag;

        canMMIAnswer.TxHeader.DataLength = sizeof(tSDaylightSavingTimeSettings);
        canMMIAnswer.TxHeader.Identifier =
          CAN_MMI_GET_DAYLIGHT_SAVING_TIME_SETTINGS_ANSWER_STD_ID;
        memcpy(canMMIAnswer.Data, &SDaylightSavingTimeSettings,
               sizeof(tSDaylightSavingTimeSettings));
        CANTxRequest(canMMIAnswer.TxHeader.DataLength,
                     CAN_ID_TYPE_STD,
                     canMMIAnswer.TxHeader.Identifier,
                     (uint8_t *) canMMIAnswer.Data);
        break;
      }

      case CAN_MMI_GET_ADMIN_USER_INFO_REQUEST_STD_ID:
      {
        uint16_t sAdminUsername = GetAdminUsername();
        uint16_t sAdminPassword = GetAdminPassword();

        memcpy(&canMMIAnswer.Data[0], &sAdminUsername, 2);
        memcpy(&canMMIAnswer.Data[4], &sAdminPassword, 2);
        canMMIAnswer.TxHeader.DataLength = sizeof(canMMIAnswer.Data);
        canMMIAnswer.TxHeader.Identifier =
          CAN_MMI_GET_ADMIN_USER_INFO_ANSWER_STD_ID;
        CANTxRequest(canMMIAnswer.TxHeader.DataLength,
                     CAN_ID_TYPE_STD,
                     canMMIAnswer.TxHeader.Identifier,
                     (uint8_t *) canMMIAnswer.Data);
        break;
      }

      case CAN_MMI_SET_ADMIN_USER_INFO_REQUEST_STD_ID:
      {
        uint8_t bAckNack;
        uint16_t sUsername;
        uint16_t sPassword;
        uint16_t sAdminUserName = GetAdminUsername();

        memcpy(&sUsername, &pcanMMIRequest->Data[0], 2);
        memcpy(&sPassword, &pcanMMIRequest->Data[4], 2);

        if ((LCD_PASSWORD_LENGTH == DigitCountsGet(sUsername))
            && (LCD_PASSWORD_LENGTH == DigitCountsGet(sPassword))
            && (sUsername == sAdminUserName) )
        {
          SetAdminPassword(sPassword);
          if (WriteAdminPassword())
          {
            SetAdminValidity(TRUE);
            if (WriteAdminValidity())
            {
              bAckNack = MCS_ASYNCH_MSG_ACK;
              canMMIAnswer.TxHeader.DataLength = sizeof(bAckNack);
              canMMIAnswer.TxHeader.Identifier =
                CAN_MMI_SET_ADMIN_USER_INFO_ANSWER_STD_ID;
              memcpy(canMMIAnswer.Data, &bAckNack, sizeof(bAckNack));
              CANTxRequest(canMMIAnswer.TxHeader.DataLength,
                           CAN_ID_TYPE_STD,
                           canMMIAnswer.TxHeader.Identifier,
                           (uint8_t *) canMMIAnswer.Data);
              break;
            }
          }
        }

        bAckNack = MCS_ASYNCH_MSG_NAK;
        canMMIAnswer.TxHeader.DataLength = sizeof(bAckNack);
        canMMIAnswer.TxHeader.Identifier =
          CAN_MMI_SET_ADMIN_USER_INFO_ANSWER_STD_ID;
        memcpy(canMMIAnswer.Data, &bAckNack, sizeof(bAckNack));
        CANTxRequest(canMMIAnswer.TxHeader.DataLength,
                     CAN_ID_TYPE_STD,
                     canMMIAnswer.TxHeader.Identifier,
                     (uint8_t *) canMMIAnswer.Data);
        break;
      }

      case CAN_MMI_GET_SIGNALS_REQUEST_STD_ID:
      {
        uint16_t sSSMNo = 0;
        uint8_t bSGNo = 0, bSONo = 0, bSignal = 0, bVoltages = 0,
                bCurrentSG = 0, bCurrentSO = 0, bSOType = 0;
        uint16_t sPeriod = 0;
        tSMMISGSignals SMMISGSignals;

        memset(&SMMISGSignals, 0, sizeof(SMMISGSignals));
        for (sSSMNo = 0; sSSMNo < MODULES_SSM_MAX / 2; sSSMNo++)
        {
          SMMISGSignals.SMMISSMSignals[sSSMNo
                                       % CAN_MMI_MAX_SSM_PER_MSG] = (sSSMNo
                                                                     + 1) << 12;
          for (bSGNo = 0; bSGNo < 4; bSGNo++)
          {
            bCurrentSG = (sSSMNo * 4) + bSGNo;
            for (bSONo = 0; bSONo < 3; bSONo++)
            {
              bCurrentSO = (bCurrentSG * 3) + bSONo;
              bSOType = GetSOType(bCurrentSO);
              bSignal = SGSignalGet(bCurrentSG);
              if (bSOType)
              {
                bVoltages = SignalVoltagesGet(bSignal);
                sPeriod = SubSignalHasFlash(bSignal, bSOType);

                if ((sPeriod == 0) || ((sPeriod > 0) && FlashOnGet(sPeriod)))
                {
                  if (bVoltages & bSOType)
                  {
                    SMMISGSignals.SMMISSMSignals[sSSMNo
                                                 % CAN_MMI_MAX_SSM_PER_MSG] |=
                      laValue2Bit[11
                                  -
                                  (
                                    bCurrentSO % 12)];
                  }
                }
              }
            }
          }
        }

        canMMIAnswer.TxHeader.DataLength = sizeof(tSMMISGSignals);
        canMMIAnswer.TxHeader.Identifier = CAN_MMI_GET_SIGNALS_ANSWER_STD_ID;
        memcpy(canMMIAnswer.Data, &SMMISGSignals, sizeof(tSMMISGSignals));
        CANTxRequest(canMMIAnswer.TxHeader.DataLength,
                     CAN_ID_TYPE_STD,
                     canMMIAnswer.TxHeader.Identifier,
                     (uint8_t *) canMMIAnswer.Data);

        memset(&SMMISGSignals, 0, sizeof(SMMISGSignals));
        for (sSSMNo = 4; sSSMNo < MODULES_SSM_MAX; sSSMNo++)
        {
          SMMISGSignals.SMMISSMSignals[sSSMNo
                                       % CAN_MMI_MAX_SSM_PER_MSG] = (sSSMNo
                                                                     + 1) << 12;
          for (bSGNo = 0; bSGNo < 4; bSGNo++)
          {
            bCurrentSG = (sSSMNo * 4) + bSGNo;
            for (bSONo = 0; bSONo < 3; bSONo++)
            {
              bCurrentSO = (bCurrentSG * 3) + bSONo;
              bSOType = GetSOType(bCurrentSO);
              bSignal = SGSignalGet(bCurrentSG);
              if (bSOType)
              {
                bVoltages = SignalVoltagesGet(bSignal);
                sPeriod = SubSignalHasFlash(bSignal,
                                            bSOType);

                if ((sPeriod == 0) || ((sPeriod > 0) && FlashOnGet(sPeriod)))
                {
                  if (bVoltages & bSOType)
                  {
                    SMMISGSignals.SMMISSMSignals[sSSMNo
                                                 % CAN_MMI_MAX_SSM_PER_MSG] |=
                      laValue2Bit[11
                                  -
                                  (
                                    bCurrentSO % 12)];
                  }
                }
              }
            }
          }
        }

        canMMIAnswer.TxHeader.DataLength = sizeof(tSMMISGSignals);
        canMMIAnswer.TxHeader.Identifier = CAN_MMI_GET_SIGNALS_ANSWER_STD_ID;
        memcpy(canMMIAnswer.Data, &SMMISGSignals, sizeof(tSMMISGSignals));
        CANTxRequest(canMMIAnswer.TxHeader.DataLength,
                     CAN_ID_TYPE_STD,
                     canMMIAnswer.TxHeader.Identifier,
                     (uint8_t *) canMMIAnswer.Data);
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
        tSMMIInputs SMMIInputs;
        uint8_t bModuleIndex = 0;
        uint8_t bitIndex = 0;

        memset(&SMMIInputs, 0, sizeof(SMMIInputs));
        /* input levels */
        for (bModuleIndex = 0; bModuleIndex < MODULES_IO_MAX; bModuleIndex++)
        {
          for (bitIndex = 0; bitIndex < 16; bitIndex++)
          {
            if (!GetBitValue(
                  SaCanDetectorIOInputs[bModuleIndex].sLoopEmptyStates,
                  bitIndex))
            {
              SetBitValue(SMMIInputs.lLoopDemands,
                          ((!bModuleIndex) ? bitIndex : (bitIndex + 16)));
            }

            if (!GetBitValue(SaCanDigitalIOInputs[bModuleIndex].sInputStates,
                             bitIndex))
            {
              SetBitValue(SMMIInputs.lDigitalInputDemands,
                          ((!bModuleIndex) ? bitIndex : (bitIndex + 16)));
            }
          }
        }

        canMMIAnswer.TxHeader.DataLength = sizeof(SMMIInputs);
        canMMIAnswer.TxHeader.Identifier = CAN_MMI_GET_INPUTS_ANSWER_STD_ID;
        memcpy(canMMIAnswer.Data, &SMMIInputs, sizeof(SMMIInputs));
        CANTxRequest(canMMIAnswer.TxHeader.DataLength,
                     CAN_ID_TYPE_STD,
                     canMMIAnswer.TxHeader.Identifier,
                     (uint8_t *) canMMIAnswer.Data);
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

        memcpy(&SMMISetGprsModem,
               pcanMMIRequest->Data,
               pcanMMIRequest->RxHeader.DataLength);

        if (MCSGetModemType() != SMMISetGprsModem.bModemType)
        {
          MCSSetModemType(SMMISetGprsModem.bModemType);
          if (MCSWriteConInfo())
          {
            SecureSystemReset();
          }
        }

        break;
      }

      case CAN_MMI_IAP_MODE_REQUEST_STD_ID:
      {
        uint8_t bAckNack = MCS_ASYNCH_MSG_NAK;

        canMMIAnswer.TxHeader.DataLength = sizeof(bAckNack);
        canMMIAnswer.TxHeader.Identifier = CAN_MMI_IAP_MODE_ANSWER_STD_ID;
        memcpy(canMMIAnswer.Data, &bAckNack, sizeof(bAckNack));
        CANTxRequest(canMMIAnswer.TxHeader.DataLength,
                     CAN_ID_TYPE_STD,
                     canMMIAnswer.TxHeader.Identifier,
                     (uint8_t *) canMMIAnswer.Data);
        break;
      }

      case CAN_MMI_FACTORY_DEFAULTS_REQUEST_STD_ID:
      {
        ReturnFactorySettings();
        break;
      }

      case CAN_MMI_SO_TEST_START_STD_ID:
      {
        StartSSMTest(SSM_TEST_FROM_MMI);
        LogRequest(LOG_REQ_APPEND_ASYNCH,
                   NULL,
                   EVENT_USER_REQ_SSM_TEST_STARTS,
                   0,
                   0,
                   0,
                   0);
        SMMIRuntime.bSOTestSONo = 0;
        SMMIRuntime.SFlags.fSOTestStream = TRUE;
        break;
      }

      case CAN_MMI_SO_TEST_STOP_STD_ID:
      {
        SMMIRuntime.SFlags.fSOTestStream = FALSE;
        StopSSMTest();
        LoadProgramEnds();
        RestartProgram();
        LogRequest(LOG_REQ_APPEND_ASYNCH,
                   NULL,
                   EVENT_USER_REQ_SSM_TEST_ENDS,
                   0,
                   0,
                   0,
                   0);
        break;
      }

      case CAN_MMI_SO_TEST_CHANGE_STD_ID:
      {
        tSMMISOTest SMMISOTest;

        memset(&SMMISOTest, 0, sizeof(SMMISOTest));
        memcpy(&SMMISOTest,
               pcanMMIRequest->Data,
               pcanMMIRequest->RxHeader.DataLength);

        SMMIRuntime.bSOTestSONo = SMMISOTest.bSONo;
        SetOnSONo(SMMIRuntime.bSOTestSONo);
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
