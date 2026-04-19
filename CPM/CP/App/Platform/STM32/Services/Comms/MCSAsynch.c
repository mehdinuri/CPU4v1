/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  include files */
#include "MCSAsynch.h"
#include "HardwarePorts.h"
#include "PersistencePorts.h"

#include <stdio.h>
#include <string.h>

#include "MCS.h"
#include "MSM.h"
#include "PPPOSAsynch.h"
#include "DomainServices.h"
#include "gps.h"
#include "IAP.h"
#include "lwip/apps/snmp.h"
#include "main.h"
#include "tcp_client.h"
/* ///////////////////////////////////////////////////////// */
/*                    Data */
#define MCS_ASYNCH_DMA_TX_TIMEOUT 1000
#define MCS_ASYNCH_LCD_LINE_COUNT 4U

/* Reception and Transmission Control */
static tSMCSAsynchTransfer SMCSAsynchRx;

/* Runtime */
static tSMCSAsynchRuntime SMCSAsynchRuntime;

/*  Log Read Index */
static uint16_t sLogReadIndex = 0;

/* SP Change */
static tSMCSAsynchSPChange SMCSAsynchSPChange;

/* Events */

/* Remote End Response */
static uint8_t bRemEndResIdx;
static char baRemEndResponse[MCS_ASYNCH_DATA_PACKET_MAX_LEN + 1];

/* Partial runtime timer */
static uint8_t bPartialRunTimeTimer = 0;

/* Error info timer */
static uint16_t sErrorInfoTimer = 0;

/* Version */
static uint8_t fVersionSent = FALSE;

static ISerialPort_t  *s_port;
static IModemPort_t   *s_driver;

/* ///////////////////////////////////////////////////////// */
/*                    Methods */
void MCSAsynchInit(ISerialPort_t *port, IModemPort_t *driver)
{
  s_port = port;
  s_driver = driver;
  memset(&SMCSAsynchRuntime, 0, sizeof(SMCSAsynchRuntime));
  bPartialRunTimeTimer = 0;
  sErrorInfoTimer = 0;
  sLogReadIndex = 0;
  fVersionSent = FALSE;
}

uint16_t MCSAsynchGeLogReadIndex(void)
{
  return sLogReadIndex;
}

void MCSAsynchSetLogReadIndex(uint16_t sIndex)
{
  sLogReadIndex = sIndex;
}

void MCSAsynchReadLogReadIndex(void)
{
  PersistenceRead(&g_persistencePort,
                  PERSIST_OBJECT_MCS_LOG_READ_INDEX,
                  0U,
                  &sLogReadIndex,
                  sizeof(uint16_t));
}

void MCSAsynchWriteLogReadIndex(void)
{
  PersistenceWrite(&g_persistencePort,
                   PERSIST_OBJECT_MCS_LOG_READ_INDEX,
                   0U,
                   &sLogReadIndex,
                   sizeof(uint16_t));
}

void MCSAsynchConnectedSet(uint8_t bState)
{
  SMCSAsynchRuntime.bConnected = bState;
}

uint8_t MCSAsynchConnectedGet(void)
{
  return SMCSAsynchRuntime.bConnected;
}

void MCSAsynchCancelOpSet(uint8_t bState)
{
  SMCSAsynchRuntime.bCancelOp = bState;
}

uint8_t MCSAsynchCancelOpGet(void)
{
  return SMCSAsynchRuntime.bCancelOp;
}

void MCSAsynchMsgRcvdSet(uint8_t bState)
{
  SMCSAsynchRuntime.bMsgRcvd = bState;
}

uint8_t MCSAsynchMsgRcvdGet(void)
{
  return SMCSAsynchRuntime.bMsgRcvd;
}

void MCSAsynchWebEngineSet(uint8_t bState)
{
  SMCSAsynchRuntime.bWebEngine = bState;
}

uint8_t MCSAsynchWebEngineGet(void)
{
  return SMCSAsynchRuntime.bWebEngine;
}

void MCSAsynchWebEngineTimeoutReset(void)
{
  SMCSAsynchRuntime.sWebEngineTimeout = 0;
}

void MCSAsynchWebEngineTimeoutIncrease(void)
{
  SMCSAsynchRuntime.sWebEngineTimeout++;
  if (SMCSAsynchRuntime.sWebEngineTimeout >= MCS_ASYNCH_STREAM_TIMEOUT)
  {
    MCSAsynchWebEngineTimeoutReset();
    MCSAsynchWebEngineSet(FALSE);
  }
}

void MCSAsynchConnectionTimeoutReset(void)
{
  SMCSAsynchRuntime.sConnectionTimeout = 0;
}

void MCSAsynchCheckConnectionTimeout(void)
{
  if (MCSAsynchConnectedGet())
  {
    SMCSAsynchRuntime.sConnectionTimeout++;
    if (SMCSAsynchRuntime.sConnectionTimeout
        >= MCS_ASYNCH_TIMEOUT_COUNTER_IN_SECONDS + 1)
    {
      SMCSAsynchRuntime.sConnectionTimeout = 0;
      MCSAsynchStop(MCS_ASYNCH_DISC_TYPE_TIMEOUT_CON_CHECK);
    }
  }
}

void MCSAsynchLCDStreamSet(uint8_t bState)
{
  SMCSAsynchRuntime.bLCDStream = bState;
}

uint8_t MCSAsynchLCDStreamGet(void)
{
  return SMCSAsynchRuntime.bLCDStream;
}

void MCSAsynchLCDStreamTimeoutReset(void)
{
  SMCSAsynchRuntime.sLCDStreamTimeout = 0;
}

void MCSAsynchLCDStreamTimeoutIncrease(void)
{
  SMCSAsynchRuntime.sLCDStreamTimeout++;
  if (SMCSAsynchRuntime.sLCDStreamTimeout >= MCS_ASYNCH_STREAM_TIMEOUT)
  {
    MCSAsynchLCDStreamTimeoutReset();
    MCSAsynchLCDStreamSet(FALSE);
  }
}

void MCSAsynchInputRuntimeStreamSet(uint8_t bState)
{
  SMCSAsynchRuntime.bIRStream = bState;
}

uint8_t MCSAsynchInputRuntimeStreamGet(void)
{
  return SMCSAsynchRuntime.bIRStream;
}

void MCSAsynchInputRuntimeStreamTimeoutReset(void)
{
  SMCSAsynchRuntime.sIRStreamTimeout = 0;
}

void MCSAsynchInputRuntimeStreamTimeoutIncrease(void)
{
  SMCSAsynchRuntime.sIRStreamTimeout++;
  if (SMCSAsynchRuntime.sIRStreamTimeout >= MCS_ASYNCH_STREAM_TIMEOUT)
  {
    MCSAsynchInputRuntimeStreamTimeoutReset();
    MCSAsynchInputRuntimeStreamSet(FALSE);
  }
}

void MCSAsynchStartReception(void)
{
  memset(&SMCSAsynchRx, 0, sizeof(SMCSAsynchRx));

  SMCSAsynchRx.eState = MCS_ASYNCH_TXRX_STATE_START;
  SMCSAsynchRx.sCurrentByte = 0;
  SMCSAsynchRx.bBusy = TRUE;

  memset(&baRemEndResponse, 0, sizeof(baRemEndResponse));
  bRemEndResIdx = 0;
}

uint8_t MCSAsynchResumeMsgSet(void)
{
  return MCSAsynchReqTxMsg(MCS_ASYNCH_HEADER_RESUME_CONNECTION,
                           MCS_MAX_IMEI_LEN,
                           (void *) MCSGetGprsModemIMEI());
}

uint8_t MCSAsynchImeiMsgSet(void)
{
  return MCSAsynchReqTxMsg(MCS_ASYNCH_HEADER_IMEI,
                           MCS_MAX_IMEI_LEN,
                           (void *) MCSGetGprsModemIMEI());
}

uint8_t MCSAsynchUSRMacMsgSet(void)
{
  return MCSAsynchReqTxMsg(MCS_ASYNCH_HEADER_MAC,
                           MCS_MAX_MAC_LEN,
                           (void *) MCSGetUSRModuleMAC());
}

uint8_t MCSAsynchEthernetMacMsgSet(void)
{
  return MCSAsynchReqTxMsg(MCS_ASYNCH_HEADER_MAC,
                           MCS_MAX_MAC_LEN,
                           (void *) MCSGetRuntimeEthernetMAC());
}

/* Description: Finishes the GPRS modem init state and start receiving MCS msgs */
/* return: TRUE: if there is no error on first MCS msg. FALSE: if there is a */
/* problem with the msg */
uint8_t MCSAsynchStart(uint8_t bGreetingType)
{
  MCSRingBufferReset();

  MCSSetConnected(TRUE);

  switch (bGreetingType)
  {
      case MODEM_GREETING_IMEI:
      {
        MCSAsynchImeiMsgSet();
        break;
      }

      case MODEM_GREETING_USR_MAC:
      {
        MCSAsynchUSRMacMsgSet();
        break;
      }

      case MODEM_GREETING_ETH_MAC:
      {
        MCSAsynchEthernetMacMsgSet();
        break;
      }

      default:
      {
        break;
      }
  }

  if (osEventFlagsWait(MCSAsyEventHandle, EVENT_FLAGS_MCS_ASY_CONNECTED,
                       osFlagsWaitAll,
                       MCS_ASYNCH_TIMEOUT_CONNECTION)
      ==
      EVENT_FLAGS_MCS_ASY_CONNECTED)
  {
    MCSAsynchInit(s_port, s_driver);
    MCSAsynchConnectedSet(TRUE);

    LogRequest(LOG_REQ_APPEND_ASYNCH, NULL, EVENT_MCS_CONNECTED, 0, 0, 0, 0);

    return TRUE;
  }

  MCSSetConnected(FALSE);

  return FALSE;
} /* MCSAsynchStart */

void MCSAsynchStop(uint8_t bDisConType)
{
  MCSAsynchConnectedSet(FALSE);
  LogRequest(LOG_REQ_APPEND_ASYNCH,
             NULL,
             EVENT_MCS_DISCONNECTED,
             bDisConType,
             0,
             0,
             0);

  MCSAsynchWebEngineSet(FALSE);

  MCSAsynchConnectionTimeoutReset();

  if (TCPClientIsConnected())
  {
    TCPClientDisconnect();
  }

  if (PPPOSAsynchGetConnected())
  {
    PPPOSAsynchStop();
  }

  MCSAsynchInit(s_port, s_driver);
  MCSRuntimeInit();
}

uint32_t MCSAsynchCalculateEpoch(void)
{
  uint32_t ulEpoch = 0;
  tSTime SMCSAsynchTime;

  TimeGet(&SMCSAsynchTime);

  TimeEpochCalculate(&SMCSAsynchTime, &ulEpoch);

  return ulEpoch;
}

uint8_t MCSAsynchReqRxMsg(uint8_t *pbData, uint16_t sLength)
{
  if (MCSGetConnected())
  {
    tpSMCSAsynchRxTxMsg pSRxReq =
      (tpSMCSAsynchRxTxMsg) osMemoryPoolAlloc(MCSAsyRxReqsMemPoolHandle,
                                              0);

    if (pSRxReq != NULL)
    {
      memset(pSRxReq, 0, sizeof(tSMCSAsynchRxTxMsg));

      pSRxReq->sDataLen = sLength;
      memcpy(pSRxReq->baData, pbData, sLength);

      if (osMessageQueuePut(MCSAsyRxReqsQueHandle, &pSRxReq, 0, 0) == osOK)
      {
        return TRUE;
      }

      osMemoryPoolFree(MCSAsyRxReqsMemPoolHandle, pSRxReq);
    }
  }

  return TRUE;
}

uint8_t MCSAsynchIsRemoteEndClosed(void)
{
  return ModemIsDisconnected(s_driver,
                             baRemEndResponse,
                             (uint16_t) strlen(baRemEndResponse));
}

void MCSAsynchInputsMsgSet(void)
{
  tSMCSAsynchIOsAndLDs SMCSAsynchInputs;
  uint8_t bModuleIndex = 0;
  uint8_t bitIndex = 0;

  memset(&SMCSAsynchInputs, 0, sizeof(SMCSAsynchInputs));
  SMCSAsynchInputs.bNumberOfLoopDedector = InputTotalGet(INPUT_TYPE_DETECTOR);
  SMCSAsynchInputs.bNumberOfDigitalInputs = InputTotalGet(INPUT_TYPE_DIGITAL);
  /* input levels */
  for (bModuleIndex = 0; bModuleIndex < MODULES_IO_MAX; bModuleIndex++)
  {
    for (bitIndex = 0; bitIndex < 16; bitIndex++)
    {
      if (!GetBitValue(SaCanDetectorIOInputs[bModuleIndex].sLoopEmptyStates,
                       bitIndex))
      {
        if (!bModuleIndex)
        {
          SetBitValue(SMCSAsynchInputs.lLoopDedectorDemands, bitIndex);
        }
        else
        {
          SetBitValue(SMCSAsynchInputs.lLoopDedectorDemands, (bitIndex + 16));
        }
      }

      if (!GetBitValue(SaCanDigitalIOInputs[bModuleIndex].sInputStates,
                       bitIndex))
      {
        if (!bModuleIndex)
        {
          SetBitValue(SMCSAsynchInputs.lDigitalInputDemands, bitIndex);
        }
        else
        {
          SetBitValue(SMCSAsynchInputs.lDigitalInputDemands, (bitIndex + 16));
        }
      }
    }
  }

  MCSAsynchReqTxMsg(MCS_ASYNCH_HEADER_IO_AND_LD,
                    sizeof(tSMCSAsynchIOsAndLDs),
                    &SMCSAsynchInputs);
} /* MCSAsynchInputsMsgSet */

void MCSAsynchTrafficCountsDigitalMsgSet(void)
{
  tSMCSInputRuntime SaInputCounts[INPUTS_DIGITAL_MAX];

  memset(SaInputCounts, 0, sizeof(SaInputCounts));

  GetMCSTrafficCountsDigital(&SaInputCounts);

  MCSAsynchReqTxMsg(MCS_ASYNCH_HEADER_TRAFFIC_COUNTS_DIGITAL,
                    (sizeof(SaInputCounts)), &SaInputCounts);
}

void MCSAsynchTrafficCountsLoopMsgSet(void)
{
  tSMCSInputRuntime SaInputCounts[INPUTS_DIGITAL_MAX];

  memset(SaInputCounts, 0, sizeof(SaInputCounts));

  GetMCSTrafficCountsDetector(&SaInputCounts);

  MCSAsynchReqTxMsg(MCS_ASYNCH_HEADER_TRAFFIC_COUNTS_LOOP,
                    sizeof(SaInputCounts),
                    &SaInputCounts);
}

void MCSAsynchSignalsMsgSet(void)
{
  uint8_t baSignals[SIGNAL_GROUPS_MAX];
  uint8_t bSGNo = 0;

  memset(&baSignals, 0, sizeof(baSignals));
  for (bSGNo = 0; bSGNo < SIGNAL_GROUPS_MAX; bSGNo++)
  {
    baSignals[bSGNo] = SGSignalGet(bSGNo);
  }

  MCSAsynchReqTxMsg(MCS_ASYNCH_HEADER_SIGNALS, SGTotalGet(), baSignals);
}

void MCSAsynchRuntimeMsgSet(void)
{
  tSMCSDeviceRuntime SMCSDevRtm;
  uint8_t sizeofData = MCS_ASYNCH_MIN_DEVICE_RUNTIME_SIZE;

  if (UserStateRunning())
  {
    SMCSDevRtm.bOperationMode = MCS_ASYNCH_OPERATION_MODE_MANUAL;
  }
  else if (SignalPlanCurrentGet())
  {
    SMCSDevRtm.bOperationMode = MCS_ASYNCH_OPERATION_MODE_CENTRAL;
  }
  else
  {
    SMCSDevRtm.bOperationMode = MCS_ASYNCH_OPERATION_MODE_LOCAL;
  }

  if (ProgramStateGet() == PROGRAM_STATE_LOADING)
  {
    SMCSDevRtm.bSignalMode = MCS_ASYNCH_SIGNAL_MODE_DOWNLOAD;
  }
  else
  {
    uint8_t bSetNo = 0;

    for (bSetNo = 0; bSetNo < SetTotalGet(); bSetNo++)
    {
      if (SetSigModeIsEmergent(bSetNo))
      {
        tSSetRuntime SMCSAsynchSetRuntime;

        SetRuntimeGet(bSetNo,
                      &SMCSAsynchSetRuntime);

        SMCSDevRtm.bSignalMode = SMCSAsynchSetRuntime.bSignalingMode;
        SMCSDevRtm.UStateData.SError.bError =
          SMCSAsynchSetRuntime.bSigModeSource;
        SMCSDevRtm.UStateData.SError.bParam1 = SMCSAsynchSetRuntime.bParam1;
        SMCSDevRtm.UStateData.SError.bParam2 = SMCSAsynchSetRuntime.bParam2;

        sizeofData += sizeof(SMCSDevRtm.UStateData.SError);
        break;
      }
    }

    if (bSetNo == SetTotalGet())
    {
      SMCSDevRtm.bSignalMode = StateCurrentGet();
      switch (SMCSDevRtm.bSignalMode)
      {
          case STATES_SEQ:
          {
            SMCSDevRtm.UStateData.SSequence.bCurrentSeq = SeqCurrentGet();
            SMCSDevRtm.UStateData.SSequence.bTotalSeq = SeqTotalGet();
            SMCSDevRtm.UStateData.SSequence.bCurrentSeqStep =
              SeqCurrentStepGet() + 1;
            SMCSDevRtm.UStateData.SSequence.bTotalSeqStep =
              SeqCurStepNumTotalGet();
            SMCSDevRtm.UStateData.SSequence.bCurrentSeqStepDur =
              SeqCurrentStepCurrentDurationGet();
            SMCSDevRtm.UStateData.SSequence.bTotalSeqStepDur =
              SeqCurrentStepDurationGet();
            SMCSDevRtm.UStateData.SSequence.bCurrentSeqDur = SeqDurCurGet();
            SMCSDevRtm.UStateData.SSequence.bTotalSeqDur =
              SeqDurGet(SeqCurrentGet() - 1) + SeqTotalExtDurGet();

            sizeofData += sizeof(SMCSDevRtm.UStateData.SSequence);
            break;
          }

          case STATES_PHASE:
          {
            SMCSDevRtm.UStateData.SPhase.bCurrentPhase = ProgramCurrentNoGet();
            SMCSDevRtm.UStateData.SPhase.bTotalPhase = PhaseTotalGet();
            SMCSDevRtm.UStateData.SPhase.bCurrentPhaseDur =
              PhaseElapsedDurGet(ProgramCurrentNoGet() - 1) + 1;
            SMCSDevRtm.UStateData.SPhase.bTotalPhaseDur =
              WorkPlanPhaseDurGet(ProgramCurrentNoGet() - 1);

            sizeofData += sizeof(SMCSDevRtm.UStateData.SPhase);
            break;
          }

          case STATES_PHASE_TRANSITION:
          {
            SMCSDevRtm.UStateData.STransition.bPhaseFrom =
              ProgramCurrentNoGet();
            SMCSDevRtm.UStateData.STransition.bPhaseTo = ProgramTargetNoGet();

            sizeofData += sizeof(SMCSDevRtm.UStateData.STransition);
            break;
          }
      } /* switch */
    }
  }

  /*  signal plan */
  SMCSDevRtm.bCurrentSignalPlan = SignalPlanCurrentGet();

  /*  module flags */
  SMCSDevRtm.bRelay = GetPowerRelay();
  /* Legacy gate/door runtime is removed; keep field zero for wire compatibility. */
  SMCSDevRtm.bGate = 0U;
  SMCSDevRtm.bGps = GpsModemAliveGet();
  SMCSDevRtm.bLampDim = GetLampDimmingState();
  SMCSDevRtm.bHeater = GetHeaterState();

  /*  measurements */
  SMCSDevRtm.SVoltages.sVoltage1 = (GetPowerSupplyNet(0) * 0.73029);
  SMCSDevRtm.SVoltages.sVoltage2 = (GetPowerSupplyNet(1) * 0.73029);
  SMCSDevRtm.SFrequencies.bFrequency1 = GetPowerSupplyFreq(0);
  SMCSDevRtm.SFrequencies.bFrequency2 = GetPowerSupplyFreq(1);

  MCSAsynchReqTxMsg(MCS_ASYNCH_HEADER_RUNTIME, sizeofData, &SMCSDevRtm);
} /* MCSAsynchRuntimeMsgSet */

void MCSAsynchPartialRuntimeMsgSet(void)
{
  tSMCSPartialRuntime SMCSPartialRuntime;
  uint8_t sizeofData = 2;

  if (ProgramStateGet() == PROGRAM_STATE_LOADING)
  {
    SMCSPartialRuntime.bSignalMode = MCS_ASYNCH_SIGNAL_MODE_DOWNLOAD;
  }
  else
  {
    uint8_t bSetNo = 0;

    for (bSetNo = 0; bSetNo < SetTotalGet(); bSetNo++)
    {
      if (SetSigModeIsEmergent(bSetNo))
      {
        tSSetRuntime SMCSAsynchPartialSetRuntime;

        SetRuntimeGet(bSetNo,
                      &SMCSAsynchPartialSetRuntime);

        SMCSPartialRuntime.bSignalMode =
          SMCSAsynchPartialSetRuntime.bSignalingMode;
        SMCSPartialRuntime.SError.bError =
          SMCSAsynchPartialSetRuntime.bSigModeSource;
        SMCSPartialRuntime.SError.bParam1 = SMCSAsynchPartialSetRuntime.bParam1;
        SMCSPartialRuntime.SError.bParam2 = SMCSAsynchPartialSetRuntime.bParam2;

        sizeofData += sizeof(SMCSPartialRuntime.SError);
        break;
      }
    }

    if (bSetNo == SetTotalGet())
    {
      SMCSPartialRuntime.bSignalMode = StateCurrentGet();
    }
  }

  /*  signal plan */
  SMCSPartialRuntime.bCurrentSignalPlan = SignalPlanCurrentGet();

  MCSAsynchReqTxMsg(MCS_ASYNCH_HEADER_PARTIAL_RUNTIME,
                    sizeofData,
                    &SMCSPartialRuntime);
} /* MCSAsynchPartialRuntimeMsgSet */

void MCSAsynchErrorInfoMsgSet(void)
{
  MCSAsynchReqTxMsg(MCS_ASYNCH_HEADER_ERROR_INFO,
                    sizeof(tSErrInfo),
                    GetErrorInfoPtr());
}

void MCSAsynchVersionMsgSet(void)
{
  tSMCSVersion SMCSVersion;

  memset(&SMCSVersion, 0, sizeof(SMCSVersion));

  SMCSVersion.bArg0 = MAESTRO_VERSION_ARG0;
  SMCSVersion.bArg1 = MAESTRO_VERSION_ARG1;
  SMCSVersion.bArg2 = MAESTRO_VERSION_ARG2;
  SMCSVersion.bArg3 = MAESTRO_VERSION_ARG3;
  SMCSVersion.bArg4 = MAESTRO_VERSION_ARG4;

  MCSAsynchReqTxMsg(MCS_ASYNCH_HEADER_VERSION,
                    sizeof(tSMCSVersion),
                    &SMCSVersion);
}

static void MCSAsynchBuildLegacyLcdStream(tSMCSLCDStream *stream)
{
  MmiRuntimeSummaryV2_t summary;
  MmiRuntimeSafetySummaryV2_t safetySummary;
  MmiRuntimePowerSummaryV2_t powerSummary;
  MmiRuntimeDoorSummaryV2_t doorSummary;

  if (stream == NULL)
  {
    return;
  }

  (void) memset(&summary, 0, sizeof(summary));
  (void) memset(&safetySummary, 0, sizeof(safetySummary));
  (void) memset(&powerSummary, 0, sizeof(powerSummary));
  (void) memset(&doorSummary, 0, sizeof(doorSummary));

  (void) MmiSnapshotCacheGetSummary(&g_mmiSnapshotCache, &summary);
  (void) MmiSnapshotCacheGetSafetySummary(&g_mmiSnapshotCache, &safetySummary);
  (void) MmiSnapshotCacheGetPowerSummary(&g_mmiSnapshotCache, &powerSummary);
  (void) MmiSnapshotCacheGetDoorSummary(&g_mmiSnapshotCache, &doorSummary);

  (void) snprintf(stream->strLCDLines[0],
                  sizeof(stream->strLCDLines[0]),
                  "M:%u A:%u S:%u",
                  summary.mode,
                  summary.actionPlanControl,
                  safetySummary.safetyAction);
  (void) snprintf(stream->strLCDLines[1],
                  sizeof(stream->strLCDLines[1]),
                  "SEQ:%u CFG:%u",
                  summary.activeSequenceNumber,
                  summary.configLoaded);
  (void) snprintf(stream->strLCDLines[2],
                  sizeof(stream->strLCDLines[2]),
                  "V1:%u.%u F1:%u",
                  (unsigned int) (powerSummary.psmVoltageTenthsVrms[0] / 10U),
                  (unsigned int) (powerSummary.psmVoltageTenthsVrms[0] % 10U),
                  (unsigned int) powerSummary.psmFrequencyRaw[0]);
  (void) snprintf(stream->strLCDLines[3],
                  sizeof(stream->strLCDLines[3]),
                  "DOOR:%s MMU:%u",
                  (doorSummary.open != 0U) ? "OPEN" : "CLOSE",
                  summary.mmuFlashActive);
}

void MCSAsynchLCDStreamMsgSet(void)
{
  uint8_t bLine = 0;
  tSMCSLCDStream SMCSLCDStream;

  memset(&SMCSLCDStream, 0, sizeof(SMCSLCDStream));

  for (bLine = 0; bLine < MCS_ASYNCH_LCD_LINE_COUNT; bLine++)
  {
    strcpy(SMCSLCDStream.strLCDLines[bLine], "");
  }

  if (SMCSAsynchRuntime.bSOMeasurements)
  {
    (void) snprintf(SMCSLCDStream.strLCDLines[0],
                    sizeof(SMCSLCDStream.strLCDLines[0]),
                    "OUTPUT TEST");
    (void) snprintf(SMCSLCDStream.strLCDLines[1],
                    sizeof(SMCSLCDStream.strLCDLines[1]),
                    "SSM:%u",
                    (unsigned int) SMCSAsynchRuntime.bSSMNo);
  }
  else
  {
    MCSAsynchBuildLegacyLcdStream(&SMCSLCDStream);
  }

  MCSAsynchReqTxMsg(MCS_ASYNCH_HEADER_LCD_STREAM,
                    sizeof(SMCSLCDStream),
                    &SMCSLCDStream);
}

void MCSAsynchIntersectionSettingsMsgSet(void)
{
  tSUserSettings SLUserSettings;
  tSMCSUserSettings SMCSUserSettings;

  UserSettingsRead();
  UserSettingsGet(&SLUserSettings);

  SMCSUserSettings.fSettingsChanged = SLUserSettings.fSettingsChanged;
  SMCSUserSettings.fConfigFlag = SLUserSettings.fConfigFlag;
  SMCSUserSettings.fLogFlag = SLUserSettings.fLogFlag;
  SMCSUserSettings.fTrafficCountsFlag = SLUserSettings.fTrafficCountsFlag;
  SMCSUserSettings.bTrafficCountsPeriod = SLUserSettings.bTrafficCountsPeriod;
  SMCSUserSettings.fStandbyInfoFlag = SLUserSettings.fStandbyInfoFlag;

  MCSAsynchReqTxMsg(MCS_ASYNCH_HEADER_USER_SETTINGS,
                    sizeof(tSMCSUserSettings),
                    &SMCSUserSettings);
}

void MCSAsynchBrokenInputSettingsMsgSet(void)
{
  tSBrokenInputSettings SSettings;
  tSMCSBrokenInputSettings SMCSSettings;

  BrokenInputSettingsRead();
  BrokenInputSettingsGet(&SSettings);

  SMCSSettings.fAlreadySet = SSettings.fAlreadySet;
  SMCSSettings.fDigitalInputFlag = SSettings.SFlags.fDigitalBusy;
  SMCSSettings.fLoopInputFlag = SSettings.SFlags.fLoopBusy;

  MCSAsynchReqTxMsg(MCS_ASYNCH_HEADER_BROKEN_INPUT_SETTINGS,
                    sizeof(SMCSSettings),
                    &SMCSSettings);
}

void MCSAsynchServerSettingsMsgSet(void)
{
  tSMCSServerSettings SSettings;

  ServerSettingsRead();

  SSettings.fMCSAvailable = ServerSettingsMCSAvailableGet();
  SSettings.fNTCIPAvailable = ServerSettingsNTCIPAvailableGet();

  MCSAsynchReqTxMsg(MCS_ASYNCH_HEADER_SERVER_SETTINGS,
                    sizeof(SSettings),
                    &SSettings);
}

void MCSAsynchLogSettingsMsgSet(void)
{
  tSLogSettings SLLogSettings;
  tSMCSLogSettings SMCSLogSettings;

  LogSettingsRead();
  LogSettingsGet(&SLLogSettings);

  memcpy(&SMCSLogSettings, &SLLogSettings, sizeof(tSMCSLogSettings));

  MCSAsynchReqTxMsg(MCS_ASYNCH_HEADER_LOG_SETTINGS,
                    sizeof(tSMCSLogSettings),
                    &SMCSLogSettings);
}

void MCSAsynchInputRuntimeMsgSet()
{
  uint8_t bStrIRLen = 0, bStrIRDataLen = 0;
  uint8_t bIndex = 0, bTotalDigitalInputs = 0, bTotalLoopInputs = 0,
          bTotalInputs = 0;

  char strIRData[MCS_ASYNCH_DATA_PACKET_MAX_LEN + 1] = { 0 };
  char strIR[MCS_ASYNCH_IR_MSG_MAX_LEN + 1] = { 0 };

  bTotalDigitalInputs = InputTotalGet(INPUT_TYPE_DIGITAL);
  if (bTotalDigitalInputs > 5)
  {
    bTotalDigitalInputs += 3;
  }

  bTotalLoopInputs = InputTotalGet(INPUT_TYPE_DETECTOR);
  bTotalInputs = bTotalDigitalInputs + bTotalLoopInputs;

  for (bIndex = 0; bIndex < bTotalInputs; bIndex++)
  {
    if (bIndex < bTotalDigitalInputs)
    {
      sprintf(strIR,
              "%u,%u,%u,%u,%u,%hu,%hu,%hu,%hu,%hu,%hu,%hu",
              INPUT_TYPE_DIGITAL,
              bIndex + 1,
              SRuntimes.SaInputRuntimes[bIndex].bDemandCntInPer,
              SRuntimes.SaInputRuntimes[bIndex].bDemandCntInRed,
              SRuntimes.SaInputRuntimes[bIndex].bDemandCntInGreen,
              (uint16_t) (SRuntimes.SaInputRuntimes[bIndex].sFDemandDurInPer
                          / 10),
              (uint16_t) (SRuntimes.SaInputRuntimes[bIndex].sFDemandDurInRed
                          / 10),
              (uint16_t) (SRuntimes.SaInputRuntimes[bIndex].sGapDurInGreen
                          / 10),
              (uint16_t) (SRuntimes.SaInputRuntimes[bIndex].sGapDurInPer / 10),
              (uint16_t) (SRuntimes.SaInputRuntimes[bIndex].sOccDurInGreen
                          / 10),
              (uint16_t) (SRuntimes.SaInputRuntimes[bIndex].sOccDurInPer / 10),
              (uint16_t) (SRuntimes.SaInputRuntimes[bIndex].sOccDurInRed / 10));
    }
    else
    {
      sprintf(strIR,
              "%u,%u,%u,%u,%u,%hu,%hu,%hu,%hu,%hu,%hu,%hu",
              INPUT_TYPE_DETECTOR,
              (bIndex - bTotalDigitalInputs) + 1,
              SRuntimes.SaDetectorRuntimes[bIndex
                                           - bTotalDigitalInputs].
              bDemandCntInPer,
              SRuntimes.SaDetectorRuntimes[bIndex
                                           - bTotalDigitalInputs].
              bDemandCntInRed,
              SRuntimes.SaDetectorRuntimes[bIndex
                                           - bTotalDigitalInputs].
              bDemandCntInGreen,
              (uint16_t) (SRuntimes.SaDetectorRuntimes[bIndex
                                                       - bTotalDigitalInputs].
                          sFDemandDurInPer / 10),
              (uint16_t) (SRuntimes.SaDetectorRuntimes[bIndex
                                                       - bTotalDigitalInputs].
                          sFDemandDurInRed / 10),
              (uint16_t) (SRuntimes.SaDetectorRuntimes[bIndex
                                                       - bTotalDigitalInputs].
                          sGapDurInGreen / 10),
              (uint16_t) (SRuntimes.SaDetectorRuntimes[bIndex
                                                       - bTotalDigitalInputs].
                          sGapDurInPer / 10),
              (uint16_t) (SRuntimes.SaDetectorRuntimes[bIndex
                                                       - bTotalDigitalInputs].
                          sOccDurInGreen / 10),
              (uint16_t) (SRuntimes.SaDetectorRuntimes[bIndex
                                                       - bTotalDigitalInputs].
                          sOccDurInPer / 10),
              (uint16_t) (SRuntimes.SaDetectorRuntimes[bIndex
                                                       - bTotalDigitalInputs].
                          sOccDurInRed / 10));
    }

    bStrIRLen = strlen(strIR);
    bStrIRDataLen = strlen(strIRData);
    if (bStrIRLen + bStrIRDataLen < MCS_ASYNCH_DATA_PACKET_MAX_LEN)
    {
      strncat(strIRData, strIR, bStrIRLen);
      strcat(strIRData, "\r");
    }
    else
    {
      MCSAsynchReqTxMsg(MCS_ASYNCH_HEADER_IR_STREAM,
                        strlen(strIRData),
                        (void *) strIRData);
      memset(strIRData, 0, sizeof(strIRData));
      bIndex--;
    }

    memset(strIR, 0, sizeof(strIR));
  }

  if (strlen(strIRData) > 0)
  {
    MCSAsynchReqTxMsg(MCS_ASYNCH_HEADER_IR_STREAM,
                      strlen(strIRData),
                      (void *) strIRData);
  }
} /* MCSAsynchInputRuntimeMsgSet */

void MCSAsynchDSTFlagMsgSet(void)
{
  uint8_t bDSTFlag = 0;

  ReadDaylightSavingTimeFlag();
  GetDaylightSavingTimeFlag(&bDSTFlag);

  MCSAsynchReqTxMsg(MCS_ASYNCH_HEADER_DST, 1, &bDSTFlag);
}

void MCSAsynchPrepTxMsg(uint8_t bHeader,
                        uint8_t bLen,
                        void *pData,
                        tpSMCSAsynchRxTxMsg pSTxMsg)
{
  tSMCSAsynchMsg SMsg;

  memset(&SMsg, 0, sizeof(SMsg));

  SMsg.bStart = MCS_ASYNCH_MSG_START;
  SMsg.bHeader = bHeader;
  SMsg.bLen = bLen + sizeof(SMsg.lEpoch);
  SMsg.lEpoch = MCSAsynchCalculateEpoch();
  memcpy(SMsg.UData.baData, pData, bLen);
  SMsg.bEnd = MCS_ASYNCH_MSG_END;
  if (bLen < MCS_ASYNCH_DATA_PACKET_MAX_LEN)
  {
    SMsg.UData.baData[bLen] = SMsg.bEnd;
  }

  pSTxMsg->sDataLen = SMsg.bLen + MCS_ASYNCH_MSG_PROTOCOL_OVERHEAD;
  memcpy(pSTxMsg->baData, &SMsg, pSTxMsg->sDataLen);
}

uint8_t MCSAsynchReqTxMsg(uint8_t bHeader, uint8_t bLen, void *pData)
{
  if (MCSGetConnected())
  {
    if (!GetStandbyState()
        || (GetStandbyState() && (bHeader == MCS_ASYNCH_HEADER_STOP_MODE) ))
    {
      tpSMCSAsynchRxTxMsg pSTxReq =
        (tpSMCSAsynchRxTxMsg) osMemoryPoolAlloc(MCSAsyTxReqsMemPoolHandle,
                                                0);

      if (pSTxReq != NULL)
      {
        memset(pSTxReq, 0, sizeof(tSMCSAsynchRxTxMsg));

        MCSAsynchPrepTxMsg(bHeader, bLen, pData, pSTxReq);

        if (osMessageQueuePut(MCSAsyTxReqsQueHandle, &pSTxReq, 0, 0) == osOK)
        {
          return TRUE;
        }

        osMemoryPoolFree(MCSAsyTxReqsMemPoolHandle, pSTxReq);
      }
    }
  }

  return TRUE;
}

void MCSAsynchSendAck(uint8_t bHeader)
{
  uint8_t bAck = MCS_ASYNCH_MSG_ACK;

  MCSAsynchReqTxMsg(bHeader, 1, &bAck);
}

void MCSAsynchSendNak(uint8_t bHeader)
{
  uint8_t bNak = MCS_ASYNCH_MSG_NAK;

  MCSAsynchReqTxMsg(bHeader, 1, &bNak);
}

uint8_t MCSAsynchSPChangeCmdSet(tpSMCSAsynchSPChange pSSPChange)
{
  if (PersistenceWrite(&g_persistencePort,
                       PERSIST_OBJECT_SP_CHANGE,
                       0U,
                       pSSPChange,
                       sizeof(tSMCSAsynchSPChange)))
  {
    return SignalPlanIsValidGet(pSSPChange->bSPNo);
  }

  return FALSE;
}

uint8_t MCSAsynchSPChangeCmdGet(tpSMCSAsynchSPChange pSSPChange)
{
  return PersistenceRead(&g_persistencePort,
                         PERSIST_OBJECT_SP_CHANGE,
                         0U,
                         pSSPChange,
                         sizeof(tSMCSAsynchSPChange));
}

uint8_t MCSAsynchSPNoGet(void)
{
  if (SMCSAsynchSPChange.fWSRunning)
  {
    uint32_t lSecOfDay = TimeSecondOfDayGet();

    if ((lSecOfDay
         >= (SMCSAsynchSPChange.sStartTime
             * (uint32_t) MAX_SECONDS_IN_A_MINUTE)) && (lSecOfDay
                                                        <
                                                        (
                                                          SMCSAsynchSPChange.
                                                          sEndTime
                                                          * (uint32_t)
                                                          MAX_SECONDS_IN_A_MINUTE)))
    {
      return SMCSAsynchSPChange.bSPNo;
    }
    else if (lSecOfDay
             >= (SMCSAsynchSPChange.sEndTime
                 * (uint32_t) MAX_SECONDS_IN_A_MINUTE))
    {
      SMCSAsynchSPChange.fWSRunning = FALSE;
    }
  }

  return 0;
}

void MCSAsynchLogSeize(void)
{
  osMutexAcquire(LogMutexHandle, osWaitForever);
}

void MCSAsynchLogRelease(void)
{
  osMutexRelease(LogMutexHandle);
}

uint8_t MCSAsynchStandbyMsgSet(void)
{
  tSMCSStopMode SMCSStopMode;

  SMCSStopMode.fStopMode = TRUE;

  return MCSAsynchReqTxMsg(MCS_ASYNCH_HEADER_STOP_MODE,
                           sizeof(tSMCSStopMode),
                           &SMCSStopMode);
}

void MCSAsynchSendMessage(tpSMCSAsynchRxTxMsg pSTxMsg)
{
  (void) ModemSend(s_driver, (const uint8_t *) pSTxMsg, pSTxMsg->sDataLen);
}

void MCSAsyMsgSenderTaskFunc(void *argument)
{
  UNUSED(argument);

  tpSMCSAsynchRxTxMsg pSTxMsg = NULL;

  while (TRUE)
  {
    if (osMessageQueueGet(MCSAsyTxReqsQueHandle, &pSTxMsg, NULL,
                          osWaitForever)
        == osOK)
    {
      MCSAsynchSendMessage(pSTxMsg);
      osMemoryPoolFree(MCSAsyTxReqsMemPoolHandle, pSTxMsg);
    }
  }
}

void MCSAsynchParseMsg(tpSMCSAsynchMsg pSRxMsg)
{
  switch (pSRxMsg->bHeader)
  {
      case MCS_ASYNCH_SO_MEAS_START_OR_STOP:
      {
        SMCSAsynchRuntime.bSSMNo = pSRxMsg->UData.SMCSSOMeas.bSSMNo;
        SMCSAsynchRuntime.bSOMeasurements = pSRxMsg->UData.SMCSSOMeas.fState;
        break;
      }

      case MCS_ASYNCH_HEADER_USER_SETTINGS:
      {
        MCSAsynchIntersectionSettingsMsgSet();
        break;
      }

      case MCS_ASYNCH_HEADER_BROKEN_INPUT_SETTINGS:
      {
        MCSAsynchBrokenInputSettingsMsgSet();
        break;
      }

      case MCS_ASYNCH_HEADER_SERVER_SETTINGS:
      {
        MCSAsynchServerSettingsMsgSet();
        break;
      }

      case MCS_ASYNCH_HEADER_USER_SETTINGS_SET:
      {
        tSUserSettings SLUserSettings;

        SLUserSettings.fSettingsChanged = USER_SETTINGS_CHANGE_CONTROL_VLAUE;
        SLUserSettings.fConfigFlag =
          pSRxMsg->UData.SMCSUserSettings.fConfigFlag;
        SLUserSettings.fLogFlag = pSRxMsg->UData.SMCSUserSettings.fLogFlag;
        SLUserSettings.fTrafficCountsFlag =
          pSRxMsg->UData.SMCSUserSettings.fTrafficCountsFlag;
        SLUserSettings.bTrafficCountsPeriod =
          pSRxMsg->UData.SMCSUserSettings.bTrafficCountsPeriod;
        SLUserSettings.fStandbyInfoFlag =
          pSRxMsg->UData.SMCSUserSettings.fStandbyInfoFlag;

        UserSettingsSet(&SLUserSettings);
        if (UserSettingsSave())
        {
          UserSettingsRead();
        }

        break;
      }

      case MCS_ASYNCH_HEADER_BROKEN_INPUT_SETTINGS_SET:
      {
        tSBrokenInputSettings SSettings;

        memset(&SSettings, 0, sizeof(SSettings));

        SSettings.fAlreadySet = BROKEN_INPUT_SETTINGS_SET_CONTROL_VLAUE;
        SSettings.SFlags.fLoopBusy =
          pSRxMsg->UData.SBrokenInputSettings.fLoopInputFlag;
        SSettings.SFlags.fDigitalBusy =
          pSRxMsg->UData.SBrokenInputSettings.fDigitalInputFlag;

        BrokenInputSettingsSet(&SSettings);
        if (BrokenInputSettingsSave())
        {
          BrokenInputSettingsRead();
        }

        break;
      }

      case MCS_ASYNCH_HEADER_SERVER_SETTINGS_SET:
      {
        tSServerSettings SSettings;

        memset(&SSettings, 0, sizeof(SSettings));

        SSettings.fAlreadySet = SERVER_SETTINGS_SET_CONTROL_VLAUE;
        SSettings.SFlags.fMCSAvailable =
          pSRxMsg->UData.SServerSettings.fMCSAvailable;
        SSettings.SFlags.fNTCIPAvailable =
          pSRxMsg->UData.SServerSettings.fNTCIPAvailable;

        ServerSettingsSet(&SSettings);
        if (ServerSettingsSave())
        {
          ServerSettingsRead();
        }

        break;
      }

      case MCS_ASYNCH_HEADER_START_IAP:
      {
        IAPRequest(IAP_REQUEST_SOURCE_MCS,
                   pSRxMsg->bLen - 4,
                   &pSRxMsg->UData.baData);
        break;
      }

      case MCS_ASYNCH_HEADER_POWER_RECORDS:
      {
        ClearSOPowers();
        LogRequest(LOG_REQ_APPEND_ASYNCH,
                   NULL,
                   EVENT_USER_REQ_POWER_LEARNING,
                   POWER_L_SOURCE_MCS,
                   0,
                   0,
                   0);
        break;
      }

      case MCS_ASYNCH_HEADER_START_WEB_ENGINE:
      {
        MCSAsynchWebEngineTimeoutReset();
        MCSAsynchWebEngineSet(TRUE);
        break;
      }

      case MCS_ASYNCH_HEADER_LCD_STREAM_START:
      {
        MCSAsynchLCDStreamTimeoutReset();
        MCSAsynchLCDStreamSet(TRUE);
        break;
      }

      case MCS_ASYNCH_HEADER_VERSION_GET:
      {
        MCSAsynchVersionMsgSet();
        break;
      }

      case MCS_ASYNCH_HEADER_SP_CHANGE:
      {
        if ((pSRxMsg->UData.SSPChange.bSPNo > 0)
            && (pSRxMsg->UData.SSPChange.bSPNo <= SignalPlanTotalGet()))
        {
          if (pSRxMsg->UData.SSPChange.sStartTime
              < pSRxMsg->UData.SSPChange.sEndTime)
          {
            if (MCSAsynchSPChangeCmdSet(&(pSRxMsg->UData.SSPChange)))
            {
              if (MCSAsynchSPChangeCmdGet(&SMCSAsynchSPChange))
              {
                if (pSRxMsg->UData.SSPChange.fWSRunning)
                {
                  LogRequest(LOG_REQ_APPEND_ASYNCH,
                             NULL,
                             EVENT_MCS_USER_REQUEST_SP_CHANGE,
                             pSRxMsg->UData.SSPChange.bSPNo,
                             pSRxMsg->UData.SSPChange.sStartTime,
                             pSRxMsg->UData.SSPChange.sEndTime,
                             0);

                  break;
                }
              }
            }
          }
        }

        break;
      }

      case MCS_ASYNCH_HEADER_DOWNLOAD:
      case MCS_ASYNCH_HEADER_UPLOAD:
      {
        break;
      }

      case MCS_ASYNCH_HEADER_USER_REQUEST:
      {
        switch (pSRxMsg->UData.bUserRequest)
        {
            case MCS_USER_REQUEST_FLASH:
            {
              UserStateReqSet(STATES_FLASH);
              LogRequest(LOG_REQ_APPEND_ASYNCH, NULL,
                         EVENT_MCS_USER_REQ_WORK_MODE_TO_FLASH,
                         0, 0, 0, 0);
              break;
            }

            case MCS_USER_REQUEST_DARK:
            {
              UserStateReqSet(STATES_NO_CONTROL);
              LogRequest(LOG_REQ_APPEND_ASYNCH,
                         NULL,
                         EVENT_MCS_USER_REQ_WORK_MODE_TO_DARK,
                         0,
                         0,
                         0,
                         0);
              break;
            }

            case MCS_USER_REQUEST_ALL_RED:
            {
              UserStateReqSet(STATES_CLOSED);
              LogRequest(LOG_REQ_APPEND_ASYNCH, NULL,
                         EVENT_MCS_USER_REQ_WORK_MODE_TO_ALL_RED,
                         0, 0, 0, 0);
              break;
            }

            case MCS_USER_REQUEST_PLAN_RETURN:
            {
              UserStateReqFree();
              LogRequest(LOG_REQ_APPEND_ASYNCH, NULL,
                         EVENT_MCS_USER_REQ_WORK_MODE_TO_WORK_PLAN,
                         0, 0, 0, 0);
              break;
            }

            default:
            {
              break;
            }
        } /* switch */

        break;
      }

      case MCS_ASYNCH_HEADER_RESET:
      {
        LogRequest(LOG_REQ_APPEND_ASYNCH,
                   NULL,
                   EVENT_MCS_USER_REQUEST_RESET,
                   0,
                   0,
                   0,
                   0);

        SecureSystemReset();
        break;
      }

      case MCS_ASYNCH_HEADER_TIME:
      {
        tSTime SCurrMCSTime;

        memset(&SCurrMCSTime, 0, sizeof(tSTime));

        SCurrMCSTime.bCentury = TIME_CURRENT_CENTURY - 1;
        SCurrMCSTime.SCurrentTime.Minutes = (pSRxMsg->UData.STime.sMinOfDay
                                             % 60);
        SCurrMCSTime.SCurrentTime.Hours = (pSRxMsg->UData.STime.sMinOfDay / 60);
        SCurrMCSTime.SCurrentDate.Date = pSRxMsg->UData.STime.bDay;
        SCurrMCSTime.SCurrentDate.Month = pSRxMsg->UData.STime.bMonth;
        SCurrMCSTime.SCurrentDate.Year = pSRxMsg->UData.STime.bYear;
        SCurrMCSTime.SCurrentDate.WeekDay =
          TimeWeekDayOfYearCalc(SCurrMCSTime.SCurrentDate.Month,
                                SCurrMCSTime.
                                SCurrentDate.Date,
                                TimeFullYearCalc
                                  (&SCurrMCSTime));

        if (!GpsModemAliveGet() || !GpsRTCInitialUpdateDoneGet())     /* Set time only if GPS is detached */
        {
          GpsTimeAdjust(&SCurrMCSTime);
          if (TimeIsValid(&SCurrMCSTime))
          {
            TimeSet(&SCurrMCSTime);
          }
        }

        break;
      }

      case MCS_ASYNCH_HEADER_CONNECTION_CHECK:
      {
        MCSAsynchConnectionTimeoutReset();
        break;
      }

      case MCS_ASYNCH_HEADER_LOG_SETTINGS_SET:
      {
        tSLogSettings SLLogSettings;

        memset(&SLLogSettings, 0xFF, sizeof(tSLogSettings));
        memcpy(&SLLogSettings, pSRxMsg->UData.baData, sizeof(tSLogSettings));
        SLLogSettings.fSettingsChanged = LOG_SETTINGS_CHANGE_CONTROL_VLAUE;

        LogSettingsSet(&SLLogSettings);
        if (LogSettingsSave())
        {
          LogSettingsRead();
        }

        break;
      }

      case MCS_ASYNCH_HEADER_LOG_SETTINGS:
      {
        MCSAsynchLogSettingsMsgSet();
        break;
      }

      case MCS_ASYNCH_HEADER_IR_STREAM_START:
      {
        MCSAsynchInputRuntimeStreamTimeoutReset();
        MCSAsynchInputRuntimeStreamSet(TRUE);
        break;
      }

      case MCS_ASYNCH_HEADER_IR_STREAM_SET:
      {
        uint8_t bIOMNo;
        uint8_t bLDPNo;
        uint8_t bLDMNo;
        tSVirtualInput SVirtualInput;

        memset(&SVirtualInput, 0, sizeof(tSVirtualInput));
        memcpy(&SVirtualInput, pSRxMsg->UData.baData, sizeof(tSVirtualInput));

        bIOMNo = (SVirtualInput.bNumber - 1) / 16;
        bLDPNo = (SVirtualInput.bNumber - 1) / 8;

        switch (SVirtualInput.bType)
        {
            case INPUT_TYPE_DIGITAL:
            {
              tSCanDigitalIOInputs SIOInputs;

              memset(&SIOInputs, 0, sizeof(SIOInputs));

              /* Set state as safe automatically as IO doesn't support sending safe */
              /* states for the moment */
              SIOInputs.sInputSafeStates = 0xFFFF;
              SIOInputs.sInputStates =
                SaCanDigitalIOInputs[bIOMNo].sInputStates;                              /* Get previous states */
              if (!SVirtualInput.bState)
              {
                SetBitValue(SIOInputs.sInputStates,
                            ((SVirtualInput.bNumber - 1) % 16));
              }
              else
              {
                ClearBitValue(SIOInputs.sInputStates,
                              ((SVirtualInput.bNumber - 1) % 16));
              }

              SIOInputs.fIsPhysicallyDriven = FALSE;

              SetIOInputs(bIOMNo, &SIOInputs);
              break;
            }

            case INPUT_TYPE_DETECTOR:
            {
              uint8_t bVirtualInputStates;
              uint16_t sCanVirtualInputs;

              SaCanDetectorIOInputs[bIOMNo].fIsPhysicallyDriven = FALSE;
              sCanVirtualInputs =
                SaCanDetectorIOInputs[bIOMNo].sLoopEmptyStates;
              if (!SVirtualInput.bState)
              {
                SetBitValue(sCanVirtualInputs,
                            ((SVirtualInput.bNumber - 1) % 16));
              }
              else
              {
                ClearBitValue(sCanVirtualInputs,
                              ((SVirtualInput.bNumber - 1) % 16));
              }

              if (bLDPNo % 2 == 0)
              {
                bVirtualInputStates = sCanVirtualInputs & 0xFF;
              }
              else
              {
                bVirtualInputStates = sCanVirtualInputs >> 8;
              }

              bLDMNo = bLDPNo * 2;

              if (bVirtualInputStates != 0xFF)
              {
                if ((0xFF - bVirtualInputStates) < 16)
                {
                  bVirtualInputStates = 0xFF - bVirtualInputStates;
                  SetLDInputs(bLDMNo, bIOMNo, &bVirtualInputStates);
                }
                else
                {
                  bVirtualInputStates = (0xFF - bVirtualInputStates) / 16;
                  SetLDInputs(bLDMNo + 1, bIOMNo, &bVirtualInputStates);
                }
              }
              else
              {
                bVirtualInputStates = 0x00;
                SetLDInputs(bLDMNo, bIOMNo, &bVirtualInputStates);
                SetLDInputs(bLDMNo + 1, bIOMNo, &bVirtualInputStates);
              }

              break;
            }

            default:
            {
              break;
            }
        } /* switch */

        break;
      }

      case MCS_ASYNCH_HEADER_DST_SET:
      {
        uint8_t bOldDSTFlag, bNewDSTFlag;

        bOldDSTFlag = 0;
        bNewDSTFlag = 0;

        ReadDaylightSavingTimeFlag();
        GetDaylightSavingTimeFlag(&bOldDSTFlag);

        bNewDSTFlag = pSRxMsg->UData.bDSTFlag;

        SetDaylightSavingTimeFlag(bNewDSTFlag);
        if (WriteDaylightSavingTimeFlag())
        {
          ReadDaylightSavingTimeFlag();
        }

        if (GpsModemAliveGet())
        {
          if (bOldDSTFlag != bNewDSTFlag)
          {
            GpsRTCInitialUpdateDoneSet(FALSE);
          }
        }

        break;
      }

      case MCS_ASYNCH_HEADER_DST:
      {
        MCSAsynchDSTFlagMsgSet();
        break;
      }

      case MCS_ASYNCH_HEADER_STOP_MODE:
      {
        if (GetStandbyState())
        {
          if (pSRxMsg->UData.bAckNak == MCS_ASYNCH_MSG_ACK)
          {
            if (StandbyEventHandle != NULL)
            {
              osEventFlagsSet(StandbyEventHandle,
                              EVENT_FLAGS_STANDBY_INFO_MSG_SENT);
              osThreadSuspend(osThreadGetId());
            }
          }
        }

        break;
      }

      default:
      {
        break;
      }
  } /* switch */
} /* MCSAsynchParseMsg */

static void MCSAsynchReceiveByte(uint8_t bRxByte)
{
  if (SMCSAsynchRx.bBusy == TRUE)
  {
    switch (SMCSAsynchRx.eState)
    {
        case MCS_ASYNCH_TXRX_STATE_START:
        {
          if (bRxByte == MCS_ASYNCH_MSG_START)
          {
            SMCSAsynchRx.SMessage.bStart = bRxByte;
            SMCSAsynchRx.eState = MCS_ASYNCH_TXRX_STATE_HEADER;
          }
          else
          {
            if (bRemEndResIdx < MCS_ASYNCH_DATA_PACKET_MAX_LEN)
            {
              baRemEndResponse[bRemEndResIdx++] = bRxByte;
            }
          }

          break;
        }

        case MCS_ASYNCH_TXRX_STATE_HEADER:
        {
          if ((bRxByte >= MCS_ASYNCH_HEADER_FIRST)
              && (bRxByte <= MCS_ASYNCH_HEADER_LAST) )
          {
            SMCSAsynchRx.SMessage.bHeader = bRxByte;
            SMCSAsynchRx.eState = MCS_ASYNCH_TXRX_STATE_LENGTH;
          }

          break;
        }

        case MCS_ASYNCH_TXRX_STATE_LENGTH:
        {
          SMCSAsynchRx.SMessage.bLen = bRxByte;
          SMCSAsynchRx.sCurrentByte = 0;
          SMCSAsynchRx.eState = MCS_ASYNCH_TXRX_STATE_DATA;
          break;
        }

        case MCS_ASYNCH_TXRX_STATE_EPOCH:
        {
          break;
        }

        case MCS_ASYNCH_TXRX_STATE_DATA:
        {
          if (SMCSAsynchRx.sCurrentByte < SMCSAsynchRx.SMessage.bLen)
          {
            if (SMCSAsynchRx.sCurrentByte
                < sizeof(SMCSAsynchRx.SMessage.lEpoch))
            {
              SMCSAsynchRx.SMessage.lEpoch |= (uint32_t) (bRxByte <<
                                                          ((SMCSAsynchRx.
                                                            sCurrentByte) * 8));
            }
            else
            {
              SMCSAsynchRx.SMessage.UData.baData[SMCSAsynchRx.sCurrentByte
                                                 - sizeof(SMCSAsynchRx.SMessage.
                                                          lEpoch)] = bRxByte;
            }

            SMCSAsynchRx.sCurrentByte++;
            if (SMCSAsynchRx.sCurrentByte == SMCSAsynchRx.SMessage.bLen)
            {
              SMCSAsynchRx.sCurrentByte = 0;
              SMCSAsynchRx.eState = MCS_ASYNCH_TXRX_STATE_END;
            }
          }

          break;
        }

        case MCS_ASYNCH_TXRX_STATE_END:
        {
          if (bRxByte == MCS_ASYNCH_MSG_END)
          {
            SMCSAsynchRx.SMessage.bEnd = bRxByte;
            SMCSAsynchRx.eState = MCS_ASYNCH_TXRX_STATE_COMPLETE;
            SMCSAsynchRx.bBusy = FALSE;

            MCSAsynchMsgRcvdSet(TRUE);

            if ((SMCSAsynchRx.SMessage.bHeader == MCS_ASYNCH_HEADER_IMEI)
                || (SMCSAsynchRx.SMessage.bHeader
                    ==
                    MCS_ASYNCH_HEADER_MAC) )
            {
              if (SMCSAsynchRx.SMessage.UData.bAckNak == MCS_ASYNCH_MSG_ACK)
              {
                if (MCSAsyEventHandle != NULL)
                {
                  osEventFlagsSet(MCSAsyEventHandle,
                                  EVENT_FLAGS_MCS_ASY_CONNECTED);
                }
              }
            }

            MCSAsynchParseMsg(&SMCSAsynchRx.SMessage);

            MCSAsynchStartReception();
          }
          else
          {
            /* Unexpected value as end of message */
            SMCSAsynchRx.eState = MCS_ASYNCH_TXRX_STATE_START;
          }

          break;
        }

        default:
        {
          break;
        }
    } /* switch */
  }
} /* MCSAsynchReceiveByte */

void MCSAsynchReceivePacket(tpSMCSAsynchRxTxMsg pSRxPacket)
{
  uint16_t sIndex;

  MCSAsynchStartReception();
  for (sIndex = 0; sIndex < pSRxPacket->sDataLen; sIndex++)
  {
    MCSAsynchReceiveByte(pSRxPacket->baData[sIndex]);
  }
}

void MCSAsyMsgParserTaskFunc(void *argument)
{
  UNUSED(argument);

  tpSMCSAsynchRxTxMsg pSRxMsg = NULL;

  while (TRUE)
  {
    if (MCSGetConnected())
    {
      if (osMessageQueueGet(MCSAsyRxReqsQueHandle, &pSRxMsg, NULL,
                            MCS_ASYNCH_TIMEOUT_MSG_BOX)
          == osOK)
      {
        MCSAsynchReceivePacket(pSRxMsg);
        osMemoryPoolFree(MCSAsyRxReqsMemPoolHandle, pSRxMsg);
      }
      else
      {
        if (MCSAsynchConnectedGet())
        {
          MCSAsynchStop(MCS_ASYNCH_DISC_TYPE_TIMEOUT_MSG_BOX);
        }
      }
    }
    else
    {
      osDelay(1000);
    }
  }
}

void MCSAsyTaskFunc(void *argument)
{
  UNUSED(argument);

  MCSAsynchInit(&g_modemPort, &g_modemDriverPort);

  while (TRUE)
  {
    if (MCSAsynchConnectedGet() && !GetStandbyState())
    {
      if (!fVersionSent)
      {
        MCSAsynchVersionMsgSet();
        fVersionSent = TRUE;
      }

      /* Partial Runtime */
      if (MCSAsynchWebEngineGet() == FALSE)
      {
        if ((bPartialRunTimeTimer % MCS_ASYNCH_PERIOD_PARTIAL_RUNTIME) == 0)
        {
          MCSAsynchPartialRuntimeMsgSet();
          bPartialRunTimeTimer = 0;
        }
      }

      if (ProgramStateGet() != PROGRAM_STATE_LOADING)
      {
        if ((sErrorInfoTimer % MCS_ASYNCH_PERIOD_ERROR_INFO) == 0)
        {
          MCSAsynchErrorInfoMsgSet();
          sErrorInfoTimer = 0;
        }

        if (MCSAsynchLCDStreamGet())
        {
          MCSAsynchLCDStreamMsgSet();
          MCSAsynchLCDStreamTimeoutIncrease();
        }

        if (MCSAsynchWebEngineGet())         /* if instant signal window is open */
        {
          bPartialRunTimeTimer = 0;
          MCSAsynchRuntimeMsgSet();
          MCSAsynchInputsMsgSet();
          MCSAsynchSignalsMsgSet();           /* send signal msgs */
          MCSAsynchWebEngineTimeoutIncrease();
        }

        if (MCSAsynchInputRuntimeStreamGet())
        {
          MCSAsynchInputRuntimeMsgSet();
          MCSAsynchInputRuntimeStreamTimeoutIncrease();
        }

        /* Traffic Counts */
        if (UserSettingsTrafficCountsFlagGet())
        {
          if (GetTrafficCountsTimer() % MCS_ASYNCH_PERIOD_TRAFFIC_COUNTS == 0)
          {
            MCSAsynchTrafficCountsDigitalMsgSet();
            MCSAsynchTrafficCountsLoopMsgSet();
          }
        }

        /* Log */
        if (UserSettingsLogFlagGet())
        {
          MCSAsynchReadLogReadIndex();

          if (LogEventNew(sLogReadIndex) != LOG_NO_NEW_LOG)
          {
            tSLogRecord SMCSAsynchLog;

            memset(&SMCSAsynchLog, 0, sizeof(tSLogRecord));

            MCSAsynchLogSeize();
            if (LogRequest(LOG_REQ_READ_NEXT,
                           &SMCSAsynchLog,
                           0,
                           0,
                           0,
                           0,
                           sLogReadIndex))
            {
              sLogReadIndex++;
              sLogReadIndex %= LOG_RECORDS_MAX;

              if (GetLogSettingsByEventID(SMCSAsynchLog.SEvent.bEvent))
              {
                switch (SMCSAsynchLog.SEvent.bEvent)
                {
                    case EVENT_POWER_ON:
                    case EVENT_POWER_NORMAL_TO_STAND_BY:
                    case EVENT_SO_SWITCH_SHORT_CIRCUIT:
                    case EVENT_SO_SWITCH_OPEN_CIRCUIT:
                    case EVENT_SO_VOLTAGE_SENSOR_FAILURE:
                    case EVENT_SO_LAMPS_DRIVEN_EXTERNALLY:
                    case EVENT_SO_WORKING_LAMP_TOTAL_CHANGE:
                    case EVENT_SG_RED_LAMP_FAILURE:
                    case EVENT_SG_LAST_RED_LAMP_FAILURE:
                    case EVENT_SG_NUMBER_OF_RED_LAMPS_FAILURE:
                    case EVENT_SG_YELLOW_LAMP_FAILURE:
                    case EVENT_SG_GREEN_LAMP_FAILURE:
                    case EVENT_YELLOW_YELLOW_CONFLICT:
                    case EVENT_YELLOW_GREEN_CONFLICT:
                    case EVENT_GREEN_GREEN_CONFLICT:
                    case EVENT_SO_POWER_RECORD:
                    case EVENT_MODULE_MISSING:
                    case EVENT_MODULE_RESPONDS:
                    case EVENT_SG_ALL_RED_LAMPS_BROKEN:
                    case EVENT_SG_ALL_YELLOW_LAMPS_BROKEN:
                    case EVENT_SG_ALL_GREEN_LAMPS_BROKEN:
                    case EVENT_SET_SIGNALING_MODE_CHANGE:
                    case EVENT_DOOR_OPEN:
                    case EVENT_DOOR_CLOSED:
                    case EVENT_USER_REQ_WORK_MODE_TO_ALL_RED:
                    case EVENT_USER_REQ_WORK_MODE_TO_DARK:
                    case EVENT_USER_REQ_WORK_MODE_TO_FLASH:
                    case EVENT_USER_REQ_WORK_MODE_TO_WORK_PLAN:
                    case EVENT_USER_REQ_POWER_LEARNING:
                    case EVENT_USER_REQ_TIME_SET:
                    case EVENT_USER_REQ_RELAY_SET_ON:
                    case EVENT_USER_REQ_RELAY_SET_OFF:
                    case EVENT_USER_REQ_LCD_LOG_IN:
                    case EVENT_USER_REQ_LCD_LOG_OUT:
                    case EVENT_DETECTOR_BROKEN:
                    case EVENT_DETECTOR_SAFE:
                    case EVENT_WORK_PLAN_CHANGE:
                    case EVENT_SIGNAL_PROGRAM_PLAN_CHANGE:
                    case EVENT_SIGNAL_PROGRAM_CHANGE:
                    case EVENT_MCS_USER_REQUEST_SP_CHANGE:
                    case EVENT_MCS_USER_REQUEST_RESET:
                    case EVENT_MCS_USER_REQUEST_DOWNLOAD:
                    case EVENT_MCS_USER_REQUEST_UPLOAD:
                    case EVENT_MCS_USER_REQ_WORK_MODE_TO_ALL_RED:
                    case EVENT_MCS_USER_REQ_WORK_MODE_TO_DARK:
                    case EVENT_MCS_USER_REQ_WORK_MODE_TO_FLASH:
                    case EVENT_MCS_USER_REQ_WORK_MODE_TO_WORK_PLAN:
                    case EVENT_MAIN_STORAGE_GET_ERROR:
                    case EVENT_MAIN_STORAGE_SET_ERROR:
                    case EVENT_MCS_USER_REQ_START_IAP:
                    {
                      MCSAsynchReqTxMsg(MCS_ASYNCH_HEADER_LOG,
                                        sizeof(SMCSAsynchLog), &SMCSAsynchLog);
                      break;
                    }
                } /* switch */
              }

              MCSAsynchWriteLogReadIndex();
            }

            MCSAsynchLogRelease();
          }
        }
      }

      bPartialRunTimeTimer++;
      sErrorInfoTimer++;
    }

    osDelay(1000);
  }
} /* MCSAsyTaskFunc */
