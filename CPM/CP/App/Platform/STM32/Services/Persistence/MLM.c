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

#include "MCSAsynch.h"
#include "defs.h"
#include "lcd.h"
#include "main.h"
#include "mmi.h"
#include "PersistencePorts.h"
#include "time.h"
#include "ui.h"

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

/*  event strings */
const char *pStrLogStrings[LANGUAGES_MAX][EVENT_LAST + 1] = { { "TANIMSIZ",
                                                                "CiHAZ AcILDI",
                                                                "Sc KISA DEVRE",
                                                                "Sc AcIK DEVRE",
                                                                "Sc GERiLiM SENS. AR.",
                                                                "Sc HARiCi BESLENiYOR",
                                                                "Sc cALIs. LAMBA SAY.",
                                                                "SG GoRuLEN SiNYAL",
                                                                "GEcERSiZ SiNYAL",
                                                                "SiNYAL SIRA HATASI",
                                                                "SG KIRM. LAMBA AR.",
                                                                "SON KIRM. LAMBA AR.",
                                                                "B. SAY. KIR. L. AR.",
                                                                "SARI LAMBA ARIZASI",
                                                                "YEsiL LAMBA ARIZASI",
                                                                "SARI-SARI cAKIsMASI",
                                                                "SARI-YEsiL cAKIsMASI",
                                                                "YEsiL-YEsiL cAKIsM.",
                                                                "Sc Guc HARC. KAYDI",
                                                                "MODuL CEVAP VERMiYOR",
                                                                "MODuL DEVREDE",
                                                                "BiLGi",
                                                                "CP VERi TOPLAMI HAT.",
                                                                "MP VERi TOPLAMI HAT.",
                                                                "CP ALIM HATASI",
                                                                "CP GoNDERiM HATASI",
                                                                "MP ALIM HATASI",
                                                                "MP GoNDERiM HATASI",
                                                                "Guc NORMALDEN UYKUYA",
                                                                "Guc UYKUDAN NORMALE",
                                                                "BEL. CHECKSUM HATASI",
                                                                "CP ZAMAN AsIMI",
                                                                "MP ZAMAN AsIMI",
                                                                "MKA KONF. HATASI",
                                                                "PROG. YuKLEME HATASI",
                                                                "HATALI PROGRAM",
                                                                "DusuK GERiLiM SEV.",
                                                                "YuKSEK GERiLiM SEV.",
                                                                "cALIs. MODU DEgisiMi",
                                                                "RESET SAYAc",
                                                                "RESET SAAT iZLEME",
                                                                "RESET DusuK GERiLiM",
                                                                "SSM KAYDI",
                                                                "PSM KAYDI",
                                                                "IO KAYDI",
                                                                "SG TuM KIR. LAM. AR.",
                                                                "SG TuM SARI LAM. AR.",
                                                                "SG TuM YEs. LAM. AR.",
                                                                "KuME SiNY. MODU DEg.",
                                                                "ANA BELLEK BOZULDU",
                                                                "YEDEK BELLEK BOZULDU",
                                                                "Y.->A. TAsIMA HATASI",
                                                                "YEDEK BEL. OKUMA HA.",
                                                                "YEDEK BEL. YAZMA HA.",
                                                                "ANA BEL. OKU. HATASI",
                                                                "ANA BEL. YAZ. HATASI",
                                                                "Y.->A. TAsIMA BAs.",
                                                                "ANA BEL. KULLANIMDA",
                                                                "YED. BEL. KULLANIMDA",
                                                                "A.->Y. TAsIMA BAs.",
                                                                "A.->Y. TAsIMA HATASI",
                                                                "RESET ic DEVRE",
                                                                "DusuK PiL GERiLiMi",
                                                                "NORMAL PiL GERiLiMi",
                                                                "KAPI AcILDI",
                                                                "KAPI KAPATILDI",
                                                                "MKA KONF. BAsLAR",
                                                                "MKA KONF. BiTER",
                                                                "LCD KUL. EKLENDi",
                                                                "LCD KUL. EKLENEMEDi",
                                                                "NORMAL GERiLiM SEV.",
                                                                "DusuK FREKANS SEV.",
                                                                "YuKSEK FREKANS SEV.",
                                                                "NORMAL FREKANS SEV.",
                                                                "LCD TuM GRUPLAR KIR.",
                                                                "LCD TuM GRUPLAR KARA",
                                                                "LCD TuM GRUPLAR FLAs",
                                                                "LCD GuNL. PLANA DoN.",
                                                                "LCD/MCS Guc ogRENME",
                                                                "LCD SSM TEST BAsLAR",
                                                                "LCD SSM TEST BiTER",
                                                                "LCD SP TEST BAsLAR",
                                                                "LCD SP TEST BiTER",
                                                                "LCD ZAMAN AYARLANDi",
                                                                "LCD RoLE KAPALi",
                                                                "LCD RoLE AcIK",
                                                                "LCD KUL. OTURUM AcTI",
                                                                "LCD KUL. OTURUM SON",
                                                                "LCD KUL. YOK",
                                                                "LCD KUL. SiFRE HATA.",
                                                                "SiNYAL SuRESi < MiN.",
                                                                "SiNY. SuRESi > MAKS.",
                                                                "DEDEKToR BOZUK",
                                                                "DEDEKToR SAgLAM",
                                                                "SAB. SuRE TAB. DEg.",
                                                                "PROG. ZAM. TAB. DEg.",
                                                                "SIG. PROG. DEg.",
                                                                "TuM KIR. LAM. SAgLAM",
                                                                "TuM SARI LAM. SAgLAM",
                                                                "TuM YEs. LAM. SAgLAM",
                                                                "RESET YAZILIM",
                                                                "RESET PIN",
                                                                "RESET POR",
                                                                "MCS BAgL. AKTiF",
                                                                "MCS BAgL. KURULDU",
                                                                "MCS BAgL. KOPTU",
                                                                "MCS BAgL. ZAMAN AsIMI",
                                                                "MCS SP DEgisiMi",
                                                                "MCS T/Z AYARI",
                                                                "MCS RESET",
                                                                "MCS PROG. YuKLEME",
                                                                "MCS PROG. OKUMA",
                                                                "PSM TESTi BASLADI",
                                                                "PSM TESTi BiTTi",
                                                                "YD SENKR. BAsLADI",
                                                                "YD SENKR. BiTTi",
                                                                "MCTS KAPALI",
                                                                "MCTS KARANLIK",
                                                                "MCTS FLAs",
                                                                "MCTS NORMAL",
                                                                "MCTS IAP BAsLADI",
                                                                "MCS BAgL. DEVAM",
                                                                "LCD IAP BAsLADI",
                                                                "RESET KULLANICI",
                                                                "DiGiTAL G. BOZUK",
                                                                "DiGiTAL G. SAgLAM",
                                                                "TASK cALIsMIYOR",
                                                                "TASK YigiN DOLU", },
                                                              { "UNDEFINED",
                                                                "POWER ON",
                                                                "SO int16_t CIRCUIT",
                                                                "SO OPEN CIRCUIT",
                                                                "SO VOLT. SENS. FAIL.",
                                                                "SO DRIVEN EXTERNALLY",
                                                                "SO WORKING LAMP TOT.",
                                                                "SG OBSERVED SIGNAL",
                                                                "SG INVALID SIGNAL",
                                                                "SG INV. SIGN. SEQ.",
                                                                "SG RED LAMP FAILURE",
                                                                "SG LAST RED LAMP F.",
                                                                "SG N. OF RED L. FAI.",
                                                                "SG YELLOW LAMP FAIL.",
                                                                "SG GREEN LAMP FAIL.",
                                                                "YELLOW-YELLOW CONFL.",
                                                                "YELLOW-GREEN CONFL.",
                                                                "GREEN-GREEN CONFLICT",
                                                                "SO POWER RECORD",
                                                                "MODULE MISSING",
                                                                "MODULE RESPONDS",
                                                                "INFO",
                                                                "CP CHECKSUM ERROR",
                                                                "MP CHECKSUM ERROR",
                                                                "CP RECEIVE ERROR",
                                                                "CP TRANSMIT ERROR",
                                                                "MP RECEIVE ERROR",
                                                                "MP TRANSMIT ERROR",
                                                                "POW. NOR. TO STANDBY",
                                                                "POW. STANDBY TO NOR.",
                                                                "FLASH CHECKSUM ERROR",
                                                                "CP TIMEOUT",
                                                                "MP TIMEOUT",
                                                                "MCT CONF. ERROR",
                                                                "PROG. LOADING ERROR",
                                                                "PROGRAM DAMAGED",
                                                                "LOW NET VOLTAGE",
                                                                "HIGH NET VOLTAGE",
                                                                "WORK MODE CHANGE",
                                                                "RESET WATCHDOG OVER.",
                                                                "RESET CLOCK MONITOR",
                                                                "RESET LOW VOLTAGE",
                                                                "SSM LOG",
                                                                "PSM LOG",
                                                                "IO LOG",
                                                                "SG ALL RED LAMPS FA.",
                                                                "SG ALL YEL. LAMPS F.",
                                                                "SG ALL GREEN L. FAI.",
                                                                "SET SIGN. MODE CH.",
                                                                "MAIN STORAGE BROKEN",
                                                                "BACKUP STOR. BROKEN",
                                                                "BACKUP->MAIN C. ERR.",
                                                                "BACKUP STOR. GET ER.",
                                                                "BACKUP STOR. SET ER.",
                                                                "MAIN STOR. GET ERROR",
                                                                "MAIN STOR. SET ERROR",
                                                                "BACKUP->MAIN C. SUC.",
                                                                "MAIN STORAGE IN USE",
                                                                "BACKUP STOR. IN USE",
                                                                "MAIN->BACKUP C. SUC.",
                                                                "MAIN->BACKUP C. ERR.",
                                                                "RES. POWERON CL. CI.",
                                                                "LOW BATTERY VOLTAGE",
                                                                "NORMAL BAT. VOLTAGE",
                                                                "DOOR OPEN",
                                                                "DOOR CLOSED",
                                                                "MCT CONF. STARTS",
                                                                "MCT CONF. ENDS",
                                                                "DEF. LCD USER ADD S.",
                                                                "DEF. LCD USER ADD E.",
                                                                "NORMAL NET VOLTAGE",
                                                                "LOW NET FREQUENCY",
                                                                "HIGH NET FREQUENCY",
                                                                "NORMAL NET FREQUENCY",
                                                                "LCD ALL GROUPS RED",
                                                                "LCD ALL GROUPS DARK",
                                                                "LCD ALL GROUPS FLASH",
                                                                "LCD RET. TO WORKPLAN",
                                                                "LCD POWER LEARNING",
                                                                "LCD SSM TEST STARTS",
                                                                "LCD SSM TEST ENDS",
                                                                "LCD SP TEST STARTS",
                                                                "LCD SP TEST ENDS",
                                                                "LCD TIME SET",
                                                                "LCD RELAY ON",
                                                                "LCD RELAY OFF",
                                                                "LCD USER LOG IN",
                                                                "LCD USER LOG OUT",
                                                                "LCD USERNAME ERROR",
                                                                "LCD PASSWORD ERROR",
                                                                "SIGNAL DUR. < MIN.",
                                                                "SIGNAL DUR. > MAX.",
                                                                "DETECTOR BROKEN",
                                                                "DETECTOR SAFE",
                                                                "FIX. TIME. TAB. CH.",
                                                                "PROG. TIME. TAB. CH.",
                                                                "SIG. PROG. CHANGE",
                                                                "ALL RED LAMPS SAFE",
                                                                "ALL YELLOW LAM. SAFE",
                                                                "ALL GREEN LAMPS SAFE",
                                                                "RESET SOFTWARE",
                                                                "RESET PIN",
                                                                "RESET POR",
                                                                "MCTS CON. ACTIVE",
                                                                "MCTS CON. SUCCEED",
                                                                "MCTS CON. FAILED",
                                                                "MCTS CON. TIMEOUT",
                                                                "MCTS SP CHANGE",
                                                                "MCTS D/T ADJUSTMENT",
                                                                "MCTS RESET",
                                                                "MCTS PROG. DOWNLOAD",
                                                                "MCTS PROG. UPLOAD",
                                                                "PSM TEST STARTS",
                                                                "PSM TEST ENDS",
                                                                "GW SYNCH. STARTED",
                                                                "GW SYNCH. FINISHED",
                                                                "MCTS WMC ALL RED",
                                                                "MCTS WMC DARK",
                                                                "MCTS WMC FLASH",
                                                                "MCTS WMC WORK PLAN",
                                                                "MCTS IAP STARTED",
                                                                "MCTS CON. RESUMED",
                                                                "LCD IAP STARTED",
                                                                "RESET USER",
                                                                "DIGITAL I. BROKEN",
                                                                "DIGTIAL I. SAFE",
                                                                "TASK NOT RUNNING",
                                                                "TASK STACK OVERFLOW", } };

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

const char *GetEventStr(uint8_t bEventCode, uint8_t bInterfaceLanguage)
{
  if ((bInterfaceLanguage < LANGUAGES_MAX) && (bEventCode <= EVENT_LAST))
  {
    return pStrLogStrings[bInterfaceLanguage][bEventCode];
  }

  return "";
}

/* the following strings are shown on LCD. they can be cause of current device */
/* emergency state. */
const char *GetEventStrShort(uint8_t bEventCode,
                             uint8_t bInterfaceLanguage,
                             uint8_t bStrType)
{
  if ((bInterfaceLanguage < LANGUAGES_MAX) && (bEventCode <= EVENT_LAST))
  {
    switch (bInterfaceLanguage)
    {
        case LANGUAGE_TURKISH:
        {
          switch (bEventCode)
          {
              case EVENT_INVALID_PROGRAM:
              {
                if (bStrType == LCD_STRING_TYPE_SHORT)
                {
                  return "GP";
                }
                else
                {
                  return "GECERSiZ PROGRAM";
                }
              }

              case EVENT_INVALID_SIGNAL:
              {
                if (bStrType == LCD_STRING_TYPE_SHORT)
                {
                  return "GS";
                }
                else
                {
                  return "GEcERSiZ SiNYAL";
                }
              }

              case EVENT_INVALID_SIGNAL_SEQUENCE:
              {
                if (bStrType == LCD_STRING_TYPE_SHORT)
                {
                  return "SSH";
                }
                else
                {
                  return "SiNYAL SIRA HATASI";
                }
              }

              case EVENT_SG_LAST_RED_LAMP_FAILURE:
              {
                if (bStrType == LCD_STRING_TYPE_SHORT)
                {
                  return "SKLA";
                }
                else
                {
                  return "SON KIRM. LAMBA AR.";
                }
              }

              case EVENT_SG_NUMBER_OF_RED_LAMPS_FAILURE:
              {
                if (bStrType == LCD_STRING_TYPE_SHORT)
                {
                  return "BSKLA";
                }
                else
                {
                  return "B. SAY. KIR. L. AR.";
                }
              }

              case EVENT_YELLOW_YELLOW_CONFLICT:
              {
                if (bStrType == LCD_STRING_TYPE_SHORT)
                {
                  return "SSc";
                }
                else
                {
                  return "SARI-SARI cAKIsMASI";
                }
              }

              case EVENT_YELLOW_GREEN_CONFLICT:
              {
                if (bStrType == LCD_STRING_TYPE_SHORT)
                {
                  return "SYc";
                }
                else
                {
                  return "SARI-YEsiL cAKIsM.";
                }
              }

              case EVENT_GREEN_GREEN_CONFLICT:
              {
                if (bStrType == LCD_STRING_TYPE_SHORT)
                {
                  return "YYc";
                }
                else
                {
                  return "YEsiL-YEsiL cAKIsM.";
                }
              }

              case EVENT_MODULE_MISSING:
              {
                if (bStrType == LCD_STRING_TYPE_SHORT)
                {
                  return "MY";
                }
                else
                {
                  return "MODuL YOK";
                }
              }

              case EVENT_FREQUENCY_VALUE_LOWER_BOUND:
              {
                if (bStrType == LCD_STRING_TYPE_SHORT)
                {
                  return "DFS";
                }
                else
                {
                  return "DusuK FREKANS SEV.";
                }
              }

              case EVENT_FREQUENCY_VALUE_UPPER_BOUND:
              {
                if (bStrType == LCD_STRING_TYPE_SHORT)
                {
                  return "YFS";
                }
                else
                {
                  return "YuKSEK FREKANS SEV.";
                }
              }

              case EVENT_VOLTAGE_VALUE_LOWER_BOUND:
              {
                if (bStrType == LCD_STRING_TYPE_SHORT)
                {
                  return "DGS";
                }
                else
                {
                  return "DusuK GERiLiM SEV.";
                }
              }

              case EVENT_VOLTAGE_VALUE_UPPER_BOUND:
              {
                if (bStrType == LCD_STRING_TYPE_SHORT)
                {
                  return "YGS";
                }
                else
                {
                  return "YuKSEK GERiLiM SEV.";
                }
              }

              case EVENT_SO_WORKING_LAMP_TOTAL_CHANGE:
              {
                if (bStrType == LCD_STRING_TYPE_SHORT)
                {
                  return "cLSD";
                }
                else
                {
                  return "LAMBA SAY. DEgisiMi";
                }
              }

              case EVENT_CPMP_COMM_MP_CHECKSUM_ERROR:
              {
                if (bStrType == LCD_STRING_TYPE_SHORT)
                {
                  return "MPCH";
                }
                else
                {
                  return "MP CHECKSUM HATASI";
                }
              }

              case EVENT_CPMP_COMM_MP_RECEIVE_ERROR:
              {
                if (bStrType == LCD_STRING_TYPE_SHORT)
                {
                  return "MPAH";
                }
                else
                {
                  return "MP ALIM HATASI";
                }
              }

              case EVENT_CPMP_COMM_CP_TIMEOUT:
              {
                if (bStrType == LCD_STRING_TYPE_SHORT)
                {
                  return "CPZH";
                }
                else
                {
                  return "CP ZAMAN AsIMI";
                }
              }
          } /* switch */

          break;
        }

        case LANGUAGE_ENGLISH:
        {
          switch (bEventCode)
          {
              case EVENT_INVALID_PROGRAM:
              {
                if (bStrType == LCD_STRING_TYPE_SHORT)
                {
                  return "IP";
                }
                else
                {
                  return "INVALID PROGRAM";
                }
              }

              case EVENT_INVALID_SIGNAL:
              {
                if (bStrType == LCD_STRING_TYPE_SHORT)
                {
                  return "IS";
                }
                else
                {
                  return "INVALID SIGNAL";
                }
              }

              case EVENT_INVALID_SIGNAL_SEQUENCE:
              {
                if (bStrType == LCD_STRING_TYPE_SHORT)
                {
                  return "ISS";
                }
                else
                {
                  return "INV. SIGN. SEQUENCE";
                }
              }

              case EVENT_SG_LAST_RED_LAMP_FAILURE:
              {
                if (bStrType == LCD_STRING_TYPE_SHORT)
                {
                  return "LRLF";
                }
                else
                {
                  return "LAST RED LAMP FAIL.";
                }
              }

              case EVENT_SG_NUMBER_OF_RED_LAMPS_FAILURE:
              {
                if (bStrType == LCD_STRING_TYPE_SHORT)
                {
                  return "NRLF";
                }
                else
                {
                  return "RED LAMP FAILURE";
                }
              }

              case EVENT_YELLOW_YELLOW_CONFLICT:
              {
                if (bStrType == LCD_STRING_TYPE_SHORT)
                {
                  return "YYC";
                }
                else
                {
                  return "YELLOW-YELLOW CONF.";
                }
              }

              case EVENT_YELLOW_GREEN_CONFLICT:
              {
                if (bStrType == LCD_STRING_TYPE_SHORT)
                {
                  return "YGC";
                }
                else
                {
                  return "YELLOW-GREEN CONF.";
                }
              }

              case EVENT_GREEN_GREEN_CONFLICT:
              {
                if (bStrType == LCD_STRING_TYPE_SHORT)
                {
                  return "GGC";
                }
                else
                {
                  return "GREEN-GREEN CONFL.";
                }
              }

              case EVENT_MODULE_MISSING:
              {
                if (bStrType == LCD_STRING_TYPE_SHORT)
                {
                  return "MM";
                }
                else
                {
                  return "MODULE MISSING";
                }
              }

              case EVENT_FREQUENCY_VALUE_LOWER_BOUND:
              {
                if (bStrType == LCD_STRING_TYPE_SHORT)
                {
                  return "LFE";
                }
                else
                {
                  return "LOW FREQ. ERROR";
                }
              }

              case EVENT_FREQUENCY_VALUE_UPPER_BOUND:
              {
                if (bStrType == LCD_STRING_TYPE_SHORT)
                {
                  return "HFE";
                }
                else
                {
                  return "HIGH FREQ. ERROR";
                }
              }

              case EVENT_VOLTAGE_VALUE_LOWER_BOUND:
              {
                if (bStrType == LCD_STRING_TYPE_SHORT)
                {
                  return "LVE";
                }
                else
                {
                  return "LOW VOLTAGE ERROR";
                }
              }

              case EVENT_VOLTAGE_VALUE_UPPER_BOUND:
              {
                if (bStrType == LCD_STRING_TYPE_SHORT)
                {
                  return "HVE";
                }
                else
                {
                  return "HIGH VOLTAGE ERROR";
                }
              }

              case EVENT_SO_WORKING_LAMP_TOTAL_CHANGE:
              {
                if (bStrType == LCD_STRING_TYPE_SHORT)
                {
                  return "TWLC";
                }
                else
                {
                  return "TOT. WOR. LAMP. CH.";
                }
              }

              case EVENT_CPMP_COMM_MP_CHECKSUM_ERROR:
              {
                if (bStrType == LCD_STRING_TYPE_SHORT)
                {
                  return "MPCE";
                }
                else
                {
                  return "MP CHECKSUM ERROR";
                }
              }

              case EVENT_CPMP_COMM_MP_RECEIVE_ERROR:
              {
                if (bStrType == LCD_STRING_TYPE_SHORT)
                {
                  return "MPRE";
                }
                else
                {
                  return "MP RECEPTION ERROR";
                }
              }

              case EVENT_CPMP_COMM_CP_TIMEOUT:
              {
                if (bStrType == LCD_STRING_TYPE_SHORT)
                {
                  return "CPTE";
                }
                else
                {
                  return "CPMP COM. TIMEOUT";
                }
              }
          } /* switch */

          break;
        }
    } /* switch */
  }

  return "";
} /* GetEventStrShort */

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

static void LogAppendSideEffects(uint16_t sWrittenIndex,
                                 tSLogRecord *pSLogRecord)
{
  if (pSLogRecord->SEvent.bEvent == EVENT_POWER_ON)
  {
    UILogReadIndexSet(sWrittenIndex);
  }
  else if (pSLogRecord->SEvent.bEvent == EVENT_DOOR_OPEN)
  {
    SetGateOpenLogRecordIndex(sWrittenIndex);
    SetGateState(FALSE);
    StreamGateStateChanged(FALSE, pSLogRecord);
  }
  else if (pSLogRecord->SEvent.bEvent == EVENT_DOOR_CLOSED)
  {
    SetGateClosedLogRecordIndex(sWrittenIndex);
    SetGateState(TRUE);
    StreamGateStateChanged(TRUE, pSLogRecord);
  }
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
        if (fResult)
        {
          LogAppendSideEffects(sWrittenIndex, &SLogRecord);
        }

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
  /* other tasks may want to read logs, in this case, read indexes are provided */
  /* per task separately, also init them */
  UILogReadIndexSet(0);
  MCSAsynchSetLogReadIndex(0);
  MCSAsynchWriteLogReadIndex();

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
