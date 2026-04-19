/* App/Domain/Lcd/LcdEventText.c */
#include "LcdEventText.h"

#include "LcdLanguage.h"

enum
{
  LCD_EVENT_TEXT_CODE_COUNT = 128U,
  LCD_SAFETY_REASON_NONE = 0U,
  LCD_SAFETY_REASON_CONFLICT_GREEN_GREEN = 1U,
  LCD_SAFETY_REASON_CONFLICT_YELLOW_GREEN = 2U,
  LCD_SAFETY_REASON_CONFLICT_YELLOW_YELLOW = 3U,
  LCD_SAFETY_REASON_DUAL_INDICATION = 4U,
  LCD_SAFETY_REASON_DARK_CHANNEL = 5U,
  LCD_SAFETY_REASON_SIGNAL_SEQUENCE = 6U,
  LCD_SAFETY_REASON_MIN_YELLOW_SHORT = 10U,
  LCD_SAFETY_REASON_CLEARANCE_SHORT = 11U,
  LCD_SAFETY_REASON_RED_FAIL = 12U,
  LCD_SAFETY_REASON_LAMP_OPEN = 20U,
  LCD_SAFETY_REASON_LAMP_SHORT = 21U,
  LCD_SAFETY_REASON_LAMP_EXTERNALLY_DRIVEN = 22U,
  LCD_SAFETY_REASON_LAMP_ALL_BROKEN = 23U,
  LCD_SAFETY_REASON_LAMP_WORKING_COUNT_CHANGE = 24U,
  LCD_SAFETY_REASON_VOLTAGE_SENSOR_FAILURE = 25U,
  LCD_SAFETY_REASON_PSM_LINE_VOLTAGE_LOW = 30U,
  LCD_SAFETY_REASON_PSM_LINE_VOLTAGE_HIGH = 31U,
  LCD_SAFETY_REASON_PSM_FREQUENCY_LOW = 32U,
  LCD_SAFETY_REASON_PSM_FREQUENCY_HIGH = 33U,
  LCD_SAFETY_REASON_PSM_RAIL_24V_FAIL = 34U,
  LCD_SAFETY_REASON_PSM_RAIL_5V_FAIL = 35U,
  LCD_SAFETY_REASON_MODULE_CP_MISSING = 40U,
  LCD_SAFETY_REASON_MODULE_PSM_MISSING = 41U,
  LCD_SAFETY_REASON_MODULE_SSM_MISSING = 42U,
  LCD_SAFETY_REASON_MP_WATCHDOG = 50U,
  LCD_SAFETY_REASON_MP_BATTERY_LOW = 51U,
  LCD_SAFETY_REASON_MP_TEMPERATURE_HIGH = 52U,
  LCD_SAFETY_REASON_MP_CONFIG_INVALID = 53U,
  LCD_SAFETY_REASON_MP_RELAY_FEEDBACK_MISMATCH = 54U
};

static const char *const pStrLogStrings[LANGUAGES_MAX][LCD_EVENT_TEXT_CODE_COUNT]
  = {
      { "TANIMSIZ",
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
        "MCTS AYRILDI",
        "MCS BAgL. DEVAM",
        "LCD AYRILDI",
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
        "MCTS REMOVED",
        "MCTS CON. RESUMED",
        "LCD REMOVED",
        "RESET USER",
        "DIGITAL I. BROKEN",
        "DIGTIAL I. SAFE",
        "TASK NOT RUNNING",
        "TASK STACK OVERFLOW", }
};

const char *LcdEventText_GetEventLong(uint8_t code, uint8_t lang)
{
  if (lang >= LANGUAGES_MAX)
  {
    lang = LANGUAGE_ENGLISH;
  }

  if (code >= LCD_EVENT_TEXT_CODE_COUNT)
  {
    return pStrLogStrings[lang][0];
  }

  return pStrLogStrings[lang][code];
}

const char *LcdEventText_GetSafetyReasonShort(uint8_t reasonCode, uint8_t lang)
{
  const uint8_t isTurkish = (uint8_t) (lang == LANGUAGE_TURKISH);

  switch (reasonCode)
  {
      case LCD_SAFETY_REASON_NONE:
      {
        return "";
      }

      case LCD_SAFETY_REASON_CONFLICT_GREEN_GREEN:
      {
        return isTurkish ? "YYC" : "GGC";
      }

      case LCD_SAFETY_REASON_CONFLICT_YELLOW_GREEN:
      {
        return isTurkish ? "SYC" : "YGC";
      }

      case LCD_SAFETY_REASON_CONFLICT_YELLOW_YELLOW:
      {
        return isTurkish ? "SSC" : "YYC";
      }

      case LCD_SAFETY_REASON_DUAL_INDICATION:
      {
        return isTurkish ? "CFT" : "DUAL";
      }

      case LCD_SAFETY_REASON_DARK_CHANNEL:
      {
        return isTurkish ? "KAR" : "DARK";
      }

      case LCD_SAFETY_REASON_SIGNAL_SEQUENCE:
      {
        return isTurkish ? "SSH" : "SEQ";
      }

      case LCD_SAFETY_REASON_MIN_YELLOW_SHORT:
      {
        return isTurkish ? "SMS" : "MYS";
      }

      case LCD_SAFETY_REASON_CLEARANCE_SHORT:
      {
        return isTurkish ? "KKS" : "CLR";
      }

      case LCD_SAFETY_REASON_RED_FAIL:
      {
        return isTurkish ? "KRF" : "RDF";
      }

      case LCD_SAFETY_REASON_LAMP_OPEN:
      {
        return isTurkish ? "ACK" : "OPN";
      }

      case LCD_SAFETY_REASON_LAMP_SHORT:
      {
        return isTurkish ? "KDV" : "SHT";
      }

      case LCD_SAFETY_REASON_LAMP_EXTERNALLY_DRIVEN:
      {
        return isTurkish ? "HBE" : "EXT";
      }

      case LCD_SAFETY_REASON_LAMP_ALL_BROKEN:
      {
        return isTurkish ? "TBR" : "ALL";
      }

      case LCD_SAFETY_REASON_LAMP_WORKING_COUNT_CHANGE:
      {
        return isTurkish ? "LSD" : "WLC";
      }

      case LCD_SAFETY_REASON_VOLTAGE_SENSOR_FAILURE:
      {
        return isTurkish ? "GSA" : "VSN";
      }

      case LCD_SAFETY_REASON_PSM_LINE_VOLTAGE_LOW:
      {
        return isTurkish ? "DGS" : "LVE";
      }

      case LCD_SAFETY_REASON_PSM_LINE_VOLTAGE_HIGH:
      {
        return isTurkish ? "YGS" : "HVE";
      }

      case LCD_SAFETY_REASON_PSM_FREQUENCY_LOW:
      {
        return isTurkish ? "DFS" : "LFE";
      }

      case LCD_SAFETY_REASON_PSM_FREQUENCY_HIGH:
      {
        return isTurkish ? "YFS" : "HFE";
      }

      case LCD_SAFETY_REASON_PSM_RAIL_24V_FAIL:
      {
        return "24V";
      }

      case LCD_SAFETY_REASON_PSM_RAIL_5V_FAIL:
      {
        return "5V";
      }

      case LCD_SAFETY_REASON_MODULE_CP_MISSING:
      {
        return isTurkish ? "CPY" : "CPM";
      }

      case LCD_SAFETY_REASON_MODULE_PSM_MISSING:
      {
        return isTurkish ? "PSY" : "PSM";
      }

      case LCD_SAFETY_REASON_MODULE_SSM_MISSING:
      {
        return isTurkish ? "SSY" : "SSM";
      }

      case LCD_SAFETY_REASON_MP_WATCHDOG:
      {
        return "WDG";
      }

      case LCD_SAFETY_REASON_MP_BATTERY_LOW:
      {
        return isTurkish ? "DPL" : "BAT";
      }

      case LCD_SAFETY_REASON_MP_TEMPERATURE_HIGH:
      {
        return isTurkish ? "ISC" : "TMP";
      }

      case LCD_SAFETY_REASON_MP_CONFIG_INVALID:
      {
        return isTurkish ? "KNF" : "CFG";
      }

      case LCD_SAFETY_REASON_MP_RELAY_FEEDBACK_MISMATCH:
      {
        return "RLY";
      }

      default:
      {
        return isTurkish ? "BIL" : "UNK";
      }
  }
}
