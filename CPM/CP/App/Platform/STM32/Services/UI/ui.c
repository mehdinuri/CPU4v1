/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  include files */
#include "ui.h"
#include "HardwarePorts.h"

#include "MCSAsynch.h"
#include "MSM.h"
#include "cmsis_os.h"
#include "cpmpcomm.h"
#include "data.h"
#include "gps.h"
#include "IAP.h"
#include "lcd.h"
#include "main.h"
#include "program.h"
#include "signalCardDrv.h"
#include "stm32h7xx_hal_def.h"
#include "time.h"
#include "usb.h"
#include "DomainServices.h"

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Definitions */
#define UI_DEFAULT_BAUDRATE 38400
#define UI_DMA_TX_TIMEOUT 1000

/* System Start Time */
#define SYSTEM_START_TIME_RESET 0
#define SYSTEM_START_TIME_SET 1

typedef struct _tSRawSCP
{
  uint8_t bStatus;

  uint32_t sCurPacketIndex;
  uint16_t sDataIndex;
} tSRawSCP, *tpSRawSCP;

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  members */

/*  os members */
static ISerialPort_t *s_port;

char strUITx[UI_COMM_MAX_PACKET_LENGTH + 1];

static uint16_t sStrUITxLen = 0;

static char strTempData[2 * UI_COMM_MAX_PACKET_LENGTH + 1];

static uint8_t bUICommResponse; /* will respond according to error state */
static uint8_t fUICommResponse;

static char *pchUIRxPacketParserPtr; /* used by the packet parsing mechanism */
static uint8_t bUIRxParserCurrentField; /* indicates the current packet field */

static uint32_t lRequestedPacket;

static uint8_t fUIChecksumEnabled;

static uint8_t bRequestParameter1;
static uint8_t bRequestParameter2;
static uint8_t bRequestParameter3;
static uint16_t sRequestParameter3; /* only used for recieving task read index */
/* whose max value is bigger than uint8_t */
/* size */
static char strRequestParameter[4]; /* only used while recieving language */

static uint16_t sUILogReadIndex;
static uint8_t fSimulationRunning = FALSE;

__attribute__((section(".ram_d3_bss"), aligned(32)))
static uint8_t baRawSCPData[UI_COMM_BACKUP_SCP_MAX_SIZE];

__attribute__((section(".ram_d3_bss"), aligned(32)))
static tSRawSCP SRawSCP;

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  methods */
static void UIOnRx(void *arg, const uint8_t *data, uint16_t len)
{
  (void) arg;
  UIRxRequest(UI_REQ_TYPE_SERIAL, (char *) data, (uint16_t) len);
}

void UIInit(ISerialPort_t *port)
{
  memset(&SRawSCP, 0, sizeof(tSRawSCP));

  s_port = port;

  GpsPortRead();
  GpsBaudRateIndexRead();

  if (GpsIsPortInternal())
  {
    /* USART2 is ours: set baud rate and register RX callback. */
    (void) SerialSetBaudRate(s_port, UI_DEFAULT_BAUDRATE);
    SerialSetRxCallback(s_port, UIOnRx, NULL);
  }

  /* When GPS is external it owns USART2; UI serial path is disabled. */

  /* init private data */
  memset(strUITx, 0, sizeof(strUITx));

  fUIChecksumEnabled = FALSE;
  sUILogReadIndex = 0;
  fUICommResponse = FALSE;
}

void UIMCSAsySucMsgSend(void)
{
  memset(strUITx, 0, sizeof(strUITx));
  strUITx[0] = UI_COMM_START_OF_PACKET;
  strUITx[1] = '\0';
  strcat(strUITx, UI_COMM_PACKET_SUCCESS_STR);
  strcat(strUITx, UI_COMM_END_OF_PACKET_STR);

  MCSAsynchReqTxMsg(MCS_ASYNCH_HEADER_UPLOAD, strlen(strUITx),
                    (void *) strUITx);
}

void UISetMCSDownloadInProgress(uint8_t bState)
{
  SRuntimes.SUIRuntime.bMCSDownloadInProg = bState;
}

void UISetMCSUploadInProgress(uint8_t bState)
{
  SRuntimes.SUIRuntime.bMCSUploadInProg = bState;
}

uint8_t UIMCSDownloadInProgressGet(void)
{
  return SRuntimes.SUIRuntime.bMCSDownloadInProg;
}

uint8_t UIMCSUploadInProgressGet(void)
{
  return SRuntimes.SUIRuntime.bMCSUploadInProg;
}

void UICheckDownloadTimeoutSet(uint8_t bState)
{
  SRuntimes.SUIRuntime.bCheckDownloadTimeout = bState;
}

uint8_t UICheckDownloadTimeoutGet(void)
{
  return SRuntimes.SUIRuntime.bCheckDownloadTimeout;
}

void UICheckUploadTimeoutSet(uint8_t bState)
{
  SRuntimes.SUIRuntime.bCheckUploadTimeout = bState;
}

uint8_t UICheckUploadTimeoutGet(void)
{
  return SRuntimes.SUIRuntime.bCheckUploadTimeout;
}

void UIProgramLoadingErrSet(void)
{
  SetProgramLoadingStatus(PROGRAM_LOADING_ERROR);
  LogRequest(LOG_REQ_APPEND, NULL, EVENT_MCT_CONFIGURATION_ERROR, 1, 0, 0, 0);
  SRuntimes.SaSignalStateRuntimes[SignalStateRuntimeCurNoGet()].bExecutionMode =
    SIGNAL_STATE_EXEC_MODE_PROGRAM_ERROR;
  DataInit(UI_REQ_TYPE_NONE, TRUE);
}

void UIProgramLoadingSucSet(void)
{
  fUICommResponse = TRUE;
  SetProgramLoadingStatus(PROGRAM_LOADING_SUCCESS);
}

uint8_t UIProgramLoadEndedSet(void)
{
  return SetProgramLoadingFlag(FALSE);
}

void UIRuntimeTimeoutsChecks(void)
{
  if (UICheckDownloadTimeoutGet())
  {
    SRuntimes.SUIRuntime.sDownloadDuration++;
    if (SRuntimes.SUIRuntime.sDownloadDuration >= UI_SAFETY_TIMEOUT)
    {
      SRuntimes.SUIRuntime.sDownloadDuration = 0;
      UICheckDownloadTimeoutSet(FALSE);

      if (SRawSCP.bStatus == UI_COMM_BACKUP_SCP_STATE_SAVING)
      {
        memset(&SRawSCP, 0, sizeof(SRawSCP));
        memset(baRawSCPData, 0, sizeof(baRawSCPData));
      }
      else
      {
        UIProgramLoadingErrSet();
        UIProgramLoadEndedSet();

        ProgramDataGet(); /* use previous program */
        ProgramRelativeDataInit();
        ResetCPMPComm(); /* reset mp connection */

        LoadProgramEnds(); /* start from the beginning */
      }
    }
  }

  if (UICheckUploadTimeoutGet())
  {
    SRuntimes.SUIRuntime.sUploadDuration++;
    if (SRuntimes.SUIRuntime.sUploadDuration >= UI_SAFETY_TIMEOUT)
    {
      ProgramStateSet(PROGRAM_STATE_DARK);
      SetProgramLoadingStatus(PROGRAM_UPLOADING_ERROR);

      SRuntimes.SUIRuntime.sUploadDuration = 0;
      UICheckUploadTimeoutSet(FALSE);
    }
  }
} /* UIRuntimeTimeoutsChecks */

void UIRemoteRequest(char *pstrBuf, uint16_t bLength)
{
  UIRxRequest(UI_REQ_TYPE_TCP_CLIENT, pstrBuf, bLength);
}

uint8_t UIStrToLong(char *pstr, int32_t *plBuffer)
{
  uint8_t bCharIndex = 0;
  uint32_t lDigitQuoff = 1;

  while ((pstr[bCharIndex] != UI_COMM_DATA_SEPARATOR)
         && (pstr[bCharIndex] != UI_COMM_CHECKSUM_SEPARATOR)
         && (pstr[bCharIndex] != UI_COMM_END_OF_PACKET))
  {
    if (pstr[bCharIndex] != '\0')
    {
      bCharIndex++;
    }
    else
    {
      /* if packet is truncated before expected remaining data is not received */
      /* for example: in '-stm:3,45,12,12', this error will occur while reading */
      /* '12' because there is no data/checksum seperator or end of packet */
      bUICommResponse = UI_COMM_RESPONSE_FRAME_ERROR;

      return FALSE;
    }
  }

  if (bCharIndex)
  {
    bCharIndex--; /* the index of last digit */
    *plBuffer = 0;
    while (bCharIndex)
    {
      *plBuffer += (pstr[bCharIndex] - '0') * lDigitQuoff;
      bCharIndex--;
      lDigitQuoff *= 10;
    }

    if (pstr[0] == '-')
    {
      *plBuffer *= -1;
    }
    else if (pstr[0] != '+')
    {
      *plBuffer += (pstr[0] - '0') * lDigitQuoff;
    }

    return TRUE;
  }

  /* there is no data. for example, in '-stm:3,,12,12', when parameter 'pstr' is */
  /* ',12,12', this error will occur */
  bUICommResponse = UI_COMM_RESPONSE_FRAME_ERROR;

  return FALSE;
} /* UIStrToLong */

char *UIConvertSGTypeToSGTypeStr(uint8_t bSGType)
{
  if (bSGType == SIGNAL_GROUP_TYPE_VEHICLE_MAINWAY)
  {
    return UI_COMM_STR_MAINWAY;
  }
  else if (bSGType == SIGNAL_GROUP_TYPE_VEHICLE_SUBWAY)
  {
    return UI_COMM_STR_SUBWAY;
  }
  else if (bSGType == SIGNAL_GROUP_TYPE_FLASHER)
  {
    return UI_COMM_STR_FLASHER;
  }
  else if (bSGType == SIGNAL_GROUP_TYPE_PEDESTRIAN)
  {
    return UI_COMM_STR_PEDESTRIAN;
  }
  else if (bSGType == SIGNAL_GROUP_TYPE_TRAM)
  {
    return UI_COMM_STR_TRAM;
  }
  else if (bSGType == SIGNAL_GROUP_TYPE_BICYCLE)
  {
    return UI_COMM_STR_BICYCLE;
  }
  else
  {
    return UI_COMM_STR_NONE;
  }
}

char *UIConvertEMToEMStr(uint8_t bEM)
{
  if (bEM == EMERGENCY_METHOD_DARK)
  {
    return UI_COMM_STR_DARK;
  }
  else if (bEM == EMERGENCY_METHOD_FLASH)
  {
    return UI_COMM_STR_FLASH;
  }
  else
  {
    return UI_COMM_STR_NONE;
  }
}

char *UIConvertSOTypeToSOTypeStr(uint8_t bSOType)
{
  if (bSOType == SIGNAL_OUTPUT_TYPE_RED)
  {
    return UI_COMM_STR_RED;
  }

  if (bSOType == SIGNAL_OUTPUT_TYPE_YELLOW)
  {
    return UI_COMM_STR_YELLOW;
  }

  if (bSOType == SIGNAL_OUTPUT_TYPE_GREEN)
  {
    return UI_COMM_STR_GREEN;
  }
  else
  {
    return UI_COMM_STR_UNDEFINED;
  }
}

uint8_t UIConvertFlagStrToFlag(char *pstrTrueFalse)
{
  if (strcmp(pstrTrueFalse, UI_COMM_STR_TRUE) == 0)
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

char *UIConvertDeviceTypeToDeviceTypeStr(uint8_t bDeviceType)
{
  if (bDeviceType == DEVICE_TYPE_MAESTRO)
  {
    return UI_COMM_STR_DEVICE_TYPE_MAESTRO;
  }
  else
  {
    return UI_COMM_STR_NONE;
  }
}

uint8_t UIConvertLanguageStrToLanguage(char *strLanguage)
{
  if (strcmp(strLanguage, UI_COMM_STR_LANGUAGE_TURKISH) == 0)
  {
    return LANGUAGE_TURKISH;
  }
  else if (strcmp(strLanguage, UI_COMM_STR_LANGUAGE_ENGLISH) == 0)
  {
    return LANGUAGE_ENGLISH;
  }
  else
  {
    return LANGUAGE_NONE;
  }
}

char *UIConvertLanguageToLanguageStr(uint8_t bLanguage)
{
  if (bLanguage == LANGUAGE_TURKISH)
  {
    return UI_COMM_STR_LANGUAGE_TURKISH;
  }
  else if (bLanguage == LANGUAGE_ENGLISH)
  {
    return UI_COMM_STR_LANGUAGE_ENGLISH;
  }
  else
  {
    return UI_COMM_STR_NONE;
  }
}

void UIRxPacketParserInit(char *pStr)
{
  pchUIRxPacketParserPtr = &pStr[UI_COMM_PACKET_NAME_INDEX];
  bUIRxParserCurrentField = UI_COMM_PACKET_FIELD_NAME;
}

/* GetPacketName: fills a uint32_t with an integer value correspondent with the */
/* packet name string returns TRUE if successful, FALSE if there is a framing */
/* error (packet structure error) within the packet name field also updates */
/* error state if returning FALSE prerequisites: pchUIRxPacketParserPtr must */
/* point to the first character of the name string effects: at the end */
/* pchUIRxPacketParserPtr points to the next field (data, checksum, '\0') in the */
/* packet */
uint8_t UIRxPacketNameGet(uint32_t *plName)
{
  uint8_t bCharNo = 0;

  *plName = 0;
  while ((*pchUIRxPacketParserPtr != UI_COMM_DATA_FIELD_SEPARATOR)
         && (*pchUIRxPacketParserPtr
             !=
             UI_COMM_DATA_SEPARATOR)
         && (*pchUIRxPacketParserPtr != UI_COMM_CHECKSUM_SEPARATOR)
         && (*pchUIRxPacketParserPtr != UI_COMM_END_OF_PACKET))
  {
    if (*pchUIRxPacketParserPtr != '\0')
    {
      if (bCharNo < sizeof(uint32_t))
      {
        *plName |= *pchUIRxPacketParserPtr << (bCharNo * 8);
        bCharNo++;
      }

      pchUIRxPacketParserPtr++;
    }
    else
    {
      bUIRxParserCurrentField = UI_COMM_PACKET_FIELD_NONE;
      bUICommResponse = UI_COMM_RESPONSE_FRAME_ERROR;

      return FALSE;
    }
  }

  if (*pchUIRxPacketParserPtr == UI_COMM_DATA_FIELD_SEPARATOR)
  {
    bUIRxParserCurrentField = UI_COMM_PACKET_FIELD_DATA;
  }
  else if (*pchUIRxPacketParserPtr == UI_COMM_CHECKSUM_SEPARATOR)
  {
    bUIRxParserCurrentField = UI_COMM_PACKET_FIELD_CHECKSUM;
  }

  if (*pchUIRxPacketParserPtr == UI_COMM_END_OF_PACKET)
  {
    bUIRxParserCurrentField = UI_COMM_PACKET_FIELD_EOP;
  }

  pchUIRxPacketParserPtr++; /* point to first char of the next field */

  return TRUE;
} /* UIRxPacketNameGet */

/* UIRxNextDataGet: fills the data buffer with the data pointed by */
/* pchUIRxPacketParserPtr. in the end, pchUIRxPacketParserPtr points to the next */
/* data or next field if this is the last data updates the error state and */
/* returns FALSE if there is a framing error */
uint8_t UIRxNextDataGet(uint8_t bType,
                        void *pvIntBuffer,
                        char *pstrBuffer,
                        uint8_t bBufferLength)
{
  switch (bType)
  {
      case UI_COMM_DATA_TYPE_STRING:
      {
        uint8_t bCharIndex = 0;

        while ((*pchUIRxPacketParserPtr != UI_COMM_DATA_SEPARATOR)
               && (*pchUIRxPacketParserPtr
                   !=
                   UI_COMM_CHECKSUM_SEPARATOR)
               && (*pchUIRxPacketParserPtr != UI_COMM_END_OF_PACKET))
        {
          if (bCharIndex < bBufferLength)
          {
            pstrBuffer[bCharIndex] = (*pchUIRxPacketParserPtr);
            bCharIndex++;
            pchUIRxPacketParserPtr++;
          }
          else
          {
            /* our parameter buffer which is allocated considering expected */
            /* maximum data size is full but there is still data inform pc user */
            /* tool of this by preparing a buffer size error packet */
            bUICommResponse = UI_COMM_RESPONSE_BUF_SIZE_ERROR;

            return FALSE;
          }
        }

        pstrBuffer[bCharIndex] = '\0';
        break;
      }

      case UI_COMM_DATA_TYPE_INTEGER:
      {
        int32_t lBuffer;

        if (UIStrToLong(pchUIRxPacketParserPtr, &lBuffer))
        {
          if (bBufferLength == sizeof(int8_t))
          {
            (*(int8_t *) pvIntBuffer) = (int8_t) lBuffer;
          }
          else if (bBufferLength == sizeof(int16_t))
          {
            (*(int16_t *) pvIntBuffer) = (int16_t) lBuffer;
          }
          else
          {
            (*(int32_t *) pvIntBuffer) = lBuffer;
          }

          /* update pchUIRxPacketParserPtr */
          while ((*pchUIRxPacketParserPtr != UI_COMM_DATA_SEPARATOR)
                 && (*pchUIRxPacketParserPtr
                     !=
                     UI_COMM_CHECKSUM_SEPARATOR)
                 && (*pchUIRxPacketParserPtr != UI_COMM_END_OF_PACKET))
          {
            pchUIRxPacketParserPtr++;
          }
        }
        else
        {
          bUICommResponse = UI_COMM_RESPONSE_FRAME_ERROR;

          return FALSE;
        }

        break;
      }

      default: /* type is wrong */
      {
        bUICommResponse = UI_COMM_RESPONSE_FRAME_ERROR;

        return FALSE;
      }
  } /* switch */

  if (*pchUIRxPacketParserPtr == UI_COMM_DATA_FIELD_SEPARATOR)
  {
    bUIRxParserCurrentField = UI_COMM_PACKET_FIELD_DATA;
  }
  else if (*pchUIRxPacketParserPtr == UI_COMM_CHECKSUM_SEPARATOR)
  {
    bUIRxParserCurrentField = UI_COMM_PACKET_FIELD_CHECKSUM;
  }

  if (*pchUIRxPacketParserPtr == UI_COMM_END_OF_PACKET)
  {
    bUIRxParserCurrentField = UI_COMM_PACKET_FIELD_EOP;
  }

  pchUIRxPacketParserPtr++; /* point to first char of the next field */

  return TRUE;
} /* UIRxNextDataGet */

void UILogReadIndexSet(uint16_t sIdx)
{
  sUILogReadIndex = sIdx;
}

void UIFailureResponseSet(void)
{
  strUITx[0] = UI_COMM_START_OF_PACKET;
  strUITx[1] = '\0';
  strcat(strUITx, UI_COMM_PACKET_OPERATION_ERROR_STR);
  bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
}

void UILogRecordResponseSet(uint16_t sIndexOfLog, tpSLogRecord pSLogRecord)
{
  char strBuffer[32];

  sprintf(strBuffer, "%u", (sIndexOfLog + 1));
  strcat(strUITx, strBuffer);
  strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

  sprintf(strBuffer, "%u", pSLogRecord->bMonthDay);
  strcat(strUITx, strBuffer);
  strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

  sprintf(strBuffer, "%u", pSLogRecord->bMonth);
  strcat(strUITx, strBuffer);
  strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

  sprintf(strBuffer, "%u", (pSLogRecord->sYear - (TimeCenturyGet() * 100)));
  strcat(strUITx, strBuffer);
  strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

  sprintf(strBuffer, "%u", pSLogRecord->bHours);
  strcat(strUITx, strBuffer);
  strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

  sprintf(strBuffer, "%u", pSLogRecord->bMinutes);
  strcat(strUITx, strBuffer);
  strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

  sprintf(strBuffer, "%u", pSLogRecord->bSeconds);
  strcat(strUITx, strBuffer);
  strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

  sprintf(strBuffer, "%u", pSLogRecord->SEvent.bEvent);
  strcat(strUITx, strBuffer);
  strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

  sprintf(strBuffer, "%u", pSLogRecord->SEvent.bParam);
  strcat(strUITx, strBuffer);
  strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

  sprintf(strBuffer, "%hd", pSLogRecord->SEvent.sParam);
  strcat(strUITx, strBuffer);
  strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

  sprintf(strBuffer, "%ld", (long) pSLogRecord->SEvent.lParam);
  strcat(strUITx, strBuffer);
} /* UILogRecordResponseSet */

uint8_t UIProgramLoadTry(void)
{
  SetProgramLoadingStatus(PROGRAM_LOADING_TO_MAIN);

  if (ProgramDataSet())
  {
    if (ProgramDataGet())
    {
      ProgramRelativeDataInit();

      return TRUE;
    }
  }

  return FALSE;
}

void UIConfEnd(void)
{
  if (UIProgramLoadEndedSet())
  {
    if (UIProgramLoadTry())
    {
      UIProgramLoadingSucSet();
    }
    else
    {
      UIProgramLoadingErrSet();
      bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
    }
  }
  else
  {
    UIProgramLoadingErrSet();
    bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
  }
}

void UIConfStart(uint8_t bUIReqID)
{
  SetProgramLoadingStatus(PROGRAM_LOADING_IN_PROGRESS);
  DataInit(bUIReqID, TRUE); /* default mode while program loading */
}

void UIPacketIOM(void)
{
  tSCanCpuIOOutputs SCanCpuIOOutputs;
  char strOutputs[16];

  memset(&SCanCpuIOOutputs, 0, sizeof(SCanCpuIOOutputs));

  if (UIRxNextDataGet(UI_COMM_DATA_TYPE_STRING, NULL, strOutputs,
                      sizeof(strOutputs)))
  {
    uint8_t bIndex;

    for (bIndex = 0; bIndex < IO_OUTPUTS_MAX; bIndex++)
    {
      if (strOutputs[bIndex] == '1')
      {
        SCanCpuIOOutputs.sOutputStates |= laValue2Bit[bIndex];
      }
    }

    if (UIRxNextDataGet(UI_COMM_DATA_TYPE_STRING, NULL, strOutputs,
                        sizeof(strOutputs)))
    {
      for (bIndex = 0; bIndex < IO_OUTPUTS_MAX; bIndex++)
      {
        if (strOutputs[bIndex] == '1')
        {
          SCanCpuIOOutputs.sOutputStates |= laValue2Bit[bIndex
                                                        + IO_OUTPUTS_MAX];
        }
      }

      SetIOOutputs(&SCanCpuIOOutputs); /* set io outputs */
    }
  }

  if (bUICommResponse == UI_COMM_RESPONSE_SUCCESS)
  {
    /* also this is a request packet */
    bUICommResponse = UI_COMM_RESPONSE_PACKET;
    lRequestedPacket = UI_COMM_PACKET_IOM;
  }
} /* UIPacketIOM */

void UIPacketSSM(void)
{
  char strSOVoltagesWithEndOfLines[2 * SIGNAL_OUTPUTS_MAX / 8];
  char strSOVoltages[SIGNAL_OUTPUTS_MAX / 8];

  memset(strSOVoltages, 0, sizeof(strSOVoltages));
  memset(strSOVoltagesWithEndOfLines, 0, sizeof(strSOVoltagesWithEndOfLines));

  /* first 8 bytes are signal output voltages, the next 8 bytes show if */
  /* corresponding byte is '\r' (end of packet char) */
  StartSSMTest(SSM_TEST_FROM_UI_RCV_SIGNALS);
  if (UIRxNextDataGet(UI_COMM_DATA_TYPE_STRING,
                      NULL,
                      strSOVoltagesWithEndOfLines,
                      sizeof(strSOVoltagesWithEndOfLines)))
  {
    uint8_t bIndex;
    uint8_t bSONo;
    uint8_t bSignal = 0;

    for (bIndex = 0; bIndex < SIGNAL_OUTPUTS_MAX / 8; bIndex++)
    {
      if (strSOVoltagesWithEndOfLines[bIndex + SIGNAL_OUTPUTS_MAX / 8])
      {
        strSOVoltages[bIndex] = '\r';
      }
      else
      {
        strSOVoltages[bIndex] = strSOVoltagesWithEndOfLines[bIndex];
      }
    }

    SeizeSGData();
    /* assign signal group signals(signal output voltages) */
    for (bSONo = 0; bSONo < SIGNAL_OUTPUTS_MAX; bSONo++)
    {
      if (bSONo % SIGNAL_OUTPUTS_PER_CURRENT_GROUP == 0)
      {
        bSignal = 0;
      }

      bSignal |= ((strSOVoltages[bSONo / 8] >> (bSONo % 8)) & 1) <<
                 (bSONo % SIGNAL_OUTPUTS_PER_CURRENT_GROUP);

      if (bSONo % SIGNAL_OUTPUTS_PER_CURRENT_GROUP == 2)
      {
        SGSignalSet((bSONo / SIGNAL_OUTPUTS_PER_CURRENT_GROUP),
                    bSignal + 1);
      }
    }

    ReleaseSGData();
  }

  if (bUICommResponse == UI_COMM_RESPONSE_SUCCESS)
  {
    /* also this is a request packet */
    bUICommResponse = UI_COMM_RESPONSE_PACKET;
    lRequestedPacket = UI_COMM_PACKET_SSM;
  }
} /* UIPacketSSM */

void UIPacketSignal(void)
{
  uint8_t bSignal;
  uint8_t bFlags;
  tSSignalDef SSignalDef;

  memset(&SSignalDef,
         0,
         sizeof(SSignalDef));

  if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, &bSignal, NULL,
                      sizeof(bSignal)))
  {
    if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                        (uint8_t *) (&SSignalDef.SFlags), NULL, sizeof(bFlags)))
    {
      if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          (void *) &(SSignalDef.SaSignal[0].sPeriod),
                          NULL,
                          sizeof(SSignalDef.SaSignal[0].sPeriod)))
      {
        if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                            (void *) &(SSignalDef.SaSignal[1].sPeriod),
                            NULL,
                            sizeof(SSignalDef.SaSignal[1].sPeriod)))
        {
          if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                              (void *) &(SSignalDef.SaSignal[2].sPeriod),
                              NULL,
                              sizeof(SSignalDef.SaSignal[2].sPeriod)))
          {
            if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                                (void *) &(SSignalDef.sFollowers),
                                NULL,
                                sizeof(SSignalDef.sFollowers)))
            {
              if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                                  &(SSignalDef.bMinDur), NULL,
                                  sizeof(SSignalDef.bMinDur)))
              {
                if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                                    &(SSignalDef.bMaxDuration), NULL,
                                    sizeof(SSignalDef.bMaxDuration)))
                {
                  if (SetSignalDefs(bSignal - 1, &SSignalDef)
                      == FALSE)        /* sets the signal defs. */
                  {
                    bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
                  }
                }
              }
            }
          }
        }
      }
    }
  }
} /* UIPacketSignal */

void UIPacketSignalsDefined(void)
{
  uint8_t bSignal = 0;
  tSSignalsDefined SSignalsDefined;

  memset(&SSignalsDefined,
         0,
         sizeof(tSSignalsDefined));

  if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, &bSignal, NULL,
                      sizeof(bSignal)))
  {
    SSignalsDefined.bBlocking = bSignal;
    if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                        &bSignal,
                        NULL,
                        sizeof(bSignal)))
    {
      SSignalsDefined.bFree = bSignal;
      if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, &bSignal, NULL,
                          sizeof(bSignal)))
      {
        SSignalsDefined.bGreenFlash = bSignal;
        if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                            &bSignal,
                            NULL,
                            sizeof(bSignal)))
        {
          SSignalsDefined.bDark = bSignal;

          if (SignalValidGet(SSignalsDefined.bBlocking)
              && SignalValidGet(SSignalsDefined.bFree)
              && SignalValidGet(SSignalsDefined.bGreenFlash)
              && SignalValidGet(SSignalsDefined.bDark))
          {
            SetSignalsDefined(&SSignalsDefined);
          }
          else
          {
            bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
          }
        }
      }
    }
  }
} /* UIPacketSignalsDefined */

void UIPacketCVS(void)
{
  uint8_t bLampType = 0;
  int16_t sValue = 0;
  tSCVSDef SCVSDef;

  memset(&SCVSDef, 0, sizeof(tSCVSDef));

  if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, &bLampType, NULL,
                      sizeof(bLampType)))
  {
    if (bLampType && (bLampType <= LAMP_TYPE_MAX))
    {
      if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          (void *) &(SCVSDef.saCurrents[bLampType - 1]), NULL,
                          sizeof(SCVSDef.saCurrents[bLampType])))
      {
        if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                            (void *) &(SCVSDef.saVoltages[bLampType - 1]), NULL,
                            sizeof(SCVSDef.saVoltages[bLampType])))
        {
          if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, &sValue, NULL,
                              sizeof(sValue)))
          {
            SCVSDef.raSlopes[bLampType - 1] = ((double) sValue)
                                              / ((double) 1000);
          }
        }
      }
    }
    else
    {
      bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
    }
  }

  if (bUICommResponse == UI_COMM_RESPONSE_SUCCESS)
  {
    if (SetCVS(&SCVSDef) == FALSE)
    {
      bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
    }
  }
} /* UIPacketCVS */

void UIPacketDeviceInfo(uint8_t bCurrentId)
{
  tSDeviceInfo SDevInfo;

  memset(&SDevInfo, 0, sizeof(tSDeviceInfo));

  if (UIRxNextDataGet(UI_COMM_DATA_TYPE_STRING,
                      NULL,
                      SDevInfo.strCountry,
                      sizeof(SDevInfo.strCountry)))
  {
    if (UIRxNextDataGet(UI_COMM_DATA_TYPE_STRING,
                        NULL,
                        SDevInfo.strCity,
                        sizeof(SDevInfo.strCity)))
    {
      if (UIRxNextDataGet(UI_COMM_DATA_TYPE_STRING,
                          NULL,
                          SDevInfo.strIntersection,
                          sizeof(SDevInfo.strIntersection)))
      {
        if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                            &SDevInfo.TimeZone,
                            NULL,
                            sizeof(SDevInfo.TimeZone)))
        {
          if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                              &(SDevInfo.bDeviceType),
                              NULL,
                              sizeof(SDevInfo.bDeviceType)))
          {
            if (UIRxNextDataGet(UI_COMM_DATA_TYPE_STRING,
                                NULL,
                                SDevInfo.strDomain,
                                sizeof(SDevInfo.strDomain)))
            {
              if (UIRxNextDataGet(UI_COMM_DATA_TYPE_STRING,
                                  NULL,
                                  SDevInfo.strAPN,
                                  sizeof(SDevInfo.strAPN)))
              {
                if (UIRxNextDataGet(UI_COMM_DATA_TYPE_STRING,
                                    NULL,
                                    SDevInfo.strUsername,
                                    sizeof(SDevInfo.strUsername)))
                {
                  if (UIRxNextDataGet(UI_COMM_DATA_TYPE_STRING,
                                      NULL,
                                      SDevInfo.strPassword,
                                      sizeof(SDevInfo.strPassword)))
                  {
                    if (SetDeviceInfo(&SDevInfo,
                                      bCurrentId) == FALSE)
                    {
                      bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
} /* UIPacketDeviceInfo */

void UIPacketSG(void)
{
  uint8_t bSGNo;
  tSSGDef SSGData;

  memset(&SSGData, 0, sizeof(SSGData));

  if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, (void *) &bSGNo, NULL,
                      sizeof(bSGNo)))
  {
    if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                        (void *) &(SSGData.bType),
                        NULL,
                        sizeof(SSGData.bType)))
    {
      if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          (void *) &(SSGData.bOpeningSignal),
                          NULL,
                          sizeof(SSGData.bOpeningSignal)))
      {
        if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                            (void *) &(SSGData.bOpeningDuration), NULL,
                            sizeof(SSGData.bOpeningDuration)))
        {
          if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                              (void *) &(SSGData.bClosingSignal), NULL,
                              sizeof(SSGData.bClosingSignal)))
          {
            if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                                (void *) &(SSGData.bClosingDur), NULL,
                                sizeof(SSGData.bClosingDur)))
            {
              if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                                  (void *) &(SSGData.bFlashSignal), NULL,
                                  sizeof(SSGData.bFlashSignal)))
              {
                if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                                    (void *) &(SSGData.bFailureFlashSignal),
                                    NULL,
                                    sizeof(SSGData.bFailureFlashSignal)))
                {
                  if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                                      (void *) &(SSGData.bGreenFlashDur),
                                      NULL,
                                      sizeof(SSGData.bGreenFlashDur)))
                  {
                    if (UIRxNextDataGet(
                          UI_COMM_DATA_TYPE_INTEGER,
                          (void *) &(SSGData.bRedLampFailureNumber), NULL,
                          sizeof(SSGData.bRedLampFailureNumber)))
                    {
                      uint8_t bEM = 0;

                      if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, &(bEM),
                                          NULL,
                                          sizeof(bEM)))
                      {
                        SSGData.SEmergencyMethods.bRedLampFailureNumberEM = bEM;
                        if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, &(bEM),
                                            NULL,
                                            sizeof(bEM)))
                        {
                          SSGData.SEmergencyMethods.bLastRedLampFailureEM = bEM;
                          if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                                              &(SSGData.bOwner),
                                              NULL, sizeof(SSGData.bOwner)))
                          {
                            if ((bSGNo) && (SSGData.bOwner)
                                && (SSGData.bOwner <= SIGNAL_SETS_MAX))
                            {
                              SSGData.bOwner--;
                              if (SetSignalGroups(bSGNo - 1, &SSGData) == FALSE)
                              {
                                bUICommResponse =
                                  UI_COMM_RESPONSE_OPERATION_ERROR;
                              }
                            }
                            else
                            {
                              bUICommResponse =
                                UI_COMM_RESPONSE_OPERATION_ERROR;
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
} /* UIPacketSG */

void UIPacketConflictEM(void)
{
  uint8_t bConflict, bConflictEM;
  tSConflictsEM SConflictsEM;

  memset(&SConflictsEM, 0, sizeof(tSConflictsEM));

  GetConflictsEM(&SConflictsEM);

  if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, &(bConflict), NULL,
                      sizeof(bConflict)))
  {
    if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, &(bConflictEM), NULL,
                        sizeof(bConflictEM)))
    {
      if (bConflictEM <= EMERGENCY_METHODS_MAX)
      {
        switch (bConflict)
        {
            case CONFLICT_GREEN_GREEN:
            {
              SConflictsEM.bGreenGreenEM = bConflictEM;
              break;
            }

            case CONFLICT_YELLOW_GREEN:
            {
              SConflictsEM.bYellowGreenEM = bConflictEM;
              break;
            }

            case CONFLICT_YELLOW_YELLOW:
            {
              SConflictsEM.bYellowYellowEM = bConflictEM;
              break;
            }

            case CONFLICT_MALFUNCTION:
            {
              SConflictsEM.bMalfunctionEM = bConflictEM;
              break;
            }

            case CONFLICT_VOLTAGE_LIMIT:
            {
              SConflictsEM.bVoltageLimitsEM = bConflictEM;
              break;
            }

            case CONFLICT_FREQUENCY_ERROR:
            {
              SConflictsEM.bFrequencyErrorEM = bConflictEM;
              break;
            }

            case CONFLICT_INVALID_SIGNAL:
            {
              SConflictsEM.bInvalidSignalEM = bConflictEM;
              break;
            }

            case CONFLICT_INVALID_SIGNAL_SEQUENCE:
            {
              SConflictsEM.bInvalidSignalSequenceEM = bConflictEM;
              break;
            }

            default:
            {
              bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
              break;
            }
        } /* switch */
      }
      else
      {
        bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
      }

      if (bUICommResponse == UI_COMM_RESPONSE_SUCCESS)
      {
        if (SetConflictsEM(&SConflictsEM) == FALSE)
        {
          bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
        }
      }
    }
  }
} /* UIPacketConflictEM */

void UIPacketSO(void)
{
  uint8_t bSONo = 0, bSSMNo = 0, bOutputNoAtSSM = 0;
  tSSODef SSOData, SSODefBuffer;

  memset(&SSOData, 0, sizeof(tSSODef));

  if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                      &(SSOData.bOwner),
                      NULL,
                      sizeof(SSOData.bOwner)))
  {
    if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, &bSONo, NULL, sizeof(bSONo)))
    {
      if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, &bSSMNo, NULL,
                          sizeof(bSSMNo)))
      {
        if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                            &bOutputNoAtSSM,
                            NULL,
                            sizeof(bOutputNoAtSSM)))
        {
          if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                              &SSOData.bType,
                              NULL,
                              sizeof(SSOData.bType)))
          {
            if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                                &(SSOData.bNoOfLamps),
                                NULL,
                                sizeof(SSOData.bNoOfLamps)))
            {
              uint8_t bEM = 0;

              if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                                  &(bEM),
                                  NULL,
                                  sizeof(bEM)))
              {
                SSOData.SFlags.bSOFailureEM = bEM;
                if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                                    &(SSOData.bLampType),
                                    NULL,
                                    sizeof(SSOData.bLampType)))
                {
                  if ((bSSMNo) && (bOutputNoAtSSM))
                  {
                    uint8_t bSONo2 = ((bSSMNo - 1) * SIGNAL_OUTPUTS_PER_SSM)
                                     + (bOutputNoAtSSM - 1);

                    GetSODef(bSONo2, &SSODefBuffer);

                    SSOData.sPower[0] = SSODefBuffer.sPower[0];
                    SSOData.sPowerRecordNet[0] =
                      SSODefBuffer.sPowerRecordNet[0];
                    SSOData.sPower[1] = SSODefBuffer.sPower[1];
                    SSOData.sPowerRecordNet[1] =
                      SSODefBuffer.sPowerRecordNet[1];
                    if (SetSignalOutputs(bSSMNo, bSONo2, &SSOData) == FALSE)
                    {
                      bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
                    }
                  }
                  else
                  {
                    bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
                  }
                }
              }
            }
          }
        }
      }
    }
  }
} /* UIPacketSO */

void UIPacketClearance(void)
{
  uint8_t bSGNo, bConflictingSGNo, bClearance;

  if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, &bSGNo, NULL, sizeof(bSGNo)))
  {
    if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                        &bConflictingSGNo,
                        NULL,
                        sizeof(bConflictingSGNo)))
    {
      if (SGIsValid(bSGNo) && SGIsValid(bConflictingSGNo))
      {
        if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                            &bClearance,
                            NULL,
                            sizeof(bClearance)))
        {
          if ((bSGNo) && (bConflictingSGNo))
          {
            if (SGClearanceSet(bSGNo - 1, bConflictingSGNo - 1, bClearance)
                == FALSE)
            {
              bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
            }
          }
          else
          {
            bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
          }
        }
      }
      else
      {
        bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
      }
    }
  }
}

void UIPacketPhase(void)
{
  uint8_t bPhaseNo;
  tSPhaseDef SPhaseDefs;

  memset(&SPhaseDefs, 0, sizeof(tSPhaseDef));

  if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, &bPhaseNo, NULL,
                      sizeof(bPhaseNo)))
  {
    if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                        (void *) &(SPhaseDefs.bMinDur),
                        NULL,
                        sizeof(SPhaseDefs.bMinDur)))
    {
      if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          (void *) &(SPhaseDefs.bMaxDur),
                          NULL,
                          sizeof(SPhaseDefs.bMaxDur)))
      {
        if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                            (void *) &(SPhaseDefs.lGroups), NULL,
                            sizeof(SPhaseDefs.lGroups)))
        {
          if (PhaseSet(bPhaseNo - 1, &SPhaseDefs) == FALSE)
          {
            bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
          }
        }
      }
    }
  }
}

/* H&D Commented */

/*
 *  void    UIPacketSettings(void)
 *  {
 *  tSHeaterLampDim SHeaterSettings;
 *  tSHeaterLampDim SLampDimmingSettings;
 *
 *  if(UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, &(SHeaterSettings.fState), NULL,
 *  sizeof(uint8_t)))
 *  {
 *  if(UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, (void
 *)&(SHeaterSettings.fLogicLevel), NULL, sizeof(uint8_t)))
 *  {
 *  if(UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
 *  &(SLampDimmingSettings.fState), NULL, sizeof(uint8_t)))
 *  {
 *  if(UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, (void
 *)&(SLampDimmingSettings.fLogicLevel), NULL, sizeof(uint8_t)))
 *  {
 *  HeaterInfoSave(&SHeaterSettings);
 *  LampDimmingInfoSave(&SLampDimmingSettings);
 *  return;
 *  }
 *  }
 *  }
 *  }
 *  bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
 *  }
 */

void UIPacketSPPlanEntry(void)
{
  uint8_t bSPPlan; /* signal program plan number */
  uint8_t bEntry; /* index of entry that will be added */
  tSSPPlanEntry SSPPlanEntry; /* this entry will be added to signal program plan */

  memset(&SSPPlanEntry, 0, sizeof(tSSPPlanEntry));

  if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, &(bSPPlan), NULL,
                      sizeof(uint8_t)))
  {
    if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, &(bEntry), NULL,
                        sizeof(uint8_t)))
    {
      if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          &(SSPPlanEntry.bHours),
                          NULL,
                          sizeof(uint8_t)))
      {
        if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                            &(SSPPlanEntry.bMinutes),
                            NULL,
                            sizeof(uint8_t)))
        {
          if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                              &(SSPPlanEntry.bSigProg),
                              NULL,
                              sizeof(uint8_t)))
          {
            if (SigProgIsValid(SSPPlanEntry.bSigProg))
            {
              if (SigProgPlanEntrySet(bSPPlan, bEntry - 1, &SSPPlanEntry)
                  == FALSE)
              {
                bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
              }
            }
            else
            {
              bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
            }
          }
        }
      }
    }
  }
} /* UIPacketSPPlanEntry */

void UIPacketWPEntry(void)
{
  uint8_t bDailyWorkPlanNo;
  uint8_t bEntryNo;
  tSWorkPlanEntryDef SWorkPlanEntry;

  memset(&SWorkPlanEntry, 0, sizeof(tSWorkPlanEntryDef));

  if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                      &bDailyWorkPlanNo,
                      NULL,
                      sizeof(bDailyWorkPlanNo)))
  {
    if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                        &bEntryNo,
                        NULL,
                        sizeof(bEntryNo)))
    {
      if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          &(SWorkPlanEntry.bHours),
                          NULL,
                          sizeof(SWorkPlanEntry.bHours)))
      {
        if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                            &(SWorkPlanEntry.bMinutes),
                            NULL,
                            sizeof(SWorkPlanEntry.bMinutes)))
        {
          uint8_t bIndex;

          for (bIndex = 0; bIndex < PhaseTotalGet(); bIndex++)
          {
            if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                                &(SWorkPlanEntry.baPhaseDur[bIndex]),
                                NULL,
                                sizeof(SWorkPlanEntry.baPhaseDur[bIndex]))
                ==
                FALSE)
            {
              break;
            }
          }

          if (bUICommResponse == UI_COMM_RESPONSE_SUCCESS)
          {
            /* 0th workplan is reserved, it is default workplan (dark mode) */
            /* embedded into device */
            if ((bDailyWorkPlanNo <= 0) || (bEntryNo <= 0)
                || (WorkPlanEntrySet(bDailyWorkPlanNo,
                                     bEntryNo - 1,
                                     &SWorkPlanEntry) == FALSE))
            {
              bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
            }
          }
        }
      }
    }
  }
} /* UIPacketWPEntry */

void UIPacketWSEntry(void)
{
  uint8_t bEntry;
  tSWorkScheduleEntryDef SWorkScheduleEntry;

  memset(&SWorkScheduleEntry, 0, sizeof(SWorkScheduleEntry));
  SWorkScheduleEntry.bStartYear = 00;
  SWorkScheduleEntry.bEndYear = 99;

  if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, (void *) &bEntry, NULL,
                      sizeof(bEntry)))
  {
    if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                        (void *) &(SWorkScheduleEntry.bDays), NULL,
                        sizeof(SWorkScheduleEntry.bDays)))
    {
      if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          (void *) &(SWorkScheduleEntry.bStartDay),
                          NULL,
                          sizeof(SWorkScheduleEntry.bStartDay)))
      {
        if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                            (void *) &(SWorkScheduleEntry.bStartMonth), NULL,
                            sizeof(SWorkScheduleEntry.bStartMonth)))
        {
          if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                              (void *) &(SWorkScheduleEntry.bEndDay), NULL,
                              sizeof(SWorkScheduleEntry.bEndDay)))
          {
            if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                                (void *) &(SWorkScheduleEntry.bEndMonth), NULL,
                                sizeof(SWorkScheduleEntry.bEndMonth)))
            {
              if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                                  (void *) &(SWorkScheduleEntry.bWorkPlanNo),
                                  NULL,
                                  sizeof(SWorkScheduleEntry.bWorkPlanNo)))
              {
                if (WorkPlanIsValid(SWorkScheduleEntry.bWorkPlanNo))
                {
                  if (UIRxNextDataGet(
                        UI_COMM_DATA_TYPE_INTEGER,
                        (void *) &(SWorkScheduleEntry.bSigProgPlan), NULL,
                        sizeof(SWorkScheduleEntry.bSigProgPlan)))
                  {
                    if (SigProgPlanIsValid(SWorkScheduleEntry.bSigProgPlan))
                    {
                      if ((bEntry <= 0) || (WorkScheduleEntrySet(bEntry - 1,
                                                                 &
                                                                 SWorkScheduleEntry)
                                            == FALSE))
                      {
                        bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
                      }
                    }
                    else
                    {
                      bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
                    }
                  }
                }
                else
                {
                  bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
                }
              }
            }
          }
        }
      }
    }
  }
} /* UIPacketWSEntry */

void UIPacketFlashPeriods(void)
{
  uint16_t sEmergencyFlashPeriod;

  if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                      &sEmergencyFlashPeriod,
                      NULL,
                      sizeof(sEmergencyFlashPeriod)))
  {
    sEmergencyFlashPeriod /= FLASH_PERIOD_CONSTANT;
    /* flash period is in terms of FLASH_PERIOD_CONSTANT */
    if (FlashPeriodEmergencySet(sEmergencyFlashPeriod) == FALSE)
    {
      bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
    }
  }
}

void UIPacketSeqStep(void)
{
  uint8_t bSignalSeqNo;
  uint8_t bSignalStepNo;

  if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                      (void *) &bSignalSeqNo,
                      NULL,
                      sizeof(bSignalSeqNo)))
  {
    if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                        (void *) &bSignalStepNo,
                        NULL,
                        sizeof(bSignalStepNo)))
    {
      if (bSignalSeqNo && (bSignalSeqNo <= SIGNAL_SEQS_MAX) && bSignalStepNo
          && (bSignalStepNo <= SIGNAL_SEQ_STEPS_MAX))
      {
        uint8_t bStepDur;

        /* convert to 0-indexing (instead, change the following function */
        /* contents as 1-indexing) */
        bSignalSeqNo--;
        bSignalStepNo--;

        if (SeqTotalGet() == bSignalSeqNo)
        {
          SeqInit(bSignalSeqNo);         /* this is a new sequence definition */
        }
        else if (SeqLoad(bSignalSeqNo) == FALSE)
        {
          bUICommResponse =
            UI_COMM_RESPONSE_OPERATION_ERROR;       /* seq. def. is already started */
        }

        /* get current content of it */

        /* read new step content for this sequence */
        if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                            (void *) &bStepDur,
                            NULL,
                            sizeof(bStepDur)))
        {
          uint8_t bSGNo;
          uint8_t bReceivedSignal;

          /* set step duration */
          if (SeqStepDurSet(bSignalSeqNo,
                            bSignalStepNo,
                            bStepDur))
          {
            /* get signal group signals for this step, we already know number of */
            /* signal groups so we expect to receive this number of sg signals, */
            /* if not, this is an error. */
            for (bSGNo = 0; bSGNo < SGTotalGet(); bSGNo++)
            {
              if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                                  (void *) &bReceivedSignal, NULL,
                                  sizeof(bReceivedSignal)))
              {
                if (bReceivedSignal < SIGNALS_MAX)
                {
                  SeqStepSGSignalSet(bSignalSeqNo,
                                     bSignalStepNo,
                                     bSGNo,
                                     bReceivedSignal);
                }
                else
                {
                  bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
                  break;
                }
              }
            }

            if (bUICommResponse == UI_COMM_RESPONSE_SUCCESS)
            {
              if ((bSignalStepNo + 1) > SeqStepNumTotalGet(bSignalSeqNo))
              {
                /* a new step definition for this sequence is successfully */
                /* received, add this step into sequence */
                if ((SeqStepInc(bSignalSeqNo) == FALSE)
                    || (SeqSave(bSignalSeqNo,
                                SEQ_PROC_ADD,
                                FALSE)
                        == FALSE))
                {
                  bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
                }
              }
              else
              {
                /* a new step definition for this sequence is successfully */
                /* received, update related step in sequence */
                if (SeqSave(bSignalSeqNo, SEQ_PROC_UPDATE, FALSE) == FALSE)
                {
                  bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
                }
              }
            }
          }
          else
          {
            bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
          }
        }
        else
        {
          bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
        }
      }
      else
      {
        bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
      }
    }
  }
} /* UIPacketSeqStep */

void UIPacketTime(void)
{
  tSTime SNewTime;

  memset(&SNewTime, 0, sizeof(SNewTime));

  if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                      (void *) &(SNewTime.SCurrentDate.Date), NULL,
                      sizeof(uint8_t)))
  {
    if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                        (void *) &(SNewTime.SCurrentDate.Month), NULL,
                        sizeof(uint8_t)))
    {
      uint16_t sYear = 0;

      if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          (void *) &(sYear),
                          NULL,
                          sizeof(uint16_t)))
      {
        SNewTime.bCentury = sYear / 100;
        SNewTime.SCurrentDate.Year = sYear % 100;
        SNewTime.SCurrentDate.WeekDay =
          TimeWeekDayOfYearCalc(SNewTime.SCurrentDate.Month,
                                SNewTime.
                                SCurrentDate.Date,
                                sYear);

        if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                            (void *) &(SNewTime.SCurrentTime.Hours),
                            NULL,
                            sizeof(uint8_t)))
        {
          if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                              (void *) &(SNewTime.SCurrentTime.Minutes),
                              NULL,
                              sizeof(uint8_t)))
          {
            if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                                (void *) &(SNewTime.SCurrentTime.Seconds), NULL,
                                sizeof(SNewTime.SCurrentTime.Seconds)))
            {
              if (!GpsModemAliveGet() || !GpsRTCInitialUpdateDoneGet())             /* Set time only if GPS is detached */
              {
                if (TimeIsValid(&SNewTime))
                {
                  TimeSet(&SNewTime);
                }
                else
                {
                  bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
                }
              }
            }
          }
        }
      }
    }
  }
} /* UIPacketTime */

void UIPacketSGSignals(void)
{
  uint8_t baNewSignals[SIGNAL_GROUPS_MAX];
  uint8_t bSGIndexNo;
  uint8_t bSGReceivedNo = 0;

  while (bUIRxParserCurrentField == UI_COMM_PACKET_FIELD_DATA)
  {
    if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                        &(baNewSignals[bSGReceivedNo]),
                        NULL,
                        sizeof(baNewSignals[bSGReceivedNo])))
    {
      bSGReceivedNo++;     /* add control if the signal value is correct */
    }
  }

  if (bUICommResponse == UI_COMM_RESPONSE_SUCCESS)
  {
    for (bSGIndexNo = 0; bSGIndexNo < bSGReceivedNo; bSGIndexNo++)
    {
      SGSignalSet(bSGIndexNo,
                  baNewSignals[bSGIndexNo]);
    }
  }
}

void UIPacketUserAdd(void)
{
}

void UIPacketUserRemove(void)
{
}

void UIPacketLCDLanguage(void)
{
  uint8_t bLanguage;
  char strLanguage[16];

  if (UIRxNextDataGet(UI_COMM_DATA_TYPE_STRING, NULL, strLanguage,
                      sizeof(strLanguage)))
  {
    bLanguage = UIConvertLanguageStrToLanguage(strLanguage);

    if (bLanguage != LANGUAGE_NONE)
    {
      LCDLanguageSet(bLanguage);
      if (LCDLanguageWrite() == FALSE)
      {
        bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
      }
    }
    else
    {
      bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
    }
  }
}

void UIPacketCommConfig(void)
{
  char strTrueFalse[8];

  if (UIRxNextDataGet(UI_COMM_DATA_TYPE_STRING, NULL, strTrueFalse,
                      sizeof(strTrueFalse)))
  {
    fUIChecksumEnabled = UIConvertFlagStrToFlag(strTrueFalse);
  }
}

void UIPacketInput(void)
{
  uint8_t bInputNo, bType;
  tSInput SInputDef;

  memset(&SInputDef, 0, sizeof(tSInput));

  if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                      (void *) &(bType),
                      NULL,
                      sizeof(bType)))
  {
    if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                        (void *) &(bInputNo),
                        NULL,
                        sizeof(bInputNo)))
    {
      if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          (void *) &(SInputDef.bActiveLevel),
                          NULL,
                          sizeof(SInputDef.bActiveLevel)))
      {
        if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                            (void *) &(SInputDef.bOwnerSG),
                            NULL,
                            sizeof(SInputDef.bOwnerSG)))
        {
          if (SGIsValid(SInputDef.bOwnerSG))
          {
            if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                                (void *) &(SInputDef.bGreenDurPerDemand),
                                NULL,
                                sizeof(SInputDef.bGreenDurPerDemand)))
            {
              if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                                  (void *) &(SInputDef.bRedDurInBroken),
                                  NULL,
                                  sizeof(SInputDef.bRedDurInBroken)))
              {
                if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                                    (void *) &(SInputDef.bPhaseInBroken),
                                    NULL,
                                    sizeof(SInputDef.bPhaseInBroken)))
                {
                  if (PhaseIsValid(SInputDef.bPhaseInBroken))
                  {
                    if (InputSet(bType, bInputNo - 1, &SInputDef) == FALSE)
                    {
                      bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
                    }
                  }
                  else
                  {
                    bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
                  }
                }
              }
            }
          }
          else
          {
            bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
          }
        }
      }
    }
  }
} /* UIPacketInput */

void UIPacketOutput(void)
{
  uint8_t bOutputNo;
  tSOutputDef SOutputDef;

  memset(&SOutputDef, 0, sizeof(tSOutputDef));

  if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                      (void *) &(bOutputNo),
                      NULL,
                      sizeof(bOutputNo)))
  {
    if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                        (void *) &(SOutputDef.bActiveLevel),
                        NULL,
                        sizeof(SOutputDef.bActiveLevel)))
    {
      if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          (void *) &(SOutputDef.bActiveLevelDur), NULL,
                          sizeof(SOutputDef.bActiveLevelDur)))
      {
        if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                            (void *) &(SOutputDef.bInActiveLevelDur),
                            NULL,
                            sizeof(SOutputDef.bInActiveLevelDur)))
        {
          if (OutputSet(bOutputNo - 1,
                        &SOutputDef) == FALSE)
          {
            bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
          }
        }
      }
    }
  }
}

void UIPacketTransition(void)
{
  uint8_t bSPNo;
  uint8_t bTransitionNo;
  tSTransition STransition;

  memset(&STransition, 0, sizeof(STransition));

  if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, (void *) &bSPNo, NULL,
                      sizeof(bSPNo)))
  {
    if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                        (void *) &bTransitionNo,
                        NULL,
                        sizeof(bTransitionNo)))
    {
      if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          (void *) &(STransition.bFrom), NULL,
                          sizeof(STransition.bFrom)))
      {
        if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                            (void *) &(STransition.bTo),
                            NULL,
                            sizeof(STransition.bTo)))
        {
          if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                              (void *) &(STransition.bValue1), NULL,
                              sizeof(STransition.bValue1)))
          {
            if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                                (void *) &(STransition.bValue2),
                                NULL,
                                sizeof(STransition.bValue2)))
            {
              if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                                  (void *) &(STransition.bRule), NULL,
                                  sizeof(STransition.bRule)))
              {
                if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                                    (void *) &(STransition.bPriority), NULL,
                                    sizeof(STransition.bPriority)))
                {
                  if (TransitionIsValid(bSPNo, bTransitionNo, &STransition))
                  {
                    if (TransitionSet(bSPNo,
                                      bTransitionNo,
                                      &STransition)
                        == FALSE)
                    {
                      bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
                    }
                  }
                  else
                  {
                    bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
                  }
                }
              }
            }
          }
        }
      }
    }
  }
} /* UIPacketTransition */

void UIPacketOperation(void)
{
  uint8_t bSPNo;
  uint8_t bField; /* field value */
  uint8_t bOperation; /* operation index */

  tSOperation SOperation; /* operation */

  memset(&SOperation, 0, sizeof(SOperation));

  if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, (void *) &bSPNo, NULL,
                      sizeof(bSPNo)))
  {
    if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                        (void *) &(bOperation),
                        NULL,
                        sizeof(bOperation)))
    {
      if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          (void *) &(SOperation.bOperator), NULL,
                          sizeof(SOperation.bOperator)))
      {
        if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                            (void *) &(bField),
                            NULL,
                            sizeof(bField)))
        {
          SOperation.SaOperands[0].bField = bField;
          if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                              (void *) &(SOperation.SaOperands[0].bSubField),
                              NULL,
                              sizeof(SOperation.SaOperands[0].bSubField)))
          {
            uint16_t sValue;

            if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                                (void *) &(sValue),
                                NULL,
                                sizeof(sValue)))
            {
              SOperation.SaOperands[0].bValueLow = (uint8_t) (sValue & 0x00FF);
              SOperation.SaOperands[0].bValueHigh = (uint8_t) ((sValue >>
                                                                8) & 0x0003);

              if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                                  (void *) &(bField),
                                  NULL,
                                  sizeof(bField)))
              {
                SOperation.SaOperands[1].bField = bField;
                if (UIRxNextDataGet(
                      UI_COMM_DATA_TYPE_INTEGER,
                      (void *) &(SOperation.SaOperands[1].bSubField),
                      NULL,
                      sizeof(SOperation.SaOperands[1].bSubField)))
                {
                  uint16_t sValue;

                  if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                                      (void *) &(sValue), NULL,
                                      sizeof(sValue)))
                  {
                    SOperation.SaOperands[1].bValueLow = (uint8_t) (sValue
                                                                    & 0x00FF);
                    SOperation.SaOperands[1].bValueHigh = (uint8_t) ((sValue >>
                                                                      8)
                                                                     & 0x0003);

                    if (OperationSet(bSPNo, bOperation, &SOperation) == FALSE)
                    {
                      bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
} /* UIPacketOperation */

void UIPacketRule(void)
{
  uint8_t bSPNo;
  uint8_t bRule; /* rule index */
  tSRule SRule; /* rule */

  memset(&SRule, 0, sizeof(SRule));

  if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, (void *) &bSPNo, NULL,
                      sizeof(bSPNo)))
  {
    if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                        (void *) &(bRule),
                        NULL,
                        sizeof(bRule)))
    {
      if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, (void *) &(SRule.sStart),
                          NULL,
                          sizeof(SRule.sStart)))
      {
        if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                            (void *) &(SRule.bTOpsStart), NULL,
                            sizeof(SRule.bTOpsStart)))
        {
          if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                              (void *) &(SRule.bTOpsEnd),
                              NULL,
                              sizeof(SRule.bTOpsEnd)))
          {
            if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                                (void *) &(SRule.bFOpsStart),
                                NULL,
                                sizeof(SRule.bFOpsStart)))
            {
              if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                                  (void *) &(SRule.bFOpsEnd),
                                  NULL,
                                  sizeof(SRule.bFOpsEnd)))
              {
                if (RuleSet(bSPNo,
                            bRule,
                            &SRule) == FALSE)
                {
                  bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
                }
              }
            }
          }
        }
      }
    }
  }
} /* UIPacketRule */

void UIPacketStatement(void)
{
  uint8_t bSPNo;
  uint8_t bStatement; /* statement index */
  tSStatement SStatement; /* statement */

  memset(&SStatement, 0, sizeof(SStatement));

  if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, (void *) &bSPNo, NULL,
                      sizeof(bSPNo)))
  {
    if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                        (void *) &(bStatement),
                        NULL,
                        sizeof(bStatement)))
    {
      if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          (void *) &(SStatement.bCmd),
                          NULL,
                          sizeof(SStatement.bCmd)))
      {
        if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                            (void *) &(SStatement.bParam1), NULL,
                            sizeof(SStatement.bParam1)))
        {
          if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                              (void *) &(SStatement.bParam2), NULL,
                              sizeof(SStatement.bParam2)))
          {
            if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                                (void *) &(SStatement.bParam3), NULL,
                                sizeof(SStatement.bParam3)))
            {
              if (StatementSet(bSPNo,
                               bStatement,
                               &SStatement) == FALSE)
              {
                bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
              }
            }
          }
        }
      }
    }
  }
} /* UIPacketStatement */

void UIPacketSigProg(void)
{
  tSSigProg SSigProg;
  uint8_t bSigProg; /* signal program no */

  memset(&SSigProg, 0, sizeof(SSigProg));

  if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                      (void *) &(bSigProg),
                      NULL,
                      sizeof(bSigProg)))
  {
    if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                        (void *) &(SSigProg.bStaStart),
                        NULL,
                        sizeof(SSigProg.bStaStart)))
    {
      if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          (void *) &(SSigProg.bStaEnd),
                          NULL,
                          sizeof(SSigProg.bStaEnd)))
      {
        if (SigProgSet(bSigProg, &SSigProg) == FALSE)
        {
          bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
        }
      }
    }
  }
}

void UIPacketSignalPlan(void)
{
  tSSignalPlan SSignalPlan;
  uint8_t bSignalPlan;
  uint8_t bData;

  memset(&SSignalPlan, 0, sizeof(SSignalPlan));

  if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                      (void *) &(bSignalPlan),
                      NULL,
                      sizeof(bSignalPlan)))
  {
    if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                        (void *) &(SSignalPlan.bSigProg), NULL,
                        sizeof(SSignalPlan.bSigProg)))
    {
      if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          (void *) &(bData),
                          NULL,
                          sizeof(bData)))
      {
        SSignalPlan.bWorkPlan = bData;
        if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                            (void *) &(bData),
                            NULL,
                            sizeof(bData)))
        {
          SSignalPlan.bWorkPlanEntry = bData;
          if (SignalPlanSet(bSignalPlan, &SSignalPlan) == FALSE)
          {
            bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
          }
        }
      }
    }
  }
}

void UIPacketLineOperator(void)
{
}

void UIPacketPop3(void)
{
}

void UIPacketSmtp(void)
{
}

void UIPacketSMSUser(void)
{
}

void UIPacketRemoteConn(void)
{
}

void UIPacketRequest(tpSUIRequest pRequest)
{
  if (UIRxPacketNameGet(&lRequestedPacket))
  {
    bUICommResponse = UI_COMM_RESPONSE_PACKET;
    switch (lRequestedPacket)
    {
        case UI_COMM_PACKET_SIGNAL:
        {
          UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          &bRequestParameter1,
                          NULL,
                          sizeof(bRequestParameter1));
          break;
        }

        case UI_COMM_PACKET_CVS:
        {
          UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          &bRequestParameter1,
                          NULL,
                          sizeof(bRequestParameter1));
          break;
        }

        case UI_COMM_PACKET_OPERATIONS:
        {
          if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, &bRequestParameter1,
                              NULL,
                              sizeof(bRequestParameter1)))
          {
            UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                            &bRequestParameter2,
                            NULL,
                            sizeof(bRequestParameter2));
          }

          break;
        }

        case UI_COMM_PACKET_RULES:
        {
          if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, &bRequestParameter1,
                              NULL,
                              sizeof(bRequestParameter1)))
          {
            UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                            &bRequestParameter2,
                            NULL,
                            sizeof(bRequestParameter2));
          }

          break;
        }

        case UI_COMM_PACKET_SMS_USER:
        {
          UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          &bRequestParameter1,
                          NULL,
                          sizeof(bRequestParameter1));
          break;
        }

        case UI_COMM_PACKET_STATEMENTS:
        {
          if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, &bRequestParameter1,
                              NULL,
                              sizeof(bRequestParameter1)))
          {
            UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                            &bRequestParameter2,
                            NULL,
                            sizeof(bRequestParameter2));
          }

          break;
        }

        case UI_COMM_PACKET_SIGNAL_PROGRAM:
        {
          UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          &bRequestParameter1,
                          NULL,
                          sizeof(bRequestParameter1));
          break;
        }

        case UI_COMM_PACKET_DEVICE_INFO:
        {
          break;
        }

        case UI_COMM_PACKET_SIGNALS_DEFINED:
        {
          break;
        }

        case UI_COMM_PACKET_SG:
        {
          UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          &bRequestParameter1,
                          NULL,
                          sizeof(bRequestParameter1));
          break;
        }

        case UI_COMM_PACKET_CONFLICT_EM:
        {
          UIRxNextDataGet(UI_COMM_DATA_TYPE_STRING,
                          NULL,
                          strRequestParameter,
                          sizeof(strRequestParameter));
          break;
        }

        case UI_COMM_PACKET_SO:
        {
          if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, &bRequestParameter1,
                              NULL,
                              sizeof(bRequestParameter1)))
          {
            UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                            &bRequestParameter2,
                            NULL,
                            sizeof(bRequestParameter2));
          }

          break;
        }

        case UI_COMM_PACKET_CLEARANCE:
        {
          if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, &bRequestParameter1,
                              NULL,
                              sizeof(bRequestParameter1)))
          {
            UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                            &bRequestParameter2,
                            NULL,
                            sizeof(bRequestParameter2));
          }

          break;
        }

        case UI_COMM_PACKET_PHASE:
        {
          UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          &bRequestParameter1,
                          NULL,
                          sizeof(bRequestParameter1));
          break;
        }

        case UI_COMM_PACKET_TRANSITION:
        {
          if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, &bRequestParameter1,
                              NULL,
                              sizeof(bRequestParameter1)))
          {
            UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                            &bRequestParameter2,
                            NULL,
                            sizeof(bRequestParameter2));
          }

          break;
        }

        case UI_COMM_PACKET_SP_PLAN_ENTRY:
        {
          if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, &bRequestParameter1,
                              NULL,
                              sizeof(bRequestParameter1)))
          {
            UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                            &bRequestParameter2,
                            NULL,
                            sizeof(bRequestParameter2));
          }

          break;
        }

        case UI_COMM_PACKET_SIGNAL_PLAN:
        {
          UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          &bRequestParameter1,
                          NULL,
                          sizeof(bRequestParameter1));
          break;
        }

        case UI_COMM_PACKET_WP_ENTRY:
        {
          if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, &bRequestParameter1,
                              NULL,
                              sizeof(bRequestParameter1)))
          {
            UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                            &bRequestParameter2,
                            NULL,
                            sizeof(bRequestParameter2));
          }

          break;
        }

        case UI_COMM_PACKET_WS_ENTRY:
        {
          UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          &bRequestParameter1,
                          NULL,
                          sizeof(bRequestParameter1));
          break;
        }

        case UI_COMM_PACKET_FLASH_PERIODS:
        {
          break;
        }

        case UI_COMM_PACKET_OPEN_RELAY:   /* ISSD function @160225 */
        case UI_COMM_PACKET_CLOSE_RELAY:   /* ISSD function @160225 */
        {
          if (GetFunctionConfByIndex(LIC_ISSD))
          {
            break;
          }
          else     /* if license is not configured apply default case here. */
          {
            bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
            SetProgramLoadingStatus(PROGRAM_UPLOADING_ERROR);
            ProgramStateSet(PROGRAM_STATE_DARK);
          }
        }

        case UI_COMM_PACKET_LD_MANIP1:
        case UI_COMM_PACKET_LB_MANIP1:
        case UI_COMM_PACKET_LD_MANIP2:
        case UI_COMM_PACKET_LB_MANIP2:
        {
          UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          &bRequestParameter1,
                          NULL,
                          sizeof(bRequestParameter1));
          UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          &bRequestParameter2,
                          NULL,
                          sizeof(bRequestParameter2));
          break;
        }

        case UI_COMM_PACKET_IO_RUNTIME:
        {
          UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          &bRequestParameter1,
                          NULL,
                          sizeof(bRequestParameter1));
          UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          &bRequestParameter2,
                          NULL,
                          sizeof(bRequestParameter2));
          break;
        }

        case UI_COMM_PACKET_INPUT_MANIP1:
        case UI_COMM_PACKET_INPUT_MANIP2:
        {
          UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          &bRequestParameter1,
                          NULL,
                          sizeof(bRequestParameter1));
          UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          &bRequestParameter2,
                          NULL,
                          sizeof(bRequestParameter2));
          break;
        }

        case UI_COMM_PACKET_SEQUENCE_STEP:
        {
          /* read signal sequence number and step number in sequence */
          if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, &bRequestParameter1,
                              NULL,
                              sizeof(bRequestParameter1)))
          {
            UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                            &bRequestParameter2,
                            NULL,
                            sizeof(bRequestParameter2));
          }

          break;
        }

        case UI_COMM_PACKET_SWITCH_ON_SEQ:
        {
          break;
        }

        case UI_COMM_PACKET_SWITCH_OFF_SEQ:
        {
          break;
        }

        case UI_COMM_PACKET_TIME:
        {
          if (pRequest->bReqId == UI_REQ_TYPE_TCP_CLIENT)
          {
            LogRequest(LOG_REQ_APPEND_ASYNCH,
                       NULL,
                       EVENT_MCS_USER_REQUEST_UPLOAD,
                       0,
                       0,
                       0,
                       0);
            SetProgramLoadingStatus(PROGRAM_UPLOADING_JUST_STARTED);
            UICheckUploadTimeoutSet(TRUE);
            UISetMCSUploadInProgress(TRUE);
          }
          else
          {
            UserSettingsRead();
            if (UserSettingsConfigFlagGet() == FALSE)
            {
              bUICommResponse = UI_COMM_RESPONSE_SECURITY_ERROR;
            }
            else
            {
              SetProgramLoadingStatus(PROGRAM_UPLOADING_JUST_STARTED);
              UICheckUploadTimeoutSet(TRUE);
            }
          }

          break;
        }

        case UI_COMM_PACKET_SIGNALS:
        {
          break;
        }

        case UI_COMM_PACKET_LOG_NEXT:
        {
          break;
        }

        case UI_COMM_PACKET_LOG_FROM:
        {
          /* read index number */
          UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          &sRequestParameter3,
                          NULL,
                          sizeof(sRequestParameter3));
          break;
        }

        case UI_COMM_PACKET_LOG_LAST_INDEX:
        {
          break;
        }

        case UI_COMM_PACKET_GET_USERNAMES:
        {
          break;
        }

        case UI_COMM_PACKET_LCD_LANGUAGE:
        {
          break;
        }

        case UI_COMM_PACKET_COMM_CONFIG:
        {
          break;
        }

        case UI_COMM_PACKET_SG_NUMBER:
        {
          break;
        }

        case UI_COMM_PACKET_SIN_NUMBER:
        {
          break;
        }

        case UI_COMM_PACKET_WS_NUMBER:
        {
          break;
        }

        case UI_COMM_PACKET_CONFLICT_NUMBER:
        {
          break;
        }

        case UI_COMM_PACKET_SO_NUMBER:
        {
          /* SG number */
          UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          &bRequestParameter1,
                          NULL,
                          sizeof(bRequestParameter1));
          break;
        }

        case UI_COMM_PACKET_CFL_NUMBER:
        {
          /* SG number */
          UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          &bRequestParameter1,
                          NULL,
                          sizeof(bRequestParameter1));
          break;
        }

        case UI_COMM_PACKET_PHA_NUMBER:
        {
          break;
        }

        case UI_COMM_PACKET_OPT_NUMBER:
        {
          break;
        }

        case UI_COMM_PACKET_SPR_NUMBER:
        {
          break;
        }

        case UI_COMM_PACKET_TRA_NUMBER:
        {
          break;
        }

        case UI_COMM_PACKET_SP_NUMBER:
        {
          break;
        }

        case UI_COMM_PACKET_WP_NUMBER:
        {
          break;
        }

        case UI_COMM_PACKET_WP_ENTRY_NUMBER:
        {
          /* WP number */
          UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          &bRequestParameter1,
                          NULL,
                          sizeof(bRequestParameter1));
          break;
        }

        case UI_COMM_PACKET_SPP_NUMBER:
        {
          break;
        }

        case UI_COMM_PACKET_SEQ_NUMBER:
        {
          break;
        }

        case UI_COMM_PACKET_SEQ_STEP_NUMBER:
        {
          UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          &bRequestParameter1,
                          NULL,
                          sizeof(bRequestParameter1));
          break;
        }

        case UI_COMM_PACKET_INPUT:
        {
          if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, &bRequestParameter1,
                              NULL,
                              sizeof(bRequestParameter1)))
          {
            UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                            &bRequestParameter2,
                            NULL,
                            sizeof(bRequestParameter2));
          }

          break;
        }

        case UI_COMM_PACKET_OUTPUT:
        {
          UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          &bRequestParameter1,
                          NULL,
                          sizeof(bRequestParameter1));
          break;
        }

        case UI_COMM_PACKET_SMS_USER_NUMBER:
        {
          break;
        }

        case UI_COMM_PACKET_OUTPUT_NUMBER:
        case UI_COMM_PACKET_DET_NUMBER:
        case UI_COMM_PACKET_INPUT_NUMBER:
        {
          break;
        }

        case UI_COMM_PACKET_STATEMENT_NUMBER:
        {
          break;
        }

        case UI_COMM_PACKET_SSM:
        case UI_COMM_PACKET_PSM:
        case UI_COMM_PACKET_IOM:
        {
          break;
        }

        case UI_COMM_PACKET_CURRENT_RUNTIME_INFO:
        {
          break;
        }

        case UI_COMM_PACKET_CHECKSUM_TOTAL:
        {
          break;
        }

        case UI_COMM_PACKET_MODULE_VERSIONS:
        {
          break;
        }

        case UI_COMM_PACKET_MAESTRO_MODULE_VERSIONS:
        {
          if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, &bRequestParameter1,
                              NULL,
                              sizeof(bRequestParameter1)))
          {
            UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                            &bRequestParameter2,
                            NULL,
                            sizeof(bRequestParameter2));
          }

          break;
        }

        case UI_COMM_PACKET_FUNCTION_CONF:
        {
          UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          &bRequestParameter1,
                          NULL,
                          sizeof(bRequestParameter1));
          break;
        }

        case UI_COMM_PACKET_INP_DATA_MANIP:
        {
          break;
        }

        case UI_COMM_PACKET_MCS_UPLOAD:
        {
          UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          &bRequestParameter1,
                          NULL,
                          sizeof(bRequestParameter1));
          UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          &bRequestParameter2,
                          NULL,
                          sizeof(bRequestParameter2));
          UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          &bRequestParameter3,
                          NULL,
                          sizeof(bRequestParameter3));
          break;
        }

        case UI_COMM_PACKET_SYSTEM_START_TIME:
        {
          break;
        }

        case UI_COMM_PACKET_DEBUG:
        {
          #ifdef DEBUG
          UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          &bRequestParameter1,
                          NULL,
                          sizeof(bRequestParameter1));
          UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          &bRequestParameter2,
                          NULL,
                          sizeof(bRequestParameter2));
          UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          &bRequestParameter3,
                          NULL,
                          sizeof(bRequestParameter3));
          #endif /* ifdef DEBUG */
          break;
        }

        case UI_COMM_PACKET_RESET_CPU:
        {
          break;
        }

        case UI_COMM_PACKET_IAP:
        {
          break;
        }

        case UI_COMM_PACKET_MCS_CON_INFO:
        {
          break;
        }

        default:
        {
          bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
          SetProgramLoadingStatus(PROGRAM_UPLOADING_ERROR);
          ProgramStateSet(PROGRAM_STATE_DARK);

          UICheckUploadTimeoutSet(FALSE);
          UISetMCSUploadInProgress(FALSE);
          break;
        }
    } /* switch */
  }
} /* UIPacketRequest */

void UIPacketRespIOM(void)
{
  tSCanCpuIOOutputs SCanCpuIOOutputs;
  uint8_t bIONo;
  uint8_t bIndex;

  strcat(strUITx, UI_COMM_PACKET_IOM_STR);
  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

  for (bIONo = 0; bIONo < MODULES_IO_MAX; bIONo++)
  {
    /* safe/broken */
    for (bIndex = 0; bIndex < IO_INPUTS_DETECTOR_MAX; bIndex++)
    {
      if (SaCanDetectorIOInputs[bIONo].sLoopSafeStates & laValue2Bit[bIndex])
      {
        strcat(strUITx, "1");
      }
      else
      {
        strcat(strUITx, "0");
      }
    }

    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

    /* empty/busy */
    for (bIndex = 0; bIndex < IO_INPUTS_DETECTOR_MAX; bIndex++)
    {
      if (SaCanDetectorIOInputs[bIONo].sLoopEmptyStates & laValue2Bit[bIndex])
      {
        strcat(strUITx, "1");
      }
      else
      {
        strcat(strUITx, "0");
      }
    }

    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

    /* input levels */
    for (bIndex = 0; bIndex < IO_INPUTS_DIGITAL_MAX; bIndex++)
    {
      if (SaCanDigitalIOInputs[bIONo].sInputStates & laValue2Bit[bIndex])
      {
        strcat(strUITx, "1");
      }
      else
      {
        strcat(strUITx, "0");
      }
    }

    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

    GetIOOutputs(&SCanCpuIOOutputs);

    /* measured output levels */
    for (bIndex = 0; bIndex < IO_OUTPUTS_MAX; bIndex++)
    {
      if (SCanCpuIOOutputs.sOutputStates & laValue2Bit[bIndex])
      {
        strcat(strUITx, "1");
      }
      else
      {
        strcat(strUITx, "0");
      }
    }

    if (bIONo != MODULES_IO_MAX - 1)
    {
      strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    }
  }
} /* UIPacketRespIOM */

void UIPacketRespPSM(void)
{
  char strData[8];
  uint8_t bIndex;

  strcat(strUITx, UI_COMM_PACKET_PSM_STR);
  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

  for (bIndex = 0; bIndex < PSMS_MAX; bIndex++)
  {
    sprintf(strData, "%d", GetPowerSupplyNet(bIndex));
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strData, "%d", GetPowerSupplyFreq(bIndex));
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strData, "%d", GetPowerSupply24V1(bIndex));
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strData, "%d", GetPowerSupply5V1(bIndex));
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strData, "%d", GetPowerSupply24V2(bIndex));
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strData, "%d", GetPowerSupply5V2(bIndex));
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strData, "%d", GetPowerSupplyIsolatedVoltage(bIndex));
    strcat(strUITx, strData);
    if (bIndex != PSMS_MAX - 1)
    {
      strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    }
  }
}

void UIPacketRespSSM(void)
{
  tSCurrentMeasurement SSGCurrent;

  uint8_t baSOVoltages[SIGNAL_OUTPUTS_MAX / 8];
  uint8_t bSGNo;
  uint16_t bDataIdx;
  uint8_t bIndex;

  strUITx[0] = '\0';

  memset(&SSGCurrent, 0, sizeof(SSGCurrent));
  memset(&baSOVoltages, 0, sizeof(baSOVoltages));
  memset(strTempData, 0, sizeof(strTempData));

  strcat(strUITx, UI_COMM_START_OF_PACKET_STR);
  strcat(strUITx, UI_COMM_PACKET_SSM_STR);
  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

  bDataIdx = 0;
  for (bSGNo = 0; bSGNo < SIGNAL_GROUPS_MAX; bSGNo++)
  {
    GetSGCurrentMeasurement(bSGNo, &SSGCurrent);

    strTempData[bDataIdx++] = (SSGCurrent.sMax & 0x00FF);
    strTempData[bDataIdx++] = (SSGCurrent.sMax & 0xFF00) >> 8;
    strTempData[bDataIdx++] = (SSGCurrent.sNow & 0x00FF);
    strTempData[bDataIdx++] = (SSGCurrent.sNow & 0xFF00) >> 8;
    strTempData[bDataIdx++] = (SSGCurrent.sMin & 0x00FF);
    strTempData[bDataIdx++] = (SSGCurrent.sMin & 0xFF00) >> 8;

    /* init to zero */
    strTempData[bDataIdx] = 0;
    strTempData[bDataIdx + 1] = 0;
    /* add information if above bytes are end of packet character */
    for (bIndex = 0; bIndex < 6; bIndex++)
    {
      if (strTempData[bDataIdx - 6] == '\r')
      {
        strTempData[bDataIdx - 6] = 0xFF;

        if (bIndex < 3)
        {
          strTempData[bDataIdx] |= (0x03 << (bIndex * 2));
        }
        else
        {
          strTempData[bDataIdx + 1] |= (0x03 << ((bIndex - 3) * 2));
        }
      }
    }

    bDataIdx += 2;
    strTempData[bDataIdx++] = UI_COMM_DATA_SEPARATOR;
  }

  for (bIndex = 0; bIndex < (SIGNAL_OUTPUTS_MAX / 8); bIndex++)
  {
    strTempData[bDataIdx++] = baSOVoltages[bIndex];
  }

  /* append three bytes to state '\r' existence in voltage value bytes */
  strTempData[bDataIdx] = 0;
  strTempData[bDataIdx + 1] = 0;
  strTempData[bDataIdx + 2] = 0;

  /* add information if above bytes are end of packet character */
  for (bIndex = 0; bIndex < (SIGNAL_OUTPUTS_MAX / 8); bIndex++)
  {
    if (strTempData[bDataIdx - (SIGNAL_OUTPUTS_MAX / 8)] == '\r')
    {
      strTempData[bDataIdx - (SIGNAL_OUTPUTS_MAX / 8)] = 0xFF;

      if (bIndex < 4)
      {
        strTempData[bDataIdx] |= (0x03 << (bIndex * 2));
      }
      else if (bIndex < 8)
      {
        strTempData[bDataIdx + 1] |= (0x03 << ((bIndex - 4) * 2));
      }
      else
      {
        strTempData[bDataIdx + 2] |= (0x03 << ((bIndex - 8) * 2));
      }
    }
  }

  bDataIdx += 3;

  strTempData[bDataIdx++] = UI_COMM_END_OF_PACKET;
  strncat(strUITx, strTempData, sizeof(strUITx) - strlen(strUITx) - 1);
} /* UIPacketRespSSM */

void UIPacketRespSignal(void)
{
  char strSignalInfo[6];
  uint8_t bFollowerSignal;

  if ((bRequestParameter1) && (bRequestParameter1 <= SignalTotalGet()))
  {
    tSSignalDef SSignalDef;

    GetSignalDefs(bRequestParameter1 - 1, &SSignalDef);

    strcat(strUITx, UI_COMM_PACKET_SIGNAL_STR);
    strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

    sprintf(strSignalInfo, "%d", bRequestParameter1);
    strcat(strUITx, strSignalInfo);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strSignalInfo, "%d", SSignalDef.SaSignal[0].sPeriod);
    strcat(strUITx, strSignalInfo);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strSignalInfo, "%d", SSignalDef.SaSignal[1].sPeriod);
    strcat(strUITx, strSignalInfo);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strSignalInfo, "%d", SSignalDef.SaSignal[2].sPeriod);
    strcat(strUITx, strSignalInfo);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strSignalInfo, "%d", SSignalDef.SFlags.fValid);
    strcat(strUITx, strSignalInfo);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strSignalInfo, "%d", SSignalDef.SFlags.fValidForFlash);
    strcat(strUITx, strSignalInfo);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strSignalInfo, "%d", SSignalDef.SFlags.fValidForEmergencyFlash);
    strcat(strUITx, strSignalInfo);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strSignalInfo, "%d", SSignalDef.bMinDur);
    strcat(strUITx, strSignalInfo);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strSignalInfo, "%d", SSignalDef.bMaxDuration);
    strcat(strUITx, strSignalInfo);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    /* if signal can follow bSignal, append TRUE for signal */
    for (bFollowerSignal = 0; bFollowerSignal < SIGNALS_MAX; bFollowerSignal++)
    {
      if (SSignalDef.sFollowers & laValue2Bit[bFollowerSignal])
      {
        sprintf(strSignalInfo, "%d", TRUE);
      }
      else
      {
        sprintf(strSignalInfo, "%d", FALSE);
      }

      strcat(strUITx, strSignalInfo);

      if (bFollowerSignal != SIGNALS_MAX - 1)
      {
        strcat(strUITx,
               UI_COMM_DATA_SEPARATOR_STR);
      }
    }
  }
  else
  {
    UIFailureResponseSet();
  }
} /* UIPacketRespSignal */

void UIPacketRespCVS(void)
{
  char strCVSData[4];
  tSCVSDef SCVSDef;

  memset(&SCVSDef, 0, sizeof(tSCVSDef));

  if ((bRequestParameter1) && (bRequestParameter1 < (LAMP_TYPE_MAX + 1)))
  {
    strcat(strUITx, UI_COMM_PACKET_CVS_STR);
    strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

    GetCVSDef(&SCVSDef);

    sprintf(strCVSData, "%hd", SCVSDef.saCurrents[bRequestParameter1 - 1]);
    strcat(strUITx, strCVSData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strCVSData, "%hd", SCVSDef.saVoltages[bRequestParameter1 - 1]);
    strcat(strUITx, strCVSData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strCVSData, "%hd",
            (uint16_t) (SCVSDef.raSlopes[bRequestParameter1 - 1] * 1000));
    strcat(strUITx, strCVSData);
  }
  else
  {
    UIFailureResponseSet();
  }
}

void UIPacketRespStatement(void)
{
  char strStatementData[16];
  tSStatement SStatement;

  if ((bRequestParameter2)
      && (bRequestParameter2 <= StatementTotalGet(bRequestParameter1)))
  {
    memset(&SStatement, 0, sizeof(tSStatement));
    StatementGet(bRequestParameter1, bRequestParameter2, &SStatement);

    strcat(strUITx, UI_COMM_PACKET_STATEMENTS_STR);
    strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);
    sprintf(strStatementData, "%d", bRequestParameter1);
    strcat(strUITx, strStatementData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strStatementData, "%d", bRequestParameter2);
    strcat(strUITx, strStatementData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strStatementData, "%d", SStatement.bCmd);
    strcat(strUITx, strStatementData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strStatementData, "%d", SStatement.bParam1);
    strcat(strUITx, strStatementData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strStatementData, "%d", SStatement.bParam2);
    strcat(strUITx, strStatementData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strStatementData, "%d", SStatement.bParam3);
    strcat(strUITx, strStatementData);
  }
  else
  {
    UIFailureResponseSet();
  }
}

void UIPacketRespRule(void)
{
  char strRuleData[16];
  tSRule SRule;

  if ((bRequestParameter2)
      && (bRequestParameter2 <= RuleTotalGet(bRequestParameter1)))
  {
    memset(&SRule, 0, sizeof(tSRule));
    RuleGet(bRequestParameter1, bRequestParameter2, &SRule);

    strcat(strUITx, UI_COMM_PACKET_RULES_STR);
    strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);
    sprintf(strRuleData, "%d", bRequestParameter1);
    strcat(strUITx, strRuleData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strRuleData, "%d", bRequestParameter2);
    strcat(strUITx, strRuleData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strRuleData, "%hd", SRule.sStart);
    strcat(strUITx, strRuleData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strRuleData, "%d", SRule.bTOpsStart);
    strcat(strUITx, strRuleData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strRuleData, "%d", SRule.bTOpsEnd);
    strcat(strUITx, strRuleData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strRuleData, "%d", SRule.bFOpsStart);
    strcat(strUITx, strRuleData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strRuleData, "%d", SRule.bFOpsEnd);
    strcat(strUITx, strRuleData);
  }
  else
  {
    UIFailureResponseSet();
  }
}

void UIPacketRespSigProg(void)
{
  char strSigProgData[16];
  tSSigProg SSigProg;

  SigProgGet(bRequestParameter1, &SSigProg);

  strcat(strUITx, UI_COMM_PACKET_SIGNAL_PROGRAM_STR);
  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

  sprintf(strSigProgData, "%d", bRequestParameter1);
  strcat(strUITx, strSigProgData);
  strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

  sprintf(strSigProgData, "%d", SSigProg.bStaStart);
  strcat(strUITx, strSigProgData);
  strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

  sprintf(strSigProgData, "%d", SSigProg.bStaEnd);
  strcat(strUITx, strSigProgData);
  strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

  sprintf(strSigProgData, "%d", SSigProg.bEndStart);
  strcat(strUITx, strSigProgData);
  strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

  sprintf(strSigProgData, "%d", SSigProg.bEndEnd);
  strcat(strUITx, strSigProgData);
}

void UIPacketRespStatementTotal(void)
{
  uint8_t bSPIndex;
  char strStatementData[16];

  strcat(strUITx, UI_COMM_PACKET_STATEMENT_NUMBER_STR);
  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

  for (bSPIndex = 0; bSPIndex < SigProgTotalGet(); bSPIndex++)
  {
    sprintf(strStatementData, "%d", (bSPIndex + 1));
    strcat(strUITx, strStatementData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strStatementData, "%d", StatementTotalGet(bSPIndex + 1));
    strcat(strUITx, strStatementData);

    if (bSPIndex != SigProgTotalGet() - 1)
    {
      strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    }
  }
}

void UIPacketRespOperationTotal(void)
{
  uint8_t bSPIndex;

  memset(strTempData, 0, sizeof(strTempData));

  strcat(strUITx, UI_COMM_PACKET_OPT_NUMBER_STR);
  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

  for (bSPIndex = 0; bSPIndex < SigProgTotalGet(); bSPIndex++)
  {
    sprintf(strTempData, "%d", (bSPIndex + 1));
    strcat(strUITx, strTempData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strTempData, "%d", OperationTotalGet(bSPIndex + 1));
    strcat(strUITx, strTempData);

    if (bSPIndex != SigProgTotalGet() - 1)
    {
      strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    }
  }
}

void UIPacketRespTransitionTotal(void)
{
  uint8_t bSPIndex;
  char strTRANumberData[16];

  strcat(strUITx, UI_COMM_PACKET_TRA_NUMBER_STR);
  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

  for (bSPIndex = 0; bSPIndex < SigProgTotalGet(); bSPIndex++)
  {
    sprintf(strTRANumberData, "%d", (bSPIndex + 1));
    strcat(strUITx, strTRANumberData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strTRANumberData, "%d", TransitionTotalGet(bSPIndex + 1));
    strcat(strUITx, strTRANumberData);

    if (bSPIndex != SigProgTotalGet() - 1)
    {
      strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    }
  }
}

void UIPacketRespOperation(void)
{
  uint8_t bIndex = 0;
  tSOperation SOperation;

  memset(strTempData, 0, sizeof(strTempData));

  if ((bRequestParameter2)
      && (bRequestParameter2 <= OperationTotalGet(bRequestParameter1)))
  {
    memset(&SOperation, 0, sizeof(tSOperation));
    OperationGet(bRequestParameter1, bRequestParameter2, &SOperation);

    strcat(strUITx, UI_COMM_PACKET_OPERATIONS_STR);
    strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

    sprintf(strTempData, "%d", bRequestParameter1);
    strcat(strUITx, strTempData);

    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

    sprintf(strTempData, "%d", bRequestParameter2);
    strcat(strUITx, strTempData);

    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

    sprintf(strTempData, "%d", SOperation.bOperator);
    strcat(strUITx, strTempData);

    for (bIndex = 0; bIndex < OP_MAX; bIndex++)
    {
      strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
      sprintf(strTempData, "%d", SOperation.SaOperands[bIndex].bField);
      strcat(strUITx, strTempData);
      strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
      sprintf(strTempData, "%d", SOperation.SaOperands[bIndex].bValueHigh);
      strcat(strUITx, strTempData);
      strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
      sprintf(strTempData, "%d", SOperation.SaOperands[bIndex].bSubField);
      strcat(strUITx, strTempData);
      strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
      sprintf(strTempData, "%d", SOperation.SaOperands[bIndex].bValueLow);
      strcat(strUITx, strTempData);
    }
  }
  else
  {
    UIFailureResponseSet();
  }
} /* UIPacketRespOperation */

void UIPacketRespSignalsDefined(void)
{
  tSSignalsDefined SSignalsDefined;
  char strSignalDefinitionData[4];

  GetSignalsDefined(&SSignalsDefined);

  strcat(strUITx, UI_COMM_PACKET_SIGNALS_DEFINED_STR);
  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);
  sprintf(strSignalDefinitionData, "%d", SSignalsDefined.bDark);
  strcat(strUITx, strSignalDefinitionData);
  strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
  sprintf(strSignalDefinitionData, "%d", SSignalsDefined.bBlocking);
  strcat(strUITx, strSignalDefinitionData);
  strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
  sprintf(strSignalDefinitionData, "%d", SSignalsDefined.bFree);
  strcat(strUITx, strSignalDefinitionData);
  strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
  sprintf(strSignalDefinitionData, "%d", SSignalsDefined.bGreenFlash);
  strcat(strUITx, strSignalDefinitionData);
}

void UIPacketRespDeviceInfo(void)
{
  int8_t bTimeZone;
  char strSDevInfo[16];
  tSDeviceInfo SDevInfo;

  memset(&SDevInfo, 0, sizeof(tSDeviceInfo));

  GetDeviceInfo(&SDevInfo);
  bTimeZone = GetDeviceTimeZone();

  strcat(strUITx, UI_COMM_PACKET_DEVICE_INFO_STR);
  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

  strcat(strUITx, SDevInfo.strIntersection);
  strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
  sprintf(strSDevInfo, "%d", SDevInfo.bCrossNo);
  strcat(strUITx, strSDevInfo);
  strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
  strcat(strUITx, SDevInfo.strCity);
  strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
  strcat(strUITx, SDevInfo.strCountry);
  strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
  if (bTimeZone > 0)
  {
    sprintf(strSDevInfo, "+%d", bTimeZone);
  }
  else
  {
    sprintf(strSDevInfo, "%d", bTimeZone);
  }

  strcat(strUITx, strSDevInfo);
  strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
  strcat(strUITx, UIConvertDeviceTypeToDeviceTypeStr(SDevInfo.bDeviceType));
  strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
  /* strncat(strUITx, SDevInfo.strIPNo, strlen(SDevInfo.strIPNo)); */
  strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
  strncat(strUITx, SDevInfo.strDomain, strlen(SDevInfo.strDomain));
  strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
  strncat(strUITx, SDevInfo.strAPN, strlen(SDevInfo.strAPN));
  strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
  strncat(strUITx, SDevInfo.strUsername, strlen(SDevInfo.strUsername));
  strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
  strncat(strUITx, SDevInfo.strPassword, strlen(SDevInfo.strPassword));
} /* UIPacketRespDeviceInfo */

void UIPacketRespSG(void)
{
  char strSGData[16];
  tSSGDef SSGData;

  memset(&SSGData, 0, sizeof(SSGData)); /* set all structures to 0 */

  if (SGGet(bRequestParameter1 - 1, &SSGData))
  {
    strcat(strUITx, UI_COMM_PACKET_SG_STR);
    strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

    sprintf(strSGData, "%d", bRequestParameter1);
    strcat(strUITx, strSGData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    strcat(strUITx, UIConvertSGTypeToSGTypeStr(SSGData.bType));
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strSGData, "%d", SSGData.bOpeningSignal);
    strcat(strUITx, strSGData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strSGData, "%d", SSGData.bOpeningDuration);
    strcat(strUITx, strSGData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strSGData, "%d", SSGData.bClosingSignal);
    strcat(strUITx, strSGData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strSGData, "%d", SSGData.bClosingDur);
    strcat(strUITx, strSGData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strSGData, "%d", SSGData.bFlashSignal);
    strcat(strUITx, strSGData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strSGData, "%d", SSGData.bFailureFlashSignal);
    strcat(strUITx, strSGData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strSGData, "%d", SSGData.bGreenFlashDur);
    strcat(strUITx, strSGData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strSGData, "%d", SSGData.bRedLampFailureNumber);
    strcat(strUITx, strSGData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strSGData, "%d", SSGData.SEmergencyMethods.bRedLampFailureNumberEM);
    strcat(strUITx, strSGData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strSGData, "%d", SSGData.SEmergencyMethods.bLastRedLampFailureEM);
    strcat(strUITx, strSGData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strSGData, "%d", (SSGData.bOwner + 1));
    strcat(strUITx, strSGData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strSGData, "%d", SSGData.bFirstOutput);
    strcat(strUITx, strSGData);
  }
  else
  {
    UIFailureResponseSet();
  }
} /* UIPacketRespSG */

void UIPacketRespConflictEM(void)
{
  tSConflictsEM SConflictsEm;

  memset(&SConflictsEm, 0, sizeof(tSConflictsEM));

  strcat(strUITx, UI_COMM_PACKET_CONFLICT_EM_STR);
  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

  GetConflictsEM(&SConflictsEm);

  strcat(strUITx, strRequestParameter);
  strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

  if (strcmp(strRequestParameter, UI_COMM_STR_GREENGREENCONFLICTEM) == 0)
  {
    strcat(strUITx, UIConvertEMToEMStr(SConflictsEm.bGreenGreenEM));
  }
  else if (strcmp(strRequestParameter, UI_COMM_STR_YELLOWYELLOWCONFLICTEM) == 0)
  {
    strcat(strUITx, UIConvertEMToEMStr(SConflictsEm.bYellowYellowEM));
  }
  else if (strcmp(strRequestParameter, UI_COMM_STR_YELLOWGREENCONFLICTEM) == 0)
  {
    strcat(strUITx, UIConvertEMToEMStr(SConflictsEm.bYellowGreenEM));
  }
  else if (strcmp(strRequestParameter, UI_COMM_STR_INVALIDSIGNALEM) == 0)
  {
    strcat(strUITx, UIConvertEMToEMStr(SConflictsEm.bInvalidSignalEM));
  }
  else if (strcmp(strRequestParameter,
                  UI_COMM_STR_INVALIDSIGNALSEQUENCEEM) == 0)
  {
    strcat(strUITx, UIConvertEMToEMStr(SConflictsEm.bInvalidSignalSequenceEM));
  }
  else if (strcmp(strRequestParameter, UI_COMM_STR_VOLTAGELIMITSEM) == 0)
  {
    strcat(strUITx, UIConvertEMToEMStr(SConflictsEm.bVoltageLimitsEM));
  }
  else if (strcmp(strRequestParameter, UI_COMM_STR_FREQUENCYERROREM) == 0)
  {
    strcat(strUITx, UIConvertEMToEMStr(SConflictsEm.bFrequencyErrorEM));
  }
  else
  {
    UIFailureResponseSet();
  }
} /* UIPacketRespConflictEM */

void UIPacketRespSO(void)
{
  uint8_t bSSMNo = 0, bOutputNoAtSSM = 0, bIndex = 0;
  char strSOData[16];
  tSSODef SSOData;

  memset(&SSOData, 0, sizeof(SSOData));

  /* parameters are SG no, SO Index in SG */
  if (GetSODefByIndex(bRequestParameter1 - 1,
                      bRequestParameter2 - 1,
                      &SSOData,
                      &bIndex))
  {
    bSSMNo = (bIndex / SIGNAL_OUTPUTS_PER_SSM) + 1;
    bOutputNoAtSSM = (bIndex % SIGNAL_OUTPUTS_PER_SSM) + 1;

    strcat(strUITx, UI_COMM_PACKET_SO_STR);
    strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

    sprintf(strSOData, "%d", bRequestParameter1);
    strcat(strUITx, strSOData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strSOData, "%d", bRequestParameter2);
    strcat(strUITx, strSOData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strSOData, "%d", bSSMNo);
    strcat(strUITx, strSOData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strSOData, "%d", bOutputNoAtSSM);
    strcat(strUITx, strSOData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    strcat(strUITx, UIConvertSOTypeToSOTypeStr(SSOData.bType));
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strSOData, "%d", SSOData.bLampType);
    strcat(strUITx, strSOData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strSOData, "%d", SSOData.bNoOfLamps);
    strcat(strUITx, strSOData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    strcat(strUITx, UIConvertEMToEMStr(SSOData.SFlags.bSOFailureEM));
  }
  else
  {
    UIFailureResponseSet();
  }
} /* UIPacketRespSO */

void UIPacketRespClearance(void)
{
  uint8_t bClearanceDuration;
  uint8_t bConflictSGNo;
  char strClearanceData[16];

  /* parameters: bSGNo, ith conflict bSGNo */
  if (SGClearanceGetbyIndex(bRequestParameter1 - 1,
                            bRequestParameter2 - 1,
                            &bClearanceDuration,
                            &bConflictSGNo) == FALSE)
  {
    UIFailureResponseSet();
  }
  else
  {
    strcat(strUITx, UI_COMM_PACKET_CLEARANCE_STR);
    strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

    sprintf(strClearanceData, "%d", bRequestParameter1);
    strcat(strUITx, strClearanceData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strClearanceData, "%d", bRequestParameter2);
    strcat(strUITx, strClearanceData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strClearanceData, "%d", bConflictSGNo);
    strcat(strUITx, strClearanceData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strClearanceData, "%d", bClearanceDuration);
    strcat(strUITx, strClearanceData);
  }
}

void UIPacketRespPhase(void)
{
  uint8_t bGroupNo;
  char strPhaseData[16];
  tSPhaseDef SPhaseData;

  memset(&SPhaseData, 0, sizeof(SPhaseData));

  if ((bRequestParameter1) && (bRequestParameter1 <= PhaseTotalGet()))
  {
    if (PhaseGet(bRequestParameter1 - 1, &SPhaseData) == FALSE)
    {
      UIFailureResponseSet();
    }
    else
    {
      strcat(strUITx, UI_COMM_PACKET_PHASE_STR);
      strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

      sprintf(strPhaseData, "%d", bRequestParameter1);
      strcat(strUITx, strPhaseData);
      strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
      sprintf(strPhaseData, "%d", SPhaseData.bMinDur);
      strcat(strUITx, strPhaseData);
      strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
      sprintf(strPhaseData, "%d", SPhaseData.bMaxDur);
      strcat(strUITx, strPhaseData);

      for (bGroupNo = 0; bGroupNo < SIGNAL_GROUPS_MAX; bGroupNo++)
      {
        if (SPhaseData.lGroups & laValue2Bit[bGroupNo])
        {
          strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
          sprintf(strPhaseData, "%d", (bGroupNo + 1));
          strcat(strUITx, strPhaseData);
        }
      }
    }
  }
  else
  {
    UIFailureResponseSet();
  }
} /* UIPacketRespPhase */

void UIPacketRespTransition(void)
{
  char strTransitionData[16];
  tSTransition STransition;

  if ((bRequestParameter2)
      && (bRequestParameter2 <= TransitionTotalGet(bRequestParameter1)))
  {
    memset(&STransition, 0, sizeof(tSTransition));
    TransitionGet(bRequestParameter1, bRequestParameter2, &STransition);

    strcat(strUITx, UI_COMM_PACKET_TRANSITION_STR);
    strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

    sprintf(strTransitionData, "%d", bRequestParameter1);
    strcat(strUITx, strTransitionData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strTransitionData, "%d", bRequestParameter2);
    strcat(strUITx, strTransitionData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strTransitionData, "%d", STransition.bFrom);
    strcat(strUITx, strTransitionData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strTransitionData, "%d", STransition.bValue1);
    strcat(strUITx, strTransitionData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strTransitionData, "%d", STransition.bRule);
    strcat(strUITx, strTransitionData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strTransitionData, "%d", STransition.bTo);
    strcat(strUITx, strTransitionData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strTransitionData, "%d", STransition.bValue2);
    strcat(strUITx, strTransitionData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strTransitionData, "%d", STransition.bPriority);
    strcat(strUITx, strTransitionData);
  }
  else
  {
    UIFailureResponseSet();
  }
} /* UIPacketRespTransition */

void UIPacketRespSignalPlan(void)
{
  char strData[8];
  tSSignalPlan SSignalPlan;

  memset(&SSignalPlan, 0, sizeof(SSignalPlan));

  if (SignalPlanGet(bRequestParameter1, &SSignalPlan) && (bRequestParameter1)
      && (bRequestParameter1 <= SignalPlanTotalGet()))
  {
    strcat(strUITx, UI_COMM_PACKET_SIGNAL_PLAN_STR);
    strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

    sprintf(strData, "%d", bRequestParameter1);
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strData, "%d", SSignalPlan.bSigProg);
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strData, "%d", SSignalPlan.bWorkPlan);
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strData, "%d", SSignalPlan.bWorkPlanEntry);
    strcat(strUITx, strData);
  }
  else
  {
    UIFailureResponseSet();
  }
}

void UIPacketRespSPPlanEntry(void)
{
  char strData[8];
  tSSPPlanEntry SSPPlanEntry; /* this entry will be added to signal program plan */

  memset(&SSPPlanEntry, 0, sizeof(tSSPPlanEntry));

  if ((bRequestParameter1) && (bRequestParameter2)
      && SigProgPlanEntryGet(bRequestParameter1,
                             bRequestParameter2 - 1,
                             &SSPPlanEntry)
      && (bRequestParameter2 <= SigProgPlanEntryTotalGet(bRequestParameter1)))
  {
    strcat(strUITx, UI_COMM_PACKET_SP_PLAN_ENTRY_STR);
    strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

    sprintf(strData, "%d", bRequestParameter1);
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strData, "%d", bRequestParameter2);
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strData, "%d", SSPPlanEntry.bHours);
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strData, "%d", SSPPlanEntry.bMinutes);
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strData, "%d", SSPPlanEntry.bSigProg);
    strcat(strUITx, strData);
  }
  else
  {
    UIFailureResponseSet();
  }
}

void UIPacketRespWPEntry(void)
{
  uint8_t bIndex;
  char strWorkPlanEntryData[16];
  tSWorkPlanEntryDef SWorkPlanEntry;

  memset(&SWorkPlanEntry, 0, sizeof(tSWorkPlanEntryDef));

  /* get workplan entry (bRequestParameter2) which belogs to workplan */
  /* (bRequestParameter1) */
  if ((bRequestParameter1) && (bRequestParameter2)
      && WorkPlanEntryGet(bRequestParameter1,
                          bRequestParameter2 - 1,
                          &SWorkPlanEntry)
      && (bRequestParameter2 <= WorkPlanEntryTotalGet(bRequestParameter1)))
  {
    strcat(strUITx, UI_COMM_PACKET_WP_ENTRY_STR);
    strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

    sprintf(strWorkPlanEntryData, "%d", bRequestParameter1);
    strcat(strUITx, strWorkPlanEntryData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strWorkPlanEntryData, "%d", bRequestParameter2);
    strcat(strUITx, strWorkPlanEntryData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strWorkPlanEntryData, "%d", SWorkPlanEntry.bHours);
    strcat(strUITx, strWorkPlanEntryData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strWorkPlanEntryData, "%d", SWorkPlanEntry.bMinutes);
    strcat(strUITx, strWorkPlanEntryData);

    for (bIndex = 0; bIndex < PhaseTotalGet(); bIndex++)
    {
      strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
      sprintf(strWorkPlanEntryData, "%d", SWorkPlanEntry.baPhaseDur[bIndex]);
      strcat(strUITx, strWorkPlanEntryData);
    }
  }
  else
  {
    UIFailureResponseSet();
  }
} /* UIPacketRespWPEntry */

void UIPacketRespWSEntry(void)
{
  char strWorkScheduleEntryData[16];
  tSWorkScheduleEntryDef SWorkScheduleEntry;

  memset(&SWorkScheduleEntry, 0, sizeof(SWorkScheduleEntry));

  if ((bRequestParameter1) && (WorkScheduleEntryGet(bRequestParameter1 - 1,
                                                    &SWorkScheduleEntry)
                               == TRUE) )
  {
    strcat(strUITx, UI_COMM_PACKET_WS_ENTRY_STR);
    strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

    sprintf(strWorkScheduleEntryData, "%d", bRequestParameter1);
    strcat(strUITx, strWorkScheduleEntryData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

    sprintf(strWorkScheduleEntryData, "%d", SWorkScheduleEntry.bDays);
    strcat(strUITx, strWorkScheduleEntryData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

    sprintf(strWorkScheduleEntryData, "%d", SWorkScheduleEntry.bStartDay);
    strcat(strUITx, strWorkScheduleEntryData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

    sprintf(strWorkScheduleEntryData, "%d", SWorkScheduleEntry.bStartMonth);
    strcat(strUITx, strWorkScheduleEntryData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

    sprintf(strWorkScheduleEntryData, "%d", SWorkScheduleEntry.bStartYear);
    strcat(strUITx, strWorkScheduleEntryData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

    sprintf(strWorkScheduleEntryData, "%d", SWorkScheduleEntry.bEndDay);
    strcat(strUITx, strWorkScheduleEntryData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

    sprintf(strWorkScheduleEntryData, "%d", SWorkScheduleEntry.bEndMonth);
    strcat(strUITx, strWorkScheduleEntryData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

    sprintf(strWorkScheduleEntryData, "%d", SWorkScheduleEntry.bEndYear);
    strcat(strUITx, strWorkScheduleEntryData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

    sprintf(strWorkScheduleEntryData, "%d", SWorkScheduleEntry.bWorkPlanNo);
    strcat(strUITx, strWorkScheduleEntryData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

    sprintf(strWorkScheduleEntryData, "%d", SWorkScheduleEntry.bSigProgPlan);
    strcat(strUITx, strWorkScheduleEntryData);
  }
  else
  {
    UIFailureResponseSet();
  }
} /* UIPacketRespWSEntry */

void UIPacketRespFlashPeriods(void)
{
  char strFlashPeriodData[16];

  strcat(strUITx, UI_COMM_PACKET_FLASH_PERIODS_STR);
  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

  sprintf(strFlashPeriodData, "%d",
          (FlashPeriodEmergencyGet() * FLASH_PERIOD_CONSTANT));
  strcat(strUITx, strFlashPeriodData);
}

void UIPacketRespOpenRelay(void)
{
  strcat(strUITx, UI_COMM_PACKET_OPEN_RELAY_STR);
  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

  strcat(strUITx, "SIM. START");

  SetLCDPowerRelayRequest(TRUE);
  fSimulationRunning = TRUE;
}

void UIPacketRespCloseRelay(void)
{
  strcat(strUITx, UI_COMM_PACKET_CLOSE_RELAY_STR);
  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

  strcat(strUITx, "SIM. FINISHED");

  SetLCDPowerRelayRequest(FALSE);
  fSimulationRunning = FALSE;
}

void UIPacketRespIORuntime(void)
{
  memset(strTempData, 0, sizeof(strTempData));

  strcat(strUITx, UI_COMM_PACKET_IO_RUNTIME_STR);
  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);
  switch (bRequestParameter1)
  {
      case INPUT_TYPE_DIGITAL:
      {
        snprintf(strTempData,
                 sizeof(strTempData),
                 "%u,%u,%u,%u,%u,%hu,%hu,%hu,%hu,%hu,%hu,%hu",
                 bRequestParameter1,
                 bRequestParameter2,
                 SRuntimes.SaInputRuntimes[bRequestParameter2
                                           - 1].bDemandCntInPer,
                 SRuntimes.SaInputRuntimes[bRequestParameter2
                                           - 1].bDemandCntInRed,
                 SRuntimes.SaInputRuntimes[bRequestParameter2
                                           - 1].bDemandCntInGreen,
                 (uint16_t) (SRuntimes.SaInputRuntimes[bRequestParameter2
                                                       - 1].sFDemandDurInPer
                             / 10),
                 (uint16_t) (SRuntimes.SaInputRuntimes[bRequestParameter2
                                                       - 1].sFDemandDurInRed
                             / 10),
                 (uint16_t) (SRuntimes.SaInputRuntimes[bRequestParameter2
                                                       - 1].sGapDurInGreen
                             / 10),
                 (uint16_t) (SRuntimes.SaInputRuntimes[bRequestParameter2
                                                       - 1].sGapDurInPer / 10),
                 (uint16_t) (SRuntimes.SaInputRuntimes[bRequestParameter2
                                                       - 1].sOccDurInGreen
                             / 10),
                 (uint16_t) (SRuntimes.SaInputRuntimes[bRequestParameter2
                                                       - 1].sOccDurInPer / 10),
                 (uint16_t) (SRuntimes.SaInputRuntimes[bRequestParameter2
                                                       - 1].sOccDurInRed / 10));
        strncat(strUITx, strTempData, sizeof(strUITx) - strlen(strUITx) - 1);
        break;
      }

      case INPUT_TYPE_DETECTOR:
      {
        snprintf(strTempData,
                 sizeof(strTempData),
                 "%u,%u,%u,%u,%u,%hu,%hu,%hu,%hu,%hu,%hu,%hu",
                 bRequestParameter1,
                 bRequestParameter2,
                 SRuntimes.SaDetectorRuntimes[bRequestParameter2
                                              - 1].bDemandCntInPer,
                 SRuntimes.SaDetectorRuntimes[bRequestParameter2
                                              - 1].bDemandCntInRed,
                 SRuntimes.SaDetectorRuntimes[bRequestParameter2
                                              - 1].bDemandCntInGreen,
                 (uint16_t) (SRuntimes.SaDetectorRuntimes[bRequestParameter2
                                                          - 1].sFDemandDurInPer
                             / 10),
                 (uint16_t) (SRuntimes.SaDetectorRuntimes[bRequestParameter2
                                                          - 1].sFDemandDurInRed
                             / 10),
                 (uint16_t) (SRuntimes.SaDetectorRuntimes[bRequestParameter2
                                                          - 1].sGapDurInGreen
                             / 10),
                 (uint16_t) (SRuntimes.SaDetectorRuntimes[bRequestParameter2
                                                          - 1].sGapDurInPer
                             / 10),
                 (uint16_t) (SRuntimes.SaDetectorRuntimes[bRequestParameter2
                                                          - 1].sOccDurInGreen
                             / 10),
                 (uint16_t) (SRuntimes.SaDetectorRuntimes[bRequestParameter2
                                                          - 1].sOccDurInPer
                             / 10),
                 (uint16_t) (SRuntimes.SaDetectorRuntimes[bRequestParameter2
                                                          - 1].sOccDurInRed
                             / 10));
        strncat(strUITx, strTempData, sizeof(strUITx) - strlen(strUITx) - 1);
        break;
      }
  } /* switch */
} /* UIPacketRespIORuntime */

void UIPacketInputDataManip(void)
{
  uint8_t bIOMNo;
  uint8_t bLDPNo;
  uint8_t bLDMNo;
  uint8_t bVirtualInputStates;
  uint16_t sVirtualInputStates;
  tSVirtualInput SVirtualInput;

  memset(&SVirtualInput, 0, sizeof(tSVirtualInput));

  if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                      &SVirtualInput.bType,
                      NULL,
                      sizeof(uint8_t)))
  {
    if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                        &SVirtualInput.bNumber,
                        NULL,
                        sizeof(uint8_t)))
    {
      if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, &SVirtualInput.bState,
                          NULL,
                          sizeof(uint8_t)))
      {
        bIOMNo = (SVirtualInput.bNumber - 1) / 16;
        bLDPNo = (SVirtualInput.bNumber - 1) / 8;

        switch (SVirtualInput.bType)
        {
            case INPUT_TYPE_DIGITAL:
            {
              tSCanDigitalIOInputs SIOInputs;

              memset(&SIOInputs, 0, sizeof(SIOInputs));

              /* Set state as safe automatically as IO doesn't support sending */
              /* safe states for the moment */
              SIOInputs.sInputSafeStates = 0xFFFF;
              SIOInputs.sInputStates =
                SaCanDigitalIOInputs[bIOMNo].sInputStates;                                /* Get previous */
              /* states */
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
              SaCanDetectorIOInputs[bIOMNo].fIsPhysicallyDriven = FALSE;
              sVirtualInputStates =
                SaCanDetectorIOInputs[bIOMNo].sLoopEmptyStates;
              (SVirtualInput.bState == FALSE)
              ?SetBitValue(sVirtualInputStates,
                           ((SVirtualInput.bNumber - 1) % 16))
              :ClearBitValue(sVirtualInputStates,
                             ((SVirtualInput.bNumber - 1) % 16));
              (bLDPNo % 2
               == 0) ? (bVirtualInputStates = (sVirtualInputStates & 0xFF))
              :(bVirtualInputStates = (sVirtualInputStates >> 8));
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
                SetLDInputs(bLDMNo + 1,
                            bIOMNo,
                            &bVirtualInputStates);
              }

              break;
            }

            default:
            {
              break;
            }
        } /* switch */
      }
    }
  }
} /* UIPacketInputDataManip */

void UIPacketRespDIManip(uint32_t lParam)
{
  tSCanDigitalIOInputs SIOInputs;

  memset(&SIOInputs, 0, sizeof(SIOInputs));

  /* Set state as safe automatically as IO doesn't support sending safe states */
  /* for the moment */
  SIOInputs.sInputSafeStates = 0xFFFF;
  SIOInputs.sInputStates = (uint16_t) ((uint16_t) (bRequestParameter1)
                                       | (uint16_t) ((uint16_t) (
                                                       bRequestParameter2) <<
                                                     8));
  SIOInputs.fIsPhysicallyDriven = FALSE;

  switch (lParam)
  {
      case UI_COMM_PACKET_INPUT_MANIP1:
      {
        strcat(strUITx, UI_COMM_PACKET_INPUT_MANIP1_STR);
        SetIOInputs(0, &SIOInputs);
        break;
      }

      case UI_COMM_PACKET_INPUT_MANIP2:
      {
        strcat(strUITx, UI_COMM_PACKET_INPUT_MANIP2_STR);
        SetIOInputs(1, &SIOInputs);
        break;
      }
  }

  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);
  strcat(strUITx, "OK");
}

void UIPacketRespLdManip1(void)
{
  strcat(strUITx, UI_COMM_PACKET_LD_MANIP1_STR);
  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

  strcat(strUITx, "OK");

  SaCanDetectorIOInputs[0].fIsPhysicallyDriven = FALSE;

  if (bRequestParameter1 != 0xFF)
  {
    if ((0xFF - bRequestParameter1) < 16)
    {
      bRequestParameter1 = 0xFF - bRequestParameter1;
      SetLDInputs(0, 0, &bRequestParameter1);
    }
    else
    {
      bRequestParameter1 = (0xFF - bRequestParameter1) / 16;
      SetLDInputs(1, 0, &bRequestParameter1);
    }
  }
  else
  {
    bRequestParameter1 = 0x00;

    SetLDInputs(0, 0, &bRequestParameter1);
    SetLDInputs(1, 0, &bRequestParameter1);
  }

  if (bRequestParameter2 != 0xFF)
  {
    if ((0xFF - bRequestParameter2) < 16)
    {
      bRequestParameter2 = 0xFF - bRequestParameter2;
      SetLDInputs(2, 0, &bRequestParameter2);
    }
    else
    {
      bRequestParameter2 = (0xFF - bRequestParameter2) / 16;
      SetLDInputs(3, 0, &bRequestParameter2);
    }
  }
  else
  {
    bRequestParameter2 = 0;

    SetLDInputs(2, 0, &bRequestParameter2);
    SetLDInputs(3, 0, &bRequestParameter2);
  }
} /* UIPacketRespLdManip1 */

void UIPacketRespLbManip1(void)
{
  strcat(strUITx, UI_COMM_PACKET_LB_MANIP1_STR);
  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

  strcat(strUITx, "OK");
}

void UIPacketRespLdManip2(void)
{
  strcat(strUITx, UI_COMM_PACKET_LD_MANIP2_STR);
  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

  strcat(strUITx, "OK");

  SaCanDetectorIOInputs[1].fIsPhysicallyDriven = FALSE;

  if (bRequestParameter1 != 0xFF)
  {
    if ((0xFF - bRequestParameter1) < 16)
    {
      bRequestParameter1 = 0xFF - bRequestParameter1;
      SetLDInputs(4, 1, &bRequestParameter1);
    }
    else
    {
      bRequestParameter1 = (0xFF - bRequestParameter1) / 16;
      SetLDInputs(5, 1, &bRequestParameter1);
    }
  }
  else
  {
    bRequestParameter1 = 0;

    SetLDInputs(4, 1, &bRequestParameter1);
    SetLDInputs(5, 1, &bRequestParameter1);
  }

  if (bRequestParameter2 != 0xFF)
  {
    if ((0xFF - bRequestParameter2) < 16)
    {
      bRequestParameter2 = 0xFF - bRequestParameter2;
      SetLDInputs(6, 1, &bRequestParameter2);
    }
    else
    {
      bRequestParameter2 = (0xFF - bRequestParameter2) / 16;
      SetLDInputs(7, 1, &bRequestParameter2);
    }
  }
  else
  {
    bRequestParameter2 = 0;

    SetLDInputs(6, 1, &bRequestParameter2);
    SetLDInputs(7, 1, &bRequestParameter2);
  }
} /* UIPacketRespLdManip2 */

void UIPacketRespLbManip2(void)
{
  strcat(strUITx, UI_COMM_PACKET_LB_MANIP2_STR);
  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

  strcat(strUITx, "OK");
}

void UIPacketRespSeqStep(void)
{
  uint8_t bSGIndex = 0;
  char strSignalSeqData[16];
  tSSeqDef SSeqDef;

  if (SeqRead(bRequestParameter1 - 1, &SSeqDef))
  {
    if (bRequestParameter2 <= SSeqDef.bNoOfSteps)
    {
      strcat(strUITx, UI_COMM_PACKET_SEQUENCE_STEP_STR);
      strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);
      sprintf(strSignalSeqData, "%d", bRequestParameter1);     /* sequence no */
      strcat(strUITx, strSignalSeqData);
      strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
      sprintf(strSignalSeqData, "%d", bRequestParameter2);     /* step in sequence */
      strcat(strUITx, strSignalSeqData);
      strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
      sprintf(strSignalSeqData,
              "%d",
              SSeqDef.baDurations[bRequestParameter2 - 1]);
      strcat(strUITx, strSignalSeqData);

      /* while defining the signal sequences we must use the all signal groups */
      for (bSGIndex = 0; bSGIndex < SGTotalGet(); bSGIndex++)
      {
        /* bRequestParameter2 stores the number of the step whose information */
        /* wanted we do not deal with the other step values of the SSignalSeq */
        strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
        sprintf(strSignalSeqData, "%d", bSGIndex + 1);
        strcat(strUITx, strSignalSeqData);
        strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
        if (bSGIndex % 2 == 0)
        {
          sprintf(strSignalSeqData, "%d",
                  (SSeqDef.baSignals[bRequestParameter2 - 1][bSGIndex / 2]
                   & 0x0F));
        }
        else
        {
          sprintf(strSignalSeqData, "%d",
                  ((SSeqDef.baSignals[bRequestParameter2 - 1][bSGIndex / 2]
                    & 0xF0) >> 4));
        }

        strcat(strUITx, strSignalSeqData);
      }

      return;
    }
  }

  UIFailureResponseSet();
} /* UIPacketRespSeqStep */

void UIPacketRespTime(void)
{
  tSTime STimeNow;
  char strTimeData[16];

  memset(&STimeNow, 0, sizeof(STimeNow));

  strcat(strUITx, UI_COMM_PACKET_TIME_STR);
  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

  TimeGet(&STimeNow);

  sprintf(strTimeData, "%d", STimeNow.SCurrentDate.Date);
  strcat(strUITx, strTimeData);
  strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
  sprintf(strTimeData, "%d", STimeNow.SCurrentDate.Month);
  strcat(strUITx, strTimeData);
  strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
  sprintf(strTimeData, "%d", TimeFullYearCalc(&STimeNow));
  strcat(strUITx, strTimeData);
  strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
  sprintf(strTimeData, "%d", STimeNow.SCurrentTime.Hours);
  strcat(strUITx, strTimeData);
  strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
  sprintf(strTimeData, "%d", STimeNow.SCurrentTime.Minutes);
  strcat(strUITx, strTimeData);
  strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
  sprintf(strTimeData, "%d", STimeNow.SCurrentTime.Seconds);
  strcat(strUITx, strTimeData);
}

void UIPacketRespSGSignals(void)
{
  uint8_t bSGNo;
  uint8_t bSignalGroupTotal;
  char strSGRuntimeData[16];
  uint8_t bCurSig;

  /* ISSD function */
  if (GetFunctionConfByIndex(LIC_ISSD) || fSimulationRunning)
  {
    strcat(strUITx, UI_COMM_PACKET_SIGNALS_STR);
    strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

    bSignalGroupTotal = SGTotalGet();

    for (bSGNo = 0; bSGNo < bSignalGroupTotal; bSGNo++)
    {
      bCurSig = SGSignalGet(bSGNo);

      if (bCurSig)
      {
        sprintf(strSGRuntimeData, "%d", bCurSig);
        strcat(strUITx, strSGRuntimeData);
      }
      else
      {
        strcat(strUITx, UI_COMM_STR_UNDEFINED);
      }

      if (bSGNo != bSignalGroupTotal - 1)
      {
        strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
      }
    }
  }
  else
  {
    SetLCDState(LCD_STATE_UNLICENSED_USAGE);

    UIFailureResponseSet();
  } /* ISSD function */
} /* UIPacketRespSGSignals */

void UIPacketRespCurrentRuntimeInfo(void)
{
  /* ISSD function */
  if (GetFunctionConfByIndex(LIC_ISSD) || fSimulationRunning)
  {
    strcat(strUITx, UI_COMM_PACKET_CURRENT_RUNTIME_INFO_STR);
    strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

    switch (StateCurrentGet())
    {
        case STATES_ANY:
        {
          strcat(strUITx, "A");
          break;
        }

        case STATES_NO_CONTROL:
        {
          strcat(strUITx, "C");
          break;
        }

        case STATES_FLASH:
        {
          strcat(strUITx, "F");
          break;
        }

        case STATES_CLOSED:
        {
          strcat(strUITx, "R");
          break;
        }

        case STATES_SEQ:
        {
          char strPartialData[50];

          sprintf(strPartialData,
                  "S,%d,%d,%d,%d,%d,%d,%d,%d",
                  SeqCurrentGet(),
                  SeqTotalGet(),
                  SeqCurrentStepGet() + 1,
                  SeqCurStepNumTotalGet(),
                  SeqCurrentStepCurrentDurationGet(),
                  SeqCurrentStepDurationGet(),
                  SeqDurCurGet(),
                  SeqDurGet(SeqCurrentGet() - 1) + SeqTotalExtDurGet());

          strncat(strUITx, strPartialData, strlen(strPartialData));
          break;
        }

        case STATES_PHASE:
        {
          char strPartialData[50];

          sprintf(strPartialData,
                  "P,%d,%d,%d,%d",
                  ProgramCurrentNoGet(),
                  PhaseTotalGet(),
                  PhaseElapsedDurGet(ProgramCurrentNoGet() - 1) + 1,
                  WorkPlanPhaseDurGet(ProgramCurrentNoGet() - 1));

          strncat(strUITx, strPartialData, strlen(strPartialData));
          break;
        }

        case STATES_PHASE_TRANSITION:
        {
          char strPartialData[50];

          sprintf(strPartialData,
                  "T,%d,%d",
                  ProgramCurrentNoGet(),
                  ProgramTargetNoGet());

          strncat(strUITx,
                  strPartialData,
                  strlen(strPartialData));
          break;
        }
    } /* switch */
  }
  else
  {
    SetLCDState(LCD_STATE_UNLICENSED_USAGE);

    UIFailureResponseSet();
  } /* ISSD function */
} /* UIPacketRespCurrentRuntimeInfo */

void UIPacketPhaseDurationChange(void)
{
  uint8_t bPhaseNo = 0;
  int8_t sbDur = 0;

  /* ISSD function */
  if (GetFunctionConfByIndex(LIC_ISSD) || fSimulationRunning)
  {
    if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, &bPhaseNo, NULL,
                        sizeof(uint8_t)))
    {
      if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                          &sbDur,
                          NULL,
                          sizeof(int8_t)))
      {
        PhaseExtDurSet((bPhaseNo - 1),
                       sbDur);
      }
    }
  }
  else
  {
    SetLCDState(LCD_STATE_UNLICENSED_USAGE);

    UIFailureResponseSet();
  } /* ISSD function */
}

void UIPacketFuncConfChange(void)
{
  uint8_t bFuncIdx = 0xFF, bConfVal = 0;

  if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                      &bFuncIdx,
                      NULL,
                      sizeof(uint8_t)))
  {
    if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, &bConfVal, NULL,
                        sizeof(int8_t)))
    {
      SetFunctionConfByIndex(bFuncIdx, bConfVal);
      WriteFunctionConf();
    }
  }
}

void UIPacketSetSystemStartTime(void)
{
  if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                      &bRequestParameter1,
                      NULL,
                      sizeof(uint8_t)))
  {
    if (UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                        &bRequestParameter2,
                        NULL,
                        sizeof(uint8_t)))
    {
      switch (bRequestParameter1)
      {
          case 0:
          {
            switch (bRequestParameter2)
            {
                case SYSTEM_START_TIME_RESET:
                {
                  SystemStartTimeInit();
                  SystemStartTimeSave();
                  break;
                }

                case SYSTEM_START_TIME_SET:
                {
                  SystemStartTimeStart();
                  break;
                }

                default:
                {
                  UIFailureResponseSet();
                  break;
                }
            }

            break;
          }

          case 1:
          {
            if ((bRequestParameter2 > 0) && (bRequestParameter2 <= 10 * 24) )      /* max 10 days */
            {
              SystemStartTimeInit();
              SystemStartTimeSetMinUpHours(bRequestParameter2);
            }
            else
            {
              UIFailureResponseSet();
            }

            break;
          }

          default:
          {
            UIFailureResponseSet();
            break;
          }
      } /* switch */
    }
  }
} /* UIPacketSetSystemStartTime */

#ifdef DEBUG

void UIPacketDebug(void)
{
}

void UIPacketRespDebug(void)
{
  switch (bRequestParameter1)
  {
      case 1:
      {
        switch (bRequestParameter2)
        {
            case 0:
            {
              SetStandbyState(FALSE);
              break;
            }

            case 1:
            {
              SetStandbyState(TRUE);
              NotifyStandbyState();
              break;
            }
        }
      }

      case 2:
      {
        switch (bRequestParameter2)
        {
            case 0:
            {
              ClearDeviceUID();
              break;
            }

            case 1:
            {
              SetDeviceUID();
              break;
            }
        }

        break;
      }
  }
} /* UIPacketRespDebug */

#endif /* ifdef DEBUG */

void UIPacketIAP(uint8_t bSrc, char *strData)
{
  char *strLength = strtok(strData, ",");

  if (strLength != NULL)
  {
    uint8_t bLen = strlen(strLength);
    uint8_t bDataLen = atoi(strLength);

    IAPRequest(bSrc, bDataLen + IAP_PACKET_OVERHEAD_SIZE,
               (void *) &strData[bLen + 1]);

    bUICommResponse = UI_COMM_RESPONSE_PACKET;
    lRequestedPacket = UI_COMM_PACKET_IAP;
  }
}

uint8_t UIIPv4FromStringGet(char *pStrSrc, tpSMCSIPv4 pSIP)
{
  uint8_t bIPPartCount = 0;
  unsigned int baIPParts[MCS_MAX_IPV4_PARTS] = { 0 };

  bIPPartCount = sscanf(pStrSrc,
                        "%u.%u.%u.%u",
                        &baIPParts[0],
                        &baIPParts[1],
                        &baIPParts[2],
                        &baIPParts[3]);
  if ((bIPPartCount != 4) || (baIPParts[0] > 255) || (baIPParts[1] > 255)
      || (baIPParts[2] > 255) || (baIPParts[3] > 255) )
  {
    UIFailureResponseSet();

    return FALSE;
  }

  pSIP->bAddress0 = (uint8_t) baIPParts[0];
  pSIP->bAddress1 = (uint8_t) baIPParts[1];
  pSIP->bAddress2 = (uint8_t) baIPParts[2];
  pSIP->bAddress3 = (uint8_t) baIPParts[3];

  return TRUE;
}

uint8_t UIMACFromStringGet(char *pStrSrc, tpSMCSMACAddress pSMAC)
{
  uint8_t bMACPartCount = 0;
  unsigned int baMACParts[MCS_MAX_MAC_PARTS] = { 0 };

  bMACPartCount = sscanf(pStrSrc,
                         "%2X:%2X:%2X:%2X:%2X:%2X",
                         &baMACParts[0],
                         &baMACParts[1],
                         &baMACParts[2],
                         &baMACParts[3],
                         &baMACParts[4],
                         &baMACParts[5]);
  if ((bMACPartCount != 6) || (baMACParts[0] > 255) || (baMACParts[1] > 255)
      || (baMACParts[2] > 255) || (baMACParts[3] > 255)
      || (baMACParts[4] > 255) || (baMACParts[5] > 255) )
  {
    UIFailureResponseSet();

    return FALSE;
  }

  pSMAC->bAddress0 = (uint8_t) baMACParts[0];
  pSMAC->bAddress1 = (uint8_t) baMACParts[1];
  pSMAC->bAddress2 = (uint8_t) baMACParts[2];
  pSMAC->bAddress3 = (uint8_t) baMACParts[3];
  pSMAC->bAddress4 = (uint8_t) baMACParts[4];
  pSMAC->bAddress5 = (uint8_t) baMACParts[5];

  return TRUE;
}

void UIPacketMCSConInfo(void)
{
  char strData[APN_MAX_SIZE + 1];
  uint32_t ulID = 0;
  tSMCSConInfo SNewMCSConInfo;

  memset(&SNewMCSConInfo, 0, sizeof(SNewMCSConInfo));

  SNewMCSConInfo.bModuleType = MCSGetModemType();

  if (!UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER, &ulID, NULL, sizeof(ulID)))
  {
    UIFailureResponseSet();

    return;
  }

  if (ulID > UINT32_MAX)
  {
    UIFailureResponseSet();

    return;
  }

  SNewMCSConInfo.SSNMPInfo.lDeviceID = ulID;

  if (!UIRxNextDataGet(UI_COMM_DATA_TYPE_STRING, NULL, strData,
                       sizeof(strData)))
  {
    UIFailureResponseSet();

    return;
  }

  if (!UIMACFromStringGet(strData, &SNewMCSConInfo.SMACAddress))
  {
    UIFailureResponseSet();

    return;
  }

  if (!UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                       &bRequestParameter1,
                       NULL,
                       sizeof(bRequestParameter1)))
  {
    UIFailureResponseSet();

    return;
  }

  if (bRequestParameter1 > 1)
  {
    UIFailureResponseSet();

    return;
  }

  SNewMCSConInfo.SFlags.fEthStaticIp = bRequestParameter1;

  if (SNewMCSConInfo.SFlags.fEthStaticIp)
  {
    memset(strData, 0, sizeof(strData));
    if (!UIRxNextDataGet(UI_COMM_DATA_TYPE_STRING,
                         NULL,
                         strData,
                         sizeof(strData)))
    {
      UIFailureResponseSet();

      return;
    }

    if (!UIIPv4FromStringGet(strData, &SNewMCSConInfo.SEthLocalIPv4))
    {
      UIFailureResponseSet();

      return;
    }

    memset(strData, 0, sizeof(strData));
    if (!UIRxNextDataGet(UI_COMM_DATA_TYPE_STRING,
                         NULL,
                         strData,
                         sizeof(strData)))
    {
      UIFailureResponseSet();

      return;
    }

    if (!UIIPv4FromStringGet(strData, &SNewMCSConInfo.SEthSubnetMask))
    {
      UIFailureResponseSet();

      return;
    }

    memset(strData, 0, sizeof(strData));
    if (!UIRxNextDataGet(UI_COMM_DATA_TYPE_STRING,
                         NULL,
                         strData,
                         sizeof(strData)))
    {
      UIFailureResponseSet();

      return;
    }

    if (!UIIPv4FromStringGet(strData, &SNewMCSConInfo.SEthGateway))
    {
      UIFailureResponseSet();

      return;
    }
  }

  memset(strData, 0, sizeof(strData));
  if (!UIRxNextDataGet(UI_COMM_DATA_TYPE_STRING, NULL, strData,
                       sizeof(strData)))
  {
    UIFailureResponseSet();

    return;
  }

  if (!UIIPv4FromStringGet(strData, &SNewMCSConInfo.SPrimaryDNSServer))
  {
    UIFailureResponseSet();

    return;
  }

  memset(strData, 0, sizeof(strData));
  if (!UIRxNextDataGet(UI_COMM_DATA_TYPE_STRING, NULL, strData,
                       sizeof(strData)))
  {
    UIFailureResponseSet();

    return;
  }

  if (!UIIPv4FromStringGet(strData, &SNewMCSConInfo.SSecondaryDNSServer))
  {
    UIFailureResponseSet();

    return;
  }

  if (!UIRxNextDataGet(UI_COMM_DATA_TYPE_STRING,
                       NULL,
                       SNewMCSConInfo.SSNMPInfo.strReadCommunityName,
                       sizeof(SNewMCSConInfo.SSNMPInfo.strReadCommunityName)))
  {
    UIFailureResponseSet();

    return;
  }

  if (strlen(SNewMCSConInfo.SSNMPInfo.strReadCommunityName)
      < SNMP_COMMUNITY_NAME_MIN_SIZE)
  {
    UIFailureResponseSet();

    return;
  }

  if (!UIRxNextDataGet(UI_COMM_DATA_TYPE_STRING,
                       NULL,
                       SNewMCSConInfo.SSNMPInfo.strWriteCommunityName,
                       sizeof(SNewMCSConInfo.SSNMPInfo.strWriteCommunityName)))
  {
    UIFailureResponseSet();

    return;
  }

  if (strlen(SNewMCSConInfo.SSNMPInfo.strWriteCommunityName)
      < SNMP_COMMUNITY_NAME_MIN_SIZE)
  {
    UIFailureResponseSet();

    return;
  }

  if (!UIRxNextDataGet(UI_COMM_DATA_TYPE_STRING,
                       NULL,
                       SNewMCSConInfo.SSNMPInfo.strTrapCommunityName,
                       sizeof(SNewMCSConInfo.SSNMPInfo.strTrapCommunityName)))
  {
    UIFailureResponseSet();

    return;
  }

  if (strlen(SNewMCSConInfo.SSNMPInfo.strTrapCommunityName)
      < SNMP_COMMUNITY_NAME_MIN_SIZE)
  {
    UIFailureResponseSet();

    return;
  }

  if (!UIRxNextDataGet(UI_COMM_DATA_TYPE_STRING,
                       NULL,
                       SNewMCSConInfo.SSNMPInfo.strV3EngineId,
                       sizeof(SNewMCSConInfo.SSNMPInfo.strV3EngineId)))
  {
    UIFailureResponseSet();

    return;
  }

  if (strlen(SNewMCSConInfo.SSNMPInfo.strV3EngineId)
      < SNMPV3_ENGINE_ID_MIN_SIZE)
  {
    UIFailureResponseSet();

    return;
  }

  if (!UIRxNextDataGet(UI_COMM_DATA_TYPE_STRING,
                       NULL,
                       SNewMCSConInfo.SSNMPInfo.strV3Username,
                       sizeof(SNewMCSConInfo.SSNMPInfo.strV3Username)))
  {
    UIFailureResponseSet();

    return;
  }

  if (strlen(SNewMCSConInfo.SSNMPInfo.strV3Username) < SNMPV3_USERNAME_MIN_SIZE)
  {
    UIFailureResponseSet();

    return;
  }

  if (!UIRxNextDataGet(UI_COMM_DATA_TYPE_STRING,
                       NULL,
                       SNewMCSConInfo.SSNMPInfo.strV3AuthPassword,
                       sizeof(SNewMCSConInfo.SSNMPInfo.strV3AuthPassword)))
  {
    UIFailureResponseSet();

    return;
  }

  if (strlen(SNewMCSConInfo.SSNMPInfo.strV3AuthPassword)
      < SNMPV3_PASSWORD_MIN_SIZE)
  {
    UIFailureResponseSet();

    return;
  }

  if (!UIRxNextDataGet(UI_COMM_DATA_TYPE_STRING,
                       NULL,
                       SNewMCSConInfo.SSNMPInfo.strV3PrivPassword,
                       sizeof(SNewMCSConInfo.SSNMPInfo.strV3PrivPassword)))
  {
    UIFailureResponseSet();

    return;
  }

  if (strlen(SNewMCSConInfo.SSNMPInfo.strV3PrivPassword)
      < SNMPV3_PASSWORD_MIN_SIZE)
  {
    UIFailureResponseSet();

    return;
  }

  if (!UIRxNextDataGet(UI_COMM_DATA_TYPE_STRING,
                       NULL,
                       SNewMCSConInfo.SSNMPInfo.strTrapDestination,
                       sizeof(SNewMCSConInfo.SSNMPInfo.strTrapDestination)))
  {
    UIFailureResponseSet();

    return;
  }

  if (!UIRxNextDataGet(UI_COMM_DATA_TYPE_INTEGER,
                       &bRequestParameter1,
                       NULL,
                       sizeof(bRequestParameter1)))
  {
    UIFailureResponseSet();

    return;
  }

  if (bRequestParameter1 > 1)
  {
    UIFailureResponseSet();

    return;
  }

  SNewMCSConInfo.SSNMPInfo.bTrapVersion = bRequestParameter1;

  SNewMCSConInfo.bInitialized = MCS_CON_INFO_INITIALIZED;

  if (memcmp(&SNewMCSConInfo, MCSGetConInfoPtr(), sizeof(SNewMCSConInfo)) != 0)
  {
    MCSSetConInfo(&SNewMCSConInfo);
    if (MCSWriteConInfo())
    {
      MCSSetConInfoChanged(TRUE);
    }
    else
    {
      UIFailureResponseSet();
    }
  }
} /* UIPacketMCSConInfo */

void UIPacketChangeWorkmode(void)
{
  char bRequestedMode;

  /* ISSD function */
  if (GetFunctionConfByIndex(LIC_ISSD) || fSimulationRunning)
  {
    if (UIRxNextDataGet(UI_COMM_DATA_TYPE_STRING, NULL, &bRequestedMode,
                        sizeof(char)))
    {
      switch (bRequestedMode)
      {
          case 'F':
          {
            UserStateReqSet(STATES_FLASH);
            break;
          }

          case 'R':
          {
            UserStateReqSet(STATES_CLOSED);
            break;
          }

          case 'C':
          {
            UserStateReqSet(STATES_NO_CONTROL);
            break;
          }

          case 'N':
          {
            UserStateReqFree();
            break;
          }
      }
    }
  }
  else
  {
    SetLCDState(LCD_STATE_UNLICENSED_USAGE);

    UIFailureResponseSet();
  } /* ISSD function */
} /* UIPacketChangeWorkmode */

void UIPacketRespLogNext(tpSUIRequest pRequest)
{
  tSLogRecord SLogRecord;

  memset(&SLogRecord, 0, sizeof(tSLogRecord));

  strcat(strUITx, UI_COMM_PACKET_LOG_NEXT_STR);
  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

  if (LogEventNew(sUILogReadIndex) != LOG_NO_NEW_LOG)
  {
    switch (pRequest->bReqId)
    {
        case UI_REQ_TYPE_SERIAL:
        case UI_REQ_TYPE_USB:
        {
          /* the last parameter is changable, it may be the read index of the task */
          /* or an index value that is sent from the client */
          /* LogRequest understands this division using the first operation */
          /* parameter */
          MCSAsynchLogSeize();

          if (LogRequest(LOG_REQ_READ_NEXT,
                         &SLogRecord,
                         0,
                         0,
                         0,
                         0,
                         sUILogReadIndex))
          {
            sUILogReadIndex++;
            sUILogReadIndex %= LOG_RECORDS_MAX;

            UILogRecordResponseSet(sUILogReadIndex, &SLogRecord);
          }
          else
          {
            UIFailureResponseSet();
          }

          MCSAsynchLogRelease();
          break;
        }

        case UI_REQ_TYPE_TCP_CLIENT:
        {
          /* the last parameter is changable, it may be the read index of the task */
          /* or an index value that is sent from the client */
          /* LogRequest understands this division using the first operation */
          /* parameter */
          MCSAsynchLogSeize();
          uint16_t sLogReadIndex = MCSAsynchGeLogReadIndex();

          if (LogRequest(LOG_REQ_READ_NEXT,
                         &SLogRecord,
                         0,
                         0,
                         0,
                         0,
                         sLogReadIndex))
          {
            sLogReadIndex++;
            sLogReadIndex %= LOG_RECORDS_MAX;
            MCSAsynchSetLogReadIndex(sLogReadIndex);

            UILogRecordResponseSet(sLogReadIndex, &SLogRecord);
          }
          else
          {
            UIFailureResponseSet();
          }

          MCSAsynchLogRelease();
          break;
        }
    } /* switch */
  }
  else
  {
    UIFailureResponseSet();
  }
} /* UIPacketRespLogNext */

void UIPacketRespLogFrom(void)
{
  tSLogRecord SLogRecord;

  memset(&SLogRecord, 0, sizeof(tSLogRecord));

  strcat(strUITx, UI_COMM_PACKET_LOG_FROM_STR);
  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

  /* the indexed log may be empty in the beginning, 0s will be sent as reply */
  /* assume this is not a problem, it is meaningless to compare the index value */
  /* with the log write index value because log has a cyclic structure */
  if (sRequestParameter3 && (sRequestParameter3 <= LOG_RECORDS_MAX))
  {
    uint16_t sIndex = (sRequestParameter3 - 1);

    MCSAsynchLogSeize();
    if (LogRequest(LOG_REQ_READ_FROM, &SLogRecord, 0, 0, 0, 0, sIndex) == TRUE)
    {
      UILogRecordResponseSet((sRequestParameter3 - 1), &SLogRecord);
    }
    else
    {
      UIFailureResponseSet();
    }

    MCSAsynchLogRelease();
  }
  else
  {
    UIFailureResponseSet();
  }
}

void UIPacketRespLogLastIndex(void)
{
  char strBuffer[8];

  if (LogExists())
  {
    strcat(strUITx, UI_COMM_PACKET_LOG_LAST_INDEX_STR);
    strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

    sprintf(strBuffer, "%d", LogEventNew(LOG_GET_WRITE_INDEX_VALUE));
    strcat(strUITx, strBuffer);
  }
  else
  {
    UIFailureResponseSet();
  }
}

void UIPacketRespUsernames(void)
{
  uint8_t bUserIndex = 0;
  char strUsernameData[20];
  uint8_t fFirstData = TRUE;

  strcat(strUITx, UI_COMM_PACKET_GET_USERNAMES_STR);
  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

  while (bUserIndex < LCD_USERS_MAX)
  {
    /* add admin username to packet */
    if (UserAuthServiceIsAdminValid(&g_userAuthService) != 0U)
    {
      /* if it is the first data don't add the separator */
      if (fFirstData == FALSE)
      {
        strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
      }
      else
      {
        fFirstData = FALSE;
      }

      sprintf(strUsernameData,
              "ADMIN,%04d",
              UserAuthServiceGetAdminUsername(&g_userAuthService));
      strcat(strUITx, strUsernameData);
    }

    /* add guest username to packet */
    if (GetGuestValidity() == TRUE)
    {
      if (fFirstData == FALSE)
      {
        strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
      }
      else
      {
        fFirstData = FALSE;
      }

      sprintf(strUsernameData,
              "GUEST,%04d",
              USER_AUTH_GUEST_USERNAME);
      strcat(strUITx, strUsernameData);
    }

    bUserIndex++;
  }
} /* UIPacketRespUsernames */

void UIPacketRespLCDLanguage(void)
{
  strcat(strUITx, UI_COMM_PACKET_LCD_LANGUAGE_STR);
  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

  if (LCDLanguageRead())
  {
    strcat(strUITx, UIConvertLanguageToLanguageStr(LCDLanguageGet()));
  }
  else
  {
    strcat(strUITx, UI_COMM_PACKET_OPERATION_ERROR_STR);
  }
}

void UIPacketRespCommConfig(void)
{
  strcat(strUITx, UI_COMM_PACKET_COMM_CONFIG_STR);
  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

  /* configuration is by means of checksum existence */
  if (fUIChecksumEnabled == TRUE)
  {
    strcat(strUITx, "TRUE");
  }
  else
  {
    strcat(strUITx, "FALSE");
  }
}

void UIPacketRespSGTotal(void)
{
  char strSGNumberData[4];

  strcat(strUITx, UI_COMM_PACKET_SG_NUMBER_STR);
  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

  sprintf(strSGNumberData, "%d", SGTotalGet());

  strcat(strUITx, strSGNumberData);
}

void UIPacketRespWSTotal(void)
{
  char strWSData[4];

  strcat(strUITx, UI_COMM_PACKET_WS_NUMBER_STR);
  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

  sprintf(strWSData, "%d", WorkScheduleTotalGet());

  strcat(strUITx, strWSData);
}

void UIPacketRespSinTotal(void)
{
  char strSignalNumberData[4];

  strcat(strUITx, UI_COMM_PACKET_SIN_NUMBER_STR);
  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

  sprintf(strSignalNumberData, "%d", SignalTotalGet());

  strcat(strUITx, strSignalNumberData);
}

void UIPacketRespSignalPlanTotal(void)
{
  char strSignalPlanNumberData[4];

  strcat(strUITx, UI_COMM_PACKET_SP_NUMBER_STR);
  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

  sprintf(strSignalPlanNumberData, "%d", SignalPlanTotalGet());

  strcat(strUITx, strSignalPlanNumberData);
}

void UIPacketRespConflictTotal(void)
{
  char strConflictNumberData[6];
  uint8_t bSgIndex;

  strcat(strUITx, UI_COMM_PACKET_CONFLICT_NUMBER_STR);
  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

  for (bSgIndex = 0; bSgIndex < SGTotalGet(); bSgIndex++)
  {
    sprintf(strConflictNumberData, "%d", (bSgIndex + 1));
    strcat(strUITx, strConflictNumberData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strConflictNumberData, "%d", SGClearanceTotalGet(bSgIndex));
    strcat(strUITx, strConflictNumberData);

    if (bSgIndex != SGTotalGet() - 1)
    {
      strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    }
  }
}

void UIPacketRespSOTotal(void)
{
  char strSONumberData[4];
  uint8_t bSOTotal;

  if (SGSOTotalGet(bRequestParameter1 - 1, &bSOTotal) == TRUE)
  {
    strcat(strUITx, UI_COMM_PACKET_SO_NUMBER_STR);
    strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

    sprintf(strSONumberData, "%d", bSOTotal);
    strcat(strUITx, strSONumberData);
  }
  else
  {
    UIFailureResponseSet();
  }
}

void UIPacketRespSGConflictTotal(void)
{
  char strConflictNumberData[4];
  uint8_t bConflictTotal;

  if ((bRequestParameter1) && (SGConflictTotalGet(bRequestParameter1 - 1,
                                                  &bConflictTotal) == TRUE) )
  {
    strcat(strUITx, UI_COMM_PACKET_CFL_NUMBER_STR);
    strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

    sprintf(strConflictNumberData, "%d", bConflictTotal);
    strcat(strUITx, strConflictNumberData);
  }
  else
  {
    UIFailureResponseSet();
  }
}

void UIPacketRespSPTotal(void)
{
  char strSPNumberData[4];

  strcat(strUITx, UI_COMM_PACKET_SPR_NUMBER_STR);
  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

  sprintf(strSPNumberData, "%d", SigProgTotalGet());

  strcat(strUITx, strSPNumberData);
}

void UIPacketRespPhaseTotal(void)
{
  char strPHANumberData[4];

  strcat(strUITx, UI_COMM_PACKET_PHA_NUMBER_STR);
  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

  sprintf(strPHANumberData, "%d", PhaseTotalGet());

  strcat(strUITx, strPHANumberData);
}

void UIPacketRespWPTotal(void)
{
  char strWPNumberData[4];
  uint8_t bWPIndex;

  strcat(strUITx, UI_COMM_PACKET_WP_NUMBER_STR);
  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

  for (bWPIndex = 0; bWPIndex < WorkPlanTotalGet(); bWPIndex++)
  {
    sprintf(strWPNumberData, "%d", (bWPIndex + 1));
    strcat(strUITx, strWPNumberData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strWPNumberData, "%d", WorkPlanEntryTotalGet(bWPIndex + 1));
    strcat(strUITx, strWPNumberData);

    if (bWPIndex != WorkPlanTotalGet() - 1)
    {
      strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    }
  }
}

void UIPacketRespWPEntryTotal(void)
{
  char strWPEntriesNumberData[4];
  uint8_t bWPEntriesNumber;

  bWPEntriesNumber = WorkPlanEntryTotalGet(bRequestParameter1);
  if (bRequestParameter1)
  {
    strcat(strUITx, UI_COMM_PACKET_WP_ENTRY_NUMBER_STR);
    strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

    sprintf(strWPEntriesNumberData, "%d", bWPEntriesNumber);

    strcat(strUITx, strWPEntriesNumberData);
  }
  else
  {
    strcat(strUITx, UI_COMM_PACKET_OPERATION_ERROR_STR);
  }
}

void UIPacketRespSPPTotal(void)
{
  char strSPPNumberData[4];
  uint8_t bSPPIndex;

  strcat(strUITx, UI_COMM_PACKET_SPP_NUMBER_STR);
  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

  for (bSPPIndex = 0; bSPPIndex < SigProgPlanTotalGet(); bSPPIndex++)
  {
    sprintf(strSPPNumberData, "%d", (bSPPIndex + 1));
    strcat(strUITx, strSPPNumberData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strSPPNumberData, "%d", SigProgPlanEntryTotalGet(bSPPIndex + 1));
    strcat(strUITx, strSPPNumberData);

    if (bSPPIndex != SigProgPlanTotalGet() - 1)
    {
      strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    }
  }
}

void UIPacketRespSeqTotal(void)
{
  char strSEQNumberData[4];

  strcat(strUITx, UI_COMM_PACKET_SEQ_NUMBER_STR);
  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);
  sprintf(strSEQNumberData, "%d", SeqTotalGet());
  strcat(strUITx, strSEQNumberData);
}

void UIPacketRespSeqStepTotal(void)
{
  char strStepNumberData[4];

  strcat(strUITx, UI_COMM_PACKET_SEQ_STEP_NUMBER_STR);
  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);
  sprintf(strStepNumberData, "%d", SeqStepTotalGet(bRequestParameter1 - 1));
  strcat(strUITx, strStepNumberData);
}

void UIPacketRespDetectorTotal(void)
{
  char strNumberData[4];

  strcat(strUITx, UI_COMM_PACKET_DET_NUMBER_STR);
  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);
  sprintf(strNumberData, "%d", InputTotalGet(INPUT_TYPE_DETECTOR));
  strcat(strUITx, strNumberData);
}

void UIPacketRespInputDigitalTotal(void)
{
  char strNumberData[4];

  strcat(strUITx, UI_COMM_PACKET_INPUT_NUMBER_STR);
  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);
  sprintf(strNumberData, "%d", InputTotalGet(INPUT_TYPE_DIGITAL));
  strcat(strUITx, strNumberData);
}

void UIPacketRespOutputTotal(void)
{
  char strNumberData[4];

  strcat(strUITx, UI_COMM_PACKET_OUTPUT_NUMBER_STR);
  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);
  sprintf(strNumberData, "%d", OutputTotalGet());
  strcat(strUITx, strNumberData);
}

void UIPacketRespInput(void)
{
  tSInput SInput;
  char strData[8];
  uint8_t bTotalInput = 0;

  memset(&SInput, 0, sizeof(tSInput));

  bTotalInput = InputTotalGet(bRequestParameter1);
  if (bRequestParameter1 == INPUT_TYPE_DIGITAL)
  {
    bTotalInput += 3;
  }

  if ((bRequestParameter2 <= bTotalInput) && InputGet(bRequestParameter1,
                                                      bRequestParameter2 - 1,
                                                      &SInput))
  {
    strcat(strUITx, UI_COMM_PACKET_INPUT_STR);
    strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

    sprintf(strData, "%d", bRequestParameter1);
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strData, "%d", bRequestParameter2);
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strData, "%d", SInput.bActiveLevel);
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strData, "%d", SInput.bOwnerSG);
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strData, "%d", SInput.bGreenDurPerDemand);
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strData, "%d", SInput.bRedDurInBroken);
    strcat(strUITx, strData);
  }
  else
  {
    UIFailureResponseSet();
  }
} /* UIPacketRespInput */

void UIPacketRespOutput(void)
{
  char strData[8];
  tSOutputDef SOutputDef;

  memset(&SOutputDef, 0, sizeof(tSOutputDef));

  if (bRequestParameter1 && OutputGet(bRequestParameter1 - 1,
                                      &SOutputDef)
      && (bRequestParameter1 <= OutputTotalGet()))
  {
    strcat(strUITx, UI_COMM_PACKET_OUTPUT_STR);
    strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

    sprintf(strData, "%d", bRequestParameter1);
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strData, "%d", SOutputDef.bActiveLevel);
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strData, "%d", SOutputDef.bActiveLevelDur);
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strData, "%d", SOutputDef.bInActiveLevelDur);
    strcat(strUITx, strData);
  }
  else
  {
    UIFailureResponseSet();
  }
}

void UIPacketRespChecksumTotal(void) /* DENIZLI FUNCTION */
{
  char strNumberData[7];

  strcat(strUITx, UI_COMM_PACKET_CHECKSUM_TOTAL_STR);
  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);
  sprintf(strNumberData, "%d", DataChecksumTotalCalculate());
  strcat(strUITx, strNumberData);
}

void UIPacketRespMaestroModuleVersion(void)
{
  char strData[8];
  tpSModulesVersion pSVer = GetModulesVersion();

  if (bRequestParameter1 <= MODULES_MAX)
  {
    switch (bRequestParameter1)
    {
        case 1:   /* CPU */
        {
          if (bRequestParameter2 <= MODULES_PU_MAX)
          {
            strcat(strUITx, UI_COMM_PACKET_MAESTRO_MODULE_VERSIONS_STR);
            strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

            sprintf(strData, "%d", bRequestParameter1);
            strcat(strUITx, strData);
            strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
            sprintf(strData, "%d", bRequestParameter2);
            strcat(strUITx, strData);
            strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

            if (bRequestParameter2 == 1)
            {
              sprintf(strData, "%d", pSVer->SCPUVersion.bArg1);
              strcat(strUITx, strData);
              strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
              sprintf(strData, "%d", pSVer->SCPUVersion.bArg2);
              strcat(strUITx, strData);
              strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
              sprintf(strData, "%d", pSVer->SCPUVersion.bArg3);
              strcat(strUITx, strData);
              strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
              sprintf(strData, "%c", pSVer->SCPUVersion.bArg4);
              strcat(strUITx, strData);
            }
            else
            {
              sprintf(strData, "%d", pSVer->SCPUVersion.bArg1);
              strcat(strUITx, strData);
              strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
              sprintf(strData, "%d", pSVer->SCPUVersion.bArg2);
              strcat(strUITx, strData);
              strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
              sprintf(strData, "%d", pSVer->SCPUVersion.bArg3);
              strcat(strUITx, strData);
              strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
              sprintf(strData, "%c", pSVer->SCPUVersion.bArg4);
              strcat(strUITx, strData);
            }
          }
          else
          {
            UIFailureResponseSet();
          }

          break;
        }

        case 2:   /* PSMS */
        {
          if (bRequestParameter2 && (bRequestParameter2 <= MODULES_PSM_MAX) )
          {
            strcat(strUITx, UI_COMM_PACKET_MAESTRO_MODULE_VERSIONS_STR);
            strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

            sprintf(strData, "%d", bRequestParameter1);
            strcat(strUITx, strData);
            strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
            sprintf(strData, "%d", bRequestParameter2);
            strcat(strUITx, strData);
            strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
            sprintf(strData,
                    "%d",
                    pSVer->SPSMVersions[bRequestParameter2 - 1].bArg1);
            strcat(strUITx, strData);
            strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
            sprintf(strData,
                    "%d",
                    pSVer->SPSMVersions[bRequestParameter2 - 1].bArg2);
            strcat(strUITx, strData);
            strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
            sprintf(strData,
                    "%d",
                    pSVer->SPSMVersions[bRequestParameter2 - 1].bArg3);
            strcat(strUITx, strData);
            strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
            sprintf(strData,
                    "%c",
                    pSVer->SPSMVersions[bRequestParameter2 - 1].bArg4);
            strcat(strUITx, strData);
          }
          else
          {
            UIFailureResponseSet();
          }

          break;
        }

        case 3:   /* SSMS */
        {
          if (bRequestParameter2 && (bRequestParameter2 <= MODULES_SSM_MAX) )
          {
            strcat(strUITx, UI_COMM_PACKET_MAESTRO_MODULE_VERSIONS_STR);
            strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

            sprintf(strData, "%d", bRequestParameter1);
            strcat(strUITx, strData);
            strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
            sprintf(strData, "%d", bRequestParameter2);
            strcat(strUITx, strData);
            strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
            sprintf(strData,
                    "%d",
                    pSVer->SSSMVersions[bRequestParameter2 - 1].bArg1);
            strcat(strUITx, strData);
            strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
            sprintf(strData,
                    "%d",
                    pSVer->SSSMVersions[bRequestParameter2 - 1].bArg2);
            strcat(strUITx, strData);
            strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
            sprintf(strData,
                    "%d",
                    pSVer->SSSMVersions[bRequestParameter2 - 1].bArg3);
            strcat(strUITx, strData);
            strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
            sprintf(strData,
                    "%c",
                    pSVer->SSSMVersions[bRequestParameter2 - 1].bArg4);
            strcat(strUITx, strData);
          }
          else
          {
            UIFailureResponseSet();
          }

          break;
        }

        case 4:   /* IOS */
        {
          if (bRequestParameter2 && (bRequestParameter2 <= MODULES_IO_MAX) )
          {
            strcat(strUITx, UI_COMM_PACKET_MAESTRO_MODULE_VERSIONS_STR);
            strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

            sprintf(strData, "%d", bRequestParameter1);
            strcat(strUITx, strData);
            strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
            sprintf(strData, "%d", bRequestParameter2);
            strcat(strUITx, strData);
            strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
            sprintf(strData,
                    "%d",
                    pSVer->SIOVersions[bRequestParameter2 - 1].bArg1);
            strcat(strUITx, strData);
            strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
            sprintf(strData,
                    "%d",
                    pSVer->SIOVersions[bRequestParameter2 - 1].bArg2);
            strcat(strUITx, strData);
            strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
            sprintf(strData,
                    "%d",
                    pSVer->SIOVersions[bRequestParameter2 - 1].bArg3);
            strcat(strUITx, strData);
            strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
            sprintf(strData,
                    "%c",
                    pSVer->SIOVersions[bRequestParameter2 - 1].bArg4);
            strcat(strUITx, strData);
          }
          else
          {
            UIFailureResponseSet();
          }

          break;
        }

        default:
        {
          UIFailureResponseSet();
          break;
        }
    } /* switch */
  }
  else
  {
    UIFailureResponseSet();
  }
} /* UIPacketRespMaestroModuleVersion */

void UIPacketRespModuleVersions(void) /* DENIZLI FUNCTION */
{
  char strData[8];
  tpSModulesVersion pSVer = GetModulesVersion();

  /* CP */
  strcat(strUITx, "CP");
  strcat(strUITx, UI_COMM_CHECKSUM_SEPARATOR_STR);
  sprintf(strData, "%d", pSVer->SCPUVersion.bArg1);
  strcat(strUITx, strData);
  sprintf(strData, "%c", '.');
  strcat(strUITx, strData);
  sprintf(strData, "%d", pSVer->SCPUVersion.bArg2);
  strcat(strUITx, strData);
  sprintf(strData, "%c", '.');
  strcat(strUITx, strData);
  sprintf(strData, "%d", pSVer->SCPUVersion.bArg3);
  strcat(strUITx, strData);
  sprintf(strData, "%c", '.');
  strcat(strUITx, strData);
  sprintf(strData, "%c", pSVer->SCPUVersion.bArg4);
  strcat(strUITx, strData);
  strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

  /* MP */
  strcat(strUITx, "MP");
  strcat(strUITx, UI_COMM_CHECKSUM_SEPARATOR_STR);
  sprintf(strData, "%d", pSVer->SCPUVersion.bArg1);
  strcat(strUITx, strData);
  sprintf(strData, "%c", '.');
  strcat(strUITx, strData);
  sprintf(strData, "%d", pSVer->SCPUVersion.bArg2);
  strcat(strUITx, strData);
  sprintf(strData, "%c", '.');
  strcat(strUITx, strData);
  sprintf(strData, "%d", pSVer->SCPUVersion.bArg3);
  strcat(strUITx, strData);
  sprintf(strData, "%c", '.');
  strcat(strUITx, strData);
  sprintf(strData, "%c", pSVer->SCPUVersion.bArg4);
  strcat(strUITx, strData);
} /* DENIZLI FUNCTION */

void UIPacketRespFuncConf(void)
{
  char strData[8];

  strcat(strUITx, UI_COMM_PACKET_FUNCTION_CONF_STR);

  strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);
  sprintf(strData, "%d", bRequestParameter1);
  strcat(strUITx, strData);

  strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
  sprintf(strData, "%d", GetFunctionConfByIndex(bRequestParameter1));
  strcat(strUITx, strData);
}

void UIPacketRespSystemStartTime(void)
{
  tSSystemStartTime SLSystemStartTime;

  memset(&SLSystemStartTime, 0, sizeof(tSSystemStartTime));

  if (SystemStartTimeRead())
  {
    char strSystemStartTimeData[8];

    SystemStartTimeGet(&SLSystemStartTime);

    strcat(strUITx, UI_COMM_PACKET_SYSTEM_START_TIME_STR);
    strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);
    sprintf(strSystemStartTimeData, "%d", SLSystemStartTime.bMonthDay);
    strcat(strUITx, strSystemStartTimeData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strSystemStartTimeData, "%d", SLSystemStartTime.bMonth);
    strcat(strUITx, strSystemStartTimeData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strSystemStartTimeData, "%d", SLSystemStartTime.sYear);
    strcat(strUITx, strSystemStartTimeData);
  }
  else
  {
    UIFailureResponseSet();
  }
}

void UISecureSystemReset(void)
{
  LogRequest(LOG_REQ_APPEND_ASYNCH, NULL, EVENT_USER_REQ_RESET, 0, 0, 0, 0);

  SecureSystemReset();
}

void UIPacketRespMCSConInfo(void)
{
  if (MCSReadConInfo())
  {
    tpSMCSConInfo pSMCSConInfo = MCSGetConInfoPtr();
    char strData[SNMP_TRAP_DESTINATION_MAX_SIZE + 1];

    strcat(strUITx, UI_COMM_PACKET_MCS_CON_INFO_STR);
    strcat(strUITx, UI_COMM_DATA_FIELD_SEPARATOR_STR);

    sprintf(strData, "%lu", (unsigned long) pSMCSConInfo->SSNMPInfo.lDeviceID);
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

    sprintf(strData, "%02X", pSMCSConInfo->SMACAddress.bAddress0);
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_MAC_SEPARATOR_STR);
    sprintf(strData, "%02X", pSMCSConInfo->SMACAddress.bAddress1);
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_MAC_SEPARATOR_STR);
    sprintf(strData, "%02X", pSMCSConInfo->SMACAddress.bAddress2);
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_MAC_SEPARATOR_STR);
    sprintf(strData, "%02X", pSMCSConInfo->SMACAddress.bAddress3);
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_MAC_SEPARATOR_STR);
    sprintf(strData, "%02X", pSMCSConInfo->SMACAddress.bAddress4);
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_MAC_SEPARATOR_STR);
    sprintf(strData, "%02X", pSMCSConInfo->SMACAddress.bAddress5);
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

    sprintf(strData, "%d", pSMCSConInfo->SFlags.fEthStaticIp);
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

    if (pSMCSConInfo->SFlags.fEthStaticIp)
    {
      sprintf(strData, "%u", pSMCSConInfo->SEthLocalIPv4.bAddress0);
      strcat(strUITx, strData);
      strcat(strUITx, UI_COMM_IP_SEPARATOR_STR);
      sprintf(strData, "%u", pSMCSConInfo->SEthLocalIPv4.bAddress1);
      strcat(strUITx, strData);
      strcat(strUITx, UI_COMM_IP_SEPARATOR_STR);
      sprintf(strData, "%u", pSMCSConInfo->SEthLocalIPv4.bAddress2);
      strcat(strUITx, strData);
      strcat(strUITx, UI_COMM_IP_SEPARATOR_STR);
      sprintf(strData, "%u", pSMCSConInfo->SEthLocalIPv4.bAddress3);
      strcat(strUITx, strData);
      strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

      sprintf(strData, "%u", pSMCSConInfo->SEthSubnetMask.bAddress0);
      strcat(strUITx, strData);
      strcat(strUITx, UI_COMM_IP_SEPARATOR_STR);
      sprintf(strData, "%u", pSMCSConInfo->SEthSubnetMask.bAddress1);
      strcat(strUITx, strData);
      strcat(strUITx, UI_COMM_IP_SEPARATOR_STR);
      sprintf(strData, "%u", pSMCSConInfo->SEthSubnetMask.bAddress2);
      strcat(strUITx, strData);
      strcat(strUITx, UI_COMM_IP_SEPARATOR_STR);
      sprintf(strData, "%u", pSMCSConInfo->SEthSubnetMask.bAddress3);
      strcat(strUITx, strData);
      strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

      sprintf(strData, "%u", pSMCSConInfo->SEthGateway.bAddress0);
      strcat(strUITx, strData);
      strcat(strUITx, UI_COMM_IP_SEPARATOR_STR);
      sprintf(strData, "%u", pSMCSConInfo->SEthGateway.bAddress1);
      strcat(strUITx, strData);
      strcat(strUITx, UI_COMM_IP_SEPARATOR_STR);
      sprintf(strData, "%u", pSMCSConInfo->SEthGateway.bAddress2);
      strcat(strUITx, strData);
      strcat(strUITx, UI_COMM_IP_SEPARATOR_STR);
      sprintf(strData, "%u", pSMCSConInfo->SEthGateway.bAddress3);
      strcat(strUITx, strData);
      strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
    }

    sprintf(strData, "%u", pSMCSConInfo->SPrimaryDNSServer.bAddress0);
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_IP_SEPARATOR_STR);
    sprintf(strData, "%u", pSMCSConInfo->SPrimaryDNSServer.bAddress1);
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_IP_SEPARATOR_STR);
    sprintf(strData, "%u", pSMCSConInfo->SPrimaryDNSServer.bAddress2);
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_IP_SEPARATOR_STR);
    sprintf(strData, "%u", pSMCSConInfo->SPrimaryDNSServer.bAddress3);
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

    sprintf(strData, "%u", pSMCSConInfo->SSecondaryDNSServer.bAddress0);
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_IP_SEPARATOR_STR);
    sprintf(strData, "%u", pSMCSConInfo->SSecondaryDNSServer.bAddress1);
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_IP_SEPARATOR_STR);
    sprintf(strData, "%u", pSMCSConInfo->SSecondaryDNSServer.bAddress2);
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_IP_SEPARATOR_STR);
    sprintf(strData, "%u", pSMCSConInfo->SSecondaryDNSServer.bAddress3);
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

    sprintf(strData, "%s", pSMCSConInfo->SSNMPInfo.strReadCommunityName);
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

    sprintf(strData, "%s", pSMCSConInfo->SSNMPInfo.strWriteCommunityName);
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

    sprintf(strData, "%s", pSMCSConInfo->SSNMPInfo.strTrapCommunityName);
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

    sprintf(strData, "%s", pSMCSConInfo->SSNMPInfo.strV3EngineId);
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

    sprintf(strData, "%s", pSMCSConInfo->SSNMPInfo.strV3Username);
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

    sprintf(strData, "%s", pSMCSConInfo->SSNMPInfo.strV3AuthPassword);
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

    sprintf(strData, "%s", pSMCSConInfo->SSNMPInfo.strV3PrivPassword);
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

    sprintf(strData, "%s", pSMCSConInfo->SSNMPInfo.strTrapDestination);
    strcat(strUITx, strData);
    strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);

    sprintf(strData, "%d", pSMCSConInfo->SSNMPInfo.bTrapVersion);
    strcat(strUITx, strData);
  }
  else
  {
    UIFailureResponseSet();
  }
} /* UIPacketRespMCSConInfo */

uint8_t UISCPBackupNextPacketGet(tpSUIRequest pSUIReq)
{
  if (pSUIReq == NULL)
  {
    return FALSE;
  }

  if (SRawSCP.sDataIndex >= UI_COMM_BACKUP_SCP_MAX_SIZE)
  {
    return FALSE;
  }

  if (SRawSCP.sCurPacketIndex >= SRawSCP.sDataIndex)
  {
    return FALSE;
  }

  if (baRawSCPData[SRawSCP.sCurPacketIndex] == '\0')
  {
    return FALSE;
  }

  while (SRawSCP.sCurPacketIndex < SRawSCP.sDataIndex
         && baRawSCPData[SRawSCP.sCurPacketIndex] != UI_COMM_START_OF_PACKET
         && baRawSCPData[SRawSCP.sCurPacketIndex] != '\0')
  {
    SRawSCP.sCurPacketIndex++;
  }

  if (SRawSCP.sCurPacketIndex >= SRawSCP.sDataIndex)
  {
    return FALSE;
  }

  if (baRawSCPData[SRawSCP.sCurPacketIndex] == '\0')
  {
    return FALSE;
  }

  uint16_t sPacketStartIdx = SRawSCP.sCurPacketIndex;
  uint16_t sStrDataIdx = 0;

  while (SRawSCP.sCurPacketIndex < SRawSCP.sDataIndex
         && baRawSCPData[SRawSCP.sCurPacketIndex] != '\0')
  {
    pSUIReq->strData[sStrDataIdx++] = baRawSCPData[SRawSCP.sCurPacketIndex];

    if (baRawSCPData[SRawSCP.sCurPacketIndex] == UI_COMM_END_OF_PACKET)
    {
      SRawSCP.sCurPacketIndex++;
      pSUIReq->sDataSize = SRawSCP.sCurPacketIndex - sPacketStartIdx + 1;
      pSUIReq->strData[sStrDataIdx] = '\0';

      return TRUE;
    }

    SRawSCP.sCurPacketIndex++;
  }

  return FALSE;
} /* UISCPBackupNextPacketGet */

uint8_t UISCPBackupNextPacketSet(tpSUIRequest pSUIReq,
                                 uint32_t lPacketName)
{
  if (SRawSCP.sDataIndex +  pSUIReq->sDataSize > UI_COMM_BACKUP_SCP_MAX_SIZE)
  {
    return FALSE;
  }

  memcpy(&baRawSCPData[SRawSCP.sDataIndex], pSUIReq->strData,
         pSUIReq->sDataSize);
  SRawSCP.sDataIndex += pSUIReq->sDataSize;

  if (lPacketName == UI_COMM_PACKET_END_CONF)
  {
    LogRequest(LOG_REQ_APPEND, NULL, EVENT_MCT_CONFIGURATION_ENDS, 0, 0, 0, 0);

    SRawSCP.bStatus = UI_COMM_BACKUP_SCP_STATE_COMPLETED;

    UICheckDownloadTimeoutSet(FALSE);
    UISetMCSDownloadInProgress(FALSE);
  }

  return TRUE;
}

void UIPacketRespMCSUpload(void)
{
  if (((bRequestParameter1 >= Time) && (bRequestParameter1 <= ScheduleRow))
      && (bRequestParameter2 > 0)
      && (bRequestParameter3 > 0))
  {
    uint8_t bIndex, bSubIndex, bPacketName, bIndexStart, bSubIndexStart;
    char strUIMCSUpload[UI_COMM_MAX_MCS_PACKET_LENGTH + 1];
    char strParams[4];

    memset(strUIMCSUpload, 0, sizeof(strUIMCSUpload));
    memset(strParams, 0, sizeof(strParams));

    strcat(strUIMCSUpload, UI_COMM_START_OF_PACKET_STR);
    strcat(strUIMCSUpload, UI_COMM_PACKET_MCS_UPLOAD_STR);
    strcat(strUIMCSUpload, UI_COMM_DATA_FIELD_SEPARATOR_STR);
    sprintf(strParams, "%d", bRequestParameter1);
    strcat(strUIMCSUpload, strParams);
    strcat(strUIMCSUpload, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strParams, "%d", bRequestParameter2);
    strcat(strUIMCSUpload, strParams);
    strcat(strUIMCSUpload, UI_COMM_DATA_SEPARATOR_STR);
    sprintf(strParams, "%d", bRequestParameter3);
    strcat(strUIMCSUpload, strParams);

    bPacketName = bRequestParameter1;
    bIndexStart = bRequestParameter2 - 1;
    bSubIndexStart = bRequestParameter3 - 1;

    while (bPacketName >= Time && bPacketName <= ScheduleRow)
    {
      switch (bPacketName)
      {
          case Time:
          {
            UISetMCSUploadInProgress(TRUE);
            memset(strUITx, 0, sizeof(strUITx));
            strcat(strUITx, UI_COMM_PACKET_SEPARATOR_STR);
            UIPacketRespTime();

            if (strlen(strUIMCSUpload) + strlen(strUITx)
                < UI_COMM_MAX_MCS_PACKET_LENGTH)
            {
              strcat(strUIMCSUpload, strUITx);
              bPacketName++;
            }
            else
            {
              bPacketName = 0;
            }

            break;
          }

          case Info:
          {
            memset(strUITx, 0, sizeof(strUITx));
            strcat(strUITx, UI_COMM_PACKET_SEPARATOR_STR);
            UIPacketRespDeviceInfo();

            if (strlen(strUIMCSUpload) + strlen(strUITx)
                < UI_COMM_MAX_MCS_PACKET_LENGTH)
            {
              strcat(strUIMCSUpload, strUITx);
              bPacketName++;
            }
            else
            {
              bPacketName = 0;
            }

            break;
          }

          case NumberofSignal:
          {
            memset(strUITx, 0, sizeof(strUITx));
            strcat(strUITx, UI_COMM_PACKET_SEPARATOR_STR);
            UIPacketRespSinTotal();

            if (strlen(strUIMCSUpload) + strlen(strUITx)
                < UI_COMM_MAX_MCS_PACKET_LENGTH)
            {
              strcat(strUIMCSUpload, strUITx);
              bIndexStart = 0;
              bPacketName++;
            }
            else
            {
              bPacketName = 0;
            }

            break;
          }

          case Signal:
          {
            for (bIndex = bIndexStart; bIndex < SignalTotalGet(); bIndex++)
            {
              memset(strUITx, 0, sizeof(strUITx));
              bRequestParameter1 = bIndex + 1;
              strcat(strUITx, UI_COMM_PACKET_SEPARATOR_STR);
              UIPacketRespSignal();

              if (strlen(strUIMCSUpload) + strlen(strUITx)
                  < UI_COMM_MAX_MCS_PACKET_LENGTH)
              {
                strcat(strUIMCSUpload, strUITx);
              }
              else
              {
                bPacketName = 0;
                break;
              }
            }

            if (bPacketName != 0)
            {
              bPacketName++;
            }

            break;
          }

          case SignalAssignments:
          {
            memset(strUITx, 0, sizeof(strUITx));
            strcat(strUITx, UI_COMM_PACKET_SEPARATOR_STR);
            UIPacketRespSignalsDefined();

            if (strlen(strUIMCSUpload) + strlen(strUITx)
                < UI_COMM_MAX_MCS_PACKET_LENGTH)
            {
              strcat(strUIMCSUpload, strUITx);
              bPacketName++;
            }
            else
            {
              bPacketName = 0;
            }

            break;
          }

          case NumberofSignalGroups:
          {
            memset(strUITx, 0, sizeof(strUITx));
            strcat(strUITx, UI_COMM_PACKET_SEPARATOR_STR);
            UIPacketRespSGTotal();

            if (strlen(strUIMCSUpload) + strlen(strUITx)
                < UI_COMM_MAX_MCS_PACKET_LENGTH)
            {
              strcat(strUIMCSUpload, strUITx);
              bIndexStart = 0;
              bPacketName++;
            }
            else
            {
              bPacketName = 0;
            }

            break;
          }

          case SignalGroup:
          {
            for (bIndex = bIndexStart; bIndex < SGTotalGet(); bIndex++)
            {
              memset(strUITx, 0, sizeof(strUITx));
              bRequestParameter1 = bIndex + 1;
              strcat(strUITx, UI_COMM_PACKET_SEPARATOR_STR);
              UIPacketRespSG();

              if (strlen(strUIMCSUpload) + strlen(strUITx)
                  < UI_COMM_MAX_MCS_PACKET_LENGTH)
              {
                strcat(strUIMCSUpload, strUITx);
              }
              else
              {
                bPacketName = 0;
                break;
              }
            }

            if (bPacketName != 0)
            {
              bIndexStart = 0;
              bSubIndexStart = 0;
              bPacketName++;
            }

            break;
          }

          case SignalOutput:
          {
            for (bIndex = bIndexStart; bIndex < SGTotalGet(); bIndex++)
            {
              uint8_t bSGSOTotal = 1;

              SGSOTotalGet(bIndex, &bSGSOTotal);
              for (bSubIndex = bSubIndexStart;
                   bSubIndex < bSGSOTotal;
                   bSubIndex++)
              {
                memset(strUITx, 0, sizeof(strUITx));
                bRequestParameter1 = bIndex + 1;
                bRequestParameter2 = bSubIndex + 1;
                strcat(strUITx, UI_COMM_PACKET_SEPARATOR_STR);
                UIPacketRespSO();

                if (strlen(strUIMCSUpload) + strlen(strUITx)
                    < UI_COMM_MAX_MCS_PACKET_LENGTH)
                {
                  strcat(strUIMCSUpload, strUITx);
                }
                else
                {
                  bPacketName = 0;
                  break;
                }
              }

              bSubIndexStart = 0;

              if (bPacketName == 0)
              {
                break;
              }
            }

            if (bPacketName != 0)
            {
              bPacketName++;
              bIndexStart = 1;
            }

            break;
          }

          case CVS:
          {
            for (bIndex = bIndexStart; bIndex < 4; bIndex++)
            {
              memset(strUITx, 0, sizeof(strUITx));
              bRequestParameter1 = bIndex;
              strcat(strUITx, UI_COMM_PACKET_SEPARATOR_STR);
              UIPacketRespCVS();

              if (strlen(strUIMCSUpload) + strlen(strUITx)
                  < UI_COMM_MAX_MCS_PACKET_LENGTH)
              {
                strcat(strUIMCSUpload, strUITx);
              }
              else
              {
                bPacketName = 0;
                break;
              }
            }

            if (bPacketName != 0)
            {
              bIndexStart = 0;
              bPacketName++;
            }

            break;
          }

          case NumberofConflicts:
          {
            memset(strUITx, 0, sizeof(strUITx));
            strcat(strUITx, UI_COMM_PACKET_SEPARATOR_STR);
            UIPacketRespConflictTotal();

            if (strlen(strUIMCSUpload) + strlen(strUITx)
                < UI_COMM_MAX_MCS_PACKET_LENGTH)
            {
              strcat(strUIMCSUpload, strUITx);
              bIndexStart = 0;
              bSubIndexStart = 0;
              bPacketName++;
            }
            else
            {
              bPacketName = 0;
            }

            break;
          }

          case Conflict:
          {
            for (bIndex = bIndexStart; bIndex < SGTotalGet(); bIndex++)
            {
              for (bSubIndex = bSubIndexStart;
                   bSubIndex < SGClearanceTotalGet(bIndex);
                   bSubIndex++)
              {
                memset(strUITx, 0, sizeof(strUITx));
                bRequestParameter1 = bIndex + 1;
                bRequestParameter2 = bSubIndex + 1;
                strcat(strUITx, UI_COMM_PACKET_SEPARATOR_STR);
                UIPacketRespClearance();

                if (strlen(strUIMCSUpload) + strlen(strUITx)
                    < UI_COMM_MAX_MCS_PACKET_LENGTH)
                {
                  strcat(strUIMCSUpload, strUITx);
                }
                else
                {
                  bPacketName = 0;
                  break;
                }
              }

              bSubIndexStart = 0;

              if (bPacketName == 0)
              {
                break;
              }
            }

            if (bPacketName != 0)
            {
              bPacketName++;
              bIndexStart = 0;
            }

            break;
          }

          case FailureAction:
          {
            const char *strFailureTypes[] = { "GG", "YG", "YY", "IS", "ISS",
                                              "VL", "FE" };

            for (bIndex = bIndexStart;
                 bIndex < sizeof(strFailureTypes) / sizeof(const char *);
                 bIndex++)
            {
              memset(strUITx, 0, sizeof(strUITx));
              memset(strRequestParameter, 0, sizeof(strRequestParameter));
              strcat(strRequestParameter, strFailureTypes[bIndex]);
              strcat(strUITx, UI_COMM_PACKET_SEPARATOR_STR);
              UIPacketRespConflictEM();

              if (strlen(strUIMCSUpload) + strlen(strUITx)
                  < UI_COMM_MAX_MCS_PACKET_LENGTH)
              {
                strcat(strUIMCSUpload, strUITx);
              }
              else
              {
                bPacketName = 0;
                break;
              }
            }

            if (bPacketName != 0)
            {
              bIndexStart = 0;
              bPacketName++;
            }

            break;
          }

          case NumberofSequence:
          {
            memset(strUITx, 0, sizeof(strUITx));
            strcat(strUITx, UI_COMM_PACKET_SEPARATOR_STR);
            UIPacketRespSeqTotal();

            if (strlen(strUIMCSUpload) + strlen(strUITx)
                < UI_COMM_MAX_MCS_PACKET_LENGTH)
            {
              strcat(strUIMCSUpload, strUITx);
              bIndexStart = 0;
              bPacketName++;
            }
            else
            {
              bPacketName = 0;
            }

            break;
          }

          case NumberofSequenceStep:
          {
            for (bIndex = bIndexStart; bIndex < SeqTotalGet(); bIndex++)
            {
              memset(strUITx, 0, sizeof(strUITx));
              bRequestParameter1 = bIndex + 1;
              strcat(strUITx, UI_COMM_PACKET_SEPARATOR_STR);
              UIPacketRespSeqStepTotal();

              if (strlen(strUIMCSUpload) + strlen(strUITx)
                  < UI_COMM_MAX_MCS_PACKET_LENGTH)
              {
                strcat(strUIMCSUpload, strUITx);
              }
              else
              {
                bPacketName = 0;
                break;
              }
            }

            if (bPacketName != 0)
            {
              bIndexStart = 0;
              bSubIndexStart = 0;
              bPacketName++;
            }

            break;
          }

          case SignalSequenceStep:
          {
            for (bIndex = bIndexStart; bIndex < SeqTotalGet(); bIndex++)
            {
              for (bSubIndex = bSubIndexStart;
                   bSubIndex < SeqStepTotalGet(bIndex);
                   bSubIndex++)
              {
                memset(strUITx, 0, sizeof(strUITx));
                bRequestParameter1 = bIndex + 1;
                bRequestParameter2 = bSubIndex + 1;
                strcat(strUITx, UI_COMM_PACKET_SEPARATOR_STR);
                UIPacketRespSeqStep();

                if (strlen(strUIMCSUpload) + strlen(strUITx)
                    < UI_COMM_MAX_MCS_PACKET_LENGTH)
                {
                  strcat(strUIMCSUpload, strUITx);
                }
                else
                {
                  bPacketName = 0;
                  break;
                }
              }

              bSubIndexStart = 0;

              if (bPacketName == 0)
              {
                break;
              }
            }

            if (bPacketName != 0)
            {
              bIndexStart = 0;
              bPacketName++;
            }

            break;
          }

          case NumberofPhase:
          {
            memset(strUITx, 0, sizeof(strUITx));
            strcat(strUITx, UI_COMM_PACKET_SEPARATOR_STR);
            UIPacketRespPhaseTotal();

            if (strlen(strUIMCSUpload) + strlen(strUITx)
                < UI_COMM_MAX_MCS_PACKET_LENGTH)
            {
              strcat(strUIMCSUpload, strUITx);
              bIndexStart = 0;
              bPacketName++;
            }
            else
            {
              bPacketName = 0;
            }

            break;
          }

          case Phase:
          {
            for (bIndex = bIndexStart; bIndex < PhaseTotalGet(); bIndex++)
            {
              memset(strUITx, 0, sizeof(strUITx));
              bRequestParameter1 = bIndex + 1;
              strcat(strUITx, UI_COMM_PACKET_SEPARATOR_STR);
              UIPacketRespPhase();

              if (strlen(strUIMCSUpload) + strlen(strUITx)
                  < UI_COMM_MAX_MCS_PACKET_LENGTH)
              {
                strcat(strUIMCSUpload, strUITx);
              }
              else
              {
                bPacketName = 0;
                break;
              }
            }

            if (bPacketName != 0)
            {
              bIndexStart = 0;
              bPacketName++;
            }

            break;
          }

          case NumberofInputs:
          {
            memset(strUITx, 0, sizeof(strUITx));
            strcat(strUITx, UI_COMM_PACKET_SEPARATOR_STR);
            UIPacketRespInputDigitalTotal();

            if (strlen(strUIMCSUpload) + strlen(strUITx)
                < UI_COMM_MAX_MCS_PACKET_LENGTH)
            {
              strcat(strUIMCSUpload, strUITx);
              bPacketName++;
            }
            else
            {
              bPacketName = 0;
            }

            break;
          }

          case NumberofDetectors:
          {
            memset(strUITx, 0, sizeof(strUITx));
            strcat(strUITx, UI_COMM_PACKET_SEPARATOR_STR);
            UIPacketRespDetectorTotal();

            if (strlen(strUIMCSUpload) + strlen(strUITx)
                < UI_COMM_MAX_MCS_PACKET_LENGTH)
            {
              strcat(strUIMCSUpload, strUITx);
              bIndexStart = 0;
              bSubIndexStart = 0;
              bPacketName++;
            }
            else
            {
              bPacketName = 0;
            }

            break;
          }

          case Input:
          {
            uint8_t bTotalInput = InputTotalGet(INPUT_TYPE_DIGITAL);

            if (bTotalInput >= HEATER_BUTTON_DIG_INPUT_NO)
            {
              bTotalInput += 1;
            }
            else if (bTotalInput >= LAMP_DIMMING_BUTTON_DIG_INPUT_NO)
            {
              bTotalInput += 2;
            }
            else if (bTotalInput >= POLICE_BUTTON_DIG_INPUT_NO)
            {
              bTotalInput += 3;
            }

            for (bIndex = bIndexStart; bIndex < bTotalInput; bIndex++)
            {
              if ((bIndex == HEATER_BUTTON_DIG_INPUT_NO - 1) || (bIndex
                                                                 ==
                                                                 LAMP_DIMMING_BUTTON_DIG_INPUT_NO
                                                                 - 1)
                  || (bIndex == POLICE_BUTTON_DIG_INPUT_NO - 1))
              {
                continue;
              }

              memset(strUITx, 0, sizeof(strUITx));
              bRequestParameter1 = INPUT_TYPE_DIGITAL;
              bRequestParameter2 = bIndex + 1;
              strcat(strUITx, UI_COMM_PACKET_SEPARATOR_STR);
              UIPacketRespInput();

              if (strlen(strUIMCSUpload) + strlen(strUITx)
                  < UI_COMM_MAX_MCS_PACKET_LENGTH)
              {
                strcat(strUIMCSUpload, strUITx);
              }
              else
              {
                bSubIndexStart = 0;
                bPacketName = 0;
                break;
              }
            }

            if (bPacketName != 0)
            {
              bTotalInput = InputTotalGet(INPUT_TYPE_DETECTOR);
              for (bIndex = bSubIndexStart; bIndex < bTotalInput; bIndex++)
              {
                memset(strUITx, 0, sizeof(strUITx));
                bRequestParameter1 = INPUT_TYPE_DETECTOR;
                bRequestParameter2 = bIndex + 1;
                strcat(strUITx, UI_COMM_PACKET_SEPARATOR_STR);
                UIPacketRespInput();

                if (strlen(strUIMCSUpload) + strlen(strUITx)
                    < UI_COMM_MAX_MCS_PACKET_LENGTH)
                {
                  strcat(strUIMCSUpload, strUITx);
                }
                else
                {
                  bPacketName = 0;
                  break;
                }
              }
            }

            if (bPacketName != 0)
            {
              bIndexStart = 0;
              bPacketName++;
            }

            break;
          }

          case NumberofOutputs:
          {
            bIndexStart = 0;
            bSubIndexStart = 0;
            bPacketName++;
            break;
          }

          case Output:
          {
            bIndexStart = 0;
            bSubIndexStart = 0;
            bPacketName++;
            break;
          }

          case NumberofSignalPrograms:
          {
            memset(strUITx, 0, sizeof(strUITx));
            strcat(strUITx, UI_COMM_PACKET_SEPARATOR_STR);
            UIPacketRespSPTotal();

            if (strlen(strUIMCSUpload) + strlen(strUITx)
                < UI_COMM_MAX_MCS_PACKET_LENGTH)
            {
              strcat(strUIMCSUpload, strUITx);
              bPacketName++;
            }
            else
            {
              bPacketName = 0;
            }

            break;
          }

          case NumberofProgramRules:
          {
            memset(strUITx, 0, sizeof(strUITx));
            strcat(strUITx, UI_COMM_PACKET_SEPARATOR_STR);
            UIPacketRespOperationTotal();

            if (strlen(strUIMCSUpload) + strlen(strUITx)
                < UI_COMM_MAX_MCS_PACKET_LENGTH)
            {
              strcat(strUIMCSUpload, strUITx);
              bIndexStart = 0;
              bSubIndexStart = 0;
              bPacketName++;
            }
            else
            {
              bPacketName = 0;
            }

            break;
          }

          case Expression:
          {
            for (bIndex = bIndexStart; bIndex < SigProgTotalGet(); bIndex++)
            {
              for (bSubIndex = bSubIndexStart;
                   bSubIndex < OperationTotalGet(bIndex + 1);
                   bSubIndex++)
              {
                memset(strUITx, 0, sizeof(strUITx));
                bRequestParameter1 = bIndex + 1;
                bRequestParameter2 = bSubIndex + 1;
                strcat(strUITx, UI_COMM_PACKET_SEPARATOR_STR);
                UIPacketRespOperation();

                if (strlen(strUIMCSUpload) + strlen(strUITx)
                    < UI_COMM_MAX_MCS_PACKET_LENGTH)
                {
                  strcat(strUIMCSUpload, strUITx);
                }
                else
                {
                  bPacketName = 0;
                  break;
                }
              }

              bSubIndexStart = 0;

              if (bPacketName == 0)
              {
                break;
              }
            }

            if (bPacketName != 0)
            {
              bIndexStart = 0;
              bPacketName++;
            }

            break;
          }

          case NumberofSignalTasks:
          {
            memset(strUITx, 0, sizeof(strUITx));
            strcat(strUITx, UI_COMM_PACKET_SEPARATOR_STR);
            UIPacketRespStatementTotal();

            if (strlen(strUIMCSUpload) + strlen(strUITx)
                < UI_COMM_MAX_MCS_PACKET_LENGTH)
            {
              strcat(strUIMCSUpload, strUITx);
              bIndexStart = 0;
              bSubIndexStart = 0;
              bPacketName++;
            }
            else
            {
              bPacketName = 0;
            }

            break;
          }

          case Statement:
          {
            for (bIndex = bIndexStart; bIndex < SigProgTotalGet(); bIndex++)
            {
              for (bSubIndex = bSubIndexStart;
                   bSubIndex < StatementTotalGet(bIndex + 1);
                   bSubIndex++)
              {
                memset(strUITx, 0, sizeof(strUITx));
                bRequestParameter1 = bIndex + 1;
                bRequestParameter2 = bSubIndex + 1;
                strcat(strUITx, UI_COMM_PACKET_SEPARATOR_STR);
                UIPacketRespStatement();

                if (strlen(strUIMCSUpload) + strlen(strUITx)
                    < UI_COMM_MAX_MCS_PACKET_LENGTH)
                {
                  strcat(strUIMCSUpload, strUITx);
                }
                else
                {
                  bPacketName = 0;
                  break;
                }
              }

              bSubIndexStart = 0;

              if (bPacketName == 0)
              {
                break;
              }
            }

            if (bPacketName != 0)
            {
              bIndexStart = 0;
              bSubIndexStart = 0;
              bPacketName++;
            }

            break;
          }

          case Rule:
          {
            for (bIndex = bIndexStart; bIndex < SigProgTotalGet(); bIndex++)
            {
              for (bSubIndex = bSubIndexStart;
                   bSubIndex < RuleTotalGet(bIndex + 1);
                   bSubIndex++)
              {
                memset(strUITx, 0, sizeof(strUITx));
                bRequestParameter1 = bIndex + 1;
                bRequestParameter2 = bSubIndex + 1;
                strcat(strUITx, UI_COMM_PACKET_SEPARATOR_STR);
                UIPacketRespRule();

                if (strlen(strUIMCSUpload) + strlen(strUITx)
                    < UI_COMM_MAX_MCS_PACKET_LENGTH)
                {
                  strcat(strUIMCSUpload, strUITx);
                }
                else
                {
                  bPacketName = 0;
                  break;
                }
              }

              bSubIndexStart = 0;

              if (bPacketName == 0)
              {
                break;
              }
            }

            if (bPacketName != 0)
            {
              bPacketName++;
            }

            break;
          }

          case NumberofTransitions:
          {
            memset(strUITx, 0, sizeof(strUITx));
            strcat(strUITx, UI_COMM_PACKET_SEPARATOR_STR);
            UIPacketRespTransitionTotal();

            if (strlen(strUIMCSUpload) + strlen(strUITx)
                < UI_COMM_MAX_MCS_PACKET_LENGTH)
            {
              strcat(strUIMCSUpload, strUITx);
              bIndexStart = 0;
              bSubIndexStart = 0;
              bPacketName++;
            }
            else
            {
              bPacketName = 0;
            }

            break;
          }

          case Transition:
          {
            for (bIndex = bIndexStart; bIndex < SigProgTotalGet(); bIndex++)
            {
              for (bSubIndex = bSubIndexStart;
                   bSubIndex < TransitionTotalGet(bIndex + 1);
                   bSubIndex++)
              {
                memset(strUITx, 0, sizeof(strUITx));
                bRequestParameter1 = bIndex + 1;
                bRequestParameter2 = bSubIndex + 1;
                strcat(strUITx, UI_COMM_PACKET_SEPARATOR_STR);
                UIPacketRespTransition();

                if (strlen(strUIMCSUpload) + strlen(strUITx)
                    < UI_COMM_MAX_MCS_PACKET_LENGTH)
                {
                  strcat(strUIMCSUpload, strUITx);
                }
                else
                {
                  bPacketName = 0;
                  break;
                }
              }

              bSubIndexStart = 0;

              if (bPacketName == 0)
              {
                break;
              }
            }

            if (bPacketName != 0)
            {
              bIndexStart = 0;
              bPacketName++;
            }

            break;
          }

          case ProgramEnd:
          {
            for (bIndex = bIndexStart; bIndex < SigProgTotalGet(); bIndex++)
            {
              memset(strUITx, 0, sizeof(strUITx));
              bRequestParameter1 = bIndex + 1;
              strcat(strUITx, UI_COMM_PACKET_SEPARATOR_STR);
              UIPacketRespSigProg();

              if (strlen(strUIMCSUpload) + strlen(strUITx)
                  < UI_COMM_MAX_MCS_PACKET_LENGTH)
              {
                strcat(strUIMCSUpload, strUITx);
              }
              else
              {
                bPacketName = 0;
                break;
              }

              if (bPacketName == 0)
              {
                break;
              }
            }

            if (bPacketName != 0)
            {
              bIndexStart = 0;
              bPacketName++;
            }

            break;
          }

          case NumberofFixedTimeTableRows:
          {
            memset(strUITx, 0, sizeof(strUITx));
            strcat(strUITx, UI_COMM_PACKET_SEPARATOR_STR);
            UIPacketRespWPTotal();

            if (strlen(strUIMCSUpload) + strlen(strUITx)
                < UI_COMM_MAX_MCS_PACKET_LENGTH)
            {
              strcat(strUIMCSUpload, strUITx);
              bIndexStart = 0;
              bSubIndexStart = 0;
              bPacketName++;
            }
            else
            {
              bPacketName = 0;
            }

            break;
          }

          case FixedTimeTableRow:
          {
            for (bIndex = bIndexStart; bIndex < WorkPlanTotalGet(); bIndex++)
            {
              for (bSubIndex = bSubIndexStart;
                   bSubIndex < WorkPlanEntryTotalGet(bIndex + 1);
                   bSubIndex++)
              {
                memset(strUITx, 0, sizeof(strUITx));
                bRequestParameter1 = bIndex + 1;
                bRequestParameter2 = bSubIndex + 1;
                strcat(strUITx, UI_COMM_PACKET_SEPARATOR_STR);
                UIPacketRespWPEntry();

                if (strlen(strUIMCSUpload) + strlen(strUITx)
                    < UI_COMM_MAX_MCS_PACKET_LENGTH)
                {
                  strcat(strUIMCSUpload, strUITx);
                }
                else
                {
                  bPacketName = 0;
                  break;
                }
              }

              bSubIndexStart = 0;

              if (bPacketName == 0)
              {
                break;
              }
            }

            if (bPacketName != 0)
            {
              bIndexStart = 0;
              bPacketName++;
            }

            break;
          }

          case NumberofProgramTableRows:
          {
            memset(strUITx, 0, sizeof(strUITx));
            strcat(strUITx, UI_COMM_PACKET_SEPARATOR_STR);
            UIPacketRespSPPTotal();

            if (strlen(strUIMCSUpload) + strlen(strUITx)
                < UI_COMM_MAX_MCS_PACKET_LENGTH)
            {
              strcat(strUIMCSUpload, strUITx);
              bIndexStart = 0;
              bSubIndexStart = 0;
              bPacketName++;
            }
            else
            {
              bPacketName = 0;
            }

            break;
          }

          case ProgramTableRow:
          {
            for (bIndex = bIndexStart; bIndex < SigProgPlanTotalGet(); bIndex++)
            {
              for (bSubIndex = bSubIndexStart;
                   bSubIndex < SigProgPlanEntryTotalGet(bIndex + 1);
                   bSubIndex++)
              {
                memset(strUITx, 0, sizeof(strUITx));
                bRequestParameter1 = bIndex + 1;
                bRequestParameter2 = bSubIndex + 1;
                strcat(strUITx, UI_COMM_PACKET_SEPARATOR_STR);
                UIPacketRespSPPlanEntry();

                if (strlen(strUIMCSUpload) + strlen(strUITx)
                    < UI_COMM_MAX_MCS_PACKET_LENGTH)
                {
                  strcat(strUIMCSUpload, strUITx);
                }
                else
                {
                  bPacketName = 0;
                  break;
                }
              }

              bSubIndexStart = 0;

              if (bPacketName == 0)
              {
                break;
              }
            }

            if (bPacketName != 0)
            {
              bIndexStart = 0;
              bPacketName++;
            }

            break;
          }

          case NumberofSignalPlans:
          {
            memset(strUITx, 0, sizeof(strUITx));
            strcat(strUITx, UI_COMM_PACKET_SEPARATOR_STR);
            UIPacketRespSignalPlanTotal();

            if (strlen(strUIMCSUpload) + strlen(strUITx)
                < UI_COMM_MAX_MCS_PACKET_LENGTH)
            {
              strcat(strUIMCSUpload, strUITx);
              bIndexStart = 0;
              bPacketName++;
            }
            else
            {
              bPacketName = 0;
            }

            break;
          }

          case SignalPlan:
          {
            for (bIndex = bIndexStart; bIndex < SignalPlanTotalGet(); bIndex++)
            {
              memset(strUITx, 0, sizeof(strUITx));
              bRequestParameter1 = bIndex + 1;
              strcat(strUITx, UI_COMM_PACKET_SEPARATOR_STR);
              UIPacketRespSignalPlan();

              if (strlen(strUIMCSUpload) + strlen(strUITx)
                  < UI_COMM_MAX_MCS_PACKET_LENGTH)
              {
                strcat(strUIMCSUpload, strUITx);
              }
              else
              {
                bPacketName = 0;
                break;
              }

              if (bPacketName == 0)
              {
                break;
              }
            }

            if (bPacketName != 0)
            {
              bIndexStart = 0;
              bPacketName++;
            }

            break;
          }

          case NumberofScheduleRows:
          {
            memset(strUITx, 0, sizeof(strUITx));
            strcat(strUITx, UI_COMM_PACKET_SEPARATOR_STR);
            UIPacketRespWSTotal();

            if (strlen(strUIMCSUpload) + strlen(strUITx)
                < UI_COMM_MAX_MCS_PACKET_LENGTH)
            {
              strcat(strUIMCSUpload, strUITx);
              bIndexStart = 0;
              bPacketName++;
            }
            else
            {
              bPacketName = 0;
            }

            break;
          }

          case ScheduleRow:
          {
            for (bIndex = bIndexStart;
                 bIndex < WorkScheduleTotalGet();
                 bIndex++)
            {
              memset(strUITx, 0, sizeof(strUITx));
              bRequestParameter1 = bIndex + 1;
              strcat(strUITx, UI_COMM_PACKET_SEPARATOR_STR);
              UIPacketRespWSEntry();

              if (strlen(strUIMCSUpload) + strlen(strUITx)
                  < UI_COMM_MAX_MCS_PACKET_LENGTH)
              {
                strcat(strUIMCSUpload, strUITx);
              }
              else
              {
                bPacketName = 0;
                break;
              }

              if (bPacketName == 0)
              {
                break;
              }
            }

            if (bPacketName != 0)
            {
              UISetMCSUploadInProgress(FALSE);
              bPacketName = 0;
            }

            break;
          }
      } /* switch */
    }

    memset(strUITx, 0, sizeof(strUITx));
    strcat(strUITx, strUIMCSUpload);
  }
  else
  {
    UIFailureResponseSet();
  }
} /* UIPacketRespMCSUpload */

void UIRxRequest(uint8_t bReqId, char *pstrData, uint16_t sDataSize)
{
  if ((bReqId >= UI_REQ_TYPE_FIRST) && (bReqId <= UI_REQ_TYPE_LAST) )
  {
    tpSUIRequest pSReq = (tpSUIRequest) osMemoryPoolAlloc(UIRxReqsMemPoolHandle,
                                                          0);

    if (pSReq != NULL)
    {
      memset(pSReq, 0, sizeof(tSUIRequest));

      pSReq->bReqId = bReqId;
      pSReq->sDataSize = sDataSize;
      memcpy(pSReq->strData, pstrData, sDataSize);

      if (osMessageQueuePut(UIRxReqsQueHandle, &pSReq, 0, 0) != osOK)
      {
        osMemoryPoolFree(UIRxReqsMemPoolHandle, pSReq);
      }
    }
  }
}

void UITxRequest(uint8_t bReqId, char *pstrData, uint16_t sDataSize)
{
  if ((bReqId >= UI_REQ_TYPE_FIRST) && (bReqId <= UI_REQ_TYPE_LAST) )
  {
    tpSUIRequest pSReq = (tpSUIRequest) osMemoryPoolAlloc(UITxReqsMemPoolHandle,
                                                          0);

    if (pSReq != NULL)
    {
      memset(pSReq, 0, sizeof(tSUIRequest));

      pSReq->bReqId = bReqId;
      pSReq->sDataSize = sDataSize;
      memcpy(pSReq->strData, pstrData, sDataSize);

      if (osMessageQueuePut(UITxReqsQueHandle, &pSReq, 0, 0) != osOK)
      {
        osMemoryPoolFree(UITxReqsMemPoolHandle, pSReq);
      }
    }
  }
}

void UIMsgParse(tpSUIRequest pSUIReq)
{
  if ((pSUIReq->bReqId >= UI_REQ_TYPE_FIRST)
      && (pSUIReq->bReqId <= UI_REQ_TYPE_LAST))
  {
    if (pSUIReq->strData[1] == '\r')
    {
      strcpy(pSUIReq->strData, "-REQ:LGN\r");
    }

    /* received a packet */
    /* reset error state */
    bUICommResponse = UI_COMM_RESPONSE_SUCCESS;

    /* process data if no errors are encountered so far */
    do
    {
      if (SRawSCP.bStatus == UI_COMM_BACKUP_SCP_STATE_READING)
      {
        UISCPBackupNextPacketGet(pSUIReq);
      }

      if (bUICommResponse == UI_COMM_RESPONSE_SUCCESS)
      {
        uint32_t lPacketName;

        /* init Rx packet parser */
        UIRxPacketParserInit(pSUIReq->strData);

        /* extract packet name */
        if (UIRxPacketNameGet(&lPacketName))
        {
          uint8_t bMsgSwitcherFlag = 1;

          if (SRawSCP.bStatus == UI_COMM_BACKUP_SCP_STATE_SAVING)
          {
            switch (lPacketName)
            {
                case UI_COMM_PACKET_DEVICE_INFO:
                case UI_COMM_PACKET_SIGNAL:
                case UI_COMM_PACKET_SIGNALS_DEFINED:
                case UI_COMM_PACKET_SG:
                case UI_COMM_PACKET_SO:
                case UI_COMM_PACKET_CVS:
                case UI_COMM_PACKET_CLEARANCE:
                case UI_COMM_PACKET_CONFLICT_EM:
                case UI_COMM_PACKET_SEQUENCE_STEP:
                case UI_COMM_PACKET_PHASE:
                case UI_COMM_PACKET_INPUT:
                case UI_COMM_PACKET_OUTPUT:
                case UI_COMM_PACKET_OPERATIONS:
                case UI_COMM_PACKET_STATEMENTS:
                case UI_COMM_PACKET_RULES:
                case UI_COMM_PACKET_TRANSITION:
                case UI_COMM_PACKET_SIGNAL_PROGRAM:
                case UI_COMM_PACKET_WP_ENTRY:
                case UI_COMM_PACKET_SP_PLAN_ENTRY:
                case UI_COMM_PACKET_SIGNAL_PLAN:
                case UI_COMM_PACKET_WS_ENTRY:
                case UI_COMM_PACKET_END_CONF:
                {
                  if (UISCPBackupNextPacketSet(pSUIReq, lPacketName) == FALSE) /* if error */
                  {
                    SRawSCP.bStatus = UI_COMM_BACKUP_SCP_STATE_ERROR;
                    bUICommResponse = UI_COMM_RESPONSE_FLASH_ERROR;
                  }

                  bMsgSwitcherFlag = 0;
                  break;
                }

                default:
                {
                  break;
                }
            }
          }

          if (bMsgSwitcherFlag) /* if stream handler directs the msg to parser */
          {
            switch (lPacketName)
            {
                /* Start of Upload (STC) */
                case UI_COMM_PACKET_START_CONF:
                {
                  if (pSUIReq->bReqId != UI_REQ_TYPE_TCP_CLIENT) /* if USB or RS232 */
                  {
                    UserSettingsRead();
                    if (UserSettingsConfigFlagGet()
                        == FALSE) /* if config lock is set */
                    {
                      bUICommResponse = UI_COMM_RESPONSE_SECURITY_ERROR;
                      break;
                    }

                    if (SetSigModeIsOK())
                    {
                      switch (StateCurrentGet())
                      {
                          case STATES_SEQ:
                          case STATES_PHASE:
                          case STATES_PHASE_TRANSITION:
                          {
                            StateCurrentSet(STATES_SECURE_TRANSITION);
                            bUICommResponse =
                              UI_COMM_RESPONSE_IN_SECURE_TRANSITION;
                            break;
                          }

                          case STATES_SECURE_TRANSITION:
                          {
                            bUICommResponse =
                              UI_COMM_RESPONSE_IN_SECURE_TRANSITION;
                            break;
                          }

                          default:
                          {
                            if (SetProgramLoadingFlag(TRUE))
                            {
                              SetProgramLoading(TRUE);
                              LogRequest(LOG_REQ_APPEND, NULL,
                                         EVENT_MCT_CONFIGURATION_STARTS,
                                         0, 0, 0, 0);
                              UIConfStart(pSUIReq->bReqId);
                              LoadProgramStarts();
                              UICheckDownloadTimeoutSet(TRUE);

                              if (GpsModemAliveGet())
                              {
                                GpsRTCInitialUpdateDoneSet(FALSE);
                              }
                            }

                            break;
                          }
                      }
                    }
                    else
                    {
                      if (SetProgramLoadingFlag(TRUE))
                      {
                        SetProgramLoading(TRUE);
                        LogRequest(LOG_REQ_APPEND, NULL,
                                   EVENT_MCT_CONFIGURATION_STARTS,
                                   0, 0, 0, 0);
                        UIConfStart(pSUIReq->bReqId);
                        LoadProgramStarts();
                        UICheckDownloadTimeoutSet(TRUE);

                        if (GpsModemAliveGet())
                        {
                          GpsRTCInitialUpdateDoneSet(FALSE);
                        }
                      }
                    }
                  }
                  else
                  {
                    if (SRawSCP.bStatus != UI_COMM_BACKUP_SCP_STATE_COMPLETED) /* if stream saving */
                    /* continues */
                    {
                      LogRequest(LOG_REQ_APPEND_ASYNCH, NULL,
                                 EVENT_MCS_USER_REQUEST_DOWNLOAD,
                                 0, 0, 0, 0);
                      SRawSCP.bStatus =
                        UI_COMM_BACKUP_SCP_STATE_SAVING; /* change the state to */
                      /* direct the download */
                      /* packages to temp */
                      /* buff */
                      LogRequest(LOG_REQ_APPEND, NULL,
                                 EVENT_MCT_CONFIGURATION_STARTS,
                                 0, 0, 0, 0);

                      UISetMCSDownloadInProgress(TRUE);
                    }
                    else /* second try before loading first sent program */
                    {
                      memset(&SRawSCP, 0, sizeof(SRawSCP));
                      SRawSCP.bStatus = UI_COMM_BACKUP_SCP_STATE_SAVING;
                    }
                  }

                  break;
                }

                /* Time (TIM) */
                case UI_COMM_PACKET_TIME:
                {
                  UIPacketTime();
                  break;
                }

                /* Device Info (INT) */
                case UI_COMM_PACKET_DEVICE_INFO:
                {
                  UIPacketDeviceInfo(pSUIReq->bReqId);
                  break;
                }

                /* Signals (SIN) */
                case UI_COMM_PACKET_SIGNAL:
                {
                  UIPacketSignal();
                  break;
                }

                /* Signal Assignments (SND) */
                case UI_COMM_PACKET_SIGNALS_DEFINED:
                {
                  UIPacketSignalsDefined();
                  break;
                }

                /* Signal Groups (SGI) */
                case UI_COMM_PACKET_SG:
                {
                  UIPacketSG();
                  break;
                }

                /* Signal Outputs (SOX) */
                case UI_COMM_PACKET_SO:
                {
                  UIPacketSO();
                  break;
                }

                /* Current Voltage Sensor (CVS) */
                case UI_COMM_PACKET_CVS:
                {
                  UIPacketCVS();
                  break;
                }

                /* Conflicts (CFX) */
                case UI_COMM_PACKET_CLEARANCE:
                {
                  UIPacketClearance();
                  break;
                }

                /* Failure Action (CEM) */
                case UI_COMM_PACKET_CONFLICT_EM:
                {
                  UIPacketConflictEM();
                  break;
                }

                /* Signal Sequence Step (SSX) */
                case UI_COMM_PACKET_SEQUENCE_STEP:
                {
                  UIPacketSeqStep();
                  break;
                }

                /* Phase (PHX) */
                case UI_COMM_PACKET_PHASE:
                {
                  UIPacketPhase();
                  break;
                }

                /* Inputs (INP) */
                case UI_COMM_PACKET_INPUT:
                {
                  UIPacketInput();
                  break;
                }

                /* Outputs (OUT) */
                case UI_COMM_PACKET_OUTPUT:
                {
                  UIPacketOutput();
                  break;
                }

                /* Expressions (OPT) */
                case UI_COMM_PACKET_OPERATIONS:
                {
                  UIPacketOperation();
                  break;
                }

                /* Statements (STM) */
                case UI_COMM_PACKET_STATEMENTS:
                {
                  UIPacketStatement();
                  break;
                }

                /* Rule (RUL) */
                case UI_COMM_PACKET_RULES:
                {
                  UIPacketRule();
                  break;
                }

                /* Transition (PTX) */
                case UI_COMM_PACKET_TRANSITION:
                {
                  UIPacketTransition();
                  break;
                }

                /* Program End (SPR) */
                case UI_COMM_PACKET_SIGNAL_PROGRAM:
                {
                  UIPacketSigProg();
                  break;
                }

                /* Fixed Time Table Row (ENX) */
                case UI_COMM_PACKET_WP_ENTRY:
                {
                  UIPacketWPEntry();
                  break;
                }

                /* Program Table Row (SPP) */
                case UI_COMM_PACKET_SP_PLAN_ENTRY:
                {
                  UIPacketSPPlanEntry();
                  break;
                }

                /* Signal Plan (SPX) */
                case UI_COMM_PACKET_SIGNAL_PLAN:
                {
                  UIPacketSignalPlan();
                  break;
                }

                /* Schedule Row (WSE) */
                case UI_COMM_PACKET_WS_ENTRY:
                {
                  UIPacketWSEntry();
                  break;
                }

                /* SMS Recipient (SMU) */
                case UI_COMM_PACKET_SMS_USER:
                {
                  UIPacketSMSUser();
                  break;
                }

                /* Implement stored packages (SNC) */
                case UI_COMM_PACKET_IMP_CONF:
                {
                  if (SRawSCP.bStatus == UI_COMM_BACKUP_SCP_STATE_COMPLETED)
                  {
                    if (SetSigModeIsOK())
                    {
                      switch (StateCurrentGet())
                      {
                          case STATES_SEQ:
                          case STATES_PHASE:
                          case STATES_PHASE_TRANSITION:
                          {
                            StateCurrentSet(STATES_SECURE_TRANSITION);
                            do
                            {
                              osDelay(100);
                            }while (StateCurrentGet()
                                    == STATES_SECURE_TRANSITION
                                    && SetSigModeIsOK());

                            break;
                          }
                      }
                    }

                    if (SetProgramLoadingFlag(TRUE))
                    {
                      SetProgramLoading(TRUE);
                      UIConfStart(pSUIReq->bReqId);
                      LoadProgramStarts();

                      SRawSCP.bStatus = UI_COMM_BACKUP_SCP_STATE_READING;

                      UIMCSAsySucMsgSend();

                      if (GpsModemAliveGet())
                      {
                        GpsRTCInitialUpdateDoneSet(FALSE);
                      }
                    }
                  }
                  else /* error */
                  {
                    bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
                  }

                  break;
                }

                /* End of Upload (ENC) */
                case UI_COMM_PACKET_END_CONF:
                {
                  UIConfEnd();
                  LoadProgramEnds(); /* use new conf. */
                  SetProgramLoading(FALSE);
                  ResetCPMPComm();
                  LogRequest(LOG_REQ_APPEND, NULL,
                             EVENT_MCT_CONFIGURATION_ENDS,
                             0, 0, 0, 0);
                  UICheckDownloadTimeoutSet(FALSE);
                  UISetMCSDownloadInProgress(FALSE);

                  if (SRawSCP.bStatus == UI_COMM_BACKUP_SCP_STATE_READING)
                  {
                    memset(&SRawSCP, 0, sizeof(SRawSCP));
                    memset(baRawSCPData, 0, sizeof(baRawSCPData));
                  }

                  break;
                }

                case UI_COMM_PACKET_FLASH_CONF:
                {
                  UserSettingsRead();
                  if (UserSettingsConfigFlagGet() == FALSE) /* if config lock is */
                  /* set */
                  {
                    bUICommResponse = UI_COMM_RESPONSE_SECURITY_ERROR;
                    break;
                  }

                  if (ProgramDataSet())
                  {
                    bUICommResponse = UI_COMM_RESPONSE_SUCCESS;
                  }
                  else
                  {
                    bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
                  }

                  break;
                }

                case UI_COMM_PACKET_LOAD_SP:
                {
                  if (SetSigModeIsOK())
                  {
                    switch (StateCurrentGet())
                    {
                        case STATES_SEQ:
                        case STATES_PHASE:
                        case STATES_PHASE_TRANSITION:
                        {
                          StateCurrentSet(STATES_SECURE_TRANSITION);
                          do
                          {
                            osDelay(100);
                          }while (StateCurrentGet()
                                  == STATES_SECURE_TRANSITION
                                  && SetSigModeIsOK());

                          break;
                        }
                    }
                  }

                  LoadProgramEnds();
                  RestartProgram();
                  UIMCSAsySucMsgSend();
                  break;
                }

                /* H&D Commented */

                /*
                 *  case UI_COMM_PACKET_SETTINGS:
                 *  UIPacketSettings();
                 *  break;
                 */

                case UI_COMM_PACKET_IOM:
                {
                  UIPacketIOM();
                  break;
                }

                case UI_COMM_PACKET_SSM:
                {
                  UIPacketSSM();
                  break;
                }

                case UI_COMM_PACKET_SSM_TEST_END:
                {
                  StartSSMTest(SSM_TEST_FROM_NONE);
                  break;
                }

                case UI_COMM_PACKET_CLEAR_FLASH:
                {
                  DataInit(UI_REQ_TYPE_NONE, TRUE); /* clear RAM */
                  ProgramDataSet();
                  break;
                }

                case UI_COMM_PACKET_LOAD_DEFAULT:
                {
                  LoadProgramStarts();
                  ProgramDataSet(); /* update storage */
                  LoadProgramEnds();
                  RestartProgram();
                  break;
                }

                case UI_COMM_PACKET_FLASH_PERIODS:
                {
                  UIPacketFlashPeriods();
                  break;
                }

                case UI_COMM_PACKET_LOG_DEL:
                {
                  DeleteLogs();
                  break;
                }

                case UI_COMM_PACKET_SIGNALS:
                {
                  UIPacketSGSignals();
                  break;
                }

                case UI_COMM_PACKET_ADD_USER:
                {
                  UIPacketUserAdd();
                  break;
                }

                case UI_COMM_PACKET_REMOVE_USER:
                {
                  UIPacketUserRemove();
                  break;
                }

                case UI_COMM_PACKET_LCD_LANGUAGE:
                {
                  UIPacketLCDLanguage();
                  break;
                }

                case UI_COMM_PACKET_COMM_CONFIG:
                {
                  UIPacketCommConfig();
                  break;
                }

                case UI_COMM_PACKET_CLR_SO_POWERS:
                {
                  ClearSOPowers();
                  break;
                }

                case UI_COMM_PACKET_LINE_OPERATOR:
                {
                  UIPacketLineOperator();
                  break;
                }

                case UI_COMM_PACKET_POP3:
                {
                  UIPacketPop3();
                  break;
                }

                case UI_COMM_PACKET_SMTP:
                {
                  UIPacketSmtp();
                  break;
                }

                case UI_COMM_PACKET_REMOTE_CONN:
                {
                  UIPacketRemoteConn();
                  break;
                }

                case UI_COMM_PACKET_REQUEST:
                {
                  UIPacketRequest(pSUIReq);
                  break;
                }

                case UI_COMM_PACKET_CHANGE_WORKMODE:
                {
                  UIPacketChangeWorkmode();
                  break;
                }

                case UI_COMM_PACKET_PHASE_DURATION_CHANGE:
                {
                  UIPacketPhaseDurationChange();
                  break;
                }

                case UI_COMM_PACKET_FUNCTION_CONF:
                {
                  UIPacketFuncConfChange();
                  break;
                }

                case UI_COMM_PACKET_INP_DATA_MANIP:
                {
                  UIPacketInputDataManip();
                  break;
                }

                case UI_COMM_PACKET_SYSTEM_START_TIME:
                {
                  UIPacketSetSystemStartTime();
                  break;
                }

                case UI_COMM_PACKET_DEBUG:
                {
                  #ifdef DEBUG
                  UIPacketDebug();
                  #endif
                  break;
                }

                case UI_COMM_PACKET_IAP:
                {
                  uint8_t bSrc =
                    pSUIReq->bReqId
                    == UI_REQ_TYPE_SERIAL ? IAP_REQUEST_SOURCE_UI_SERIAL
                    :IAP_REQUEST_SOURCE_UI_USB;

                  UIPacketIAP(bSrc,
                              &pSUIReq->strData[UI_COMM_IAP_DATA_LEN_INDEX]);
                  break;
                }

                case UI_COMM_PACKET_MCS_CON_INFO:
                {
                  UIPacketMCSConInfo();
                  break;
                }

                default:
                {
                  /* packet name is not a valid */
                  bUICommResponse = UI_COMM_RESPONSE_OPERATION_ERROR;
                  break;
                }
            } /* switch */
          }
        }
      }
      else
      {
        SRawSCP.bStatus = UI_COMM_BACKUP_SCP_STATE_NONE;
      }
    } while (SRawSCP.bStatus == UI_COMM_BACKUP_SCP_STATE_READING);

    if (SRawSCP.bStatus == UI_COMM_BACKUP_SCP_STATE_ERROR)
    {
      memset(&SRawSCP, 0, sizeof(SRawSCP));
      memset(baRawSCPData, 0, sizeof(baRawSCPData));

      UICheckDownloadTimeoutSet(FALSE);
      UISetMCSDownloadInProgress(FALSE);
    }

    /* prepare response */
    /* start of packet */
    memset(strUITx, 0, sizeof(strUITx));
    strUITx[0] = UI_COMM_START_OF_PACKET;
    strUITx[1] = '\0';
    /* packet name and data */
    switch (bUICommResponse)
    {
        case UI_COMM_RESPONSE_SUCCESS:
        {
          strcat(strUITx, UI_COMM_PACKET_SUCCESS_STR);
          break;
        }

        case UI_COMM_RESPONSE_CHECKSUM_ERROR:
        {
          strcat(strUITx, UI_COMM_PACKET_CHECKSUM_ERROR_STR);
          break;
        }

        case UI_COMM_RESPONSE_FRAME_ERROR:
        {
          strcat(strUITx, UI_COMM_PACKET_FRAME_ERROR_STR);
          break;
        }

        case UI_COMM_RESPONSE_BUF_SIZE_ERROR:
        {
          strcat(strUITx, UI_COMM_PACKET_BUF_SIZE_ERROR_STR);
          break;
        }

        case UI_COMM_RESPONSE_OPERATION_ERROR:
        {
          strcat(strUITx, UI_COMM_PACKET_OPERATION_ERROR_STR);
          break;
        }

        case UI_COMM_RESPONSE_SECURITY_ERROR:
        {
          strcat(strUITx, UI_COMM_PACKET_SECURITY_ERROR_STR);
          break;
        }

        case UI_COMM_RESPONSE_IN_SECURE_TRANSITION:
        {
          strcat(strUITx, UI_COMM_PACKET_IN_SECURE_TRANSITION_STR);
          break;
        }

        case UI_COMM_RESPONSE_FLASH_ERROR:
        {
          strcat(strUITx, UI_COMM_PACKET_FLASH_ERROR_STR);
          break;
        }

        case UI_COMM_RESPONSE_PACKET:
        {
          switch (lRequestedPacket)
          {
              case UI_COMM_PACKET_END_CONF:
              {
                char strData[4];

                strcat(strUITx, UI_COMM_PACKET_SUCCESS_STR);
                strcat(strUITx, UI_COMM_DATA_SEPARATOR_STR);
                sprintf(strData, "%d", fUICommResponse);
                strcat(strUITx, strData);
                break;
              }

              case UI_COMM_PACKET_IOM:
              {
                UIPacketRespIOM();
                break;
              }

              case UI_COMM_PACKET_PSM:
              {
                UIPacketRespPSM();
                break;
              }

              case UI_COMM_PACKET_SSM:
              {
                UIPacketRespSSM();
                break;
              }

              case UI_COMM_PACKET_SIGNAL:
              {
                UIPacketRespSignal();
                break;
              }

              case UI_COMM_PACKET_CVS:
              {
                UIPacketRespCVS();
                break;
              }

              case UI_COMM_PACKET_OPERATIONS:
              {
                UIPacketRespOperation();
                break;
              }

              case UI_COMM_PACKET_SIGNAL_PROGRAM:
              {
                UIPacketRespSigProg();
                break;
              }

              case UI_COMM_PACKET_RULES:
              {
                UIPacketRespRule();
                break;
              }

              case UI_COMM_PACKET_STATEMENTS:
              {
                UIPacketRespStatement();
                break;
              }

              case UI_COMM_PACKET_SIGNALS_DEFINED:
              {
                UIPacketRespSignalsDefined();
                break;
              }

              case UI_COMM_PACKET_DEVICE_INFO:
              {
                UIPacketRespDeviceInfo();
                break;
              }

              case UI_COMM_PACKET_SG:
              {
                UIPacketRespSG();
                break;
              }

              case UI_COMM_PACKET_CONFLICT_EM:
              {
                UIPacketRespConflictEM();
                break;
              }

              case UI_COMM_PACKET_SO:
              {
                UIPacketRespSO();
                break;
              }

              case UI_COMM_PACKET_CLEARANCE:
              {
                UIPacketRespClearance();
                break;
              }

              case UI_COMM_PACKET_PHASE:
              {
                UIPacketRespPhase();
                break;
              }

              case UI_COMM_PACKET_TRANSITION:
              {
                UIPacketRespTransition();
                break;
              }

              case UI_COMM_PACKET_SIGNAL_PLAN:
              {
                UIPacketRespSignalPlan();
                break;
              }

              case UI_COMM_PACKET_SP_PLAN_ENTRY:
              {
                UIPacketRespSPPlanEntry();
                break;
              }

              case UI_COMM_PACKET_WP_ENTRY:
              {
                UIPacketRespWPEntry();
                break;
              }

              case UI_COMM_PACKET_WS_ENTRY:
              {
                UIPacketRespWSEntry();
                UICheckUploadTimeoutSet(FALSE);
                ProgramStateSet(PROGRAM_STATE_DARK);
                SetProgramLoadingStatus(PROGRAM_UPLOADING_SUCCESS);
                UICheckUploadTimeoutSet(FALSE);
                UISetMCSUploadInProgress(FALSE);
                break;
              }

              case UI_COMM_PACKET_FLASH_PERIODS:
              {
                UIPacketRespFlashPeriods();
                break;
              }

              /* ISSD function */
              case UI_COMM_PACKET_OPEN_RELAY:
              {
                if (GetFunctionConfByIndex(LIC_ISSD) == 0)
                {
                  UIPacketRespOpenRelay();
                }

                break;
              }

              case UI_COMM_PACKET_CLOSE_RELAY:
              {
                if (GetFunctionConfByIndex(LIC_ISSD) == 0)
                {
                  UIPacketRespCloseRelay();
                }

                break;
              } /* ISSD function */

              case UI_COMM_PACKET_LD_MANIP1:
              {
                UIPacketRespLdManip1();
                break;
              }

              case UI_COMM_PACKET_LB_MANIP1:
              {
                UIPacketRespLbManip1();
                break;
              }

              case UI_COMM_PACKET_LD_MANIP2:
              {
                UIPacketRespLdManip2();
                break;
              }

              case UI_COMM_PACKET_LB_MANIP2:
              {
                UIPacketRespLbManip2();
                break;
              }

              case UI_COMM_PACKET_INPUT_MANIP1:
              case UI_COMM_PACKET_INPUT_MANIP2:
              {
                UIPacketRespDIManip(lRequestedPacket);
                break;
              }

              case UI_COMM_PACKET_IO_RUNTIME:
              {
                UIPacketRespIORuntime();
                break;
              }

              case UI_COMM_PACKET_SEQUENCE_STEP:
              {
                UIPacketRespSeqStep();
                break;
              }

              case UI_COMM_PACKET_TIME:
              {
                ProgramStateSet(PROGRAM_STATE_UPLOADING);
                SetProgramLoadingStatus(PROGRAM_UPLODING_IN_PROGRESS);
                UIPacketRespTime();
                break;
              }

              case UI_COMM_PACKET_SIGNALS:
              {
                UIPacketRespSGSignals();
                break;
              }

              case UI_COMM_PACKET_LOG_NEXT:
              {
                UIPacketRespLogNext(pSUIReq); /* read the next new log */
                break;
              }

              case UI_COMM_PACKET_LOG_FROM:
              {
                UIPacketRespLogFrom(); /* read the log whose index is given in */
                /* parameter */
                break;
              }

              case UI_COMM_PACKET_LOG_LAST_INDEX:
              {
                UIPacketRespLogLastIndex();
                break;
              }

              case UI_COMM_PACKET_GET_USERNAMES:
              {
                UIPacketRespUsernames();
                break;
              }

              case UI_COMM_PACKET_LCD_LANGUAGE:
              {
                UIPacketRespLCDLanguage();
                break;
              }

              case UI_COMM_PACKET_COMM_CONFIG:
              {
                UIPacketRespCommConfig();
                break;
              }

              case UI_COMM_PACKET_SG_NUMBER:
              {
                UIPacketRespSGTotal();
                break;
              }

              case UI_COMM_PACKET_WS_NUMBER:
              {
                UIPacketRespWSTotal();
                break;
              }

              case UI_COMM_PACKET_SIN_NUMBER:
              {
                UIPacketRespSinTotal();
                break;
              }

              case UI_COMM_PACKET_CONFLICT_NUMBER:
              {
                UIPacketRespConflictTotal();
                break;
              }

              case UI_COMM_PACKET_SP_NUMBER:
              {
                UIPacketRespSignalPlanTotal();
                break;
              }

              case UI_COMM_PACKET_SO_NUMBER:
              {
                UIPacketRespSOTotal();
                break;
              }

              case UI_COMM_PACKET_CFL_NUMBER:
              {
                UIPacketRespSGConflictTotal();
                break;
              }

              case UI_COMM_PACKET_SPR_NUMBER:
              {
                UIPacketRespSPTotal();
                break;
              }

              case UI_COMM_PACKET_OPT_NUMBER:
              {
                UIPacketRespOperationTotal();
                break;
              }

              case UI_COMM_PACKET_PHA_NUMBER:
              {
                UIPacketRespPhaseTotal();
                break;
              }

              case UI_COMM_PACKET_TRA_NUMBER:
              {
                UIPacketRespTransitionTotal();
                break;
              }

              case UI_COMM_PACKET_WP_NUMBER:
              {
                UIPacketRespWPTotal();
                break;
              }

              case UI_COMM_PACKET_WP_ENTRY_NUMBER:
              {
                UIPacketRespWPEntryTotal();
                break;
              }

              case UI_COMM_PACKET_SPP_NUMBER:
              {
                UIPacketRespSPPTotal();
                break;
              }

              case UI_COMM_PACKET_SEQ_NUMBER:
              {
                UIPacketRespSeqTotal();
                break;
              }

              case UI_COMM_PACKET_SEQ_STEP_NUMBER:
              {
                UIPacketRespSeqStepTotal();
                break;
              }

              case UI_COMM_PACKET_DET_NUMBER:
              {
                UIPacketRespDetectorTotal();
                break;
              }

              case UI_COMM_PACKET_STATEMENT_NUMBER:
              {
                UIPacketRespStatementTotal();
                break;
              }

              case UI_COMM_PACKET_INPUT_NUMBER:
              {
                UIPacketRespInputDigitalTotal();
                break;
              }

              case UI_COMM_PACKET_SMS_USER_NUMBER:
              {
                break;
              }

              case UI_COMM_PACKET_OUTPUT_NUMBER:
              {
                UIPacketRespOutputTotal();
                break;
              }

              case UI_COMM_PACKET_INPUT:
              {
                UIPacketRespInput();
                break;
              }

              case UI_COMM_PACKET_OUTPUT:
              {
                UIPacketRespOutput();
                break;
              }

              case UI_COMM_PACKET_SMS_USER:
              {
                break;
              }

              case UI_COMM_PACKET_CURRENT_RUNTIME_INFO:
              {
                UIPacketRespCurrentRuntimeInfo();
                break;
              }

              case UI_COMM_PACKET_CHECKSUM_TOTAL: /* DENIZLI FUNCTION */
              {
                if (GetFunctionConfByIndex(LIC_DEN))
                {
                  UIPacketRespChecksumTotal();
                }

                break;
              }

              case UI_COMM_PACKET_MODULE_VERSIONS: /* DENIZLI FUNCTION */
              {
                if (GetFunctionConfByIndex(LIC_DEN))
                {
                  UIPacketRespModuleVersions();
                }

                break;
              }

              case UI_COMM_PACKET_MAESTRO_MODULE_VERSIONS:
              {
                UIPacketRespMaestroModuleVersion();
                break;
              }

              case UI_COMM_PACKET_FUNCTION_CONF:
              {
                UIPacketRespFuncConf();
                break;
              }

              case UI_COMM_PACKET_MCS_UPLOAD:
              {
                UIPacketRespMCSUpload();
                break;
              }

              case UI_COMM_PACKET_SYSTEM_START_TIME:
              {
                UIPacketRespSystemStartTime();
                break;
              }

              case UI_COMM_PACKET_RESET_CPU:
              {
                UISecureSystemReset();
                break;
              }

              case UI_COMM_PACKET_IAP:
              {
                break;
              }

              case UI_COMM_PACKET_MCS_CON_INFO:
              {
                UIPacketRespMCSConInfo();
                break;
              }

              case UI_COMM_PACKET_DEBUG:
              {
                #ifdef DEBUG
                UIPacketRespDebug();
                #endif
                break;
              }

              default:
              {
                break;
              }
          } /* switch */

          break;
        }
    } /* switch */

    /* end of packet */
    switch (lRequestedPacket)
    {
        case UI_COMM_PACKET_SSM:
        {
          /* end of packet is already appended when preparing packet content */
          sStrUITxLen = strlen(strUITx);
          break;
        }

        default:
        {
          strcat(strUITx, UI_COMM_END_OF_PACKET_STR);
          sStrUITxLen = strlen(strUITx);
        }
    }

    UITxRequest(pSUIReq->bReqId, strUITx, sStrUITxLen);
  }
} /* UIMsgParse */

void UIMsgSend(tpSUIRequest pSReq)
{
  switch (pSReq->bReqId)
  {
      case UI_REQ_TYPE_SERIAL:
      {
        (void) SerialSend(s_port,
                          (const uint8_t *) pSReq->strData,
                          pSReq->sDataSize,
                          UI_DMA_TX_TIMEOUT);
        break;
      }

      case UI_REQ_TYPE_USB:
      {
        USBStartTx((uint8_t *) pSReq->strData, pSReq->sDataSize);
        break;
      }

      case UI_REQ_TYPE_TCP_CLIENT:
      {
        MCSAsynchReqTxMsg(MCS_ASYNCH_HEADER_UPLOAD,
                          pSReq->sDataSize,
                          (uint8_t *) pSReq->strData);
        break;
      }
  }
}

/* /////////////////////////// */
/*  tasks */
void UIMsgParserTaskFunc(void *argument)
{
  UNUSED(argument);

  tpSUIRequest pSReq = NULL;

  osDelay(1000);

  UIInit(&g_auxSerialPort);
  USBStartRx();

  while (FOREVER)
  {
    if (osMessageQueueGet(UIRxReqsQueHandle, &pSReq, NULL,
                          osWaitForever) == osOK)
    {
      UIMsgParse(pSReq);
      osMemoryPoolFree(UIRxReqsMemPoolHandle, pSReq);

      if (UICheckDownloadTimeoutGet())
      {
        SRuntimes.SUIRuntime.sDownloadDuration = 0;
      }

      if (UICheckUploadTimeoutGet())
      {
        SRuntimes.SUIRuntime.sUploadDuration = 0;
      }

      USBStartRx();
    }
  }
}

void UIMsgSenderTaskFunc(void *argument)
{
  UNUSED(argument);
  tpSUIRequest pSReq = NULL;

  osDelay(1000);

  while (FOREVER)
  {
    if (osMessageQueueGet(UITxReqsQueHandle, &pSReq, NULL,
                          osWaitForever) == osOK)
    {
      UIMsgSend(pSReq);
      osMemoryPoolFree(UITxReqsMemPoolHandle, pSReq);
    }
  }
}
