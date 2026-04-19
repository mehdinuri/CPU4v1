/**
 ******************************************************************************
 * @file    MLM.c
 * @author  Okan KILIC - Teknotel Electronics
 * @version V1.0.0
 * @date    08/11/2011
 * @brief  Maestro Log Management
 *       This file includes all definitions and Methods of MLM
 ******************************************************************************
 */

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Include Files */
#include "MLM.h"

#include <string.h>

#include "defs.h"
#include "main.h"
#include "PersistencePorts.h"
#include "time.h"

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  members */
/*  private members */
/*  log operation request queue */

/*  log indexes */
/*  store these variables in flash, sLogWriteIndex value is between (1) and */
/* (LOG_RECORDS_MAX - 1), but the log pointer   is meaningful, only fLogExists */
/* is TRUE */
static uint16_t sLogWriteIndex; /* points to the space new log will be written */
static uint8_t fLogExists; /* if at least one log exists, assign TRUE */
static uint16_t sNumberOfLogRecords; /* number of meaningful records, value is */
/* between 0..LOG_RECORDS_MAX */

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  methods */
/*  private methods */
/*  public methods */
void MLMInit(void)
{
  sLogWriteIndex = 0;   /* this will be assigned to the real value by reading it */
  /* from flash memory */
  fLogExists = FALSE;
  sNumberOfLogRecords = 0;
}

uint8_t LogExists(void)
{
  return LogRepositoryExists(&g_logRepositoryPort);
}

/*  parameter is the log read index value of the caller task */
/*  function has two aims */
/*  first: is a new log is written to the log after the last read from log? */
/*  second: get the sLogWriteIndexValue */
uint16_t LogEventNew(uint16_t sTaskLogReadIndex)
{
  uint16_t sLogWriteIndexValue =
    LogRepositoryGetWriteIndex(&g_logRepositoryPort);

  if (sLogWriteIndexValue != sTaskLogReadIndex)
  {
    if (sLogWriteIndexValue == 0U)
    {
      return LOG_RECORDS_MAX - 1;
    }

    return sLogWriteIndexValue - 1U;
  }

  return LOG_NO_NEW_LOG;
}

uint8_t LogRequest(uint8_t bReqID,
                   tpSLogRecord pSLogReadBuf,
                   uint8_t bEvent,
                   uint8_t bParam,
                   uint16_t sParam,
                   uint32_t lParam,
                   uint16_t sTaskReadIndex)
{
  if ((osThreadGetId() == NULL) && (bReqID != LOG_REQ_APPEND_ASYNCH))
  {
    Error_Handler();
  }

  switch (bReqID)
  {
      case LOG_REQ_APPEND_ASYNCH:
      case LOG_REQ_APPEND:
      {
        tSTime SLogTime = { 0 };
        tSLogRecord SLogRecord = { 0 };
        uint16_t sWrittenIndex = 0U;
        uint8_t fResult;

        TimeGet(&SLogTime);

        SLogRecord.sYear = TimeFullYearCalc(&SLogTime);
        SLogRecord.bMonth = SLogTime.SCurrentDate.Month;
        SLogRecord.bMonthDay = SLogTime.SCurrentDate.Date;
        SLogRecord.bHours = SLogTime.SCurrentTime.Hours;
        SLogRecord.bMinutes = SLogTime.SCurrentTime.Minutes;
        SLogRecord.bSeconds = SLogTime.SCurrentTime.Seconds;
        SLogRecord.SEvent.bEvent = bEvent;
        SLogRecord.SEvent.bParam = bParam;
        SLogRecord.SEvent.sParam = sParam;
        SLogRecord.SEvent.lParam = lParam;

        fResult = LogRepositoryAppend(&g_logRepositoryPort,
                                      &SLogRecord,
                                      sizeof(SLogRecord),
                                      &sWrittenIndex);

        return fResult;
      }

      case LOG_REQ_READ_NEXT:
      case LOG_REQ_READ_FROM:
      {
        return LogRepositoryRead(&g_logRepositoryPort,
                                 sTaskReadIndex,
                                 pSLogReadBuf,
                                 sizeof(*pSLogReadBuf));
      }

      default:
      {
        break;
      }
  } /* switch */

  return FALSE;
} /* LogRequest */

void DeleteLogs(void)
{
  LogRepositoryClear(&g_logRepositoryPort);
}

uint8_t LogIndexIsValid(uint16_t sLogRecordIndex)
{
  return LogRepositoryIsIndexValid(&g_logRepositoryPort, sLogRecordIndex);
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Task code */
void MLMTaskFunc(void *argument)
{
  UNUSED(argument);

  MLMInit();
  osThreadTerminate(osThreadGetId());
} /* MLMTaskFunc */
