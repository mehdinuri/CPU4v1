/* App/Adapters/STM32/MmiLegacyStatusAdapter.c */
#include "MmiLegacyStatusAdapter.h"

#include <string.h>

#include "MCS.h"
#include "MCSAsynch.h"
#include "cpmpcomm.h"
#include "data.h"
#include "gps.h"
#include "program.h"
#include "time.h"

enum
{
  MMI_LEGACY_SIGNAL_BLOCK_COUNT = 2U,
  MMI_LEGACY_SIGNAL_WORDS_PER_BLOCK = 4U,
  MMI_LEGACY_STATE_PROGRAM_LOAD = 9U
};

static uint8_t ReadMeasurement(void *ctx, MmiLegacyMeasurement_t *measurement)
{
  (void) ctx;

  if (measurement == NULL)
  {
    return FALSE;
  }

  measurement->psmVoltageTenths[0] =
    (uint16_t) ((float) GetPowerSupplyNet(0U) * 0.73029f);
  measurement->psmVoltageTenths[1] =
    (uint16_t) ((float) GetPowerSupplyNet(1U) * 0.73029f);
  measurement->psmFrequency[0] = (uint8_t) GetPowerSupplyFreq(0U);
  measurement->psmFrequency[1] = (uint8_t) GetPowerSupplyFreq(1U);

  return TRUE;
}

static uint8_t ReadTimeValue(void *ctx, MmiLegacyTime_t *timeValue)
{
  tSTime currentTime;

  (void) ctx;

  if (timeValue == NULL)
  {
    return FALSE;
  }

  (void) memset(&currentTime, 0, sizeof(currentTime));
  TimeGet(&currentTime);

  timeValue->timeSource = TimeSourceGet();
  timeValue->second = currentTime.SCurrentTime.Seconds;
  timeValue->minute = currentTime.SCurrentTime.Minutes;
  timeValue->hour = currentTime.SCurrentTime.Hours;
  timeValue->day = currentTime.SCurrentDate.Date;
  timeValue->month = currentTime.SCurrentDate.Month;
  timeValue->year = currentTime.SCurrentDate.Year;

  return TRUE;
}

static uint8_t ReadWorkmode(void *ctx, MmiLegacyWorkmode_t *workmode)
{
  (void) ctx;

  if (workmode == NULL)
  {
    return FALSE;
  }

  (void) memset(workmode, 0, sizeof(*workmode));

  if (CPMPStateGet() == PACKET_TYPE_CP_DEFAULT)
  {
    workmode->state = StateCurrentGet();
  }
  else
  {
    workmode->state = MMI_LEGACY_STATE_PROGRAM_LOAD;
  }

  switch (workmode->state)
  {
      case STATES_SEQ:
      {
        workmode->arg1 = SeqCurrentGet();
        workmode->arg2 = SeqTotalGet();
        workmode->arg3 = (uint8_t) (SeqCurrentStepGet() + 1U);
        workmode->arg4 = SeqCurStepNumTotalGet();
        workmode->arg5 = SeqCurrentStepCurrentDurationGet();
        workmode->arg6 = SeqCurrentStepDurationGet();
        workmode->arg7 = SeqDurCurGet();
        workmode->arg8 =
          (uint8_t) (SeqDurGet((uint8_t) (SeqCurrentGet() - 1U))
                     + SeqTotalExtDurGet());
        break;
      }

      case STATES_PHASE:
      {
        workmode->arg1 = ProgramCurrentNoGet();
        workmode->arg2 = PhaseTotalGet();
        workmode->arg3 = PhaseMinDurationGet((uint8_t) (ProgramCurrentNoGet()
                                                         - 1U));
        workmode->arg4 = WorkPlanPhaseDurGet((uint8_t) (ProgramCurrentNoGet()
                                                         - 1U));
        workmode->arg5 = PhaseElapsedDurGet((uint8_t) (ProgramCurrentNoGet()
                                                        - 1U));
        break;
      }

      case STATES_PHASE_TRANSITION:
      {
        workmode->arg1 = ProgramCurrentNoGet();
        workmode->arg2 = ProgramTargetNoGet();
        break;
      }

      case MMI_LEGACY_STATE_PROGRAM_LOAD:
      {
        workmode->arg1 = ProgramLoadingStatusGet();
        break;
      }

      default:
      {
        break;
      }
  }

  return TRUE;
}

static uint8_t ReadModuleStatus(void *ctx, MmiLegacyModuleStatus_t *moduleStatus)
{
  (void) ctx;

  if (moduleStatus == NULL)
  {
    return FALSE;
  }

  (void) memset(moduleStatus, 0, sizeof(*moduleStatus));
  moduleStatus->gpsModemConnected = GpsModemAliveGet();
  moduleStatus->gpsAntennaConnected = GpsAntStatusGet();
  moduleStatus->gprsModemConnected = MCSAsynchConnectedGet();
  moduleStatus->gprsCenterConnected = MCSAsynchConnectedGet();
  moduleStatus->relayClosed = GetPowerRelay();
  moduleStatus->lastDigitalInputDemand = GetLastInputDemandIssued();
  moduleStatus->lastLoopDetectorDemand = GetLastDetectorDemandIssued();
  moduleStatus->gpsModeType = GpsPortGet();

  return TRUE;
}

static uint8_t GetSetTotal(void *ctx)
{
  (void) ctx;
  return SetTotalGet();
}

static uint8_t ReadErrorRecord(void *ctx,
                               uint8_t setIndex,
                               MmiLegacyErrorRecord_t *record)
{
  tSSetRuntime runtime;

  (void) ctx;

  if ((record == NULL) || (setIndex >= SetTotalGet()))
  {
    return FALSE;
  }

  if (SetSigModeIsEmergent(setIndex) == FALSE)
  {
    return FALSE;
  }

  (void) memset(record, 0, sizeof(*record));
  (void) memset(&runtime, 0, sizeof(runtime));
  SetRuntimeGet(setIndex, &runtime);
  record->setNumber = (uint8_t) (setIndex + 1U);
  record->signalingMode = runtime.bSignalingMode;
  record->signalingModeSource = runtime.bSigModeSource;
  record->param1 = runtime.bParam1;
  record->param2 = runtime.bParam2;

  return TRUE;
}

static uint8_t ReadSignalsBlock(void *ctx,
                                uint8_t blockIndex,
                                uint16_t words[4])
{
  uint16_t startSsm;
  uint16_t endSsm;
  uint16_t ssmNo;
  uint8_t sgNo;
  uint8_t soNo;
  uint8_t currentSignalGroup;
  uint8_t currentOutput;
  uint8_t outputType;
  uint8_t signalNo;
  uint8_t voltages;
  uint16_t period;

  (void) ctx;

  if ((words == NULL) || (blockIndex >= MMI_LEGACY_SIGNAL_BLOCK_COUNT))
  {
    return FALSE;
  }

  (void) memset(words, 0, sizeof(uint16_t) * MMI_LEGACY_SIGNAL_WORDS_PER_BLOCK);
  startSsm = (uint16_t) (blockIndex * 4U);
  endSsm = (uint16_t) (startSsm + 4U);
  if (endSsm > MODULES_SSM_MAX)
  {
    endSsm = MODULES_SSM_MAX;
  }

  for (ssmNo = startSsm; ssmNo < endSsm; ++ssmNo)
  {
    words[ssmNo % 4U] = (uint16_t) ((ssmNo + 1U) << 12U);

    for (sgNo = 0U; sgNo < 4U; ++sgNo)
    {
      currentSignalGroup = (uint8_t) ((ssmNo * 4U) + sgNo);

      for (soNo = 0U; soNo < 3U; ++soNo)
      {
        currentOutput = (uint8_t) ((currentSignalGroup * 3U) + soNo);
        outputType = GetSOType(currentOutput);
        signalNo = SGSignalGet(currentSignalGroup);
        if (outputType == 0U)
        {
          continue;
        }

        voltages = SignalVoltagesGet(signalNo);
        period = SubSignalHasFlash(signalNo, outputType);
        if (((period == 0U) || ((period > 0U) && FlashOnGet(period)))
            && ((voltages & outputType) != 0U))
        {
          words[ssmNo % 4U] |= laValue2Bit[11U - (currentOutput % 12U)];
        }
      }
    }
  }

  return TRUE;
}

static uint8_t ReadInputs(void *ctx,
                          uint32_t *loopDemands,
                          uint32_t *digitalDemands)
{
  uint8_t moduleIndex;
  uint8_t bitIndex;

  (void) ctx;

  if ((loopDemands == NULL) || (digitalDemands == NULL))
  {
    return FALSE;
  }

  *loopDemands = 0UL;
  *digitalDemands = 0UL;

  for (moduleIndex = 0U; moduleIndex < MODULES_IO_MAX; ++moduleIndex)
  {
    for (bitIndex = 0U; bitIndex < 16U; ++bitIndex)
    {
      if (GetBitValue(SaCanDetectorIOInputs[moduleIndex].sLoopEmptyStates,
                      bitIndex) == FALSE)
      {
        SetBitValue(*loopDemands,
                    (uint8_t) ((moduleIndex == 0U) ? bitIndex
                              : (bitIndex + 16U)));
      }

      if (GetBitValue(SaCanDigitalIOInputs[moduleIndex].sInputStates,
                      bitIndex) == FALSE)
      {
        SetBitValue(*digitalDemands,
                    (uint8_t) ((moduleIndex == 0U) ? bitIndex
                              : (bitIndex + 16U)));
      }
    }
  }

  return TRUE;
}

static uint8_t ReadGprsLog(void *ctx, MmiLegacyGprsLog_t *logState)
{
  (void) ctx;

  if (logState == NULL)
  {
    return FALSE;
  }

  (void) memset(logState, 0, sizeof(*logState));
  logState->modemType = MCSGetModemType();
  logState->state = MCSGetGPRSState();
  logState->signalQuality = (uint8_t) (MCSGetGprsSignalQuality() / 6U);
  return TRUE;
}

static const char *GetGprsImei(void *ctx)
{
  (void) ctx;
  return MCSGetGprsModemIMEI();
}

static const char *GetUsrMac(void *ctx)
{
  (void) ctx;
  return MCSGetUSRModuleMAC();
}

static const char *GetEthernetMac(void *ctx)
{
  (void) ctx;
  return MCSGetRuntimeEthernetMAC();
}

static const char *GetGsmOperator(void *ctx)
{
  (void) ctx;
  return MCSGetGprsGsmOperator();
}

void MmiLegacyStatusAdapterInit(MmiLegacyStatusAdapterCtx_t *ctx)
{
  if (ctx != NULL)
  {
    ctx->reserved0 = 0U;
  }
}

IMmiLegacyStatusPort_t MmiLegacyStatusAdapterCreatePort(
  MmiLegacyStatusAdapterCtx_t *ctx)
{
  IMmiLegacyStatusPort_t port;

  port.ctx = ctx;
  port.ReadMeasurement = ReadMeasurement;
  port.ReadTime = ReadTimeValue;
  port.ReadWorkmode = ReadWorkmode;
  port.ReadModuleStatus = ReadModuleStatus;
  port.GetSetTotal = GetSetTotal;
  port.ReadErrorRecord = ReadErrorRecord;
  port.ReadSignalsBlock = ReadSignalsBlock;
  port.ReadInputs = ReadInputs;
  port.ReadGprsLog = ReadGprsLog;
  port.GetGprsImei = GetGprsImei;
  port.GetUsrMac = GetUsrMac;
  port.GetEthernetMac = GetEthernetMac;
  port.GetGsmOperator = GetGsmOperator;

  return port;
}
