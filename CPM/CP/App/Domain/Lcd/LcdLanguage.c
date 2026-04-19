/* App/Domain/Lcd/LcdLanguage.c
 */
#include "LcdLanguage.h"
#include <stddef.h>

static const char *const pStrStates[LANGUAGES_MAX][8] = {
  { "-", "DEVRE DIsI", "FLAs", "KAPALI", "FAZ", "FAZ GEcisi", "SEK",
    "GuVENLiK GEcisi" },
  { "ANY", "DISABLED", "FLASH", "CLOSED", "PHASE", "PHASE TRANS.", "SEQ",
    "SECURE TRANSITION" }
};

static const char *const pStrDays[LANGUAGES_MAX][8] = {
  { "-", "PT", "SA", "cA", "PE", "CU", "CT", "PA" },
  { "-", "MO", "TU", "WE", "TH", "FR", "SA", "SU" }
};

static const char *const pStrClockAdvanceMode[LANGUAGES_MAX][4] = {
  { "S  ", "RTC", "GPS", "M  " },
  { "N  ", "RTC", "GPS", "C  " }
};

static const char *const pStrProgramLoadingStatus[LANGUAGES_MAX][19] = {
  { "-", "PROG.YuK.BAsLADI", "ANA->YEDEK BEL.", "PROGRAM YuKLENDi.",
    "ANA->YEDEK BEL.HA",
    "YEDEK->ANA BEL.", "PROGRAM YuKLENDi.", "PROG.YEDEK->ANA H",
    "PROGRAM YuKLENiYOR..",
    "PROGRAM YuKLENDi.", "PROGRAM YuKLENEMEDi.", "PROG.->ANA BEL.",
    "PROG.->YEDEK BEL.",
    "PROG.GoNDERME.BAsLADI", "PROG.->PC", "PROG.->PC BAsARiLi",
    "PROG.->PC BAsARISIZ",
    "FLAs OKUMA HATASI", "EEPROM OKUMA HATASI" },
  { "-", "PROG.LOAD.START", "PROG. MAIN->BACKUP", "PROG. MAIN->BACKUP S",
    "PROG. MAIN->BACKUP E",
    "PROG. BACKUP->MAIN", "PROG. BACKUP->MAIN S", "PROG. BACKUP->MAIN E",
    "PROG. LOADING...",
    "PROG. LOADED.", "PROG. NOT LOADED.", "PROG.->MAIN MEM.",
    "PROG.->BACKUP MEM.",
    "PROG.->PC STARTED", "PROG.->PC IN PROGRESS", "PROG.->PC SUCCEEDED",
    "PROG.->PC FAILED",
    "FLASH READ ERROR", "EEPROM READ ERROR" }
};

static const char *const pStrValsSignalSourceParameters[LANGUAGES_MAX][9] = {
  { "MODuL NO:", "Sc NO:", "SG NO:", "SG NO:", "cAKiSAN GRUP:", "cAKiSAN GRUP:",
    "cAKiSAN GRUP:", "SG NO:", "SG NO:" },
  { "MODULE NO:", "SO NO:", "SG NO:", "SG NO:", "CONFL. GROUP:",
    "CONFL. GROUP:", "CONFL. GROUP:", "SG NO:", "SG NO:" }
};

const char *Lcd_GetStateStr(uint8_t state, uint8_t lang)
{
  if ((lang >= LANGUAGES_MAX) || (state == 0) || (state > 8))
  {
    return "-";
  }

  return pStrStates[lang][state - 1];
}

const char *Lcd_GetDayStr(uint8_t day, uint8_t lang)
{
  if ((lang >= LANGUAGES_MAX) || (day > 7))
  {
    return "-";
  }

  return pStrDays[lang][day];
}

const char *Lcd_GetAdvanceModeStr(uint8_t mode, uint8_t lang)
{
  if ((lang >= LANGUAGES_MAX) || (mode == 0) || (mode > 4))
  {
    return "???";
  }

  return pStrClockAdvanceMode[lang][mode - 1];
}

const char *Lcd_GetProgramLoadingStatusStr(uint8_t status, uint8_t lang)
{
  if ((lang >= LANGUAGES_MAX) || (status >= 19))
  {
    return "-";
  }

  return pStrProgramLoadingStatus[lang][status];
}

const char *Lcd_GetSignalSourceParamStr(uint8_t source, uint8_t lang)
{
  if ((lang >= LANGUAGES_MAX) || (source >= 9))
  {
    return ":";
  }

  return pStrValsSignalSourceParameters[lang][source];
}

const char *Lcd_GetNoEmergencyStr(uint8_t lang)
{
  return (lang == LANGUAGE_TURKISH) ? "ACiL DURUM YOK" : "NO EMERGENCY STATE";
}

const char *Lcd_GetHelpStr(uint8_t lang)
{
  return (lang == LANGUAGE_TURKISH) ? "YARDIM - SET " : "HELP - SET ";
}

const char *Lcd_GetLoginUserStr(uint8_t lang)
{
  return (lang == LANGUAGE_TURKISH) ? "KULLANICI ADI" : "USERNAME";
}

const char *Lcd_GetLoginPassStr(uint8_t lang)
{
  return (lang == LANGUAGE_TURKISH) ? "siFRE" : "PASSWORD";
}

const char *Lcd_GetNoLogStr(uint8_t lang)
{
  return (lang == LANGUAGE_TURKISH) ? "KAYITLI LOG YOK" : "NO LOG EXISTS";
}

const char *Lcd_GetErrorStr(uint8_t lang)
{
  return (lang == LANGUAGE_TURKISH) ? "HATA..." : "ERROR...";
}

const char *Lcd_GetLocalIpStr(uint8_t lang)
{
  return (lang == LANGUAGE_TURKISH) ? "LOKAL iP" : "LOCAL IP";
}

const char *Lcd_GetServerIpStr(uint8_t lang)
{
  return (lang == LANGUAGE_TURKISH) ? "SUNUCU iP" : "SERVER IP";
}

static const char *const pStrDeviceSettingsMenuEntries[LANGUAGES_MAX][11] = {
  { "TARiH/ZAMAN AYAR.", "MENu DiLi", "GPS AYARLARI", "BAgLANTI AYARLARI",
    "SUNUCU AYARLARI",
    "KUL. isLEMLERi", "KONFIG. AYARLARI", "ARIZALI GiRis AY.",
    "SKLA ALGI. SURESI", "FABRiKA AYAR. DoN." },
  { "DATE/TIME SETTING", "MENU LANGUAGE", "GPS SETTINGS", "CONNECTION SET.",
    "SERVER SET.",
    "USER ACCOUNT SET.", "CONFIG. SETTINGS", "BROKEN INPUT SET.",
    "LRLF DETECT. TIME", "RETURN FACT. SET." }
};

const char *Lcd_GetSettingsMenuEntryStr(uint8_t index, uint8_t lang)
{
  if ((lang >= LANGUAGES_MAX) || (index >= 10))
  {
    return "???";
  }

  return pStrDeviceSettingsMenuEntries[lang][index];
}
