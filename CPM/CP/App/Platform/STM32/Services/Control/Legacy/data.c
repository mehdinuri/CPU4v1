/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Include Files */
#include "data.h"

#include <string.h>

#include "CanMsgParser.h"
#include "MLM.h"
#include "MSM.h"
#include "defs.h"
#include "ethernetif.h"
#include "gps.h"
#include "iwdg.h"
#include "rng.h"
#include "signalCardDrv.h"
#include "snmp_client.h"
#include "tim.h"
#include "usb.h"
#include "i2c.h"
#include "crc.h"
#include "gpio.h"
#include "program.h"
#include "DomainServices.h"
#include "HardwarePorts.h"
#include "PersistencePorts.h"

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Definitions */
/* #define CHECK_DEVICE_UID */
/* #define DEBUG */

#ifndef DEBUG
/* #define CHECK_DEVICE_UID */
#endif

#define STANDBY_CNTR_MIN 50
#define TRAFFIC_COUNTS_PERIOD_SECONDS 0x5A
#define TRAFFIC_COUNTS_PERIOD_DEFAULT 0x0F

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Members */
__attribute__((section(".dtcm_bss"), aligned(32)))
tSRuntimes SRuntimes;
tSCanDigitalIOInputs SaCanDigitalIOInputs[MODULES_IO_MAX];
tSCanDetectorIOInputs SaCanDetectorIOInputs[MODULES_IO_MAX];

static uint16_t sSuperAdminUsername, sAdminUsername, sGuestUsername;
static uint16_t sSuperAdminPassword, sAdminPassword, sGuestPassword;
static uint8_t fIsSuperAdminValid, fIsAdminValid, fIsGuestValid;

/* communication and general errors */
tSErrInfo SErrInfo;

__attribute__((section(".dtcm_bss"), aligned(32)))
tSCP SCP;

tSCPRuntime SCPRuntime;

static uint8_t DataPersistenceWrite(PersistenceObjectId_t objectId,
                                    uint32_t offset,
                                    const void *src,
                                    uint32_t size)
{
  return PersistenceWrite(&g_persistencePort, objectId, offset, src, size);
}

static uint8_t DataPersistenceRead(PersistenceObjectId_t objectId,
                                   uint32_t offset,
                                   void *dst,
                                   uint32_t size)
{
  return PersistenceRead(&g_persistencePort, objectId, offset, dst, size);
}

static uint8_t DataPersistenceErase(PersistenceObjectId_t objectId,
                                    uint32_t offset,
                                    uint32_t size)
{
  return PersistenceErase(&g_persistencePort, objectId, offset, size);
}

/*  ssm */
static tSSSMTest SSSMTest;
tSCurrentMeasurement SaCurrents[SIGNAL_OUTPUT_CURRENT_GROUPS_MAX];

/*  psm */
tSPowerSupply SaPSMs[PSMS_MAX];

/*  input validation */
static uint8_t bIOValuesValid;
static uint8_t bLDValuesValid;

/*  flasher signal groups */
static uint32_t sSGFlashers;
static uint16_t sFlashCntr;

/*  inputs */
static uint8_t baIOMessagePeriodCounter[MODULES_IO_MAX];
static uint8_t baLDMessagePeriodCounter[MODULES_IO_MAX];
static tSCanDigitalIOInputs SaPrevCanDigitalIOInputs[MODULES_IO_MAX];
static tSCanDetectorIOInputs SaPrevCanDetectorIOInputs[MODULES_IO_MAX];
static uint8_t bLastDetectorDemandIssued;
static uint8_t bLastInputDemandIssued;

/*  outputs */
static tSCanCpuIOOutputs SCanCpuIOOutputs;

/*  signal sequences */
static uint8_t bCurrentSeqNo; /* this sequence is loaded to common sequence */

__attribute__((section(".dtcm_bss"), aligned(32)))
static tSSeqExtension SaSeqExtDur[SIGNAL_SEQS_MAX];

/*  signal program */
static tSaSPPlan SaSPPlanDefault; /* if there is no user defined signal program */
/* or there is an error, use this signal */
/* program */

/*  counters */
static tSCounter SaCounters[COUNTERS_MAX];

/*  after state transition, in statements, there may transition lock command */
static uint32_t laTransitonsLock[TRANSITION_LOCK_SIZE]; /* 1 bit per */
/* transitions, there */
/* are TRANSITIONS_MAX */
/* transition (64 for */
/* the present) */

/*  green-wave sequence extension */
static tSOperation SCurrentOperation;
static uint32_t bOffsetVal = 0;

/* User settings */
static tSUserSettings SUserSettings;

/* Log Settings */
static tSLogSettings SLogSettings;

/* System Start Time */
static tSSystemStartTime SSystemStartTime;
static uint8_t bMinSystemUpHours = SYSTEM_START_TIME_DEF_MIN_SYSTEM_UP_HOURS;
static uint8_t bSystemUpHours = 0;

/* Module Versions */
static tSModulesVersion SModuleVersions = { 0 };

/* Functionality Configuration for ISSD & Denizli */
static tSFuncConf SFuncConf = { 0 };

/* Daylight Saving Time Flag */
static uint8_t bDaylightSavingTimeFlag = FALSE;

/* MCS Traffic Countsstatic */
static tSMCSTrafficCountsRuntimes SMCSTrafficCountsRuntimes;
static uint16_t sTrafficCntTimer = 0;

/* Input Busy Settings */
static tSBrokenInputSettings SBrokenInputSettings;

/* Input Busy Settings */
static tSServerSettings SServerSettings;

static uint8_t bLRLFDetectTime = LRLF_DETECT_TIME_800_MS;

/* Device UID */
static tSSDeviceUID SDeviceUID;

static tSSDeviceUID SEEPROMDeviceUID;

/* User Operations */
static tSUserOperations SUseroperations;

/* Global Configuration */
static tSGlobalConfiguration SGlobalConfiguration;
static const uint32_t baAscModuleDeviceNode[] = { 1, 3, 6, 1, 4, 1, 59873, 4, 2,
                                                  1 };

/* Global DB Management */
static tSGlobalDbManagement SGlobalDbMangement;

/* Global Time Management */
static tSGlobalTimeManagement SGlobalTimeManagement;

/* TR Patterns and Coords */
static tSTRPatternsAndCoords STRPatternsAndCoords;

/* ASC Phases */
tSAscPhase SAscPhase;

/* ASC Channel */
tSASCChannel SASCChannel;

/* ASC Detector */
tSASCDetector SASCDetector;

/* ASC Cabinet Environment */
tSASCCabinetEnvironment SASCCabinetEnvironment;

/* ASC Unit */
tSAscUnit SAscUnit;

/* ASC Coord */
tSAscCoord SAscCoord;

/* Timebase ASC */
tSTimebaseAsc STimebaseAsc;

/* ASC Ring */
tSAscRing SAscRing;

/* Overlap */
tSOverlap SOverlap;

/* ASC Block */
tSAscBlock SAscBlock;

/* Preempt */
tSPreempt SPreempt;

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Bit Values */
const uint32_t laValue2Bit[32] = { 0x00000001, 0x00000002, 0x00000004,
                                   0x00000008, 0x00000010, 0x00000020,
                                   0x00000040,
                                   0x00000080, 0x00000100, 0x00000200,
                                   0x00000400, 0x00000800, 0x00001000,
                                   0x00002000,
                                   0x00004000, 0x00008000, 0x00010000,
                                   0x00020000, 0x00040000, 0x00080000,
                                   0x00100000,
                                   0x00200000, 0x00400000, 0x00800000,
                                   0x01000000, 0x02000000, 0x04000000,
                                   0x08000000,
                                   0x10000000, 0x20000000, 0x40000000,
                                   0x80000000 };

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Checksum */
uint8_t ByteChecksum(void *pvData, uint16_t sLength)
{
  uint8_t bChecksum = 0;
  uint8_t sLen = sLength;
  uint8_t *pbData = (uint8_t *) pvData;

  while (sLen)
  {
    IWDGRefresh();

    bChecksum += *pbData;

    pbData++;
    sLen--;
  }

  return bChecksum;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  DWT */
void DWTInit(void)
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  DWT->CYCCNT = 0;
}

void DWTDelayuSeconds(volatile uint32_t uSeconds)
{
  uint32_t lStart;
  uint32_t lTarget;
  const uint32_t lCyclesPeruSec = (HAL_RCC_GetHCLKFreq() / 1000000UL);

  if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk))
  {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  }

  if (!(DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk))
  {
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    DWT->CYCCNT = 0;
  }

  lStart = DWT->CYCCNT;
  lTarget = lStart + uSeconds * lCyclesPeruSec;

  if (lTarget > lStart)
  {
    while (DWT->CYCCNT < lTarget)
    {
      ;
    }
  }
  else
  {
    while (DWT->CYCCNT > lStart || DWT->CYCCNT < lTarget)
    {
      ;
    }
  }
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Modules Version */
void SetModulesVersion(void)
{
  memset(&SModuleVersions, 0, sizeof(tSModulesVersion));

  SModuleVersions.SCPUVersion.bArg1 = MAESTRO_VERSION_ARG1;
  SModuleVersions.SCPUVersion.bArg2 = MAESTRO_VERSION_ARG2;
  SModuleVersions.SCPUVersion.bArg3 = MAESTRO_VERSION_ARG3;
  SModuleVersions.SCPUVersion.bArg4 = MAESTRO_VERSION_ARG4;
}

tpSModulesVersion GetModulesVersion(void)
{
  return &SModuleVersions;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Police Button */
uint8_t GetPoliceButtonState(void)
{
  return SCPRuntime.SFlags.fPoliceButton;
}

void SetPoliceButtonState(uint8_t fState)
{
  SCPRuntime.SFlags.fPoliceButton = fState;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Lamp Dimming Input */
uint8_t LampDimmingInputGet(void)
{
  return LAMP_DIMMING_BUTTON_DIG_INPUT_NO;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Heater Input */
uint8_t HeaterInputGet(void)
{
  return HEATER_BUTTON_DIG_INPUT_NO;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Program Loading Flag */
uint8_t WriteProgramLoadingFlag(uint8_t bNewValue)
{
  return DataPersistenceWrite(PERSIST_OBJECT_PROGRAM_LOADING_FLAG,
                              0U,
                              &bNewValue,
                              sizeof(uint8_t));
}

uint8_t ReadProgramLoadingFlag(uint8_t *pNewValue)
{
  return DataPersistenceRead(PERSIST_OBJECT_PROGRAM_LOADING_FLAG,
                             0U,
                             pNewValue,
                             sizeof(uint8_t));
}

uint8_t SetProgramLoadingFlag(uint8_t bNewValue)
{
  uint8_t Counter;
  uint8_t fProgramLoadingFlag;

  Counter = 0;

  if (WriteProgramLoadingFlag(bNewValue))
  {
    if (ReadProgramLoadingFlag(&fProgramLoadingFlag) == FALSE)
    {
      return FALSE;
    }

    if (fProgramLoadingFlag == bNewValue)
    {
      return TRUE;
    }
    else
    {
      while (Counter <= PROGRAM_LOADING_FLAG_RW_TRY
             && fProgramLoadingFlag != bNewValue)
      {
        WriteProgramLoadingFlag(bNewValue);
        ReadProgramLoadingFlag(&fProgramLoadingFlag);
        Counter++;
      }

      return (fProgramLoadingFlag == bNewValue) ? TRUE : FALSE;
    }
  }
  else
  {
    return FALSE;
  }
}

uint8_t GetMostCommonElement(uint8_t *bArray, uint8_t bLen)
{
  uint8_t bCounter, bMax, bResult;

  bResult = 0;

  for (bCounter = 0; bCounter < bLen; bCounter++)
  {
    bArray[bArray[bCounter] % bLen] += bLen;
  }

  bMax = bArray[0];
  for (bCounter = 1; bCounter < bLen; bCounter++)
  {
    if (bArray[bCounter] > bMax)
    {
      bMax = bArray[bCounter];
      bResult = bCounter;
    }
  }

  return bResult;
}

uint8_t GetProgramLoadingFlag(uint8_t *pNewValue)
{
  uint8_t bArray[PROGRAM_LOADING_FLAG_RW_TRY];
  uint8_t Counter;

  Counter = 0;

  if (ReadProgramLoadingFlag(pNewValue) == FALSE)
  {
    return FALSE;
  }
  else
  {
    while (Counter < PROGRAM_LOADING_FLAG_RW_TRY)
    {
      ReadProgramLoadingFlag(pNewValue);
      bArray[Counter] = *pNewValue;
      Counter++;
    }

    *pNewValue = GetMostCommonElement(bArray, PROGRAM_LOADING_FLAG_RW_TRY);

    return TRUE;
  }
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Daylight Saving Time */
uint8_t WriteDaylightSavingTimeFlag(void)
{
  return DataPersistenceWrite(PERSIST_OBJECT_DAYLIGHT_SAVING_FLAG,
                              0U,
                              &bDaylightSavingTimeFlag,
                              sizeof(uint8_t));
}

uint8_t ReadDaylightSavingTimeFlag(void)
{
  return DataPersistenceRead(PERSIST_OBJECT_DAYLIGHT_SAVING_FLAG,
                             0U,
                             &bDaylightSavingTimeFlag,
                             sizeof(uint8_t));
}

void SetDaylightSavingTimeFlag(uint8_t bNewValue)
{
  bDaylightSavingTimeFlag = bNewValue;
}

void GetDaylightSavingTimeFlag(uint8_t *pNewValue)
{
  *pNewValue = bDaylightSavingTimeFlag;
}

uint8_t IsDaylightSavingTimeFlagSet(void)
{
  return bDaylightSavingTimeFlag;
}

void CheckDaylightSavingTimeFlag(void)
{
  if (bDaylightSavingTimeFlag > 1) /* Can be TRUE or FALSE, prevent reading random data */
  {
    SetDaylightSavingTimeFlag(FALSE); /* Default is FALSE */
    WriteDaylightSavingTimeFlag();
  }
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Usernames / Passwords */
int DigitCountsGet(int lNum)
{
  if (lNum)
  {
    return 1;
  }

  int bCntr = 0;

  if (lNum < 0)
  {
    lNum = -lNum;
  }

  while (lNum > 0)
  {
    lNum /= 10;
    bCntr++;
  }

  return bCntr;
}

uint8_t GetSuperAdminValidity(void)
{
  return fIsSuperAdminValid;
}

void SetSuperAdminValidity(uint8_t fValid)
{
  fIsSuperAdminValid = fValid;
}

uint16_t GetSuperAdminUsername(void)
{
  return sSuperAdminUsername;
}

void SetSuperAdminUsername(uint16_t sUsername)
{
  sSuperAdminUsername = sUsername;
}

uint16_t GetSuperAdminPassword(void)
{
  return sSuperAdminPassword;
}

void SetSuperAdminPassword(uint16_t sPassword)
{
  sSuperAdminPassword = sPassword;
}

uint8_t ReadAdminValidity(void)
{
  return DataPersistenceRead(PERSIST_OBJECT_ADMIN_VALIDITY,
                             0U,
                             &fIsAdminValid,
                             sizeof(uint8_t));
}

uint8_t GetAdminValidity(void)
{
  return fIsAdminValid == TRUE;
}

uint8_t WriteAdminValidity(void)
{
  return DataPersistenceWrite(PERSIST_OBJECT_ADMIN_VALIDITY,
                              0U,
                              &fIsAdminValid,
                              sizeof(uint8_t));
}

void SetAdminValidity(uint8_t fValid)
{
  fIsAdminValid = fValid;
}

uint8_t ReadAdminUsername(void)
{
  return DataPersistenceRead(PERSIST_OBJECT_ADMIN_USERNAME,
                             0U,
                             &sAdminUsername,
                             sizeof(uint16_t));
}

uint16_t GetAdminUsername(void)
{
  return sAdminUsername;
}

uint8_t WriteAdminUsername(void)
{
  return DataPersistenceWrite(PERSIST_OBJECT_ADMIN_USERNAME,
                              0U,
                              &sAdminUsername,
                              sizeof(uint16_t));
}

void SetAdminUsername(uint16_t sUsername)
{
  sAdminUsername = sUsername;
}

uint8_t ReadAdminPassword(void)
{
  return DataPersistenceRead(PERSIST_OBJECT_ADMIN_PASSWORD,
                             0U,
                             &sAdminPassword,
                             sizeof(uint16_t));
}

uint16_t GetAdminPassword(void)
{
  return sAdminPassword;
}

uint8_t WriteAdminPassword(void)
{
  return DataPersistenceWrite(PERSIST_OBJECT_ADMIN_PASSWORD,
                              0U,
                              &sAdminPassword,
                              sizeof(uint16_t));
}

void SetAdminPassword(uint16_t sPassword)
{
  sAdminPassword = sPassword;
}

uint8_t GetGuestValidity(void)
{
  return fIsGuestValid == TRUE;
}

void SetGuestValidity(uint8_t fValid)
{
  fIsGuestValid = fValid;
}

uint16_t GetGuestUsername(void)
{
  return sGuestUsername;
}

void SetGuestUsername(uint16_t sUsername)
{
  sGuestUsername = sUsername;
}

uint16_t GetGuestPassword(void)
{
  return sGuestPassword;
}

void SetGuestPassword(uint16_t sPassword)
{
  sGuestPassword = sPassword;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Heater & Lamp Dimming */
/* H&D Commented */

/*
 *  uint8_t   GetHeaterActiveState(void)
 *  {
 *  return SCPRuntime.SHeater.fState;
 *  }
 *
 *  void    SetHeaterActiveState(uint8_t fState)
 *  {
 *  SCPRuntime.SHeater.fState = fState;
 *  }
 *
 *  uint16_t  GetHeaterStartTime(void)
 *  {
 *  return SCPRuntime.SHeater.sStartTime;
 *  }
 *
 *  void    SetHeaterStartTime(uint16_t sStartTime)
 *  {
 *  SCPRuntime.SHeater.sStartTime = sStartTime;
 *  }
 *
 *  uint16_t  GetHeaterEndTime(void)
 *  {
 *  return SCPRuntime.SHeater.sEndTime;
 *  }
 *
 *  void    SetHeaterEndTime(uint16_t sEndTime)
 *  {
 *  SCPRuntime.SHeater.sEndTime = sEndTime;
 *  }
 *
 *  uint8_t   GetLampDimmingActiveState(void)
 *  {
 *  return SCPRuntime.SLampDimming.fState;
 *  }
 *
 *  void    SetLampDimmingActiveState(uint8_t fState)
 *  {
 *  SCPRuntime.SLampDimming.fState = fState;
 *  }
 *
 *  uint16_t  GetLamDimmingStartTime(void)
 *  {
 *  return SCPRuntime.SLampDimming.sStartTime;
 *  }
 *
 *  void    SetLampDimmingStartTime(uint16_t sStartTime)
 *  {
 *  SCPRuntime.SLampDimming.sStartTime = sStartTime;
 *  }
 *
 *  uint16_t  GetLampDimmingEndTime(void)
 *  {
 *  return SCPRuntime.SLampDimming.sEndTime;
 *  }
 *
 *  void    SetLampDimmingEndTime(uint16_t sEndTime)
 *  {
 *  SCPRuntime.SLampDimming.sEndTime = sEndTime;
 *  }
 *
 *  uint8_t   HeaterInfoSave(ptSHeaterLampDim pSHeaterLampDim)
 *  {
 *  memcpy(&(SCPRuntime.SHeater), pSHeaterLampDim, sizeof(tSHeaterLampDim));
 *  return (MSMRequest(MSM_REQ_EEPROM_WRITE, EEPROM_STORAGE_ADDR_HEATER, (void
 *)&SCPRuntime.SHeater, sizeof(tSHeaterLampDim)));
 *  }
 *
 *  uint8_t   HeaterInfoRead(void)
 *  {
 *  if(MSMRequest(MSM_REQ_EEPROM_READ, EEPROM_STORAGE_ADDR_HEATER, (void
 *)&SCPRuntime.SHeater, sizeof(tSHeaterLampDim)))
 *  {
 *  if(SCPRuntime.SHeater.sStartTime > 1439 || SCPRuntime.SHeater.sEndTime >
 *  1439)
 *  {
 *  memset(&SCPRuntime.SHeater, 0, sizeof(SCPRuntime.SHeater));
 *  return (MSMRequest(MSM_REQ_EEPROM_WRITE, (void *)&SCPRuntime.SHeater,
 *  EEPROM_STORAGE_ADDR_HEATER, sizeof(tSHeaterLampDim)));
 *  }
 *  return TRUE;
 *  }
 *  return FALSE;
 *  }
 *
 *  void    HeaterInfoGet(ptSHeaterLampDim pSHeater)
 *  {
 *  memcpy(pSHeater, &SCPRuntime.SHeater, sizeof(tSHeaterLampDim));
 *  }
 *
 *  uint8_t   LampDimmingInfoSave(ptSHeaterLampDim pSHeaterLampDim)
 *  {
 *  memcpy(&(SCPRuntime.SLampDimming), pSHeaterLampDim, sizeof(tSHeaterLampDim));
 *  return (MSMRequest(MSM_REQ_EEPROM_WRITE, (void *)&SCPRuntime.SLampDimming,
 *  EEPROM_STORAGE_ADDR_LAMP_DIM, sizeof(tSHeaterLampDim)));
 *  }
 *
 *  uint8_t   LampDimmingInfoRead(void)
 *  {
 *  if(MSMRequest(MSM_REQ_EEPROM_READ, EEPROM_STORAGE_ADDR_LAMP_DIM, (void
 *)&SCPRuntime.SLampDimming, sizeof(tSHeaterLampDim)))
 *  {
 *  if(SCPRuntime.SLampDimming.sStartTime > 1439 ||
 *  SCPRuntime.SLampDimming.sEndTime > 1439)
 *  {
 *  memset(&SCPRuntime.SLampDimming, 0, sizeof(SCPRuntime.SLampDimming));
 *  return (MSMRequest(MSM_REQ_EEPROM_WRITE, EEPROM_STORAGE_ADDR_LAMP_DIM,
 *  (void
 *)&SCPRuntime.SLampDimming, sizeof(tSHeaterLampDim)));
 *  }
 *  return TRUE;
 *  }
 *  return FALSE;
 *  }
 *
 *  void    LampDimmingInfoGet(ptSHeaterLampDim pSLampDimm)
 *  {
 *  memcpy(pSLampDimm, &SCPRuntime.SLampDimming, sizeof(tSHeaterLampDim));
 *  }
 *
 *  void    ProcessHeaterAndLampDimRequests(void)
 *  {
 *  if(GetHeaterActiveState())
 *  {
 *  if(GetHeaterStartTime() < GetHeaterEndTime())
 *  ((TimeMinuteOfDayGet() >= GetHeaterStartTime()) && (TimeMinuteOfDayGet() <
 *  GetHeaterEndTime())) ? SetHeaterState(TRUE) : SetHeaterState(FALSE); else
 *  if(GetHeaterStartTime() > GetHeaterEndTime())
 *  ((TimeMinuteOfDayGet() >= GetHeaterStartTime()) && (TimeMinuteOfDayGet()
 *  != GetHeaterEndTime())) ? SetHeaterState(TRUE) : SetHeaterState(FALSE);
 *  }
 *  else
 *  SetHeaterState(FALSE);
 *
 *  if(GetLampDimmingActiveState())
 *  {
 *  if(GetLamDimmingStartTime() < GetLampDimmingEndTime())
 *  ((TimeMinuteOfDayGet() >= GetLamDimmingStartTime()) &&
 *  (TimeMinuteOfDayGet() < GetLampDimmingEndTime())) ? SetLampDimmingState(TRUE) :
 *  SetLampDimmingState(FALSE); else if(GetLamDimmingStartTime() >
 *  GetLampDimmingEndTime())
 *  ((TimeMinuteOfDayGet() >= GetLamDimmingStartTime()) &&
 *  (TimeMinuteOfDayGet() != GetLampDimmingEndTime())) ? SetLampDimmingState(TRUE) :
 *  SetLampDimmingState(FALSE);
 *  }
 *  else
 *  SetLampDimmingState(FALSE);
 *  }
 */

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  User Settings */
void UserSettingsSet(tpSUserSettings pSUserSettings)
{
  memcpy(&SUserSettings, pSUserSettings, sizeof(tSUserSettings));
  SetExternalBatteryState(pSUserSettings->fStandbyInfoFlag);
}

uint8_t UserSettingsSave(void)
{
  return DataPersistenceWrite(PERSIST_OBJECT_USER_SETTINGS,
                              0U,
                              &SUserSettings,
                              sizeof(tSUserSettings));
}

uint8_t UserSettingsRead(void)
{
  return DataPersistenceRead(PERSIST_OBJECT_USER_SETTINGS,
                             0U,
                             &SUserSettings,
                             sizeof(tSUserSettings));
}

void UserSettingsGet(tpSUserSettings pSUserSettings)
{
  memcpy(pSUserSettings, &SUserSettings, sizeof(tSUserSettings));
}

void UserSettingsInit(void)
{
  tSUserSettings SLUserSettings;

  memset(&SLUserSettings, 0, sizeof(tSUserSettings));

  SLUserSettings.fSettingsChanged = USER_SETTINGS_CHANGE_CONTROL_VLAUE;
  SLUserSettings.fConfigFlag = TRUE;
  SLUserSettings.fLogFlag = TRUE;
  SLUserSettings.fTrafficCountsFlag = FALSE;
  SLUserSettings.bTrafficCountsPeriod =
    TRAFFIC_COUNTS_PERIOD_DEFAULT; /* 15 minutes */
  SLUserSettings.fStandbyInfoFlag = FALSE;

  UserSettingsSet(&SLUserSettings);
  if (UserSettingsSave())
  {
    UserSettingsRead();
  }
}

uint8_t IsUserSettingsChanged(void)
{
  return SUserSettings.fSettingsChanged == USER_SETTINGS_CHANGE_CONTROL_VLAUE;
}

uint8_t UserSettingsConfigFlagGet(void)
{
  return SUserSettings.fConfigFlag;
}

uint8_t UserSettingsLogFlagGet(void)
{
  return SUserSettings.fLogFlag;
}

uint8_t UserSettingsTrafficCountsFlagGet(void)
{
  return SUserSettings.fTrafficCountsFlag;
}

uint8_t UserSettingsTrafficCountsPeriodGet(void)
{
  return SUserSettings.bTrafficCountsPeriod;
}

uint8_t UserSettingsStandbyFlagGet(void)
{
  return SUserSettings.fStandbyInfoFlag;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Broken Input Settings */
uint8_t BrokenInputSettingsSave(void)
{
  return DataPersistenceWrite(PERSIST_OBJECT_BROKEN_INPUT_SETTINGS,
                              0U,
                              &SBrokenInputSettings,
                              sizeof(SBrokenInputSettings));
}

uint8_t BrokenInputSettingsRead(void)
{
  return DataPersistenceRead(PERSIST_OBJECT_BROKEN_INPUT_SETTINGS,
                             0U,
                             &SBrokenInputSettings,
                             sizeof(SBrokenInputSettings));
}

void BrokenInputSettingsInit(void)
{
  tSBrokenInputSettings SSettings;

  memset(&SSettings, 0, sizeof(SSettings));

  SSettings.fAlreadySet = BROKEN_INPUT_SETTINGS_SET_CONTROL_VLAUE;
  SSettings.SFlags.fLoopBusy = TRUE;
  SSettings.SFlags.fDigitalBusy = FALSE;

  BrokenInputSettingsSet(&SSettings);
  if (BrokenInputSettingsSave())
  {
    BrokenInputSettingsRead();
  }
}

void BrokenInputSettingsSet(tpSBrokenInputSettings pSBrokenInputSettings)
{
  memcpy(&SBrokenInputSettings, pSBrokenInputSettings,
         sizeof(SBrokenInputSettings));
}

void BrokenInputSettingsGet(tpSBrokenInputSettings pSBrokenInputSettings)
{
  memcpy(pSBrokenInputSettings, &SBrokenInputSettings,
         sizeof(SBrokenInputSettings));
}

uint8_t IsBrokenInputSettingsSet(void)
{
  return SBrokenInputSettings.fAlreadySet
         == BROKEN_INPUT_SETTINGS_SET_CONTROL_VLAUE;
}

uint8_t BrokenInputSettingsLoopFlagGet(void)
{
  return SBrokenInputSettings.SFlags.fLoopBusy;
}

uint8_t BrokenInputSettingsDigitalFlagGet(void)
{
  return SBrokenInputSettings.SFlags.fDigitalBusy;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Server Settings */
uint8_t ServerSettingsSave(void)
{
  return DataPersistenceWrite(PERSIST_OBJECT_SERVER_SETTINGS,
                              0U,
                              &SServerSettings,
                              sizeof(SServerSettings));
}

uint8_t ServerSettingsRead(void)
{
  return DataPersistenceRead(PERSIST_OBJECT_SERVER_SETTINGS,
                             0U,
                             &SServerSettings,
                             sizeof(SServerSettings));
}

void ServerSettingsSet(tpSServerSettings pSServerSettings)
{
  memcpy(&SServerSettings, pSServerSettings, sizeof(SServerSettings));
}

void ServerSettingsGet(tpSServerSettings pSServerSettings)
{
  memcpy(pSServerSettings, &SServerSettings, sizeof(SServerSettings));
}

void ServerSettingsInit(void)
{
  tSServerSettings SSettings;

  memset(&SSettings, 0, sizeof(SSettings));

  SSettings.fAlreadySet = SERVER_SETTINGS_SET_CONTROL_VLAUE;
  SSettings.SFlags.fMCSAvailable = TRUE;
  SSettings.SFlags.fNTCIPAvailable = FALSE;

  ServerSettingsSet(&SSettings);
  if (ServerSettingsSave())
  {
    ServerSettingsRead();
  }
}

uint8_t IsServerSettingsSet(void)
{
  return SServerSettings.fAlreadySet == SERVER_SETTINGS_SET_CONTROL_VLAUE;
}

uint8_t ServerSettingsMCSAvailableGet(void)
{
  return SServerSettings.SFlags.fMCSAvailable;
}

uint8_t ServerSettingsNTCIPAvailableGet(void)
{
  return SServerSettings.SFlags.fNTCIPAvailable;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Reset Source */
void SetDeviceResetEvent(void)
{
  SCPRuntime.bResetEvent = EVENT_RESET_POWER_ON_CLEAR_CIRCUIT;

  if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDG1RST))
  {
    SCPRuntime.bResetEvent = EVENT_RESET_WINDOW_WATCHDOG;
  }
  else if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDG1RST))
  {
    SCPRuntime.bResetEvent = EVENT_RESET_INDEPENDENT_WATCHDOG;
  }
  else if (__HAL_RCC_GET_FLAG(RCC_FLAG_LPWR1RST)
           || __HAL_RCC_GET_FLAG(RCC_FLAG_LPWR2RST))
  {
    SCPRuntime.bResetEvent = EVENT_RESET_LOW_POWER;
  }
  else if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST))
  {
    SCPRuntime.bResetEvent = EVENT_RESET_SOFTWARE;
  }
  else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST))
  {
    SCPRuntime.bResetEvent = EVENT_RESET_PIN;
  }
  else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST))
  {
    SCPRuntime.bResetEvent = EVENT_RESET_PORRST;
  }
}

uint8_t GetDeviceResetEvent(void)
{
  return SCPRuntime.bResetEvent;
}

void SetDeviceResetSource(void)
{
  SCPRuntime.bResetEvent = RESET_SOURCE_NONE;

  if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDG1RST))
  {
    SCPRuntime.bResetEvent |= (RESET_SOURCE_WWDG << 1);
  }

  if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDG1RST))
  {
    SCPRuntime.bResetEvent |= (RESET_SOURCE_IWDG << 1);
  }

  if (__HAL_RCC_GET_FLAG(RCC_FLAG_LPWR1RST)
      || __HAL_RCC_GET_FLAG(RCC_FLAG_LPWR2RST))
  {
    SCPRuntime.bResetEvent |= (RESET_SOURCE_LOW_POWER << 1);
  }

  if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST))
  {
    SCPRuntime.bResetEvent |= (RESET_SOURCE_SOFTWARE << 1);
  }

  if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST))
  {
    SCPRuntime.bResetEvent |= (RESET_SOURCE_PIN << 1);
  }

  if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST))
  {
    SCPRuntime.bResetEvent |= (RESET_SOURCE_POR << 1);
  }
}

uint8_t GetDeviceResetSource(void)
{
  return SCPRuntime.bResetSource;
}

void ClearResetFlags(void)
{
  __HAL_RCC_CLEAR_RESET_FLAGS();
}

void ClearStandbyFlag(void)
{
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
}

void ClearWakeupFlag(void)
{
  HAL_PWREx_ClearWakeupFlag(PWR_WAKEUP_FLAG1);
}

void ClearAllFlags(void)
{
  ClearResetFlags();
  ClearWakeupFlag();
  ClearStandbyFlag();
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Source Of Time */
void TimeSourceSet(uint8_t bNewTimeSource)
{
  SCPRuntime.bTimeSource = bNewTimeSource;
}

uint8_t TimeSourceGet(void)
{
  return SCPRuntime.bTimeSource;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Relay State Request */
void RelayStateRequestSet(uint8_t bState)
{
  SCPRuntime.bRelayStateRequest = bState;
}

uint8_t RelayStateRequestGet(void)
{
  return SCPRuntime.bRelayStateRequest;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*    Standby */
void DisableDebug(void)
{
  #ifndef DEBUG
  DBGMCU->CR = 0x00000000;
  #endif
}

void EnableDebug(void)
{
  #ifdef DEBUG
  HAL_DBGMCU_EnableDBGSleepMode();
  HAL_DBGMCU_EnableDBGStopMode();
  HAL_DBGMCU_EnableDBGStandbyMode();
  #endif
}

void DisableInterruptRequests(void)
{
  __disable_irq();
}

void LogStandbyState(void)
{
  LogRequest(LOG_REQ_APPEND, NULL, EVENT_POWER_NORMAL_TO_STAND_BY, 0, 0, 0, 0);
}

uint8_t GetStandbyState(void)
{
  return SCPRuntime.SFlags.fStandby;
}

/*
 *  void PrepareForStandbyMode(void)
 *  {
 *  DisableDebug();
 *
 *  TurnOffLCD();
 *
 *  USBDeInit();
 *
 *  CANDeInit(&hfdcan1);
 *
 *  //CANDeInit(&hfdcan2);
 *
 *  EthDeInit();
 *
 *  //I2CDeInit(&hi2c1);
 *  I2CDeInit(&hi2c4);
 *
 *  UARTDeInit(&huart2);
 *  UARTDeInit(&huart4);
 *  UARTDeInit(&huart5);
 *  UARTDeInit(&huart8);
 *
 *  RNGDeInit();
 *
 *  // CRCDeInit();
 *
 *  Tim2DeInit();
 *
 *  GPIODeInit();
 *
 *  HAL_RCC_DeInit();
 *
 *  Tim1DeInit();
 *  }
 *
 *  void EnterStandbyMode(void)
 *  {
 *  PWREx_WakeupPinTypeDef SWkupPin = { 0 };
 *
 *  // Allow access to Backup
 *  HAL_PWR_EnableBkUpAccess();
 *
 *  // Disable all used wake-up sources: PWR_WAKEUP_PIN1
 *  HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);
 *
 *  // Clear all related wakeup flags
 *  HAL_PWREx_ClearWakeupFlag(PWR_WAKEUP_FLAG1);
 *
 *  // Enable WakeUp Pin PWR_WAKEUP_PIN1 connected to PA.00
 *  SWkupPin.WakeUpPin = PWR_WAKEUP_PIN1;
 *  SWkupPin.PinPolarity = PWR_PIN_POLARITY_LOW;
 *  SWkupPin.PinPull = PWR_PIN_NO_PULL;
 *  HAL_PWREx_EnableWakeUpPin(&SWkupPin);
 *
 *  GPIOChargerShutdownDisable();
 *
 *  // Enter Standby Mode
 *  HAL_PWR_EnterSTANDBYMode();
 *  }
 *
 *  void EnterStandbyModeWithPreparation(uint8_t fPrep)
 *  {
 *  DisableInterruptRequests();
 *  IWDGSetMaxTimeout();
 *  ClearAllFlags();
 *
 *  HeaterDisable(&g_heaterPort);
 *  GPIOGPRSPowerDisable();
 *
 *  if (fPrep)
 *  {
 *   PrepareForStandbyMode();
 *  }
 *
 *  EnterStandbyMode();
 *  SystemReset();
 *  }
 */

void SetStandbyState(uint8_t fState)
{
  SCPRuntime.SFlags.fStandby = fState;
}

void NotifyStandbyState(void)
{
  if (GetStandbyState())
  {
    if (StandbyEventHandle != NULL)
    {
      osEventFlagsSet(StandbyEventHandle, EVENT_FLAGS_STANDBY_100HZ_MISSING);

      return;
    }

    osKernelLock();
    GPIOChargerShutdownDisable();
  }
}

void CheckWakeupState(void)
{
  if (__HAL_PWR_GET_FLAG(PWR_FLAG_SB))
  {
    IWDGSetMaxTimeout();
    ClearStandbyFlag();

    if (!HAL_PWREx_GetWakeupFlag(PWR_WAKEUP_FLAG1))
    {
      ClearWakeupFlag();
      ClearResetFlags();

      GPIOChargerShutdownDisable();
    }
  }
}

void ExecStandbyInfoOps(void)
{
  LogStandbyState();

  if (MCSGetModemType() != MCS_NETWORK_TYPE_NONE)
  {
    SNMPSendPowerDownTrap();
  }

  osDelay(100);

  GPIOChargerShutdownDisable();
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Flash */
uint16_t FlashPeriodEmergencyGet(void)
{
  return SCP.SFlashPeriods.sEmergencyFlashPeriod;
}

uint8_t FlashPeriodEmergencySet(uint16_t sNewPeriod)
{
  if ((sNewPeriod == FLASH_PERIOD_500ms)
      || (sNewPeriod == FLASH_PERIOD_1000ms)
      || (sNewPeriod == FLASH_PERIOD_2000ms)
      || (sNewPeriod == FLASH_PERIOD_4000ms))
  {
    SCP.SFlashPeriods.sEmergencyFlashPeriod = sNewPeriod;

    return TRUE;
  }

  return FALSE;
}

void FlashCntrInc(void)
{
  sFlashCntr++;
  if (sFlashCntr >= FLASH_PERIOD_MAX)
  {
    sFlashCntr = 0;
  }
}

void FlashCntrClear(void)
{
  sFlashCntr = 0;
}

uint8_t FlashOnGet(uint16_t sPeriod)
{
  if ((sFlashCntr % sPeriod) < (sPeriod / 2))
  {
    return FALSE;
  }

  return TRUE;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Device Info */
uint8_t SetDeviceInfo(tpSDeviceInfo pSDeviceInfo, uint8_t keepConnectionInfo)
{
  if (keepConnectionInfo != FALSE)
  {
    /* Keep current connection related info */
    memcpy(pSDeviceInfo->strDomain, SCP.SDevInfo.strDomain,
           sizeof(SCP.SDevInfo.strDomain));
    memcpy(pSDeviceInfo->strAPN,
           SCP.SDevInfo.strAPN,
           sizeof(SCP.SDevInfo.strAPN));
  }
  else
  {
    if ((strncmp(SCP.SDevInfo.strDomain, pSDeviceInfo->strDomain,
                 sizeof(pSDeviceInfo->strDomain)) != 0) || (strncmp(
                                                              SCP
                                                              .SDevInfo.strAPN,
                                                              pSDeviceInfo->
                                                              strAPN,
                                                              sizeof(
                                                                pSDeviceInfo->
                                                                strAPN))
                                                            !=
                                                            0))
    {
      MCSSetConInfoChanged(TRUE);
    }
  }

  memcpy(&(SCP.SDevInfo), pSDeviceInfo, sizeof(tSDeviceInfo));

  return TRUE;
}

void GetDeviceInfo(tpSDeviceInfo pSDeviceInfo)
{
  memcpy(pSDeviceInfo, &(SCP.SDevInfo), sizeof(tSDeviceInfo));
}

int8_t GetDeviceTimeZone(void)
{
  return SCP.SDevInfo.TimeZone;
}

void SetDeviceTimeZone(int8_t bTimeZone)
{
  SCP.SDevInfo.TimeZone = bTimeZone;
}

char *GetDeviceDomainName(void)
{
  return SCP.SDevInfo.strDomain;
}

char *GetDeviceInfoAPNName(void)
{
  return SCP.SDevInfo.strAPN;
}

char *GetDeviceInfoUsername(void)
{
  return SCP.SDevInfo.strUsername;
}

char *GetDeviceInfoPassword(void)
{
  return SCP.SDevInfo.strPassword;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Signal Definitions */
uint8_t SignalTotalGet(void)
{
  return SCP.SConsumed.bSignalTotal;
}

uint8_t SetSignalDefs(uint8_t bSignal, tpSSignalDef pSSignalDefBuffer)
{
  if (bSignal < SIGNALS_MAX)
  {
    memcpy(&(SCP.SaSignalDefs[bSignal]), pSSignalDefBuffer,
           sizeof(tSSignalDef));
    SCP.SConsumed.bSignalTotal++;

    return TRUE;
  }

  return FALSE;
}

void GetSignalDefs(uint8_t bSignal, tpSSignalDef pSSignalDefBuffer)
{
  if (bSignal < SIGNALS_MAX)
  {
    memcpy(pSSignalDefBuffer, &(SCP.SaSignalDefs[bSignal]),
           sizeof(tSSignalDef));
  }
  else
  {
    memset(pSSignalDefBuffer, 0, sizeof(tSSignalDef));
  }
}

uint8_t SignalVoltagesGet(uint8_t bSignal)
{
  uint8_t bVoltages = 0;

  if (bSignal && (bSignal <= SIGNALS_MAX))
  {
    if (SCP.SaSignalDefs[bSignal - 1].SFlags.fValid)
    {
      /* this is a valid signal */
      if (SCP.SaSignalDefs[bSignal - 1].SaSignal[SUBSIGNAL_RED].sPeriod
          != FLASH_PERIOD_INFINITE)
      {
        bVoltages |= SIGNAL_OUTPUT_TYPE_RED; /* this signal is not off */
      }

      if (SCP.SaSignalDefs[bSignal - 1].SaSignal[SUBSIGNAL_YELLOW].sPeriod
          != FLASH_PERIOD_INFINITE)
      {
        bVoltages |= SIGNAL_OUTPUT_TYPE_YELLOW; /* this signal is not off */
      }

      if (SCP.SaSignalDefs[bSignal - 1].SaSignal[SUBSIGNAL_GREEN].sPeriod
          != FLASH_PERIOD_INFINITE)
      {
        bVoltages |= SIGNAL_OUTPUT_TYPE_GREEN; /* this signal is not off */
      }
    }
  }

  return bVoltages;
}

uint16_t SubSignalHasFlash(uint8_t bSignal, uint8_t bOutputType)
{
  switch (bOutputType)
  {
      case SIGNAL_OUTPUT_TYPE_RED:
      {
        if (SCP.SaSignalDefs[bSignal - 1].SaSignal[SUBSIGNAL_RED].sPeriod
            > FLASH_PERIOD_INFINITE)
        {
          return SCP.SaSignalDefs[bSignal - 1].SaSignal[SUBSIGNAL_RED].sPeriod;
        }

        break;
      }

      case SIGNAL_OUTPUT_TYPE_YELLOW:
      {
        if (SCP.SaSignalDefs[bSignal - 1].SaSignal[SUBSIGNAL_YELLOW].sPeriod
            > FLASH_PERIOD_INFINITE)
        {
          return SCP.SaSignalDefs[bSignal
                                  - 1].SaSignal[SUBSIGNAL_YELLOW].sPeriod;
        }

        break;
      }

      case SIGNAL_OUTPUT_TYPE_GREEN:
      {
        if (SCP.SaSignalDefs[bSignal - 1].SaSignal[SUBSIGNAL_GREEN].sPeriod
            > FLASH_PERIOD_INFINITE)
        {
          return SCP.SaSignalDefs[bSignal
                                  - 1].SaSignal[SUBSIGNAL_GREEN].sPeriod;
        }

        break;
      }
  }

  return 0;
} /* SubSignalHasFlash */

uint8_t SignalValidGet(uint8_t bSignal)
{
  if (bSignal && (bSignal <= SIGNALS_MAX))
  {
    return SCP.SaSignalDefs[bSignal - 1].SFlags.fValid;
  }

  return FALSE;
}

uint8_t SignalValidForFlashGet(uint8_t bSignal)
{
  if (bSignal && (bSignal <= SIGNALS_MAX))
  {
    return SCP.SaSignalDefs[bSignal - 1].SFlags.fValidForFlash;
  }

  return FALSE;
}

uint8_t SignalValidForEmergencyFlashGet(uint8_t bSignal)
{
  if (bSignal && (bSignal <= SIGNALS_MAX))
  {
    return SCP.SaSignalDefs[bSignal - 1].SFlags.fValidForEmergencyFlash;
  }

  return FALSE;
}

uint8_t SignalDurationUnlimitedGet(uint8_t bSignal)
{
  if (bSignal && (bSignal <= SIGNALS_MAX))
  {
    return SCP.SaSignalDefs[bSignal - 1].SFlags.fDurationUnlimited;
  }

  return FALSE;
}

uint8_t SignalHasFlash(uint8_t bSignal)
{
  if (bSignal && (bSignal <= SIGNALS_MAX))
  {
    uint8_t bIndex;

    for (bIndex = 0; bIndex < SUBSIGNALS_MAX; bIndex++)
    {
      if (SCP.SaSignalDefs[bSignal - 1].SaSignal[bIndex].sPeriod
          > FLASH_PERIOD_INFINITE)
      {
        return TRUE;
      }
    }
  }

  return FALSE;
}

uint8_t SignalHasGreen(uint8_t bSignal)
{
  if (bSignal && (bSignal <= SIGNALS_MAX))
  {
    if (SCP.SaSignalDefs[bSignal - 1].SaSignal[SUBSIGNAL_GREEN].sPeriod
        != FLASH_PERIOD_INFINITE)
    {
      return TRUE;
    }
  }

  return FALSE;
}

uint8_t SignalHasYellow(uint8_t bSignal)
{
  if (bSignal && (bSignal <= SIGNALS_MAX))
  {
    if (SCP.SaSignalDefs[bSignal - 1].SaSignal[SUBSIGNAL_YELLOW].sPeriod
        != FLASH_PERIOD_INFINITE)
    {
      return TRUE;
    }
  }

  return FALSE;
}

uint8_t SignalHasRed(uint8_t bSignal)
{
  if (bSignal && (bSignal <= SIGNALS_MAX))
  {
    if (SCP.SaSignalDefs[bSignal - 1].SaSignal[SUBSIGNAL_RED].sPeriod
        != FLASH_PERIOD_INFINITE)
    {
      return TRUE;
    }
  }

  return FALSE;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Signals Defined */
void SetSignalsDefined(tpSSignalsDefined pSSignalsDefined)
{
  memcpy(&(SCP.SSignalsDefined), pSSignalsDefined, sizeof(tSSignalsDefined));
}

void GetSignalsDefined(tpSSignalsDefined pSSignalsDefined)
{
  memcpy(pSSignalsDefined, &(SCP.SSignalsDefined), sizeof(tSSignalsDefined));
}

uint8_t SignalsDefinedBlockingGet(void)
{
  return SCP.SSignalsDefined.bBlocking;
}

uint8_t SignalsDefinedFreeGet(void)
{
  return SCP.SSignalsDefined.bFree;
}

uint8_t SignalsDefinedGreenFlashGet(void)
{
  return SCP.SSignalsDefined.bGreenFlash;
}

uint8_t SignalsDefinedDarkGet(void)
{
  return SCP.SSignalsDefined.bDark;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Signal Group Sets */
void SetRuntimeSet(uint8_t bSetNo, tpSSetRuntime pSSetRuntime)
{
  memcpy(&(SRuntimes.SaSetRuntime[bSetNo]), pSSetRuntime, sizeof(tSSetRuntime));
}

void SetRuntimeGet(uint8_t bSetNo, tpSSetRuntime pSSetRuntime)
{
  memcpy(pSSetRuntime, &(SRuntimes.SaSetRuntime[bSetNo]), sizeof(tSSetRuntime));
}

uint8_t IsSetValid(uint8_t bSetNo)
{
  uint8_t bSGNo;

  for (bSGNo = 0; bSGNo < SGTotalGet(); bSGNo++)
  {
    if (SCP.SaSGDefs[bSGNo].bOwner == bSetNo)
    {
      return TRUE; /* this set includes a group in it so it is valid */
    }
  }

  return FALSE;
}

uint8_t SetTotalGet(void)
{
  uint8_t bSGNo;
  uint8_t bSetTotal = 1; /* minimum one set exists */

  for (bSGNo = 0; bSGNo < SGTotalGet(); bSGNo++)
  {
    if (SCP.SaSGDefs[bSGNo].bOwner >= bSetTotal)
    {
      bSetTotal = (SCP.SaSGDefs[bSGNo].bOwner + 1);
    }
  }

  return bSetTotal;
}

void SetRuntimesInit(void)
{
  uint8_t bSetNo;
  uint8_t bSetTotal = SetTotalGet();

  memset(&SRuntimes.SaSetRuntime, 0, sizeof(SRuntimes.SaSetRuntime));
  for (bSetNo = 0; bSetNo < bSetTotal; bSetNo++)
  {
    if (IsSetValid(bSetNo))
    {
      SetSigModeSet(bSetNo, SIGNALING_MODE_NORMAL);
      SRuntimes.SaSignalStateRuntimes[SignalStateRuntimeCurNoGet()].bSetNo =
        bSetNo + 1;
    }
  }
}

uint8_t SetSigModeIsOK(void)
{
  uint8_t bSetNo = 0;

  for (bSetNo = 0; bSetNo < SetTotalGet(); bSetNo++)
  {
    switch (SetSigModeGet(bSetNo))
    {
        case SIGNALING_MODE_NONE:
        {
          break;
        }

        case SIGNALING_MODE_NORMAL:
        case SIGNALING_MODE_FLASH:
        {
          return TRUE;
        }
    }
  }

  return FALSE;
}

uint8_t SetSigModeIsThis(uint8_t bMode)
{
  uint8_t bSetNo;
  uint8_t bSetTotal = SetTotalGet();

  for (bSetNo = 0; bSetNo < bSetTotal; bSetNo++)
  {
    if (SetSigModeGet(bSetNo) == bMode)
    {
      return TRUE;
    }
  }

  return FALSE;
}

uint8_t SetSigModeGet(uint8_t bSetNo)
{
  return SRuntimes.SaSetRuntime[bSetNo].bSignalingMode;
}

uint8_t SetSigModeIsEmergent(uint8_t bSetNo)
{
  uint8_t bSingalingMode = SetSigModeGet(bSetNo);

  return bSingalingMode == SIGNALING_MODE_EMERGENCY_DARK
         || bSingalingMode == SIGNALING_MODE_EMERGENCY_FLASH;
}

uint8_t SetSigModeSourceGet(uint8_t bSetNo)
{
  return SRuntimes.SaSetRuntime[bSetNo].bSigModeSource;
}

void SetSigModeSet(uint8_t bSetNo, uint8_t bNewMode)
{
  if (SRuntimes.SaSetRuntime[bSetNo].bSignalingMode != bNewMode)
  {
    uint8_t PrevSigMode = SRuntimes.SaSetRuntime[bSetNo].bSignalingMode;

    SRuntimes.SaSetRuntime[bSetNo].bSignalingMode = bNewMode;

    SRuntimes.SaSignalStateRuntimes[SignalStateRuntimeCurNoGet()].bExecutionMode
      =
        SIGNAL_STATE_EXEC_MODE_FAIL_FLASH;

    LogRequest(LOG_REQ_APPEND_ASYNCH, NULL, EVENT_SET_SIGNALING_MODE_CHANGE,
               (bSetNo + 1), PrevSigMode, bNewMode, 0);
  }
}

void ApplyEMToOneSet(uint8_t bSetNo,
                     uint8_t bEM,
                     uint8_t bSigModeSource,
                     uint8_t bParam1,
                     uint8_t bParam2)
{
  uint8_t bSGNo = 0;

  SeizeSGData();
  if (bSetNo < SIGNAL_SETS_MAX)
  {
    SetSigModeSet(bSetNo,
                  (bEM
                   == EMERGENCY_METHOD_DARK) ? SIGNALING_MODE_EMERGENCY_DARK
                  : SIGNALING_MODE_EMERGENCY_FLASH);

    SRuntimes.SaSetRuntime[bSetNo].bSigModeSource = bSigModeSource;
    SRuntimes.SaSetRuntime[bSetNo].bParam1 = bParam1;
    SRuntimes.SaSetRuntime[bSetNo].bParam2 = bParam2;

    for (bSGNo = 0; bSGNo < SGTotalGet(); bSGNo++)
    {
      if (SCP.SaSGDefs[bSGNo].bOwner == bSetNo)
      {
        SGSignalSet(bSGNo,
                    (bEM
                     == EMERGENCY_METHOD_DARK) ? SignalsDefinedDarkGet()
                    : GetSGFlashFailureSignal(bSGNo));
      }
    }
  }

  ReleaseSGData();
}

void ApplyEMToAllSets(uint8_t bEM, uint8_t bSigModeSource)
{
  uint8_t bSGNo = 0, bSetNo = 0;

  SeizeSGData();
  for (bSetNo = 0; bSetNo < SIGNAL_SETS_MAX; bSetNo++)
  {
    SetSigModeSet(bSetNo,
                  (bEM
                   == EMERGENCY_METHOD_DARK) ? SIGNALING_MODE_EMERGENCY_DARK
                  : SIGNALING_MODE_EMERGENCY_FLASH);
    SRuntimes.SaSetRuntime[bSetNo].bSigModeSource = bSigModeSource;
  }

  for (bSGNo = 0; bSGNo < SGTotalGet(); bSGNo++)
  {
    SGSignalSet(bSGNo,
                (bEM
                 == EMERGENCY_METHOD_DARK) ? SignalsDefinedDarkGet()
                : GetSGFlashFailureSignal(bSGNo));
  }

  ReleaseSGData();
}

void SetInvalidSignalState(uint8_t bSetNo, uint8_t fValue)
{
  SRuntimes.SaSetRuntime[bSetNo].fInvalidSignalDetected = fValue;
}

uint8_t GetInvalidSignalState(uint8_t bSetNo)
{
  if (bSetNo < SIGNAL_SETS_MAX)
  {
    return SRuntimes.SaSetRuntime[bSetNo].fInvalidSignalDetected;
  }

  return FALSE;
}

void SetInvalidSignalSequenceState(uint8_t bSetNo, uint8_t fValue)
{
  if (bSetNo < SIGNAL_SETS_MAX)
  {
    SRuntimes.SaSetRuntime[bSetNo].fInvalidSignalSequenceDetected = fValue;
  }
}

uint8_t GetInvalidSignalSequenceState(uint8_t bSetNo)
{
  if (bSetNo < SIGNAL_SETS_MAX)
  {
    return SRuntimes.SaSetRuntime[bSetNo].fInvalidSignalSequenceDetected;
  }

  return FALSE;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Signal Switching Modules */
uint8_t SSMTotalGet(void)
{
  return SCP.SConsumed.bSSMTotal;
}

void SSMTotalSet(uint8_t bSSMNo)
{
  if (bSSMNo <= SIG_DEV_SSM_MAX)
  {
    SCP.SConsumed.bSSMTotal = bSSMNo;
  }
}

void SetRuntimeSSMStatus(void)
{
  uint8_t bSSMNo = 0;

  for (bSSMNo = 0; bSSMNo < SSMTotalGet(); bSSMNo++)
  {
    SRuntimes.sSSMStatus |= laValue2Bit[bSSMNo];
  }
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Signal Groups */
uint8_t SetSignalGroups(uint8_t bSGNo, tpSSGDef pSSGBuffer)
{
  if (bSGNo < SIGNAL_GROUPS_MAX)
  {
    if ((SCP.SaSGDefs[bSGNo].bType == SIGNAL_GROUP_TYPE_NONE)
        && (pSSGBuffer->bType != SIGNAL_GROUP_TYPE_NONE))
    {
      SCP.SConsumed.bSGTotal++;
    }
    else if ((SCP.SaSGDefs[bSGNo].bType != SIGNAL_GROUP_TYPE_NONE)
             && (pSSGBuffer->bType == SIGNAL_GROUP_TYPE_NONE))
    {
      SCP.SConsumed.bSGTotal--;
    }

    if (pSSGBuffer->bClosingDur > 10)
    {
      pSSGBuffer->bClosingDur--;
    }

    memcpy(&(SCP.SaSGDefs[bSGNo]), pSSGBuffer, sizeof(tSSGDef));

    return TRUE;
  }

  return FALSE;
}

uint8_t GetSGRedLampFailureNumberEM(uint8_t bSGNo)
{
  if (bSGNo < SIGNAL_GROUPS_MAX)
  {
    return SCP.SaSGDefs[bSGNo].SEmergencyMethods.bRedLampFailureNumberEM;
  }

  return EMERGENCY_METHOD_NONE;
}

uint8_t SGGet(uint8_t bSGNo, tpSSGDef pSSGBuffer)
{
  if (bSGNo < SGTotalGet())
  {
    memcpy(pSSGBuffer, &SCP.SaSGDefs[bSGNo], sizeof(tSSGDef));

    return TRUE;
  }
  else
  {
    memset(pSSGBuffer, 0, sizeof(tSSGDef));
  }

  return FALSE;
}

uint8_t SGTypeGet(uint8_t bSGNo)
{
  if (bSGNo < SGTotalGet())
  {
    return SCP.SaSGDefs[bSGNo].bType;
  }

  return SIGNAL_GROUP_TYPE_NONE;
}

uint8_t SGOpeningSignalGet(uint8_t bSGNo)
{
  if (bSGNo < SGTotalGet())
  {
    return SCP.SaSGDefs[bSGNo].bOpeningSignal;
  }

  return 0;
}

uint8_t SGOpeningDurGet(uint8_t bSGNo)
{
  if (bSGNo < SGTotalGet())
  {
    return SCP.SaSGDefs[bSGNo].bOpeningDuration;
  }

  return 0;
}

uint8_t SGClosingSignalGet(uint8_t bSGNo)
{
  if (bSGNo < SGTotalGet())
  {
    return SCP.SaSGDefs[bSGNo].bClosingSignal;
  }

  return 0;
}

uint8_t SGClosingDurGet(uint8_t bSGNo)
{
  if (bSGNo < SGTotalGet())
  {
    return SCP.SaSGDefs[bSGNo].bClosingDur;
  }

  return 0;
}

uint8_t SGFlashSignalGet(uint8_t bSGNo)
{
  if (bSGNo < SGTotalGet())
  {
    return SCP.SaSGDefs[bSGNo].bFlashSignal;
  }

  return 0;
}

uint8_t GetSGFlashFailureSignal(uint8_t bSGNo)
{
  if (bSGNo < SGTotalGet())
  {
    return SCP.SaSGDefs[bSGNo].bFailureFlashSignal;
  }

  return 0;
}

uint8_t SGGreenFlashDurGet(uint8_t bSGNo)
{
  if (bSGNo < SGTotalGet())
  {
    return SCP.SaSGDefs[bSGNo].bGreenFlashDur;
  }

  return 0;
}

uint8_t SGConflictGet(uint8_t bSGNo1, uint8_t bSGNo2)
{
  if ((bSGNo1 < SGTotalGet())
      && (SCP.SaSGDefs[bSGNo1].bType != SIGNAL_GROUP_TYPE_NONE) )
  {
    if ((bSGNo2 < SGTotalGet())
        && (SCP.SaSGDefs[bSGNo2].bType != SIGNAL_GROUP_TYPE_NONE) )
    {
      return SCP.SaSGDefs[bSGNo1].SaConflicts[bSGNo2].fConflict;
    }
  }

  return FALSE;
}

uint8_t SGConflictSet(uint8_t bSGNo, uint8_t bTargetSGNo, uint8_t fValue)
{
  if ((bSGNo < SGTotalGet()) && (bTargetSGNo < SIGNAL_GROUPS_MAX))
  {
    SCP.SaSGDefs[bSGNo].SaConflicts[bTargetSGNo].fConflict = fValue;
    SCP.SaSGDefs[bTargetSGNo].SaConflicts[bSGNo].fConflict = fValue;

    return TRUE;
  }

  return FALSE;
}

uint8_t SGClearanceGet(uint8_t bSGNo, uint8_t bConflictSGNo)
{
  if ((bSGNo < SGTotalGet())
      && (SCP.SaSGDefs[bSGNo].bType != SIGNAL_GROUP_TYPE_NONE)
      && (bConflictSGNo < SGTotalGet())
      && (SCP.SaSGDefs[bConflictSGNo].bType != SIGNAL_GROUP_TYPE_NONE))
  {
    return SCP.SaSGDefs[bSGNo].SaConflicts[bConflictSGNo].bClearance;
  }

  return 0;
}

uint8_t SGClearanceGetbyIndex(uint8_t bSGNo,
                              uint8_t bConflictSGIndex,
                              uint8_t *pbClearanceDuration,
                              uint8_t *pbConflictSGNo)
{
  if ((bSGNo < SGTotalGet()) && (bConflictSGIndex < SGTotalGet()))
  {
    uint8_t bSGIndex;
    uint8_t bSearchIndex = 0;

    for (bSGIndex = 0; bSGIndex < SIGNAL_GROUPS_MAX; bSGIndex++)
    {
      if (SCP.SaSGDefs[bSGNo].SaConflicts[bSGIndex].fConflict == TRUE)
      {
        if (bSearchIndex == bConflictSGIndex)
        {
          *pbClearanceDuration =
            SCP.SaSGDefs[bSGNo].SaConflicts[bSGIndex].bClearance;
          *pbConflictSGNo = bSGIndex + 1;

          return TRUE;
        }

        bSearchIndex++;
      }
    }
  }

  return FALSE;
}

uint8_t SGClearanceSet(uint8_t bSGNo, uint8_t bTargetSGNo, uint8_t bClearance)
{
  /* upper bound(SIGNAL_GROUPS_MAX) may be incorrect */
  if ((bSGNo < SGTotalGet()) && (bTargetSGNo < SIGNAL_GROUPS_MAX))
  {
    /* these two groups cannot be open at the same time */
    /* (bTargetSGNo) must wait for closing of bSGNo at least (bClearance) */
    /* seconds before opening */
    SCP.SaSGDefs[bSGNo].SaConflicts[bTargetSGNo].bClearance = bClearance;
    SCP.SaSGDefs[bSGNo].SaConflicts[bTargetSGNo].fConflict = TRUE;
    SCP.SaSGDefs[bTargetSGNo].SaConflicts[bSGNo].fConflict = TRUE;

    return TRUE;
  }

  return FALSE;
}

uint8_t SGClearanceTotalGet(uint8_t bSGNo)
{
  uint8_t bConflictIndex;
  uint8_t bTotalConflictNumber = 0;

  if (bSGNo < SGTotalGet())
  {
    for (bConflictIndex = 0; bConflictIndex < SGTotalGet(); bConflictIndex++)
    {
      if ((SCP.SaSGDefs[bSGNo].SaConflicts[bConflictIndex].bClearance != 0)
          || (SCP.SaSGDefs[bSGNo].SaConflicts[bConflictIndex]
              .
              fConflict
              !=
              0))
      {
        bTotalConflictNumber++;
      }
    }
  }

  return bTotalConflictNumber;
}

uint8_t SGFirstOutputGet(uint8_t bSGNo)
{
  return SCP.SaSGDefs[bSGNo].bFirstOutput;
}

uint8_t SGIsValid(uint8_t bSGNo)
{
  if (bSGNo && (bSGNo <= SGTotalGet()))
  {
    return TRUE;
  }

  return FALSE;
}

uint8_t GetSGOwner(uint8_t bSGNo)
{
  return SCP.SaSGDefs[bSGNo].bOwner;
}

uint8_t GetSGLastRedLampFailureEM(uint8_t bSGNo)
{
  if (bSGNo < SIGNAL_GROUPS_MAX)
  {
    return SCP.SaSGDefs[bSGNo].SEmergencyMethods.bLastRedLampFailureEM;
  }

  return EMERGENCY_METHOD_NONE;
}

uint8_t SGConflictTotalGet(uint8_t bSGNo, uint8_t *pbConflictTotal)
{
  uint8_t bSGIndex;

  if (bSGNo < SGTotalGet())
  {
    *pbConflictTotal = 0;
    for (bSGIndex = 0; bSGIndex < SGTotalGet(); bSGIndex++)
    {
      /* if signal group type is decided, take SG definition into account */
      if (SCP.SaSGDefs[bSGNo].SaConflicts[bSGIndex].fConflict == TRUE)
      {
        (*pbConflictTotal)++;
      }
    }

    return TRUE;
  }

  return FALSE;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Signal Groups - Runtime */
uint8_t SGTotalGet(void)
{
  return SCP.SConsumed.bSGTotal;
}

uint8_t SGSignalGet(uint8_t bSGNo)
{
  if (bSGNo < SIGNAL_GROUPS_MAX)
  {
    return SRuntimes.SaSGRuntimes[bSGNo].bCurrentSignal;
  }

  return 0;
}

void SGSignalSet(uint8_t bSGNo, uint8_t bSignal)
{
  if ((SCP.SaSGDefs[bSGNo].bType != SIGNAL_GROUP_TYPE_NONE)
      && (bSGNo < SGTotalGet()))
  {
    SRuntimes.SaSGRuntimes[bSGNo].bCurrentSignal = bSignal;
  }
}

uint8_t SGDurationGet(uint8_t bSGNo)
{
  if ((SCP.SaSGDefs[bSGNo].bType != SIGNAL_GROUP_TYPE_NONE)
      && (bSGNo < SGTotalGet()))
  {
    return SRuntimes.SaSGRuntimes[bSGNo].bDuration;
  }

  return 0;
}

uint8_t SGStateGet(uint8_t bSGNo)
{
  if ((SCP.SaSGDefs[bSGNo].bType != SIGNAL_GROUP_TYPE_NONE)
      && (bSGNo < SGTotalGet()))
  {
    return SRuntimes.SaSGRuntimes[bSGNo].bState;
  }

  return SIGNAL_GROUP_STATE_NONE;
}

void SGRuntimeDataSet(uint8_t bSGNo, uint8_t bSignal, uint8_t bState)
{
  if ((SRuntimes.SaSGRuntimes[bSGNo].bCurrentSignal != bSignal)
      || (SRuntimes.SaSGRuntimes[bSGNo].bState != bState) )   /* ayni state+ayni signal tekrar gelirse islem yapmiyor */
  {
    /* if new signal or new state is different from the current ones, init new */
    /* running values else do not change the signal duration because it is used */
    /* by the program task */
    switch (SetSigModeGet(SCP.SaSGDefs[bSGNo].bOwner))
    {
        case SIGNALING_MODE_EMERGENCY_FLASH:
        {
          SRuntimes.SaSGRuntimes[bSGNo].bCurrentSignal =
            GetSGFlashFailureSignal(bSGNo);
          break;
        }

        case SIGNALING_MODE_EMERGENCY_DARK:
        {
          SRuntimes.SaSGRuntimes[bSGNo].bCurrentSignal =
            SignalsDefinedDarkGet();
          break;
        }

        default:
        {
          SRuntimes.SaSGRuntimes[bSGNo].bCurrentSignal = bSignal;
          break;
        }
    }

    SRuntimes.SaSGRuntimes[bSGNo].bDuration = 0;
    SRuntimes.SaSGRuntimes[bSGNo].bState = bState;
  }
}

void SGDurInc(void)
{
  uint8_t bGroup;

  for (bGroup = 0; bGroup < SGTotalGet(); bGroup++)
  {
    if (SCP.SaSGDefs[bGroup].bType != SIGNAL_GROUP_TYPE_NONE)
    {
      if (SRuntimes.SaSGRuntimes[bGroup].bState != SIGNAL_GROUP_STATE_NONE)
      {
        SRuntimes.SaSGRuntimes[bGroup].bDuration++;
      }
    }
  }
}

uint8_t SGFlasherAdd(uint8_t bSGNo)
{
  if (bSGNo && (bSGNo <= SIGNAL_GROUPS_MAX))
  {
    sSGFlashers |= laValue2Bit[bSGNo - 1];

    return TRUE;
  }

  return FALSE;
}

uint8_t SGFlasherSub(uint8_t bSGNo)
{
  if (bSGNo && (bSGNo <= SIGNAL_GROUPS_MAX))
  {
    sSGFlashers &= (~(laValue2Bit[bSGNo - 1]));

    return TRUE;
  }

  return FALSE;
}

uint8_t SGIsFlasher(uint8_t bSGNo)
{
  if (bSGNo && (bSGNo <= SIGNAL_GROUPS_MAX))
  {
    if (sSGFlashers & laValue2Bit[bSGNo - 1])
    {
      return TRUE;
    }
  }

  return FALSE;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Signal Outputs */
uint8_t SetSignalOutputs(uint8_t bSSMNo, uint8_t bSONo, tpSSODef pSNewSODef)
{
  if (bSONo < SIGNAL_OUTPUTS_MAX)
  {
    if (bSSMNo > SSMTotalGet())
    {
      SCP.SConsumed.bSSMTotal++;
    }

    memcpy(&(SCP.SaSODefs[bSONo]), pSNewSODef, sizeof(tSSODef));
    SCP.SConsumed.bSOTotal++;

    /* append SO definition to SG */
    SCP.SaSODefs[bSONo].bNextOutput = 0; /* new SO definition will be the last one */

    /* it may be first SO definition for SG */
    if (SCP.SaSGDefs[SCP.SaSODefs[bSONo].bOwner - 1].bFirstOutput == 0)
    {
      SCP.SaSGDefs[SCP.SaSODefs[bSONo].bOwner - 1].bFirstOutput = (bSONo + 1);
    }
    else
    {
      /* at least one SO definition exists in SG */
      uint8_t bOutputNo = SCP.SaSGDefs[(SCP.SaSODefs[bSONo].bOwner
                                        - 1)].bFirstOutput;

      /* go to the end of the SO list for the SG */
      while (SCP.SaSODefs[bOutputNo - 1].bNextOutput)
      {
        bOutputNo = SCP.SaSODefs[bOutputNo - 1].bNextOutput;
      }

      SCP.SaSODefs[bOutputNo - 1].bNextOutput = (bSONo + 1);
    }

    return TRUE;
  }

  return FALSE;
}

uint8_t GetSOTotal(void)
{
  return SCP.SConsumed.bSOTotal;
}

void GetSODef(uint8_t bSONo, tpSSODef pSSOBuffer)
{
  if (bSONo < SIGNAL_OUTPUTS_MAX)
  {
    memcpy(pSSOBuffer, &SCP.SaSODefs[bSONo], sizeof(tSSODef));
  }
  else
  {
    memset(pSSOBuffer, 0, sizeof(tSSODef));
  }
}

uint8_t GetSODefByIndex(uint8_t bSGNo,
                        uint8_t bIndexInSG,
                        tpSSODef pSSOBuffer,
                        uint8_t *pbIndex)
{
  uint8_t bTempIndexInSG = 0; /* point to the first SO in the SG, the signal */

  /* output number is (bFirstOutput - 1) */

  if (bSGNo < SGTotalGet())
  {
    if (bIndexInSG < SIGNAL_OUTPUTS_MAX)
    {
      memcpy(pSSOBuffer,
             &SCP.SaSODefs[SCP.SaSGDefs[bSGNo].bFirstOutput - 1],
             sizeof(tSSODef));

      (*pbIndex) = SCP.SaSGDefs[bSGNo].bFirstOutput - 1;

      /* the first output information */
      if (bIndexInSG == bTempIndexInSG)
      {
        return TRUE;
      }

      bTempIndexInSG++;

      do
      {
        if (pSSOBuffer->bNextOutput == 0) /* means that we are at the end of the SO list */
        {
          return FALSE;
        }

        (*pbIndex) = pSSOBuffer->bNextOutput - 1;

        memcpy(pSSOBuffer, &SCP.SaSODefs[pSSOBuffer->bNextOutput - 1],
               sizeof(tSSODef));

        if (bTempIndexInSG == bIndexInSG)
        {
          return TRUE;
        }

        bTempIndexInSG++;
      }while (bTempIndexInSG <= bIndexInSG);
    }
  }

  return FALSE;
} /* GetSODefByIndex */

uint16_t GetSOPower(uint8_t bSONo)
{
  if (bSONo < SIGNAL_OUTPUTS_MAX)
  {
    return SCP.SaSODefs[bSONo].sPower[(GetLampDimmingState()) ? 0 : 1];
  }
  else
  {
    return 0;
  }
}

uint16_t GetSOPowerRecordNet(uint8_t bSONo)
{
  if (bSONo < SIGNAL_OUTPUTS_MAX)
  {
    return SCP.SaSODefs[bSONo].sPowerRecordNet[(GetLampDimmingState()) ? 0 : 1];
  }
  else
  {
    return 0;
  }
}

uint8_t GetSGNextOutputNo(uint8_t bSONo)
{
  return SCP.SaSODefs[bSONo].bNextOutput;
}

uint8_t GetSOEM(uint8_t bSONo)
{
  if (bSONo < SIGNAL_OUTPUTS_MAX)
  {
    return SCP.SaSODefs[bSONo].SFlags.bSOFailureEM;
  }

  return EMERGENCY_METHOD_NONE;
}

uint8_t GetSOOwner(uint8_t bSONo)
{
  if (bSONo < SIGNAL_OUTPUTS_MAX)
  {
    return SCP.SaSODefs[bSONo].bOwner;
  }

  return 0;
}

uint8_t SGSOTotalGet(uint8_t bSGNo, uint8_t *pbSOTotal)
{
  uint8_t bSOIndex;

  if (bSGNo < SGTotalGet())
  {
    *pbSOTotal = 1; /* assign 1 for the first output, a SG has at least 1 output */
    bSOIndex = SCP.SaSGDefs[bSGNo].bFirstOutput - 1;
    while (SCP.SaSODefs[bSOIndex].bNextOutput)
    {
      bSOIndex = SCP.SaSODefs[bSOIndex].bNextOutput - 1;
      (*pbSOTotal)++;
    }

    return TRUE;
  }

  return FALSE;
}

uint8_t GetSOType(uint8_t bSONo)
{
  if (bSONo < SIGNAL_OUTPUTS_MAX)
  {
    return SCP.SaSODefs[bSONo].bType;
  }

  return SIGNAL_OUTPUT_TYPE_NONE;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Current / Voltage / Slope */
void GetCVSDef(tpSCVSDef pSCVSDef)
{
  memcpy(pSCVSDef, &SCP.SCVSDef, sizeof(tSCVSDef));
}

uint8_t SetCVS(tpSCVSDef pSCVSDef)
{
  memcpy(&(SCP.SCVSDef), pSCVSDef, sizeof(tSCVSDef));

  return TRUE;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Conflicts EM */
void GetConflictsEM(tpSConflictsEM pSConflictsEM)
{
  memcpy(pSConflictsEM, &SCP.SConflictsEM, sizeof(tSConflictsEM));
}

uint8_t SetConflictsEM(tpSConflictsEM pSConflictsEM)
{
  memcpy(&(SCP.SConflictsEM), pSConflictsEM, sizeof(tSConflictsEM));

  return TRUE;
}

uint8_t GetVoltageLimitsEM(void)
{
  return SCP.SConflictsEM.bVoltageLimitsEM;
}

uint8_t GetFrequencyErrorEM(void)
{
  return SCP.SConflictsEM.bFrequencyErrorEM;
}

uint8_t GetGGEM(void)
{
  return SCP.SConflictsEM.bGreenGreenEM;
}

uint8_t GetYGEM(void)
{
  return SCP.SConflictsEM.bYellowGreenEM;
}

uint8_t GetYYEM(void)
{
  return SCP.SConflictsEM.bYellowYellowEM;
}

uint8_t GetInvalidSignalSequenceEM(void)
{
  return SCP.SConflictsEM.bInvalidSignalSequenceEM;
}

uint8_t GetInvalidSignalEM(void)
{
  return SCP.SConflictsEM.bInvalidSignalEM;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Sequences */
uint8_t SeqGet(uint8_t bSeqNo)
{
  if (bSeqNo < SeqTotalGet())
  {
    /* Memory Checksum Control */
    if (SCP.SChecksum.baSeqDefs[bSeqNo] != ByteChecksum(&SCP.SaSeqDefs[bSeqNo],
                                                        sizeof(tSSeqDef)))
    {
      LogRequest(LOG_REQ_APPEND_ASYNCH, NULL, EVENT_CHECKSUM_FLASH_ERROR, 0,
                 EVENT_PARAM_CHECKSUM_FLASH_ERROR_SeqDefs,
                 0, 0);

      return FALSE;
    }

    bCurrentSeqNo = bSeqNo;

    return TRUE;
  }

  return FALSE;
}

uint8_t SeqLoad(uint8_t bSeqNo)
{
  if (bSeqNo < SIGNAL_SEQS_MAX)
  {
    /* Memory Checksum Control */
    if (SCP.SChecksum.baSeqDefs[bSeqNo] != ByteChecksum(&SCP.SaSeqDefs[bSeqNo],
                                                        sizeof(tSSeqDef)))
    {
      LogRequest(LOG_REQ_APPEND_ASYNCH,
                 NULL,
                 EVENT_CHECKSUM_FLASH_ERROR,
                 0,
                 EVENT_PARAM_CHECKSUM_FLASH_ERROR_SeqDefs,
                 0,
                 0);

      return FALSE;
    }

    bCurrentSeqNo = bSeqNo;

    return TRUE;
  }

  return FALSE;
}

uint8_t SeqRead(uint8_t bSeqNo, tpSSeqDef pSSignalSeqBuffer)
{
  if (bSeqNo < SeqTotalGet())
  {
    memcpy(pSSignalSeqBuffer, &SCP.SaSeqDefs[bSeqNo], sizeof(tSSeqDef));

    return TRUE;
  }

  return FALSE;
}

uint8_t SeqSet(uint8_t bSeqNo, tpSSeqDef pSSignalSeqBuffer)
{
  if (bSeqNo < SIGNAL_SEQS_MAX)
  {
    if (SCP.SConsumed.baSeqStepTotal[bSeqNo] == 0) /* first step definition means a new sequence definition */
    {
      SCP.SConsumed.bSeqTotal++;
    }

    SCP.SChecksum.baSeqDefs[bSeqNo] = ByteChecksum(pSSignalSeqBuffer,
                                                   sizeof(tSSeqDef));                    /* update checksum */

    SCP.SConsumed.baSeqStepTotal[bSeqNo]++;

    return TRUE;
  }

  return FALSE;
}

uint8_t SeqSave(uint8_t bSeqNo, uint8_t bSeqProc, uint8_t LCDReq)
{
  if (bSeqNo < SIGNAL_SEQS_MAX)
  {
    if (LCDReq)
    {
      ProgramDataSet();
    }

    if (bSeqProc == SEQ_PROC_ADD)
    {
      if (SCP.SConsumed.baSeqStepTotal[bSeqNo] == 0) /* first step definition means a new sequence definition */
      {
        SCP.SConsumed.bSeqTotal++;
      }

      SCP.SChecksum.baSeqDefs[bSeqNo] = ByteChecksum(&SCP.SaSeqDefs[bSeqNo],
                                                     sizeof(tSSeqDef));                         /* update checksum */

      SCP.SConsumed.baSeqStepTotal[bSeqNo]++;

      return TRUE;
    }
    else if (bSeqProc == SEQ_PROC_DEL)
    {
      SCP.SChecksum.baSeqDefs[bSeqNo] = ByteChecksum(&SCP.SaSeqDefs[bSeqNo],
                                                     sizeof(tSSeqDef));                         /* update checksum */

      SCP.SConsumed.baSeqStepTotal[bSeqNo]--;

      return TRUE;
    }
    else if (bSeqProc == SEQ_PROC_UPDATE)
    {
      SCP.SChecksum.baSeqDefs[bSeqNo] = ByteChecksum(&SCP.SaSeqDefs[bSeqNo],
                                                     sizeof(tSSeqDef));                         /* update checksum */

      return TRUE;
    }
  }

  return FALSE;
} /* SeqSave */

void SeqInit(uint8_t bSeqNo)
{
  memset(&SCP.SaSeqDefs[bSeqNo], 0, sizeof(tSSeqDef));
  bCurrentSeqNo = bSeqNo;
}

uint8_t SeqStepSGSignalGet(uint8_t bSeqNo, uint8_t bStepNo, uint8_t bSGNo)
{
  if (bSeqNo <= SeqTotalGet())
  {
    if (bStepNo <= SCP.SConsumed.baSeqStepTotal[bSeqNo])
    {
      uint8_t bSignal = SCP.SaSeqDefs[bSeqNo].baSignals[bStepNo][bSGNo / 2];

      if (bSGNo % 2)
      {
        bSignal >>= 4;
      }
      else
      {
        bSignal &= 0x0F;
      }

      return bSignal;
    }

    return 0;
  }

  return 0;
}

uint8_t SeqStepSGSignalSet(uint8_t bSeqNo,
                           uint8_t bStepNo,
                           uint8_t bSGNo,
                           uint8_t bReceivedSignal)
{
  if (bSeqNo <= SIGNAL_SEQS_MAX)
  {
    if ((bStepNo <= SCP.SConsumed.baSeqStepTotal[bSeqNo])
        && (bSGNo <= SGTotalGet()))
    {
      if (bSGNo % 2)
      {
        bReceivedSignal <<= 4;
      }

      SCP.SaSeqDefs[bSeqNo].baSignals[bStepNo][bSGNo / 2] |= bReceivedSignal;

      return TRUE;
    }
  }

  return FALSE;
}

uint8_t SeqStepDurGet(uint8_t bSeqNo, uint8_t bStepNo)
{
  if (bSeqNo < SIGNAL_SEQS_MAX)
  {
    if (bStepNo < SCP.SConsumed.baSeqStepTotal[bSeqNo])
    {
      return SCP.SaSeqDefs[bSeqNo].baDurations[bStepNo];
    }

    return 0;
  }

  return 0;
}

uint8_t SeqStepDurSet(uint8_t bSeqNo, uint8_t bStepNo, uint8_t bDur)
{
  if (bSeqNo <= SIGNAL_SEQS_MAX)
  {
    if (bStepNo <= SCP.SConsumed.baSeqStepTotal[bSeqNo])
    {
      SCP.SaSeqDefs[bSeqNo].baDurations[bStepNo] = bDur;

      return TRUE;
    }

    return FALSE;
  }

  return FALSE;
}

uint8_t SeqStepInc(uint8_t bSeqNo)
{
  if (bSeqNo <= SIGNAL_SEQS_MAX)
  {
    SCP.SaSeqDefs[bSeqNo].bNoOfSteps++;

    return TRUE;
  }

  return FALSE;
}

uint8_t SeqStepDecr(uint8_t bSeqNo)
{
  if (bSeqNo < SIGNAL_SEQS_MAX)
  {
    SCP.SaSeqDefs[bCurrentSeqNo].bNoOfSteps--;

    return TRUE;
  }

  return FALSE;
}

uint8_t SeqCurStepNumTotalGet(void)
{
  return SCP.SaSeqDefs[bCurrentSeqNo].bNoOfSteps;
}

uint8_t SeqStepNumTotalGet(uint8_t bSeqNo)
{
  return SCP.SaSeqDefs[bSeqNo].bNoOfSteps;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Sequences - Runtime */
uint8_t SeqStepGet(uint8_t bSeqNo, uint8_t bReferenceSecond)
{
  uint8_t bIndex;
  uint16_t sDurationTotal = 0;

  for (bIndex = 0; bIndex < SeqStepNumTotalGet(bSeqNo); bIndex++)
  {
    sDurationTotal += SCP.SaSeqDefs[bSeqNo].baDurations[bIndex];
    if (sDurationTotal >= bReferenceSecond)
    {
      return bIndex + 1;
    }
  }

  return 0;
}

uint8_t SeqStart(uint8_t bSeqNo, uint8_t bReferenceSecond)
{
  /* there is a rule constraint and it is true or there is no rule constraint */
  if (((bCurrentSeqNo != bSeqNo) && SeqGet(bSeqNo))
      || (bCurrentSeqNo == bSeqNo))
  {
    /* current sequence is not bSeqNo and bSeqNo successfully read to RAM or */
    /* current sequence is bSeqNo */
    if (SeqStepGet(bSeqNo, bReferenceSecond))
    {
      /* start sequence from reference second */
      SRuntimes.SSeqRuntime.bCurrentStep = SeqStepGet(bSeqNo,
                                                      bReferenceSecond) - 1;
      SRuntimes.SSeqRuntime.bCurrentStepCurrentDuration = bReferenceSecond;

      return TRUE;
    }
  }

  return FALSE;
}

uint8_t SeqStop(uint8_t bSeqNo)
{
  if (bSeqNo < SeqTotalGet())
  {
    /* init runtime variables */
    memset(&SRuntimes.SSeqRuntime, 0, sizeof(SRuntimes.SSeqRuntime));

    return TRUE;
  }

  return FALSE;
}

uint8_t SeqDurInc(void)
{
  SRuntimes.SSeqRuntime.bCurrentStepCurrentDuration++;
  if (SRuntimes.SSeqRuntime.bCurrentStepCurrentDuration
      >= (SCP.SaSeqDefs[bCurrentSeqNo].baDurations[SRuntimes
                                                   .
                                                   SSeqRuntime.bCurrentStep]
          + SeqStepExtDurGet(
            SRuntimes.SSeqRuntime.bCurrentStep)))
  {
    if (SRuntimes.SSeqRuntime.bCurrentStep == (SeqCurStepNumTotalGet() - 1))
    {
      /* we are in last step, wait here */
    }
    else if (SRuntimes.SSeqRuntime.bCurrentStep < SeqCurStepNumTotalGet())
    {
      SRuntimes.SSeqRuntime.bCurrentStepCurrentDuration = 0;
      SRuntimes.SSeqRuntime.bCurrentStep++;
    }
    else
    {
      return FALSE;
    }
  }

  return TRUE;
}

uint8_t SeqCurrentGet(void)
{
  return bCurrentSeqNo + 1;
}

uint8_t SeqCurrentStepGet(void)
{
  return SRuntimes.SSeqRuntime.bCurrentStep;
}

uint8_t SeqCurrentStepDurationGet(void)
{
  return SCP.SaSeqDefs[bCurrentSeqNo].baDurations[SRuntimes.SSeqRuntime.
                                                  bCurrentStep];
}

uint8_t SeqCurrentStepCurrentDurationGet(void)
{
  return SRuntimes.SSeqRuntime.bCurrentStepCurrentDuration;
}

uint8_t SeqDurCurGet(void)
{
  uint8_t bIndex;
  uint8_t bCurDur = 0;

  /* find current duration of sequence by adding 'sum of previous step */
  /* durations' and 'current step current duration' */
  for (bIndex = 0; bIndex < SeqCurStepNumTotalGet(); bIndex++)
  {
    if (bIndex < SRuntimes.SSeqRuntime.bCurrentStep)
    {
      bCurDur += SCP.SaSeqDefs[bCurrentSeqNo].baDurations[bIndex]
                 + SeqStepExtDurGet(bIndex);
    }
  }

  bCurDur += SRuntimes.SSeqRuntime.bCurrentStepCurrentDuration;

  return bCurDur;
}

uint8_t SeqDurGet(uint8_t bSeqNo)
{
  uint8_t bIndex;
  uint8_t bCurDur = 0;

  SeqGet(bSeqNo);

  /* find current duration of sequence by adding 'sum of previous step */
  /* durations' and 'current step current duration' */
  for (bIndex = 0; bIndex < SeqCurStepNumTotalGet(); bIndex++)
  {
    bCurDur += SCP.SaSeqDefs[bCurrentSeqNo].baDurations[bIndex];
  }

  return bCurDur;
}

uint8_t SeqTotalGet(void)
{
  return SCP.SConsumed.bSeqTotal;
}

uint8_t SeqStepTotalGet(uint8_t bSeqNo)
{
  if (bSeqNo < SeqTotalGet())
  {
    return SCP.SConsumed.baSeqStepTotal[bSeqNo];
  }

  return 0;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Phases */
uint8_t PhaseGet(uint8_t bPhaseNo, tpSPhaseDef pSPhaseBuffer)
{
  if (bPhaseNo < PhaseTotalGet())
  {
    memcpy(pSPhaseBuffer, &SCP.SaPhaseDefs[bPhaseNo], sizeof(tSPhaseDef));

    return TRUE;
  }

  return FALSE;
}

uint8_t PhaseSet(uint8_t bPhaseNo, tpSPhaseDef pSPhaseBuffer)
{
  if (bPhaseNo < PHASES_MAX)
  {
    memcpy(&SCP.SaPhaseDefs[bPhaseNo], pSPhaseBuffer, sizeof(tSPhaseDef));
    if ((bPhaseNo + 1) > PhaseTotalGet())
    {
      SCP.SConsumed.bPhaseTotal++;
    }

    return TRUE;
  }

  return FALSE;
}

uint8_t PhaseMinDurationGet(uint8_t bPhaseNo)
{
  if (bPhaseNo < PhaseTotalGet())
  {
    return SCP.SaPhaseDefs[bPhaseNo].bMinDur;
  }

  return 0;
}

uint8_t PhaseMaxDurationGet(uint8_t bPhaseNo)
{
  if (bPhaseNo < PhaseTotalGet())
  {
    return SCP.SaPhaseDefs[bPhaseNo].bMaxDur;
  }

  return 0;
}

uint8_t PhaseTotalGet(void)
{
  return SCP.SConsumed.bPhaseTotal;
}

uint8_t PhaseHasSG(uint8_t bPhaseNo, uint8_t bSGNo)
{
  if (bPhaseNo < PhaseTotalGet())
  {
    if (bSGNo < SIGNAL_GROUPS_MAX)
    {
      if (SCP.SaPhaseDefs[bPhaseNo].lGroups & laValue2Bit[bSGNo])
      {
        return TRUE;
      }
    }
  }

  return FALSE;
}

uint8_t PhaseIsValid(uint8_t bPhaseNo)
{
  if (bPhaseNo && (bPhaseNo <= PhaseTotalGet()))
  {
    return TRUE;
  }

  return FALSE;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Phases - Runtime */
uint8_t PhaseStart(uint8_t bPhaseNo, uint8_t bReferenceSecond)
{
  SCPRuntime.bRunningPhase = bPhaseNo;
  if (bPhaseNo < PhaseTotalGet())
  {
    /* init runtime variables */
    SRuntimes.SaPhaseRuntimes[bPhaseNo].bRefSec = bReferenceSecond;
    SRuntimes.SaPhaseRuntimes[bPhaseNo].sElapsedDur = 0;
    SRuntimes.SaPhaseRuntimes[bPhaseNo].fRun = FALSE;
  }

  return TRUE;
}

uint8_t PhaseStop(uint8_t bPhaseNo)
{
  if (bPhaseNo < PhaseTotalGet())
  {
    /* init runtime variables */
    memset(&SRuntimes.SaPhaseRuntimes[bPhaseNo], 0,
           sizeof(SRuntimes.SaPhaseRuntimes[bPhaseNo]));
    SRuntimes.SaPhaseRuntimes[bPhaseNo].bExtDur = 0;
    SRuntimes.SaPhaseRuntimes[bPhaseNo].fRun = TRUE;

    return TRUE;
  }

  return FALSE;
}

void PhaseRunSet(uint8_t bPhaseNo, uint8_t fValue)
{
  if (bPhaseNo < PhaseTotalGet())
  {
    SRuntimes.SaPhaseRuntimes[bPhaseNo].fRun = fValue;
  }
}

uint8_t PhaseRunGet(uint8_t bPhaseNo)
{
  if (bPhaseNo < PhaseTotalGet())
  {
    return SRuntimes.SaPhaseRuntimes[bPhaseNo].fRun;
  }

  return FALSE;
}

void PhaseExtDurSet(uint8_t bPhaseNo, int8_t bExtDur)
{
  if (bPhaseNo < PhaseTotalGet())
  {
    SRuntimes.SaPhaseRuntimes[bPhaseNo].bExtDur = bExtDur;
  }
}

int8_t PhaseExtDurGet(uint8_t bPhaseNo)
{
  if (bPhaseNo < PhaseTotalGet())
  {
    return SRuntimes.SaPhaseRuntimes[bPhaseNo].bExtDur;
  }

  return 0;
}

uint8_t PhaseDurInc(uint8_t bPhaseNo)
{
  if (bPhaseNo < PhaseTotalGet())
  {
    SRuntimes.SaPhaseRuntimes[bPhaseNo].sElapsedDur++;

    return SRuntimes.SaPhaseRuntimes[bPhaseNo].sElapsedDur;
  }

  return 0;
}

uint16_t PhaseElapsedDurGet(uint8_t bPhaseNo)
{
  if (bPhaseNo < PhaseTotalGet())
  {
    return SRuntimes.SaPhaseRuntimes[bPhaseNo].sElapsedDur % 1000;
  }

  return 0;
}

uint16_t PhaseTotalElapsedDurGet(void)
{
  uint8_t bPhaseNo = 0;
  uint16_t sTotalDur = 0;

  for (bPhaseNo = 0; bPhaseNo < PhaseTotalGet(); bPhaseNo++)
  {
    if (PhaseRunGet(bPhaseNo))
    {
      sTotalDur += WorkPlanPhaseDurGet(bPhaseNo);
    }
  }

  return sTotalDur + PhaseElapsedDurGet(ProgramCurrentNoGet() - 1);
}

uint8_t PhaseCurrentDurGet(uint8_t bPhaseNo)
{
  if (bPhaseNo < PhaseTotalGet())
  {
    return SRuntimes.SaPhaseRuntimes[bPhaseNo].sElapsedDur
           + SRuntimes.SaPhaseRuntimes[bPhaseNo].bRefSec;
  }

  return 0;
}

uint8_t PhaseMinDurHasElapsed(uint8_t bPhaseNo)
{
  if (SRuntimes.SaPhaseRuntimes[bPhaseNo].sElapsedDur
      >= SCP.SaPhaseDefs[bPhaseNo].bMinDur)
  {
    return TRUE;
  }

  return FALSE;
}

uint8_t PhaseSGAdd(uint8_t bPhaseNo, uint8_t bSGNo)
{
  if (bPhaseNo && (bPhaseNo <= PhaseTotalGet()))
  {
    if (bSGNo && (bSGNo <= SGTotalGet()))
    {
      SCP.SaPhaseDefs[bPhaseNo - 1].lGroups |= laValue2Bit[bSGNo - 1];

      return TRUE;
    }
  }

  return FALSE;
}

uint8_t PhaseSGSub(uint8_t bPhaseNo, uint8_t bSGNo)
{
  if (bPhaseNo && (bPhaseNo <= PhaseTotalGet()))
  {
    if (bSGNo && (bSGNo <= SGTotalGet()))
    {
      SCP.SaPhaseDefs[bPhaseNo - 1].lGroups &= ~((laValue2Bit[bSGNo - 1]));

      return TRUE;
    }
  }

  return FALSE;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Inputs */
uint8_t InputSet(uint8_t bType, uint8_t bInputNo, tpSInput pSInput)
{
  switch (bType)
  {
      case INPUT_TYPE_DETECTOR:
      {
        if (bInputNo < INPUTS_DETECTOR_MAX)
        {
          memcpy(&SCP.SaDetectorDefs[bInputNo], pSInput, sizeof(tSInput));
          SCP.SConsumed.bDetectorTotal++;

          return TRUE;
        }

        break;
      }

      case INPUT_TYPE_DIGITAL:
      {
        if (bInputNo < INPUTS_DIGITAL_MAX)
        {
          memcpy(&SCP.SaInputDefs[bInputNo], pSInput, sizeof(tSInput));
          SCP.SConsumed.bInputDigitalTotal++;

          return TRUE;
        }

        break;
      }
  }

  return FALSE;
}

uint8_t InputGet(uint8_t bType, uint8_t bInputNo, tpSInput pSInput)
{
  switch (bType)
  {
      case INPUT_TYPE_DETECTOR:
      {
        if (bInputNo < INPUTS_DETECTOR_MAX)
        {
          memcpy(pSInput, &SCP.SaDetectorDefs[bInputNo], sizeof(tSInput));

          return TRUE;
        }
      }

      case INPUT_TYPE_DIGITAL:
      {
        if (bInputNo < INPUTS_DIGITAL_MAX)
        {
          memcpy(pSInput,
                 &SCP.SaInputDefs[bInputNo],
                 sizeof(tSInput));

          return TRUE;
        }
      }
  }

  return FALSE;
}

uint8_t InputOwnerSGGet(uint8_t bType, uint8_t bInputNo)
{
  switch (bType)
  {
      case INPUT_TYPE_DETECTOR:
      {
        if (bInputNo < INPUTS_DETECTOR_MAX)
        {
          return SCP.SaDetectorDefs[bInputNo].bOwnerSG;
        }
      }

      case INPUT_TYPE_DIGITAL:
      {
        if (bInputNo < INPUTS_DIGITAL_MAX)
        {
          return SCP.SaInputDefs[bInputNo].bOwnerSG;
        }
      }
  }

  return 0;
}

uint8_t InputGreenDurPerDemandGet(uint8_t bType, uint8_t bInputNo)
{
  switch (bType)
  {
      case INPUT_TYPE_DETECTOR:
      {
        if (bInputNo < INPUTS_DETECTOR_MAX)
        {
          return SCP.SaDetectorDefs[bInputNo].bGreenDurPerDemand;
        }
      }

      case INPUT_TYPE_DIGITAL:
      {
        if (bInputNo < INPUTS_DIGITAL_MAX)
        {
          return SCP.SaInputDefs[bInputNo].bGreenDurPerDemand;
        }
      }
  }

  return 0;
}

uint8_t InputRedDurInBrokenGet(uint8_t bType, uint8_t bInputNo)
{
  switch (bType)
  {
      case INPUT_TYPE_DETECTOR:
      {
        if (bInputNo < INPUTS_DETECTOR_MAX)
        {
          return SCP.SaDetectorDefs[bInputNo].bRedDurInBroken;
        }
      }

      case INPUT_TYPE_DIGITAL:
      {
        if (bInputNo < INPUTS_DIGITAL_MAX)
        {
          return SCP.SaInputDefs[bInputNo].bRedDurInBroken;
        }
      }
  }

  return 0;
}

uint8_t InputPhaseInBrokenGet(uint8_t bType, uint8_t bInputNo)
{
  switch (bType)
  {
      case INPUT_TYPE_DETECTOR:
      {
        if (bInputNo < INPUTS_DETECTOR_MAX)
        {
          return SCP.SaDetectorDefs[bInputNo].bPhaseInBroken;
        }
      }

      case INPUT_TYPE_DIGITAL:
      {
        if (bInputNo < INPUTS_DIGITAL_MAX)
        {
          return SCP.SaInputDefs[bInputNo].bPhaseInBroken;
        }
      }
  }

  return 0;
}

uint8_t InputTotalGet(uint8_t bType)
{
  switch (bType)
  {
      case INPUT_TYPE_DETECTOR:
      {
        return SCP.SConsumed.bDetectorTotal;
      }

      case INPUT_TYPE_DIGITAL:
      {
        return SCP.SConsumed.bInputDigitalTotal;
      }
  }

  return 0;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Input Runtime */
void SetIOInputs(uint8_t bModuleNo, tpSCanDigitalIOInputs pSCanIOInputs)
{
  if (!(bIOValuesValid & laValue2Bit[bModuleNo]))
  {
    memcpy(&SaCanDigitalIOInputs[bModuleNo], pSCanIOInputs,
           sizeof(SaCanDigitalIOInputs[bModuleNo]));

    bIOValuesValid |= laValue2Bit[bModuleNo];
  }
}

uint8_t IOInputErrGet(void)
{
  uint8_t bIndex;

  for (bIndex = 0; bIndex < INPUTS_DIGITAL_MAX; bIndex++)
  {
    if (SCP.SaInputDefs[bIndex].bOwnerSG
        && SRuntimes.SaInputRuntimes[bIndex].fBroken)
    {
      return TRUE;
    }
  }

  return FALSE;
}

void SetLDInputs(uint8_t bLDNo, uint8_t bIOMNo, uint8_t *pData)
{
  uint8_t bLoopIndex = 0;
  uint8_t bLoopData = (0xFF - pData[0]);

  for (bLoopIndex = 0; bLoopIndex < LOOP_NUM_IN_1_LD; bLoopIndex++)
  {
    uint8_t bitIndex = bLoopIndex + (bLDNo * LOOP_NUM_IN_1_LD);

    if (GetBitValue(bLoopData, bLoopIndex))
    {
      (bIOMNo
       == 0) ? SetBitValue(SaCanDetectorIOInputs[bIOMNo].sLoopEmptyStates,
                           bitIndex)
      :SetBitValue(SaCanDetectorIOInputs[bIOMNo].sLoopEmptyStates,
                   (bitIndex - 16));
    }
    else
    {
      (bIOMNo
       == 0) ? ClearBitValue(SaCanDetectorIOInputs[bIOMNo].sLoopEmptyStates,
                             bitIndex)
      :ClearBitValue(SaCanDetectorIOInputs[bIOMNo].sLoopEmptyStates,
                     (bitIndex - 16));
    }

    if (GetBitValue(bLoopData, (bLoopIndex + 4)))
    {
      (bIOMNo == 0) ? SetBitValue(SaCanDetectorIOInputs[bIOMNo].sLoopSafeStates,
                                  bitIndex)
      :SetBitValue(SaCanDetectorIOInputs[bIOMNo].sLoopSafeStates,
                   (bitIndex - 16));
    }
    else
    {
      (bIOMNo
       == 0) ? ClearBitValue(SaCanDetectorIOInputs[bIOMNo].sLoopSafeStates,
                             bitIndex)
      :ClearBitValue(SaCanDetectorIOInputs[bIOMNo].sLoopSafeStates,
                     (bitIndex - 16));
    }
  }

  if (!(bLDValuesValid & laValue2Bit[bIOMNo]))
  {
    bLDValuesValid |= laValue2Bit[bIOMNo];
  }
} /* SetLDInputs */

uint8_t IOLoopErrGet(void)
{
  uint8_t bIndex;

  for (bIndex = 0; bIndex < INPUTS_DETECTOR_MAX; bIndex++)
  {
    if (SCP.SaDetectorDefs[bIndex].bOwnerSG
        && SRuntimes.SaDetectorRuntimes[bIndex].fBroken)
    {
      return TRUE;
    }
  }

  return FALSE;
}

uint8_t IOLoopErrNoGet(void)
{
  uint8_t bIndex;

  for (bIndex = 0; bIndex < INPUTS_DETECTOR_MAX; bIndex++)
  {
    if (SCP.SaDetectorDefs[bIndex].bOwnerSG
        && SRuntimes.SaDetectorRuntimes[bIndex].fBroken)
    {
      return bIndex;
    }
  }

  return 0;
}

void UseIOValues(void)
{
  uint8_t bIndex = 0;

  /* we have received io data which are detector/input data from mp side. These */
  /* data */
  for (bIndex = 0; bIndex < MODULES_IO_MAX; bIndex++)
  {
    if (bIOValuesValid & laValue2Bit[bIndex])
    {
      /* update all time io input values and target phases or sequences, when a */
      /* change occurs in daily work plan, these values will be used immediately */
      UseDigitalIOValues(bIndex, &(SaCanDigitalIOInputs[bIndex]));
      bIOValuesValid &= ~laValue2Bit[bIndex];
    }
    else if (!SaCanDigitalIOInputs[bIndex].fIsPhysicallyDriven)
    {
      if (++baIOMessagePeriodCounter[bIndex] > IO_MESSAGE_PERIOD)
      {
        baIOMessagePeriodCounter[bIndex] = 0;
        UseDigitalIOValues(bIndex, &(SaCanDigitalIOInputs[bIndex]));
      }
    }

    if (bLDValuesValid & laValue2Bit[bIndex])
    {
      /* update all time io input values and target phases or sequences, when a */
      /* change occurs in daily work plan, these values will be used immediately */
      UseLDIOValues(bIndex, &(SaCanDetectorIOInputs[bIndex]));
      bLDValuesValid &= ~laValue2Bit[bIndex];
    }
    else if (!SaCanDetectorIOInputs[bIndex].fIsPhysicallyDriven)
    {
      if (++baLDMessagePeriodCounter[bIndex] > IO_MESSAGE_PERIOD)
      {
        baLDMessagePeriodCounter[bIndex] = 0;
        UseLDIOValues(bIndex, &(SaCanDetectorIOInputs[bIndex]));
      }
    }
  }

  SErrInfo.SErrLD.fError = IOLoopErrGet();

  if (SErrInfo.SErrLD.fError)
  {
    SErrInfo.SErrLD.bLDNo = IOLoopErrNoGet() + 1;
    SErrInfo.SErrLD.bSGNo = SCP.SaDetectorDefs[SErrInfo.SErrLD.bLDNo
                                               - 1].bOwnerSG;
  }
  else
  {
    SErrInfo.SErrLD.bLDNo = 0;
    SErrInfo.SErrLD.bSGNo = 0;
  }
} /* UseIOValues */

void IOPerValsInit(void)
{
  uint8_t bIndex;

  IncTrafficCountsTimer(); /* Inc timer every second */

  /* Init IO Period Values */
  if ((GetTrafficCountsTimer() % TRAFFIC_COUNTS_PERIOD_SECONDS) == 0) /* 90 secs */
  {
    /* digital inputs */
    for (bIndex = 0; bIndex < INPUTS_DIGITAL_MAX; bIndex++)
    {
      if (SCP.SaInputDefs[bIndex].bOwnerSG)
      {
        /* Add to MCS traffic counts runtime */
        SMCSTrafficCountsRuntimes.SaMCSDigitalInputRuntimes[bIndex].
        sDemandCntInPer +=
          SRuntimes.SaInputRuntimes[bIndex].bDemandCntInPer;
        SMCSTrafficCountsRuntimes.SaMCSDigitalInputRuntimes[bIndex].sOccDurInPer
          +=
            SRuntimes.SaInputRuntimes[bIndex]
            .
            sOccDurInPer;

        /* init run-time periodical values */
        SRuntimes.SaInputRuntimes[bIndex].bDemandCntInPer = 0;
        SRuntimes.SaInputRuntimes[bIndex].sFDemandDurInPer = 0;
        SRuntimes.SaInputRuntimes[bIndex].sGapDurInPer = 0;
        SRuntimes.SaInputRuntimes[bIndex].sOccDurInPer = 0;
      }
    }

    /* detectors */
    for (bIndex = 0; bIndex < INPUTS_DETECTOR_MAX; bIndex++)
    {
      if (SCP.SaDetectorDefs[bIndex].bOwnerSG)
      {
        /* Add to MCS traffic counts runtime */
        SMCSTrafficCountsRuntimes.SaMCSDetectorInputRuntimes[bIndex].
        sDemandCntInPer +=
          SRuntimes.SaDetectorRuntimes[bIndex].bDemandCntInPer;
        SMCSTrafficCountsRuntimes.SaMCSDetectorInputRuntimes[bIndex].
        sOccDurInPer +=
          SRuntimes.SaDetectorRuntimes[bIndex].sOccDurInPer;

        /* init run-time periodical values */
        SRuntimes.SaDetectorRuntimes[bIndex].bDemandCntInPer = 0;
        SRuntimes.SaDetectorRuntimes[bIndex].sFDemandDurInPer = 0;
        SRuntimes.SaDetectorRuntimes[bIndex].sGapDurInPer = 0;
        SRuntimes.SaDetectorRuntimes[bIndex].sOccDurInPer = 0;
      }
    }
  }

  if ((UserSettingsTrafficCountsPeriodGet() != 0) && ((GetTrafficCountsTimer()
                                                       % (
                                                         UserSettingsTrafficCountsPeriodGet()
                                                         * 60))
                                                      == 0)) /* Traffic counts period (15, 30, 45, 60 minutes) */
  {
    SetTrafficCountsTimer(0);
    /* digital inputs */
    for (bIndex = 0; bIndex < INPUTS_DIGITAL_MAX; bIndex++)
    {
      if (SCP.SaInputDefs[bIndex].bOwnerSG)
      {
        /* init MCS traffic counts run-time periodical values */
        SMCSTrafficCountsRuntimes.SaMCSDigitalInputRuntimes[bIndex].
        sDemandCntInPer = 0;
        SMCSTrafficCountsRuntimes.SaMCSDigitalInputRuntimes[bIndex].sOccDurInPer
          =
            0;
      }
    }

    /* detectors */
    for (bIndex = 0; bIndex < INPUTS_DETECTOR_MAX; bIndex++)
    {
      if (SCP.SaDetectorDefs[bIndex].bOwnerSG)
      {
        /* init MCS traffic counts run-time periodical values */
        SMCSTrafficCountsRuntimes.SaMCSDetectorInputRuntimes[bIndex].
        sDemandCntInPer = 0;
        SMCSTrafficCountsRuntimes.SaMCSDetectorInputRuntimes[bIndex].
        sOccDurInPer = 0;
      }
    }
  }
} /* IOPerValsInit */

uint8_t GetLastDetectorDemandIssued(void)
{
  return bLastDetectorDemandIssued;
}

uint8_t GetLastInputDemandIssued(void)
{
  return bLastInputDemandIssued;
}

void UseDigitalIOValues(uint8_t bModuleNo, tpSCanDigitalIOInputs pSCanIOInputs)
{
  uint8_t bIndex;
  uint8_t bIndex2;
  tSCanDigitalIOInputs SCanDigitalIOInputs;

  memcpy(&SCanDigitalIOInputs, pSCanIOInputs, sizeof(tSCanDigitalIOInputs));

  for (bIndex = 0; bIndex < SGTotalGet(); bIndex++)
  {
    if (SignalHasGreen(SRuntimes.SaSGRuntimes[bIndex].bCurrentSignal) == FALSE)
    {
      SRuntimes.SaSGIORuntime[bIndex].bDemandCntInGreen = 0;
      SRuntimes.SaSGIORuntime[bIndex].sOccDurInGreenMax = 0;
    }
    else
    {
      SRuntimes.SaSGIORuntime[bIndex].bDemandCntInRed = 0;
      SRuntimes.SaSGIORuntime[bIndex].sFDemandDurInRedMax = 0; /* signal is not red in group, initialize counter */
    }
  }

  /* digital inputs */
  for (bIndex = 0; bIndex < IO_INPUTS_DIGITAL_MAX; bIndex++)
  {
    /* there are two io modules, one per cage */
    uint8_t bInputNo = ((bModuleNo * IO_INPUTS_DIGITAL_MAX) + bIndex); /* 0..15 for module 1, 16..31 for module 2 */

    /* digital input 6 is assigned for heater, digital 7 is assigned for lamp */
    /* dimming. */
    if ((bInputNo == (HEATER_BUTTON_DIG_INPUT_NO - 1))
        || (bInputNo == (LAMP_DIMMING_BUTTON_DIG_INPUT_NO - 1)))
    {
      continue;
    }

    /* digital input 8 is assigned for police button */
    /* to pause program */
    if (bInputNo == (POLICE_BUTTON_DIG_INPUT_NO - 1))
    {
      if (SCanDigitalIOInputs.sInputStates & laValue2Bit[bIndex])
      {
        SetPoliceButtonState(FALSE);
      }
      else
      {
        SetPoliceButtonState(TRUE);
      }
    }
    else
    {
      uint8_t bOwnerSG = SCP.SaInputDefs[bInputNo].bOwnerSG;

      if (bOwnerSG)
      {
        /* input belongs to a signal group, evaluate it */
        if (SCanDigitalIOInputs.sInputSafeStates & laValue2Bit[bIndex])
        {
          /* detector is safe so it is also safe to operate on this detector */
          if (SRuntimes.SaInputRuntimes[bInputNo].fBroken)
          {
            /* previous state of detector is broken but it is safe now */
            SRuntimes.SaInputRuntimes[bInputNo].sBrokenDur = 0;
            SRuntimes.SaInputRuntimes[bInputNo].fBroken = FALSE;
            LogRequest(LOG_REQ_APPEND_ASYNCH, NULL, EVENT_DIGITAL_INPUT_SAFE,
                       (bInputNo + 1), bOwnerSG, 0, 0);
          }

          /* initialize counters */
          if (SignalHasGreen(SRuntimes.SaSGRuntimes[bOwnerSG
                                                    - 1].bCurrentSignal))
          {
            SRuntimes.SaInputRuntimes[bInputNo].bDemandCntInRed = 0;
            SRuntimes.SaInputRuntimes[bInputNo].sFDemandDurInRed = 0; /* signal is not red in group, initialize counter */
          }
          else
          {
            /* signal is not green so do not count gap duration anymore, */
            /* initialize it gap duration in green is meaningful when sg has */
            /* green signal but prevent increase continuously in case of no */
            /* vehicle existence */
            SRuntimes.SaInputRuntimes[bInputNo].bDemandCntInGreen = 0;
            SRuntimes.SaInputRuntimes[bInputNo].sGapDurInGreen = 0;
          }

          /* increase demand durations, this doesn't depend on the current state */
          /* of the device in period */
          if (SRuntimes.SaInputRuntimes[bInputNo].bDemandCntInPer != 0)
          {
            SRuntimes.SaInputRuntimes[bInputNo].sFDemandDurInPer++; /* increase in period, initialize this */
          }

          /* value at the beginning of period */

          /* in red */
          if ((SignalHasGreen(SRuntimes.SaSGRuntimes[bOwnerSG
                                                     - 1].bCurrentSignal)
               == FALSE)
              && (SRuntimes.SaInputRuntimes[bInputNo].bDemandCntInRed != 0))
          {
            SRuntimes.SaInputRuntimes[bInputNo].sFDemandDurInRed++; /* increase when group signal is red */
            if (SRuntimes.SaSGIORuntime[bOwnerSG - 1].sFDemandDurInRedMax
                < SRuntimes.SaInputRuntimes[bInputNo]
                .sFDemandDurInRed)
            {
              SRuntimes.SaSGIORuntime[bOwnerSG
                                      - 1].sFDemandDurInRedMax =
                SRuntimes.SaInputRuntimes[bInputNo]
                .
                sFDemandDurInRed;
            }
          }

          /* empty state */
          if (SCanDigitalIOInputs.sInputStates & laValue2Bit[bIndex])
          {
            /* no demand */
            SRuntimes.SaInputRuntimes[bInputNo].sOccDurInRed = 0;
            SRuntimes.SaInputRuntimes[bInputNo].sOccDurInGreen = 0;
            /* increase gap duration */
            /* in period */
            SRuntimes.SaInputRuntimes[bInputNo].sGapDurInPer++; /* use this for central mode, send this value */
            /* to center in period start */
            /* in green */
            if (SignalHasGreen(SRuntimes.SaSGRuntimes[bOwnerSG
                                                      - 1].bCurrentSignal))
            {
              SRuntimes.SaInputRuntimes[bInputNo].sGapDurInGreen++;
            }
          }
          else
          {
            uint8_t bSignalHasGreen =
              SignalHasGreen(SRuntimes.SaSGRuntimes[bOwnerSG
                                                    - 1].bCurrentSignal);

            /* there is demand */
            /* increase occupation duration */
            /* in period */
            SRuntimes.SaInputRuntimes[bInputNo].sOccDurInPer++; /* initialize this value at the beginning of */
            /* period */
            /* in green */
            if (bSignalHasGreen)
            {
              SRuntimes.SaInputRuntimes[bInputNo].sOccDurInGreen++;
              bLastInputDemandIssued = (bInputNo + 1);
              if (SRuntimes.SaSGIORuntime[bOwnerSG - 1].sOccDurInGreenMax
                  < SRuntimes.SaInputRuntimes[bInputNo]
                  .sOccDurInGreen)
              {
                SRuntimes.SaSGIORuntime[bOwnerSG
                                        - 1].sOccDurInGreenMax =
                  SRuntimes.SaInputRuntimes[bInputNo]
                  .
                  sOccDurInGreen;
              }
            }
            else /* in red */
            {
              SRuntimes.SaInputRuntimes[bInputNo].sOccDurInRed++;
              bLastInputDemandIssued = (bInputNo + 1);
              if (SRuntimes.SaSGIORuntime[bOwnerSG - 1].sOccDurInRedMax
                  < SRuntimes.SaInputRuntimes[bInputNo]
                  .sOccDurInRed)
              {
                SRuntimes.SaSGIORuntime[bOwnerSG - 1].sOccDurInRedMax =
                  SRuntimes.SaInputRuntimes[bInputNo].sOccDurInRed;
              }
            }

            /* examine previous state */
            if ((SaPrevCanDigitalIOInputs[bModuleNo].sInputStates
                 & laValue2Bit[bIndex]) || (bSignalHasGreen
                                            &&
                                            ((SRuntimes.SaInputRuntimes[bInputNo]
                                              .bDemandCntInGreen == 0)
                                             || (SRuntimes.SaSGIORuntime[
                                                   bOwnerSG - 1]
                                                 .
                                                 bDemandCntInGreen
                                                 ==
                                                 0)))
                || (!bSignalHasGreen
                    && ((SRuntimes.SaInputRuntimes[bInputNo].bDemandCntInRed
                         == 0)
                        || (SRuntimes.SaSGIORuntime[bOwnerSG
                                                    - 1].bDemandCntInRed
                            == 0))))
            {
              /* previous state is 'no demand', current state is 'there is */
              /* demand' increase number of demands in period */
              SRuntimes.SaInputRuntimes[bInputNo].bDemandCntInPer++;
              /* in green */
              if (bSignalHasGreen)
              {
                SRuntimes.SaInputRuntimes[bInputNo].bDemandCntInGreen++;
                SRuntimes.SaSGIORuntime[bOwnerSG - 1].bDemandCntInGreen++; /* increment owner sg demand count */
              }
              else /* in red */
              {
                SRuntimes.SaInputRuntimes[bInputNo].bDemandCntInRed++;
                SRuntimes.SaSGIORuntime[bOwnerSG - 1].bDemandCntInRed++; /* increment owner sg demand count */
              }
            }

            /* zero gap duration */
            SRuntimes.SaInputRuntimes[bInputNo].sGapDurInPer = 0;
            SRuntimes.SaInputRuntimes[bInputNo].sGapDurInGreen = 0;
          }
        }
        else
        {
          uint8_t bSignalHasGreen = FALSE;

          /* input is broken */
          if (SRuntimes.SaInputRuntimes[bInputNo].fBroken)
          {
            SRuntimes.SaInputRuntimes[bInputNo].sBrokenDur++;
          }
          else
          {
            /* input is just broken, log this */
            memset(&(SRuntimes.SaInputRuntimes[bInputNo]), 0,
                   sizeof(SRuntimes.SaInputRuntimes[bInputNo]));
            SRuntimes.SaInputRuntimes[bInputNo].fBroken = TRUE;
            LogRequest(LOG_REQ_APPEND_ASYNCH, NULL, EVENT_DIGITAL_INPUT_BROKEN,
                       (bInputNo + 1), bOwnerSG, 0, 0);
          }

          /*** Detector is broken, treat it as busy if flag is set ***/
          if (BrokenInputSettingsDigitalFlagGet())
          {
            /* initialize counters */
            if (SignalHasGreen(SRuntimes.SaSGRuntimes[bOwnerSG
                                                      - 1].bCurrentSignal))
            {
              SRuntimes.SaInputRuntimes[bInputNo].bDemandCntInRed = 0;
              SRuntimes.SaInputRuntimes[bInputNo].sFDemandDurInRed = 0; /* signal is not red in group, initialize counter */
            }
            else
            {
              /* signal is not green so do not count gap duration anymore, */
              /* initialize it gap duration in green is meaningful when sg has */
              /* green signal but prevent increase continuously in case of no */
              /* vehicle existence */
              SRuntimes.SaInputRuntimes[bInputNo].bDemandCntInGreen = 0;
              SRuntimes.SaInputRuntimes[bInputNo].sGapDurInGreen = 0;
            }

            /* increase demand durations, this doesn't depend on the current */
            /* state of the device in period */
            if (SRuntimes.SaInputRuntimes[bInputNo].bDemandCntInPer != 0)
            {
              SRuntimes.SaInputRuntimes[bInputNo].sFDemandDurInPer++; /* increase in period, initialize this */
            }

            /* value at the beginning of period */

            /* in red */
            if ((SignalHasGreen(SRuntimes.SaSGRuntimes[bOwnerSG
                                                       - 1].bCurrentSignal)
                 == FALSE)
                && (SRuntimes.SaInputRuntimes[bInputNo].bDemandCntInRed != 0))
            {
              SRuntimes.SaInputRuntimes[bInputNo].sFDemandDurInRed++; /* increase when group signal is red */
              if (SRuntimes.SaSGIORuntime[bOwnerSG - 1].sFDemandDurInRedMax
                  < SRuntimes.SaInputRuntimes[bInputNo]
                  .sFDemandDurInRed)
              {
                SRuntimes.SaSGIORuntime[bOwnerSG
                                        - 1].sFDemandDurInRedMax =
                  SRuntimes.SaInputRuntimes[bInputNo]
                  .
                  sFDemandDurInRed;
              }
            }

            bSignalHasGreen = SignalHasGreen(SRuntimes.SaSGRuntimes[bOwnerSG
                                                                    - 1].
                                             bCurrentSignal);
            /* there is demand */
            /* increase occupation duration */
            /* in period */
            SRuntimes.SaInputRuntimes[bInputNo].sOccDurInPer++; /* initialize this value at the beginning of */
            /* period */
            /* in green */
            if (bSignalHasGreen)
            {
              SRuntimes.SaInputRuntimes[bInputNo].sOccDurInGreen++;
              bLastInputDemandIssued = (bInputNo + 1);
              if (SRuntimes.SaSGIORuntime[bOwnerSG - 1].sOccDurInGreenMax
                  < SRuntimes.SaInputRuntimes[bInputNo]
                  .sOccDurInGreen)
              {
                SRuntimes.SaSGIORuntime[bOwnerSG
                                        - 1].sOccDurInGreenMax =
                  SRuntimes.SaInputRuntimes[bInputNo]
                  .
                  sOccDurInGreen;
              }
            }
            else /* in red */
            {
              SRuntimes.SaInputRuntimes[bInputNo].sOccDurInRed++;
              bLastInputDemandIssued = (bInputNo + 1);
              if (SRuntimes.SaSGIORuntime[bOwnerSG - 1].sOccDurInRedMax
                  < SRuntimes.SaInputRuntimes[bInputNo]
                  .sOccDurInRed)
              {
                SRuntimes.SaSGIORuntime[bOwnerSG - 1].sOccDurInRedMax =
                  SRuntimes.SaInputRuntimes[bInputNo].sOccDurInRed;
              }
            }

            /* examine previous state */
            if ((SaPrevCanDigitalIOInputs[bModuleNo].sInputStates
                 & laValue2Bit[bIndex]) || (bSignalHasGreen
                                            &&
                                            ((SRuntimes.SaInputRuntimes[bInputNo]
                                              .bDemandCntInGreen == 0)
                                             || (SRuntimes.SaSGIORuntime[
                                                   bOwnerSG - 1]
                                                 .
                                                 bDemandCntInGreen
                                                 ==
                                                 0)))
                || (!bSignalHasGreen
                    && ((SRuntimes.SaInputRuntimes[bInputNo].bDemandCntInRed
                         == 0)
                        || (SRuntimes.SaSGIORuntime[bOwnerSG
                                                    - 1].bDemandCntInRed
                            == 0))))
            {
              /* previous state is 'no demand', current state is 'there is */
              /* demand' increase number of demands in period */
              SRuntimes.SaInputRuntimes[bInputNo].bDemandCntInPer++;
              /* in green */
              if (bSignalHasGreen)
              {
                SRuntimes.SaInputRuntimes[bInputNo].bDemandCntInGreen++;
                SRuntimes.SaSGIORuntime[bOwnerSG - 1].bDemandCntInGreen++; /* increment owner sg demand count */
              }
              else /* in red */
              {
                SRuntimes.SaInputRuntimes[bInputNo].bDemandCntInRed++;
                SRuntimes.SaSGIORuntime[bOwnerSG - 1].bDemandCntInRed++; /* increment owner sg demand count */
              }
            }

            /* zero gap duration */
            SRuntimes.SaInputRuntimes[bInputNo].sGapDurInPer = 0;
            SRuntimes.SaInputRuntimes[bInputNo].sGapDurInGreen = 0;
          }
        }
      }
    }
  }

  /* signal group gap and occupation related maximum and minimum durations are */
  /* considered below */
  for (bIndex = 0; bIndex < SGTotalGet(); bIndex++)
  {
    /* initialize parameters for '<','>' comparisions */
    SRuntimes.SaSGIORuntime[bIndex].sGapDurInGreenMin = 0xFFFF;
    SRuntimes.SaSGIORuntime[bIndex].sOccDurInGreenMax = 0;
    SRuntimes.SaSGIORuntime[bIndex].sOccDurInRedMax = 0;

    /* scan inputs */
    for (bIndex2 = 0; bIndex2 < IO_INPUTS_DIGITAL_MAX; bIndex2++)
    {
      if (SCP.SaInputDefs[bIndex2].bOwnerSG == (bIndex + 1))
      {
        if (SignalHasGreen(SRuntimes.SaSGRuntimes[bIndex].bCurrentSignal))
        {
          /* signal has a green component */
          if ((SRuntimes.SaInputRuntimes[bIndex2].sGapDurInGreen)
              && (SRuntimes.SaSGIORuntime[bIndex].sGapDurInGreenMin
                  >
                  SRuntimes.SaInputRuntimes[bIndex2].sGapDurInGreen))
          {
            SRuntimes.SaSGIORuntime[bIndex].sGapDurInGreenMin =
              SRuntimes.SaInputRuntimes[bIndex2].sGapDurInGreen;
          }

          if (SRuntimes.SaSGIORuntime[bIndex].sOccDurInGreenMax
              < SRuntimes.SaInputRuntimes[bIndex2].sOccDurInGreen)
          {
            SRuntimes.SaSGIORuntime[bIndex].sOccDurInGreenMax =
              SRuntimes.SaInputRuntimes[bIndex2].sOccDurInGreen;
          }
        }
        else
        {
          /* signal has not a green component */
          if (SRuntimes.SaSGIORuntime[bIndex].sOccDurInRedMax
              < SRuntimes.SaInputRuntimes[bIndex2].sOccDurInRed)
          {
            SRuntimes.SaSGIORuntime[bIndex].sOccDurInRedMax =
              SRuntimes.SaInputRuntimes[bIndex2].sOccDurInRed;
          }
        }
      }
    }
  }

  /* update previous states */
  SaPrevCanDigitalIOInputs[bModuleNo].sInputStates =
    SCanDigitalIOInputs.sInputStates;
  SaPrevCanDigitalIOInputs[bModuleNo].sInputSafeStates =
    SCanDigitalIOInputs.sInputSafeStates;
} /* UseDigitalIOValues */

void UseLDIOValues(uint8_t bModuleNo, tpSCanDetectorIOInputs pSCanIOInputs)
{
  uint8_t bIndex = 0, bIndex2 = 0;
  tSCanDetectorIOInputs SCanDetectorIOInputs;

  memcpy(&SCanDetectorIOInputs, pSCanIOInputs, sizeof(tSCanDetectorIOInputs));

  for (bIndex = 0; bIndex < SGTotalGet(); bIndex++)
  {
    if (SignalHasGreen(SRuntimes.SaSGRuntimes[bIndex].bCurrentSignal) == FALSE)
    {
      SRuntimes.SaSGIORuntime[bIndex].bDemandCntInGreen = 0;
      SRuntimes.SaSGIORuntime[bIndex].sOccDurInGreenMax = 0;
    }
    else
    {
      SRuntimes.SaSGIORuntime[bIndex].bDemandCntInRed = 0;
      SRuntimes.SaSGIORuntime[bIndex].sFDemandDurInRedMax = 0; /* signal is not red in group, initialize counter */
    }
  }

  /* detectors */
  for (bIndex = 0; bIndex < IO_INPUTS_DETECTOR_MAX; bIndex++)
  {
    /* there are two io modules, one per cage */
    uint8_t bDedectorNo = ((bModuleNo * IO_INPUTS_DETECTOR_MAX) + bIndex); /* 0..16 for module 1, 17..31 for module 2 */
    uint8_t bOwnerSG = SCP.SaDetectorDefs[bDedectorNo].bOwnerSG;

    if (bOwnerSG)
    {
      /* detector belongs to a signal group, evaluate it */
      if (SCanDetectorIOInputs.sLoopSafeStates & laValue2Bit[bIndex])
      {
        /* detector is safe so it is also safe to operate on this detector */
        if (SRuntimes.SaDetectorRuntimes[bDedectorNo].fBroken)
        {
          /* previous state of detector is broken but it is safe now */
          SRuntimes.SaDetectorRuntimes[bDedectorNo].sBrokenDur = 0;
          SRuntimes.SaDetectorRuntimes[bDedectorNo].fBroken = FALSE;
          LogRequest(LOG_REQ_APPEND_ASYNCH, NULL, EVENT_DETECTOR_SAFE,
                     (bDedectorNo + 1), bOwnerSG, 0, 0);
        }

        /* initialize counters */
        if (SignalHasGreen(SRuntimes.SaSGRuntimes[bOwnerSG - 1].bCurrentSignal))
        {
          /* signal has a green component */
          SRuntimes.SaDetectorRuntimes[bDedectorNo].bDemandCntInRed = 0;
          SRuntimes.SaDetectorRuntimes[bDedectorNo].sFDemandDurInRed = 0;
        }
        else
        {
          /* signal has not a green component */
          SRuntimes.SaDetectorRuntimes[bDedectorNo].bDemandCntInGreen = 0;
          SRuntimes.SaDetectorRuntimes[bDedectorNo].sGapDurInGreen = 0;
        }

        /* increase durations */
        if (SRuntimes.SaDetectorRuntimes[bDedectorNo].bDemandCntInPer != 0)
        {
          SRuntimes.SaDetectorRuntimes[bDedectorNo].sFDemandDurInPer++;
        }

        if ((SignalHasGreen(SRuntimes.SaSGRuntimes[bOwnerSG - 1].bCurrentSignal)
             == FALSE)
            && (SRuntimes.SaDetectorRuntimes[bDedectorNo].bDemandCntInRed != 0))
        {
          /* signal has not a green component */
          SRuntimes.SaDetectorRuntimes[bDedectorNo].sFDemandDurInRed++;
          if (SRuntimes.SaSGIORuntime[bOwnerSG - 1].sFDemandDurInRedMax
              < SRuntimes.SaDetectorRuntimes[bDedectorNo]
              .sFDemandDurInRed)
          {
            SRuntimes.SaSGIORuntime[bOwnerSG
                                    - 1].sFDemandDurInRedMax =
              SRuntimes.SaDetectorRuntimes[bDedectorNo]
              .
              sFDemandDurInRed;
          }
        }

        /* empty/busy state examination */
        if (SCanDetectorIOInputs.sLoopEmptyStates & laValue2Bit[bIndex])
        {
          /* dedector state is empty */
          /* zero to occupation durations */
          SRuntimes.SaDetectorRuntimes[bDedectorNo].sOccDurInRed = 0;
          SRuntimes.SaDetectorRuntimes[bDedectorNo].sOccDurInGreen = 0;

          /* increase gap durations */
          SRuntimes.SaDetectorRuntimes[bDedectorNo].sGapDurInPer++;
          if (SignalHasGreen(SRuntimes.SaSGRuntimes[bOwnerSG
                                                    - 1].bCurrentSignal))
          {
            SRuntimes.SaDetectorRuntimes[bDedectorNo].sGapDurInGreen++; /* signal has a green component */
          }
        }
        else
        {
          /* dedector state is busy */
          uint8_t bSignalHasGreen =
            SignalHasGreen(SRuntimes.SaSGRuntimes[bOwnerSG - 1].bCurrentSignal);

          bLastDetectorDemandIssued = (bDedectorNo + 1);

          /* zero to gap durations */
          SRuntimes.SaDetectorRuntimes[bDedectorNo].sGapDurInPer = 0;
          SRuntimes.SaDetectorRuntimes[bDedectorNo].sGapDurInGreen = 0;

          /* increase occupation durations */
          SRuntimes.SaDetectorRuntimes[bDedectorNo].sOccDurInPer++;
          if (bSignalHasGreen)
          {
            /* signal has a green component */
            SRuntimes.SaDetectorRuntimes[bDedectorNo].sOccDurInGreen++;
          }
          else
          {
            /* signal has not a green component */
            SRuntimes.SaDetectorRuntimes[bDedectorNo].sOccDurInRed++;
          }

          /* examine previous state */
          if ((SaPrevCanDetectorIOInputs[bModuleNo].sLoopEmptyStates
               & laValue2Bit[bIndex]) || (bSignalHasGreen
                                          &&
                                          ((SRuntimes.SaDetectorRuntimes[
                                              bDedectorNo].bDemandCntInGreen
                                            == 0)
                                           || (SRuntimes.SaSGIORuntime[bOwnerSG
                                                                       -
                                                                       1].
                                               bDemandCntInGreen
                                               ==
                                               0)))
              || (!bSignalHasGreen
                  && ((SRuntimes.SaDetectorRuntimes[bDedectorNo].bDemandCntInRed
                       == 0)
                      || (SRuntimes.SaSGIORuntime[bOwnerSG
                                                  - 1].bDemandCntInRed == 0))))
          {
            /* previous state is empty, current state is busy, there is flowing */
            /* traffic increase number of demands */
            SRuntimes.SaDetectorRuntimes[bDedectorNo].bDemandCntInPer++;
            if (bSignalHasGreen)
            {
              /* signal has a green component */
              SRuntimes.SaDetectorRuntimes[bDedectorNo].bDemandCntInGreen++;
              SRuntimes.SaSGIORuntime[bOwnerSG - 1].bDemandCntInGreen++;
            }
            else
            {
              /* signal has not a green component */
              SRuntimes.SaDetectorRuntimes[bDedectorNo].bDemandCntInRed++;
              SRuntimes.SaSGIORuntime[bOwnerSG - 1].bDemandCntInRed++;
            }
          }
        }
      }
      else
      {
        uint8_t bSignalHasGreen = FALSE;

        /* detector is broken */
        if (SRuntimes.SaDetectorRuntimes[bDedectorNo].fBroken)
        {
          SRuntimes.SaDetectorRuntimes[bDedectorNo].sBrokenDur++;
        }
        else
        {
          /* detector is just broken, log this */
          memset(&(SRuntimes.SaDetectorRuntimes[bDedectorNo]), 0,
                 sizeof(SRuntimes.SaDetectorRuntimes[bDedectorNo]));
          SRuntimes.SaDetectorRuntimes[bDedectorNo].fBroken = TRUE;
          LogRequest(LOG_REQ_APPEND_ASYNCH, NULL, EVENT_DETECTOR_BROKEN,
                     (bDedectorNo + 1), bOwnerSG, 0, 0);
        }

        /*** Detector is broken, treat it as busy ***/
        if (BrokenInputSettingsLoopFlagGet())
        {
          /* initialize counters */
          if (SignalHasGreen(SRuntimes.SaSGRuntimes[bOwnerSG
                                                    - 1].bCurrentSignal))
          {
            /* signal has a green component */
            SRuntimes.SaDetectorRuntimes[bDedectorNo].bDemandCntInRed = 0;
            SRuntimes.SaDetectorRuntimes[bDedectorNo].sFDemandDurInRed = 0;
          }
          else
          {
            /* signal has not a green component */
            SRuntimes.SaDetectorRuntimes[bDedectorNo].bDemandCntInGreen = 0;
            SRuntimes.SaDetectorRuntimes[bDedectorNo].sGapDurInGreen = 0;
          }

          /* increase durations */
          if (SRuntimes.SaDetectorRuntimes[bDedectorNo].bDemandCntInPer != 0)
          {
            SRuntimes.SaDetectorRuntimes[bDedectorNo].sFDemandDurInPer++;
          }

          if ((SignalHasGreen(SRuntimes.SaSGRuntimes[bOwnerSG
                                                     - 1].bCurrentSignal)
               == FALSE)
              && (SRuntimes.SaDetectorRuntimes[bDedectorNo].bDemandCntInRed
                  != 0))
          {
            /* signal has not a green component */
            SRuntimes.SaDetectorRuntimes[bDedectorNo].sFDemandDurInRed++;
            if (SRuntimes.SaSGIORuntime[bOwnerSG - 1].sFDemandDurInRedMax
                < SRuntimes.SaDetectorRuntimes[bDedectorNo]
                .sFDemandDurInRed)
            {
              SRuntimes.SaSGIORuntime[bOwnerSG
                                      - 1].sFDemandDurInRedMax =
                SRuntimes.SaDetectorRuntimes[bDedectorNo]
                .
                sFDemandDurInRed;
            }
          }

          /* dedector state is busy */
          bSignalHasGreen = SignalHasGreen(SRuntimes.SaSGRuntimes[bOwnerSG
                                                                  - 1].
                                           bCurrentSignal);
          bLastDetectorDemandIssued = (bDedectorNo + 1);

          /* zero to gap durations */
          SRuntimes.SaDetectorRuntimes[bDedectorNo].sGapDurInPer = 0;
          SRuntimes.SaDetectorRuntimes[bDedectorNo].sGapDurInGreen = 0;

          /* increase occupation durations */
          SRuntimes.SaDetectorRuntimes[bDedectorNo].sOccDurInPer++;
          if (bSignalHasGreen)
          {
            /* signal has a green component */
            SRuntimes.SaDetectorRuntimes[bDedectorNo].sOccDurInGreen++;
          }
          else
          {
            /* signal has not a green component */
            SRuntimes.SaDetectorRuntimes[bDedectorNo].sOccDurInRed++;
          }

          /* examine previous state */
          if ((SaPrevCanDetectorIOInputs[bModuleNo].sLoopSafeStates
               & laValue2Bit[bIndex]) || (bSignalHasGreen
                                          &&
                                          ((SRuntimes.SaDetectorRuntimes[
                                              bDedectorNo].bDemandCntInGreen
                                            == 0)
                                           || (SRuntimes.SaSGIORuntime[bOwnerSG
                                                                       -
                                                                       1].
                                               bDemandCntInGreen
                                               ==
                                               0)))
              || (!bSignalHasGreen
                  && ((SRuntimes.SaDetectorRuntimes[bDedectorNo].bDemandCntInRed
                       == 0)
                      || (SRuntimes.SaSGIORuntime[bOwnerSG
                                                  - 1].bDemandCntInRed == 0))))
          {
            /* previous state is safe, current state is broken, there is flowing */
            /* traffic increase number of demands */
            SRuntimes.SaDetectorRuntimes[bDedectorNo].bDemandCntInPer++;
            if (bSignalHasGreen)
            {
              /* signal has a green component */
              SRuntimes.SaDetectorRuntimes[bDedectorNo].bDemandCntInGreen++;
              SRuntimes.SaSGIORuntime[bOwnerSG - 1].bDemandCntInGreen++;
            }
            else
            {
              /* signal has not a green component */
              SRuntimes.SaDetectorRuntimes[bDedectorNo].bDemandCntInRed++;
              SRuntimes.SaSGIORuntime[bOwnerSG - 1].bDemandCntInRed++;
            }
          }
        }
      }
    }
  }

  /* signal group gap and occupation related maximum and minimum durations are */
  /* considered below */
  for (bIndex = 0; bIndex < SGTotalGet(); bIndex++)
  {
    /* initialize parameters for '<','>' comparisions */
    SRuntimes.SaSGIORuntime[bIndex].sGapDurInGreenMin = 0xFFFF;
    SRuntimes.SaSGIORuntime[bIndex].sOccDurInGreenMax = 0;
    SRuntimes.SaSGIORuntime[bIndex].sOccDurInRedMax = 0;

    /* scan detectors */
    for (bIndex2 = 0; bIndex2 < IO_INPUTS_DETECTOR_MAX; bIndex2++)
    {
      if (SCP.SaDetectorDefs[bIndex2].bOwnerSG == (bIndex + 1))
      {
        if (SignalHasGreen(SRuntimes.SaSGRuntimes[bIndex].bCurrentSignal))
        {
          /* signal has a green component */
          if ((SRuntimes.SaDetectorRuntimes[bIndex2].sGapDurInGreen)
              && (SRuntimes.SaSGIORuntime[bIndex]
                  .
                  sGapDurInGreenMin
                  >
                  SRuntimes.SaDetectorRuntimes[bIndex2]
                  .
                  sGapDurInGreen))
          {
            SRuntimes.SaSGIORuntime[bIndex].sGapDurInGreenMin =
              SRuntimes.SaDetectorRuntimes[bIndex2].sGapDurInGreen;
          }

          if (SRuntimes.SaSGIORuntime[bIndex].sOccDurInGreenMax
              < SRuntimes.SaDetectorRuntimes[bIndex2].sOccDurInGreen)
          {
            SRuntimes.SaSGIORuntime[bIndex].sOccDurInGreenMax =
              SRuntimes.SaDetectorRuntimes[bIndex2].sOccDurInGreen;
          }
        }
        else
        {
          /* signal has not a green component */
          if (SRuntimes.SaSGIORuntime[bIndex].sOccDurInRedMax
              < SRuntimes.SaDetectorRuntimes[bIndex2].sOccDurInRed)
          {
            SRuntimes.SaSGIORuntime[bIndex].sOccDurInRedMax =
              SRuntimes.SaDetectorRuntimes[bIndex2].sOccDurInRed;
          }
        }
      }
    }
  }

  /* update previous states */
  SaPrevCanDetectorIOInputs[bModuleNo].sLoopEmptyStates =
    SCanDetectorIOInputs.sLoopEmptyStates;
  SaPrevCanDetectorIOInputs[bModuleNo].sLoopSafeStates =
    SCanDetectorIOInputs.sLoopSafeStates;
} /* UseLDIOValues */

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Outputs */
uint8_t OutputSet(uint8_t bOutputNo, tpSOutputDef pSOutputDef)
{
  if (bOutputNo < OUTPUTS_MAX)
  {
    memcpy(&SCP.SaOutputDefs[bOutputNo], pSOutputDef, sizeof(tSOutputDef));
    SCP.SConsumed.bOutputTotal++;

    return TRUE;
  }

  return FALSE;
}

uint8_t OutputGet(uint8_t bOutputNo, tpSOutputDef pSOutputDef)
{
  if (bOutputNo < OUTPUTS_MAX)
  {
    memcpy(pSOutputDef, &SCP.SaOutputDefs[bOutputNo], sizeof(tSOutputDef));

    return TRUE;
  }

  return FALSE;
}

uint16_t OutputActiveLevelGet(uint8_t bOutputNo)
{
  return SCP.SaOutputDefs[bOutputNo].bActiveLevel;
}

uint16_t OutputActiveLevelDurGet(uint8_t bOutputNo)
{
  return SCP.SaOutputDefs[bOutputNo].bActiveLevelDur;
}

uint16_t OutputInActiveLevelDurGet(uint8_t bOutputNo)
{
  return SCP.SaOutputDefs[bOutputNo].bInActiveLevelDur;
}

uint8_t OutputTotalGet(void)
{
  return SCP.SConsumed.bOutputTotal;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Outputs Runtime */
void GetIOOutputs(tpSCanCpuIOOutputs pSCanCpuIOOutputs)
{
  memcpy(pSCanCpuIOOutputs, &SCanCpuIOOutputs, sizeof(tSCanCpuIOOutputs));
}

void SetIOOutputs(tpSCanCpuIOOutputs pSCanCpuIOOutputs)
{
  memcpy(&SCanCpuIOOutputs, pSCanCpuIOOutputs, sizeof(tSCanCpuIOOutputs));
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Work Plan */
void WorkPlanCurNoSet(uint8_t bWPNo)
{
  SCPRuntime.bCurWorkPlan = bWPNo;

  CoordSplitTimeSet();
}

uint8_t WorkPlanCurNoGet(void)
{
  return SCPRuntime.bCurWorkPlan;
}

uint8_t WorkPlanEntryCurNoGet(void)
{
  uint8_t bEntry = 0, bIndex = 0;
  uint32_t lEntrySeconds;

  if (SignalPlanCurrentGet())
  {
    /* signal plans has precedence over work schedules */
    /* so if there is an active signal plan, get phase duration from it */
    bEntry = SCP.SaSignalPlans[SignalPlanCurrentGet() - 1].bWorkPlanEntry;
  }
  else
  {
    for (bIndex = 0;
         bIndex < SCP.SConsumed.baWPEntriesTotal[WorkPlanCurNoGet()];
         bIndex++)
    {
      lEntrySeconds =
        (TimeMinuteOfDayCalc(SCP.SaWorkPlan[WorkPlanCurNoGet()][bIndex].bHours,
                             SCP.SaWorkPlan[WorkPlanCurNoGet()][
                               bIndex].bMinutes)
         *
         MAX_SECONDS_IN_A_MINUTE);
      if (lEntrySeconds <= TimeSecondOfDayGet())
      {
        bEntry = bIndex + 1;
      }
    }
  }

  return bEntry;
}

uint8_t WorkPlanEntrySet(uint8_t bWorkPlan,
                         uint8_t bEntry,
                         tpSWorkPlanEntryDef pSWorkPlanEntryBuffer)
{
  if ((bWorkPlan < WORK_PLANS_MAX) && (bEntry < WORK_PLAN_ENTRIES_MAX))
  {
    memcpy(&SCP.SaWorkPlan[bWorkPlan][bEntry], pSWorkPlanEntryBuffer,
           sizeof(tSWorkPlanEntryDef));
    SCP.SChecksum.baWPDefs[bWorkPlan] = ByteChecksum(SCP.SaWorkPlan[bWorkPlan],
                                                     (sizeof(tSWorkPlanEntryDef)
                                                      * (bEntry + 1)));                            /* update checksum */
    if (SCP.SConsumed.baWPEntriesTotal[bWorkPlan] == 0)
    {
      SCP.SConsumed.bWPTotal++; /* this is the first entry for the work plan, */
    }

    /* means that a new work plan definition is */
    /* being done */

    if ((bEntry + 1) > SCP.SConsumed.baWPEntriesTotal[bWorkPlan])
    {
      SCP.SConsumed.baWPEntriesTotal[bWorkPlan]++; /* Increase total entry for */
    }

    /* related work plan if new */
    /* entry added. */

    return TRUE;
  }

  return FALSE;
}

uint8_t WorkPlanEntryGet(uint8_t bWorkPlan,
                         uint8_t bEntry,
                         tpSWorkPlanEntryDef pSWorkPlanEntryBuffer)
{
  if ((bWorkPlan < WORK_PLANS_MAX) && (bEntry < WORK_PLAN_ENTRIES_MAX))
  {
    memcpy(pSWorkPlanEntryBuffer, &SCP.SaWorkPlan[bWorkPlan][bEntry],
           sizeof(tSWorkPlanEntryDef));

    return TRUE;
  }

  return FALSE;
}

uint8_t WorkPlanEntryPhaseDurGet(uint8_t bWorkPlanEntry, uint8_t bPhaseNo)
{
  return SCP.SaWorkPlan[WorkPlanCurNoGet()][bWorkPlanEntry].baPhaseDur[bPhaseNo];
}

uint8_t WorkPlanEntryPhaseDurSet(uint8_t bWorkPlanEntry,
                                 uint8_t bPhaseNo,
                                 uint8_t bDur)
{
  if (bWorkPlanEntry < WORK_PLAN_ENTRIES_MAX)
  {
    if (bPhaseNo < PHASES_MAX)
    {
      if ((bDur >= PhaseMinDurationGet(bPhaseNo))
          && (bDur <= PhaseMaxDurationGet(bPhaseNo)))
      {
        SCP.SaWorkPlan[WorkPlanCurNoGet()][bWorkPlanEntry].baPhaseDur[bPhaseNo]
          =
            bDur;

        return TRUE;
      }
    }
  }

  return FALSE;
}

uint8_t WorkPlanPhaseDurGet(uint8_t bPhaseNo)
{
  if (bPhaseNo < PHASES_MAX)
  {
    return SCP.SaWorkPlan[WorkPlanCurNoGet()][WorkPlanEntryCurNoGet()
                                              - 1].baPhaseDur[bPhaseNo]
           + PhaseExtDurGet(bPhaseNo);
  }

  return 0;
}

uint16_t WorkPlanTotalPhaseDurGet(void)
{
  uint8_t bPhaseNo = 0;
  uint16_t sPhaseDur = 0;

  for (bPhaseNo = 0; bPhaseNo < PhaseTotalGet(); bPhaseNo++)
  {
    sPhaseDur += SCP.SaWorkPlan[WorkPlanCurNoGet()][WorkPlanEntryCurNoGet()
                                                    - 1].baPhaseDur[bPhaseNo]
                 + PhaseExtDurGet(bPhaseNo);
  }

  return sPhaseDur;
}

uint8_t WorkPlanEntryTotalGet(uint8_t bWorkPlan)
{
  if (bWorkPlan <= WorkPlanTotalGet())
  {
    return SCP.SConsumed.baWPEntriesTotal[bWorkPlan];
  }

  return 0;
}

uint8_t WorkPlanTotalGet(void)
{
  return SCP.SConsumed.bWPTotal;
}

uint8_t WorkPlanIsValid(uint8_t bWorkPlan)
{
  if (bWorkPlan && (bWorkPlan <= WorkPlanTotalGet()))
  {
    return TRUE;
  }

  return FALSE;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  SP Plan */
void SigProgPlanCurNoSet(uint8_t bSPPlanNo)
{
  SCPRuntime.bCurSPPlan = bSPPlanNo;
}

uint8_t SigProgPlanCurNoGet(void)
{
  return SCPRuntime.bCurSPPlan;
}

void SigProgPlanDefaultActivate(void)
{
  /* default signal program plan */
  SaSPPlanDefault[0].bHours = 0; /* always use this plan entry */
  SaSPPlanDefault[0].bMinutes = 0;
  SaSPPlanDefault[0].bSigProg = 1;
  SCP.SaSignalPrograms[0].SSigProg.bStaStart = 0; /* there is no statement that will be executed at the time of signal */
  /* program start */
  SCP.SaSignalPrograms[0].SSigProg.bStaEnd = 0;

  /* default settings */
  SCP.SaSignalPrograms[0].SaTransitions[0].bFrom =
    STATES_ANY; /* these states may be only user states: no control, closed, */
  /* and flash */
  SCP.SaSignalPrograms[0].SaTransitions[0].bTo =
    STATES_ANY; /* these states may be only user states: no control, closed, */
  /* and flash */
  SCP.SaSignalPrograms[0].SaTransitions[0].bValue1 = 0;
  SCP.SaSignalPrograms[0].SaTransitions[0].bValue2 = 0;
  SCP.SaSignalPrograms[0].SaTransitions[0].bRule = 1;
  SCP.SaSignalPrograms[0].SaTransitions[0].bPriority = 1;
  SCP.SaSignalPrograms[0].SaOperations[0].bOperator =
    OPR_EQUAL; /* operation is userRequestStateAny == 1 */
  SCP.SaSignalPrograms[0].SaOperations[0].SaOperands[0].bField = OP_FIELD_USER;
  SCP.SaSignalPrograms[0].SaOperations[0].SaOperands[0].bSubField =
    OP_SUBFIELD_STATE_ANY;
  SCP.SaSignalPrograms[0].SaOperations[0].SaOperands[0].bValueHigh = 0;
  SCP.SaSignalPrograms[0].SaOperations[0].SaOperands[0].bValueLow = 0;
  SCP.SaSignalPrograms[0].SaOperations[0].SaOperands[1].bField =
    OP_FIELD_CONSTANT;
  SCP.SaSignalPrograms[0].SaOperations[0].SaOperands[1].bSubField =
    OP_SUBFIELD_NO_MEANING;
  SCP.SaSignalPrograms[0].SaOperations[0].SaOperands[1].bValueHigh = 0;
  SCP.SaSignalPrograms[0].SaOperations[0].SaOperands[1].bValueLow = 1; /* means that TRUE */
  SCP.SaSignalPrograms[0].SaRules[0].sStart = 1;
  SCP.SaSignalPrograms[0].SaRules[0].bTOpsStart = 1;
  SCP.SaSignalPrograms[0].SaRules[0].bTOpsEnd = 2;
  SCP.SaSignalPrograms[0].SaRules[0].bFOpsStart = 0;
  SCP.SaSignalPrograms[0].SaRules[0].bFOpsEnd = 0;
  SCP.SaSignalPrograms[0].SaStatements[0].bCmd =
    COMMAND_USER_STATE_TO_CURRENT_STATE;
  SCP.SaSignalPrograms[0].SaStatements[0].bParam1 = 0;
  SCP.SaSignalPrograms[0].SaStatements[0].bParam2 = 0;
  SCP.SaSignalPrograms[0].SaStatements[0].bParam3 = 0;
  SCP.SaSignalPrograms[0].SaStatements[1].bCmd = COMMAND_USER_STATE_REQ_END;
  SCP.SaSignalPrograms[0].SaStatements[1].bParam1 = 0;
  SCP.SaSignalPrograms[0].SaStatements[1].bParam2 = 0;
  SCP.SaSignalPrograms[0].SaStatements[1].bParam3 = 0;
} /* SigProgPlanDefaultActivate */

uint8_t SigProgPlanEntrySet(uint8_t bSPPlanNo,
                            uint8_t bEntry,
                            tpSSPPlanEntry pSSPPlanEntry)
{
  if ((bSPPlanNo < SIGNAL_PROGRAM_PLANS_MAX)
      && (bEntry < SIGNAL_PROGRAM_ENTRIES_MAX))
  {
    /* add new entry to signal program plan */
    memcpy(&SCP.SaSPPlan[bSPPlanNo][bEntry],
           pSSPPlanEntry,
           sizeof(tSSPPlanEntry));
    /* update checksum */
    SCP.SChecksum.baSigProgPlans[bSPPlanNo] =
      ByteChecksum(SCP.SaSPPlan[bSPPlanNo],
                   (sizeof(tSSPPlanEntry)
                    * (bEntry + 1)));
    if (SCP.SConsumed.baSPPlanEntriesTotal[bSPPlanNo] == 0)
    {
      SCP.SConsumed.bSPPlanTotal++; /* this is the first entry for the SP plan, */
    }

    /* means that a new SP plan definition is */
    /* being done */

    if ((bEntry + 1) > SCP.SConsumed.baSPPlanEntriesTotal[bSPPlanNo])
    {
      SCP.SConsumed.baSPPlanEntriesTotal[bSPPlanNo]++; /* Increase total entry */
    }

    /* for related signal */
    /* plan if new entry */
    /* added. */

    return TRUE;
  }

  return FALSE;
}

uint8_t SigProgPlanEntryGet(uint8_t bSPPlanNo,
                            uint8_t bEntry,
                            tpSSPPlanEntry pSSPPlanEntry)
{
  if ((bSPPlanNo < SIGNAL_PROGRAM_PLANS_MAX)
      && (bEntry < SIGNAL_PROGRAM_ENTRIES_MAX))
  {
    memcpy(pSSPPlanEntry,
           &SCP.SaSPPlan[bSPPlanNo][bEntry],
           sizeof(tSSPPlanEntry));

    return TRUE;
  }

  return FALSE;
}

uint8_t SigProgPlanEntryTotalGet(uint8_t bSPPlanNo)
{
  if (bSPPlanNo <= SigProgPlanTotalGet())
  {
    return SCP.SConsumed.baSPPlanEntriesTotal[bSPPlanNo];
  }

  return 0;
}

uint8_t SigProgPlanGet(uint8_t bSPPlanNo)
{
  if (bSPPlanNo < SIGNAL_PROGRAM_PLANS_MAX)
  {
    uint8_t bEntry;

    for (bEntry = 0;
         bEntry < SCP.SConsumed.baSPPlanEntriesTotal[bSPPlanNo];
         bEntry++)
    {
      /* read next signal program plan entry for the signal program */
      if (SigProgPlanEntryGet(bSPPlanNo,
                              bEntry,
                              &SCP.SaSPPlan[bSPPlanNo][bEntry]) == FALSE)
      {
        return FALSE;
      }
    }
  }

  /* Memory checksum control */
  if (SCP.SChecksum.baSigProgPlans[bSPPlanNo] != ByteChecksum(
        SCP.SaSPPlan[bSPPlanNo],
        (sizeof(tSSPPlanEntry)
         * SCP.SConsumed.baSPPlanEntriesTotal[bSPPlanNo])))
  {
    LogRequest(LOG_REQ_APPEND_ASYNCH, NULL, EVENT_CHECKSUM_FLASH_ERROR, 0, 0,
               EVENT_PARAM_CHECKSUM_FLASH_ERROR_SPPlans,
               0);

    return FALSE;
  }

  return TRUE;
}

uint8_t SigProgPlanEntryCurNoGet(void)
{
  uint8_t bIndex = 0, bEntry = 0;
  uint32_t lEntrySeconds = 0;

  for (bIndex = 0;
       bIndex < SCP.SConsumed.baSPPlanEntriesTotal[SigProgPlanCurNoGet()];
       bIndex++)
  {
    lEntrySeconds =
      (TimeMinuteOfDayCalc(SCP.SaSPPlan[SigProgPlanCurNoGet()][bIndex].bHours,
                           SCP.SaSPPlan[SigProgPlanCurNoGet()][
                             bIndex].bMinutes)
       *
       MAX_SECONDS_IN_A_MINUTE);
    if (lEntrySeconds <= TimeSecondOfDayGet())
    {
      bEntry = (bIndex + 1);
    }
  }

  return bEntry;
}

uint8_t SigProgPlanTotalGet(void)
{
  return SCP.SConsumed.bSPPlanTotal;
}

uint8_t SigProgPlanIsValid(uint8_t bSPPlanNo)
{
  if (bSPPlanNo && (bSPPlanNo <= SigProgPlanTotalGet()))
  {
    return TRUE;
  }

  return FALSE;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Signal Plan */
uint8_t SignalPlanSet(uint8_t bPlanNo, tpSSignalPlan pSSignalPlan)
{
  if (bPlanNo && (bPlanNo <= SIGNAL_PLANS_MAX))
  {
    memcpy(&SCP.SaSignalPlans[bPlanNo - 1], pSSignalPlan, sizeof(tSSignalPlan));
    if (bPlanNo > SignalPlanTotalGet())
    {
      SCP.SConsumed.bSignalPlanTotal++;
    }

    return TRUE;
  }

  return FALSE;
}

uint8_t SignalPlanTotalGet(void)
{
  return SCP.SConsumed.bSignalPlanTotal;
}

uint8_t SignalPlanGet(uint8_t bPlanNo, tpSSignalPlan pSSignalPlan)
{
  if (bPlanNo && (bPlanNo <= SIGNAL_PLANS_MAX))
  {
    memcpy(pSSignalPlan, &SCP.SaSignalPlans[bPlanNo - 1], sizeof(tSSignalPlan));

    return TRUE;
  }

  return FALSE;
}

uint8_t SignalPlanCurrentSet(uint8_t bPlanNo)
{
  if (bPlanNo && (bPlanNo <= SignalPlanTotalGet()))
  {
    SCPRuntime.bCurSignalPlan = bPlanNo;
    SRuntimes.SaSignalStateRuntimes[SignalStateRuntimeCurNoGet()].bPlanNo =
      bPlanNo;

    return TRUE;
  }

  SCPRuntime.bCurSignalPlan = 0;
  SRuntimes.SaSignalStateRuntimes[SignalStateRuntimeCurNoGet()].bPlanNo = 0;

  return FALSE;
}

uint8_t SignalPlanIsValidGet(uint8_t bPlanNo)
{
  if (bPlanNo && (bPlanNo <= SignalPlanTotalGet()))
  {
    return TRUE;
  }

  return FALSE;
}

uint8_t SignalPlanCurrentGet(void)
{
  return SCPRuntime.bCurSignalPlan;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Work Schedule */
uint8_t WorkScheduleEntrySet(uint8_t bWorkScheduleEntry,
                             tpSWorkScheduleEntryDef pSWorkScheduleEntry)
{
  if (bWorkScheduleEntry < WORK_SCHEDULE_ENTRIES_MAX)
  {
    memcpy(&SCP.SaWorkSchedule[bWorkScheduleEntry], pSWorkScheduleEntry,
           sizeof(tSWorkScheduleEntryDef));
    if ((bWorkScheduleEntry + 1) > WorkScheduleTotalGet())
    {
      SCP.SConsumed.bWSEntriesTotal++;
    }

    return TRUE;
  }

  return FALSE;
}

uint8_t WorkScheduleEntryGet(uint8_t bWorkScheduleEntry,
                             tpSWorkScheduleEntryDef pSWorkScheduleEntry)
{
  if (bWorkScheduleEntry < WorkScheduleTotalGet())
  {
    memcpy(pSWorkScheduleEntry,
           &SCP.SaWorkSchedule[bWorkScheduleEntry],
           sizeof(tSWorkScheduleEntryDef));

    return TRUE;
  }

  return FALSE;
}

void WorkScheduleDefaultSettings(void)
{
  /* load default work plan */
  WorkPlanCurNoSet(WORK_PLAN_DEFAULT);

  SignalStateRuntimeCurNoSet(SIGNAL_STATE_DEFAULT);

  SCP.SConsumed.baWPEntriesTotal[WORK_PLAN_DEFAULT] = 1;

  memcpy(&SCP.SaWorkPlan[WorkPlanCurNoGet()],
         &SCP.SaWorkPlan[WORK_PLAN_DEFAULT],
         sizeof(tSaWorkPlan));

  /* load default signal program plan */
  SigProgPlanDefaultActivate();

  SigProgPlanCurNoSet(SIGNAL_PROGRAM_PLAN_DEFAULT);
  SigProgCurNoSet(SIGNAL_PROGRAM_DEFAULT);

  SCP.SConsumed.baSPPlanEntriesTotal[SIGNAL_PROGRAM_PLAN_DEFAULT] = 1;
  memcpy(&SCP.SaSPPlan[SigProgPlanCurNoGet()], &SaSPPlanDefault,
         sizeof(tSaSPPlan));
}

void WorkScheduleUpdate(void)
{
  uint8_t bWorkScheduleEntry = 0, bWorkPlan = 0, bSigProgPlan = 0,
          bWorkPlanEntry = 0, bSPPlanEntry = 0;

  /* if there is no schedule in Maestro, default work plan will run */
  if (WorkScheduleTotalGet())
  {
      /* get work plan number */
      for (bWorkScheduleEntry = 0;
           bWorkScheduleEntry < WorkScheduleTotalGet();
           bWorkScheduleEntry++)
      {
        /*
         ************************************************************
         *  Here, start and end year info are not considered in order to
         *  maintain Config Tool version compatibility.
         *  Start and end year are not sent through Config Tool for now.
         *  If start and end year info are to be sent through Config
         *  Tool then the below parts of code must be revised.
         ************************************************************
         */
        uint16_t sStartDayOfYear =
          TimeDayOfYearCalc(SCP.SaWorkSchedule[bWorkScheduleEntry].bStartMonth,
                            SCP.SaWorkSchedule[
                              bWorkScheduleEntry].bStartDay,
                            TimeFullYearGet());
        uint16_t sEndDayOfYear =
          TimeDayOfYearCalc(SCP.SaWorkSchedule[bWorkScheduleEntry].bEndMonth,
                            SCP.SaWorkSchedule[
                              bWorkScheduleEntry].bEndDay, TimeFullYearGet());

        if ((sStartDayOfYear <= TimeDayOfYearGet())
            && (TimeDayOfYearGet() <= sEndDayOfYear))
        {
          if (TimeWeekdayGet())
          {
            if (SCP.SaWorkSchedule[bWorkScheduleEntry].bDays
                & laValue2Bit[TimeWeekdayGet() - 1])
            {
              bWorkPlan = SCP.SaWorkSchedule[bWorkScheduleEntry].bWorkPlanNo;
              bSigProgPlan =
                SCP.SaWorkSchedule[bWorkScheduleEntry].bSigProgPlan;
              bWorkScheduleEntry = WorkScheduleTotalGet(); /* end loop */
            }
          }
        }
      }

      /* requested bWorkPlan must be loaded to ram */
      if (bWorkPlan != WorkPlanCurNoGet())
      {
        /* update current plan ids */
        WorkPlanCurNoSet(bWorkPlan);
        if (StateCurrentGet() != STATES_SEQ)
        {
          LogRequest(LOG_REQ_APPEND_ASYNCH,
                     NULL,
                     EVENT_WORK_PLAN_CHANGE,
                     WorkPlanCurNoGet(),
                     0,
                     0,
                     0);
        }
      }

      bWorkPlanEntry = WorkPlanEntryCurNoGet();

      /* requested bSigProgPlan must be loaded to ram */
      if (bSigProgPlan != SigProgPlanCurNoGet())
      {
        SigProgPlanCurNoSet(bSigProgPlan);
        LogRequest(LOG_REQ_APPEND_ASYNCH,
                   NULL,
                   EVENT_SIGNAL_PROGRAM_PLAN_CHANGE,
                   SigProgPlanCurNoGet(),
                   0,
                   0,
                   0);
      }

      /* get current active entry in current signal program plan */
      bSPPlanEntry = SigProgPlanEntryCurNoGet();

      /* requested bSigProg must be loaded to ram */
      if (bSPPlanEntry)
      {
        if (SCP.SaSPPlan[SigProgPlanCurNoGet()][bSPPlanEntry - 1].bSigProg
            != SigProgCurNoGet())
        {
          /* execute statements */
          StatementExecuteRange(SCP.SaSignalPrograms[SigProgCurNoGet()
                                                     - 1].SSigProg.bStaStart,
                                SCP.SaSignalPrograms[SigProgCurNoGet()
                                                     - 1].SSigProg.bStaEnd);
          /* successful operations, update current plan ids */
          SigProgCurNoSet(SCP.SaSPPlan[SigProgPlanCurNoGet()][bSPPlanEntry
                                                              - 1].bSigProg);
          /* init signal program runtime values */
          SigProgCurTimeInPerClr();
          LogRequest(LOG_REQ_APPEND_ASYNCH,
                     NULL,
                     EVENT_SIGNAL_PROGRAM_CHANGE,
                     SigProgCurNoGet(),
                     TimeSourceGet(),
                     0,
                     0);
          ProgramSigProgChangeSet(TRUE);
        }
      }

      SAscCoord.SaPatterns[SigProgCurNoGet() - 1].bSplitNumber = bWorkPlanEntry;
      STRPatternsAndCoords.SaaPatterns[TRPatternsAndCoordsGetCurJunctionNo()][
        SigProgCurNoGet() - 1].bSplitNo =
        bWorkPlanEntry;
  }
} /* WorkScheduleUpdate */

uint8_t WorkScheduleTotalGet(void)
{
  return SCP.SConsumed.bWSEntriesTotal;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Signal Program */
uint8_t SigProgSet(uint8_t bSPNo, tpSSigProg pSSigProg)
{
  if ((bSPNo) && (bSPNo <= SIGNAL_PROGRAMS_MAX))
  {
    if (bSPNo > SigProgTotalGet()) /* Increase bSPTotal if new Signal Program is added. */
    {
      memcpy(&(SCP.SaSignalPrograms[bSPNo - 1].SSigProg), pSSigProg,
             sizeof(tSSigProg));

      /* Calculate Cheksums */
      SCP.SChecksum.baSigProgs[bSPNo] =
        ByteChecksum(&SCP.SaSignalPrograms[bSPNo - 1].SSigProg,
                     sizeof(tSSigProg));
      SCP.SChecksum.baTransitions[bSPNo] =
        ByteChecksum(&SCP.SaSignalPrograms[bSPNo - 1].SaTransitions,
                     sizeof(tSTransition));
      SCP.SChecksum.baOperations[bSPNo] =
        ByteChecksum(&SCP.SaSignalPrograms[bSPNo - 1].SaOperations,
                     sizeof(tSOperation));
      SCP.SChecksum.baRules[bSPNo] = ByteChecksum(&SCP.SaSignalPrograms[bSPNo
                                                                        - 1].
                                                  SaRules,
                                                  sizeof(tSRule));
      SCP.SChecksum.baStatements[bSPNo] =
        ByteChecksum(&SCP.SaSignalPrograms[bSPNo - 1].SaStatements,
                     sizeof(tSStatement));

      SCP.SConsumed.bSPTotal++;

      return TRUE;
    }
    else
    {
      memcpy(&(SCP.SaSignalPrograms[bSPNo - 1].SSigProg), pSSigProg,
             sizeof(tSSigProg));

      /* Calculate Cheksums */
      SCP.SChecksum.baSigProgs[bSPNo] =
        ByteChecksum(&SCP.SaSignalPrograms[bSPNo - 1].SSigProg,
                     sizeof(tSSigProg));
      SCP.SChecksum.baTransitions[bSPNo] =
        ByteChecksum(&SCP.SaSignalPrograms[bSPNo - 1].SaTransitions,
                     sizeof(tSTransition));
      SCP.SChecksum.baOperations[bSPNo] =
        ByteChecksum(&SCP.SaSignalPrograms[bSPNo - 1].SaOperations,
                     sizeof(tSOperation));
      SCP.SChecksum.baRules[bSPNo] = ByteChecksum(&SCP.SaSignalPrograms[bSPNo
                                                                        - 1].
                                                  SaRules,
                                                  sizeof(tSRule));
      SCP.SChecksum.baStatements[bSPNo] =
        ByteChecksum(&SCP.SaSignalPrograms[bSPNo - 1].SaStatements,
                     sizeof(tSStatement));

      return TRUE;
    }
  }

  return FALSE;
} /* SigProgSet */

void SigProgGet(uint8_t bSPNo, tpSSigProg pSSigProg)
{
  memcpy(pSSigProg, &(SCP.SaSignalPrograms[bSPNo - 1].SSigProg),
         sizeof(tSSigProg));
}

void SigProgCurClr(void)
{
  memset(&SCPRuntime.SSigProgRuntime, 0, sizeof(tSSigProgRuntime));
  memset(&SCP.SaSignalPrograms[SigProgCurNoGet() - 1], 0,
         sizeof(tSSignalPrograms));
  memset(&SaCounters, 0, sizeof(SaCounters));
}

uint8_t SigProgTotalGet(void)
{
  return SCP.SConsumed.bSPTotal;
}

uint8_t SigProgIsValid(uint8_t bSPNo)
{
  if (bSPNo && (bSPNo <= SigProgTotalGet()))
  {
    return TRUE;
  }

  return FALSE;
}

void SigProgCurNoSet(uint8_t bState)
{
  SCPRuntime.bCurSigProg = bState;
  SRuntimes.SaSignalStateRuntimes[SignalStateRuntimeCurNoGet()].bPatternNo =
    bState;
}

uint8_t SigProgCurNoGet(void)
{
  return SCPRuntime.bCurSigProg;
}

uint8_t SigProgCurTimeInPerGet(void)
{
  return SCPRuntime.SSigProgRuntime.bCurTimeInPer;
}

void SigProgCurTimeInPerInc(void)
{
  SCPRuntime.SSigProgRuntime.bCurTimeInPer++;
}

void SigProgCurTimeInPerClr(void)
{
  SCPRuntime.SSigProgRuntime.bCurTimeInPer = 0;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Transitions */
uint8_t TransitionSet(uint8_t bSPNo,
                      uint8_t bTraNo,
                      tpSTransition pSTransitionDef)
{
  if ((bSPNo && (bSPNo <= SIGNAL_PROGRAMS_MAX))
      && (bTraNo && (bTraNo == SCP.SConsumed.baTransitionTotal[bSPNo - 1] + 1)))                                           /* Add new Transition */
  {
    memcpy(&(SCP.SaSignalPrograms[bSPNo - 1].SaTransitions[bTraNo - 1]),
           pSTransitionDef,
           sizeof(tSTransition));
    SCP.SConsumed.baTransitionTotal[bSPNo - 1]++;

    return TRUE;
  }
  else if ((bSPNo && (bSPNo <= SIGNAL_PROGRAMS_MAX))
           && (bTraNo
               && (bTraNo <= SCP.SConsumed.baTransitionTotal[bSPNo - 1])))   /* Edit already added */
  /* Transition */
  {
    memcpy(&(SCP.SaSignalPrograms[bSPNo - 1].SaTransitions[bTraNo - 1]),
           pSTransitionDef,
           sizeof(tSTransition));
    SCP.SChecksum.baTransitions[bSPNo] =
      ByteChecksum(&SCP.SaSignalPrograms[bSPNo - 1].SaTransitions,
                   sizeof(tSTransition));

    return TRUE;
  }

  return FALSE;
}

void TransitionGet(uint8_t bSPNo, uint8_t bTraNo, tpSTransition pSTransitionDef)
{
  memcpy(pSTransitionDef,
         &(SCP.SaSignalPrograms[bSPNo - 1].SaTransitions[bTraNo - 1]),
         sizeof(tSTransition));
}

uint8_t TransitionFromGet(uint8_t bTraNo)
{
  return SCP.SaSignalPrograms[SigProgCurNoGet() - 1].SaTransitions[bTraNo
                                                                   - 1].bFrom;
}

uint8_t TransitionToGet(uint8_t bTraNo)
{
  return SCP.SaSignalPrograms[SigProgCurNoGet() - 1].SaTransitions[bTraNo
                                                                   - 1].bTo;
}

uint8_t TransitionFromNoGet(uint8_t bTraNo)
{
  return SCP.SaSignalPrograms[SigProgCurNoGet() - 1].SaTransitions[bTraNo
                                                                   - 1].bValue1
         & TRANSITION_VALUE_NO_GET;
}

uint8_t TransitionSimCloseGet(uint8_t bTraNo)
{
  if (SCP.SaSignalPrograms[SigProgCurNoGet() - 1].SaTransitions[bTraNo
                                                                - 1].bValue1
      & TRANSITION_VALUE_SIM_GET)
  {
    return TRUE;
  }

  return FALSE;
}

uint8_t TransitionToNoGet(uint8_t bTraNo)
{
  return SCP.SaSignalPrograms[SigProgCurNoGet() - 1].SaTransitions[bTraNo
                                                                   - 1].bValue2
         & TRANSITION_VALUE_NO_GET;
}

uint8_t TransitionSimOpenGet(uint8_t bTraNo)
{
  if (SCP.SaSignalPrograms[SigProgCurNoGet() - 1].SaTransitions[bTraNo
                                                                - 1].bValue2
      & TRANSITION_VALUE_SIM_GET)
  {
    return TRUE;
  }

  return FALSE;
}

uint8_t TransitionRuleGet(uint8_t bTraNo)
{
  return SCP.SaSignalPrograms[SigProgCurNoGet() - 1].SaTransitions[bTraNo
                                                                   - 1].bRule;
}

uint8_t TransitionPriorityGet(uint8_t bTraNo)
{
  return SCP.SaSignalPrograms[SigProgCurNoGet() - 1].SaTransitions[bTraNo
                                                                   - 1].
         bPriority;
}

uint8_t TransitionTotalGet(uint8_t bSPNo)
{
  return SCP.SConsumed.baTransitionTotal[bSPNo - 1];
}

uint8_t TransitionAllocate(void)
{
  uint8_t bIndex;

  for (bIndex = 0; bIndex < TRANSITIONS_MAX; bIndex++)
  {
    if (SCP.SaSignalPrograms[SigProgCurNoGet()
                             - 1].SaTransitions[bIndex].bFrom == STATES_NONE)
    {
      return bIndex + 1;
    }
  }

  return 0;
}

uint8_t TransitionIsValid(uint8_t bSPNo,
                          uint8_t bTraNo,
                          tpSTransition pSTransitionDef)
{
  if ((bTraNo && (bTraNo <= TRANSITIONS_MAX))
      && (pSTransitionDef->bFrom && (pSTransitionDef->bFrom <= STATES_MAX) )
      && (pSTransitionDef->bTo && (pSTransitionDef->bTo <= STATES_MAX))
      && (pSTransitionDef->bRule
          && (pSTransitionDef->bRule <= SCP.SConsumed.baRuleTotal[bSPNo - 1])))
  {
    return TRUE;
  }

  return FALSE;
}

uint8_t TransitionIsCircle(uint8_t bTraNo)
{
  if ((TransitionFromGet(bTraNo) == TransitionToGet(bTraNo))
      && (TransitionFromNoGet(bTraNo)
          ==
          TransitionToNoGet(bTraNo)))
  {
    return TRUE;
  }

  return FALSE;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Transition Lock */
void TransitionLockSet(uint8_t bTraNo)
{
  laTransitonsLock[(bTraNo - 1) / 32] |= laValue2Bit[bTraNo - 1];
}

uint8_t TransitionLockGet(uint8_t bTraNo)
{
  if (laTransitonsLock[(bTraNo - 1) / 32] & laValue2Bit[bTraNo - 1])
  {
    return TRUE;
  }

  return FALSE;
}

uint8_t TransitionLockIsActive(void)
{
  uint8_t bIndex;

  for (bIndex = 0; bIndex < TRANSITION_LOCK_SIZE; bIndex++)
  {
    if (laTransitonsLock[bIndex] != 0)
    {
      return TRUE;
    }
  }

  return FALSE;
}

void TransitionLockEnd(void)
{
  memset(&laTransitonsLock, 0, sizeof(laTransitonsLock));
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Statements */
uint8_t StatementSet(uint8_t bSPNo, uint8_t bAddress, tpSStatement pSStatement)
{
  if ((bSPNo && (bSPNo <= SIGNAL_PROGRAMS_MAX)) && (bAddress
                                                    && (bAddress
                                                        == SCP.SConsumed.
                                                        baStatementTotal[bSPNo
                                                                         - 1]
                                                        + 1)))         /* Add new Statement */
  {
    memcpy(&(SCP.SaSignalPrograms[bSPNo - 1].SaStatements[bAddress - 1]),
           pSStatement,
           sizeof(tSStatement));
    SCP.SConsumed.baStatementTotal[bSPNo - 1]++;

    return TRUE;
  }
  else if ((bSPNo && (bSPNo <= SIGNAL_PROGRAMS_MAX))
           && (bAddress
               && (bAddress <= SCP.SConsumed.baStatementTotal[bSPNo - 1])))     /* Edit already added */
  /* Statement */
  {
    memcpy(&(SCP.SaSignalPrograms[bSPNo - 1].SaStatements[bAddress - 1]),
           pSStatement,
           sizeof(tSStatement));
    SCP.SChecksum.baStatements[bSPNo] =
      ByteChecksum(&SCP.SaSignalPrograms[bSPNo - 1].SaStatements,
                   sizeof(tSStatement));

    return TRUE;
  }

  return FALSE;
}

void StatementGet(uint8_t bSPNo, uint8_t bAddress, tpSStatement pSStatement)
{
  memcpy(pSStatement,
         &(SCP.SaSignalPrograms[bSPNo - 1].SaStatements[bAddress - 1]),
         sizeof(tSStatement));
}

uint8_t StatementTotalGet(uint8_t bSPNo)
{
  return SCP.SConsumed.baStatementTotal[bSPNo - 1];
}

uint8_t StatementCommandGet(uint8_t bAddress)
{
  return SCP.SaSignalPrograms[SigProgCurNoGet() - 1].SaStatements[bAddress
                                                                  - 1].bCmd;
}

uint8_t StatementExecute(uint8_t bAddress)
{
  if (bAddress)
  {
    switch (SCP.SaSignalPrograms[SigProgCurNoGet() - 1].SaStatements[bAddress
                                                                     - 1].bCmd)
    {
        case COMMAND_MEMORY_ALLOCATE:
        {
          break;
        }

        case COMMAND_MEMORY_DEALLOCATE:
        {
          break;
        }

        case COMMAND_MEMORY_INIT:
        {
          switch (SCP.SaSignalPrograms[SigProgCurNoGet()
                                       - 1].SaStatements[bAddress - 1].bParam1)
          {
              case PARAM_OPERAND:
              {
                break;
              }

              case PARAM_OPERATION:
              {
                break;
              }

              case PARAM_COUNTER:
              {
                CounterValueSet(SCP.SaSignalPrograms[SigProgCurNoGet()
                                                     - 1].SaStatements[bAddress
                                                                       - 1].
                                bParam2,
                                SCP.SaSignalPrograms[SigProgCurNoGet()
                                                     - 1].SaStatements[bAddress
                                                                       - 1].
                                bParam3);
                break;
              }
          }

          break;
        }

        case COMMAND_MEMORY_ADD:
        {
          switch (SCP.SaSignalPrograms[SigProgCurNoGet()
                                       - 1].SaStatements[bAddress - 1].bParam1)
          {
              case PARAM_OPERAND:
              {
                break;
              }

              case PARAM_OPERATION:
              {
                break;
              }

              case PARAM_COUNTER:
              {
                CounterValueAdd(SCP.SaSignalPrograms[SigProgCurNoGet()
                                                     - 1].SaStatements[bAddress
                                                                       - 1].
                                bParam2,
                                SCP.SaSignalPrograms[SigProgCurNoGet()
                                                     - 1].SaStatements[bAddress
                                                                       - 1].
                                bParam3);
                break;
              }
          }

          break;
        }

        case COMMAND_COUNTER_START:
        {
          CounterRunningSet(bAddress, TRUE);
          break;
        }

        case COMMAND_COUNTER_STOP:
        {
          CounterRunningSet(bAddress, FALSE);
          break;
        }

        case COMMAND_PHASE_START:
        {
          PhaseStart(SCP.SaSignalPrograms[SigProgCurNoGet()
                                          - 1].SaStatements[bAddress
                                                            - 1].bParam1 - 1,
                     SCP.SaSignalPrograms[SigProgCurNoGet()
                                          - 1].SaStatements[bAddress
                                                            - 1].bParam2);
          break;
        }

        case COMMAND_PHASE_STOP:
        {
          PhaseStop(SCP.SaSignalPrograms[SigProgCurNoGet()
                                         - 1].SaStatements[bAddress
                                                           - 1].bParam1 - 1);
          break;
        }

        case COMMAND_PHASE_EXTEND:
        case COMMAND_PHASE_END:
        {
          PhaseExtDurSet(SCP.SaSignalPrograms[SigProgCurNoGet()
                                              - 1].SaStatements[bAddress
                                                                - 1].bParam1
                         - 1,
                         SCP.SaSignalPrograms[SigProgCurNoGet()
                                              - 1].SaStatements[bAddress
                                                                - 1].bParam3);
          break;
        }

        case COMMAND_PHASE_DELETE_RUN_INFO:
        {
          uint8_t bIndex;

          for (bIndex = 0; bIndex < PhaseTotalGet(); bIndex++)
          {
            PhaseRunSet(bIndex,
                        FALSE);
          }

          break;
        }

        case COMMAND_SEQ_START:
        {
          SeqStart(SCP.SaSignalPrograms[SigProgCurNoGet()
                                        - 1].SaStatements[bAddress
                                                          - 1].bParam1 - 1,
                   SCP.SaSignalPrograms[SigProgCurNoGet()
                                        - 1].SaStatements[bAddress
                                                          - 1].bParam2);
          break;
        }

        case COMMAND_SEQ_STOP:
        {
          SeqStop(SCP.SaSignalPrograms[SigProgCurNoGet()
                                       - 1].SaStatements[bAddress - 1].bParam1
                  - 1);
          break;
        }

        case COMMAND_SEQ_ADD_STEP:
        {
          break;
        }

        case COMMAND_SEQ_REMOVE_SECONDS:
        {
          break;
        }

        case COMMAND_USER_STATE_TO_CURRENT_STATE:
        {
          StateCurrentSet(UserStateCurrentGet());
          UserStateReqEnd(); /* user request ends, namely transition to user */
          /* requested mode is successfully completed */
          break;
        }

        case COMMAND_USER_STATE_REQ_END:
        {
          UserStateReqEnd(); /* user request ends, namely transition to user */
          /* requested mode is successfully completed */
          break;
        }

        case COMMAND_TRANSITIONS_LOCK_ADD:
        {
          if (SCP.SaSignalPrograms[SigProgCurNoGet()
                                   - 1].SaStatements[bAddress - 1].bParam1)
          {
            TransitionLockSet(SCP.SaSignalPrograms[SigProgCurNoGet()
                                                   - 1].SaStatements[bAddress
                                                                     - 1].
                              bParam1);
          }

          if (SCP.SaSignalPrograms[SigProgCurNoGet()
                                   - 1].SaStatements[bAddress - 1].bParam2)
          {
            TransitionLockSet(SCP.SaSignalPrograms[SigProgCurNoGet()
                                                   - 1].SaStatements[bAddress
                                                                     - 1].
                              bParam2);
          }

          if (SCP.SaSignalPrograms[SigProgCurNoGet()
                                   - 1].SaStatements[bAddress - 1].bParam3)
          {
            TransitionLockSet(SCP.SaSignalPrograms[SigProgCurNoGet()
                                                   - 1].SaStatements[bAddress
                                                                     - 1].
                              bParam3);
          }

          break;
        }

        case COMMAND_TRANSITIONS_LOCK_END:
        {
          TransitionLockEnd();
          break;
        }

        case COMMAND_SG_ADD_TO_FLASHER_LIST:
        {
          if (SCP.SaSignalPrograms[SigProgCurNoGet()
                                   - 1].SaStatements[bAddress - 1].bParam1)
          {
            SGFlasherAdd(SCP.SaSignalPrograms[SigProgCurNoGet()
                                              - 1].SaStatements[bAddress
                                                                - 1].bParam1);
            SGRuntimeDataSet(
              SCP.SaSignalPrograms[SigProgCurNoGet()
                                   - 1].SaStatements[bAddress - 1].bParam1 - 1,
              SGFlashSignalGet(SCP.SaSignalPrograms[SigProgCurNoGet()
                                                    - 1].SaStatements[bAddress
                                                                      - 1].
                               bParam1 - 1),
              SIGNAL_GROUP_STATE_FLASHER);
          }

          if (SCP.SaSignalPrograms[SigProgCurNoGet()
                                   - 1].SaStatements[bAddress - 1].bParam2)
          {
            SGFlasherAdd(SCP.SaSignalPrograms[SigProgCurNoGet()
                                              - 1].SaStatements[bAddress
                                                                - 1].bParam2);
            SGRuntimeDataSet(
              SCP.SaSignalPrograms[SigProgCurNoGet()
                                   - 1].SaStatements[bAddress - 1].bParam2 - 1,
              SGFlashSignalGet(SCP.SaSignalPrograms[SigProgCurNoGet()
                                                    - 1].SaStatements[bAddress
                                                                      - 1].
                               bParam2 - 1),
              SIGNAL_GROUP_STATE_FLASHER);
          }

          if (SCP.SaSignalPrograms[SigProgCurNoGet()
                                   - 1].SaStatements[bAddress - 1].bParam3)
          {
            SGFlasherAdd(SCP.SaSignalPrograms[SigProgCurNoGet()
                                              - 1].SaStatements[bAddress
                                                                - 1].bParam3);
            SGRuntimeDataSet(
              SCP.SaSignalPrograms[SigProgCurNoGet()
                                   - 1].SaStatements[bAddress - 1].bParam3 - 1,
              SGFlashSignalGet(SCP.SaSignalPrograms[SigProgCurNoGet()
                                                    - 1].SaStatements[bAddress
                                                                      - 1].
                               bParam3 - 1),
              SIGNAL_GROUP_STATE_FLASHER);
          }

          break;
        }

        case COMMAND_SG_SUB_FROM_FLASHER_LIST:
        {
          if (SCP.SaSignalPrograms[SigProgCurNoGet()
                                   - 1].SaStatements[bAddress - 1].bParam1)
          {
            SGFlasherSub(SCP.SaSignalPrograms[SigProgCurNoGet()
                                              - 1].SaStatements[bAddress
                                                                - 1].bParam1);
          }

          if (SCP.SaSignalPrograms[SigProgCurNoGet()
                                   - 1].SaStatements[bAddress - 1].bParam2)
          {
            SGFlasherSub(SCP.SaSignalPrograms[SigProgCurNoGet()
                                              - 1].SaStatements[bAddress
                                                                - 1].bParam2);
          }

          if (SCP.SaSignalPrograms[SigProgCurNoGet()
                                   - 1].SaStatements[bAddress - 1].bParam3)
          {
            SGFlasherSub(SCP.SaSignalPrograms[SigProgCurNoGet()
                                              - 1].SaStatements[bAddress
                                                                - 1].bParam3);
          }

          break;
        }

        case COMMAND_SG_ADD_TO_PHASE:
        {
          if (SCP.SaSignalPrograms[SigProgCurNoGet()
                                   - 1].SaStatements[bAddress - 1].bParam1)
          {
            /* the new states of the signal groups are SIGNAL_GROUP_STATE_OPEN */
            /* because these statements are executed after phase transition is */
            /* completed at the end of the STATES_PHASE_TRANSITION */
            if (SCP.SaSignalPrograms[SigProgCurNoGet()
                                     - 1].SaStatements[bAddress - 1].bParam2)
            {
              PhaseSGAdd(SCP.SaSignalPrograms[SigProgCurNoGet()
                                              - 1].SaStatements[bAddress
                                                                - 1].bParam1,
                         SCP.SaSignalPrograms[SigProgCurNoGet()
                                              - 1].SaStatements[bAddress
                                                                - 1].bParam2);
              SGRuntimeDataSet(SCP.SaSignalPrograms[SigProgCurNoGet()
                                                    - 1].SaStatements[bAddress
                                                                      - 1].
                               bParam2 - 1,
                               SignalsDefinedFreeGet(),
                               SIGNAL_GROUP_STATE_OPEN);
            }

            if (SCP.SaSignalPrograms[SigProgCurNoGet()
                                     - 1].SaStatements[bAddress - 1].bParam3)
            {
              PhaseSGAdd(SCP.SaSignalPrograms[SigProgCurNoGet()
                                              - 1].SaStatements[bAddress
                                                                - 1].bParam1,
                         SCP.SaSignalPrograms[SigProgCurNoGet()
                                              - 1].SaStatements[bAddress
                                                                - 1].bParam3);
              SGRuntimeDataSet(SCP.SaSignalPrograms[SigProgCurNoGet()
                                                    - 1].SaStatements[bAddress
                                                                      - 1].
                               bParam3 - 1,
                               SignalsDefinedFreeGet(),
                               SIGNAL_GROUP_STATE_OPEN);
            }
          }

          break;
        }

        case COMMAND_SG_SUB_FROM_PHASE:
        {
          if (SCP.SaSignalPrograms[SigProgCurNoGet()
                                   - 1].SaStatements[bAddress - 1].bParam1)
          {
            if (SCP.SaSignalPrograms[SigProgCurNoGet()
                                     - 1].SaStatements[bAddress - 1].bParam2)
            {
              PhaseSGSub(SCP.SaSignalPrograms[SigProgCurNoGet()
                                              - 1].SaStatements[bAddress
                                                                - 1].bParam1,
                         SCP.SaSignalPrograms[SigProgCurNoGet()
                                              - 1].SaStatements[bAddress
                                                                - 1].bParam2);
            }

            if (SCP.SaSignalPrograms[SigProgCurNoGet()
                                     - 1].SaStatements[bAddress - 1].bParam3)
            {
              PhaseSGSub(SCP.SaSignalPrograms[SigProgCurNoGet()
                                              - 1].SaStatements[bAddress
                                                                - 1].bParam1,
                         SCP.SaSignalPrograms[SigProgCurNoGet()
                                              - 1].SaStatements[bAddress
                                                                - 1].bParam3);
            }
          }

          break;
        }

        case COMMAND_SIG_PROG_PER_RESTART:
        {
          /* restart signal program period counter */
          SigProgCurTimeInPerClr();
          break;
        }
    } /* switch */
  }

  return 0;
} /* StatementExecute */

uint8_t StatementExecuteRange(uint8_t bStartAddr, uint8_t bEndAddr)
{
  if ((bStartAddr) && (bEndAddr) && (bStartAddr <= bEndAddr))
  {
    uint8_t bIndex;

    for (bIndex = bStartAddr; bIndex <= bEndAddr; bIndex++)
    {
      StatementExecute(bIndex);
    }
  }

  return 0;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Operations */
uint8_t OperationSet(uint8_t bSPNo, uint8_t bAddress, tpSOperation pSOperation)
{
  if ((bSPNo && (bSPNo <= SIGNAL_PROGRAMS_MAX)) && (bAddress
                                                    && (bAddress
                                                        == SCP.SConsumed.
                                                        baOperationTotal[bSPNo
                                                                         - 1]
                                                        + 1)))         /* Add new Operation */
  {
    memcpy(&(SCP.SaSignalPrograms[bSPNo - 1].SaOperations[bAddress - 1]),
           pSOperation,
           sizeof(tSOperation));
    SCP.SConsumed.baOperationTotal[bSPNo - 1]++;

    return TRUE;
  }
  else if ((bSPNo && (bSPNo <= SIGNAL_PROGRAMS_MAX))
           && (bAddress
               && (bAddress <= SCP.SConsumed.baOperationTotal[bSPNo - 1])))     /* Edit already added */
  /* Operation */
  {
    memcpy(&(SCP.SaSignalPrograms[bSPNo - 1].SaOperations[bAddress - 1]),
           pSOperation,
           sizeof(tSOperation));
    SCP.SChecksum.baOperations[bSPNo] =
      ByteChecksum(&SCP.SaSignalPrograms[bSPNo - 1].SaOperations,
                   sizeof(tSOperation));

    return TRUE;
  }

  return FALSE;
}

void OperationGet(uint8_t bSPNo, uint8_t bAddress, tpSOperation pSOperation)
{
  memcpy(pSOperation,
         &(SCP.SaSignalPrograms[bSPNo - 1].SaOperations[bAddress - 1]),
         sizeof(tSOperation));
}

uint8_t OperationAllocate(void)
{
  uint16_t sIndex;

  for (sIndex = 0; sIndex < RULE_OPERATIONS_MAX; sIndex++)
  {
    if (SCP.SaSignalPrograms[SigProgCurNoGet()
                             - 1].SaOperations[sIndex].bOperator == OPR_NONE)
    {
      return sIndex + 1;
    }
  }

  return 0;
}

uint8_t OperationTotalGet(uint8_t bSPNo)
{
  return SCP.SConsumed.baOperationTotal[bSPNo - 1];
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Rules */
uint8_t RuleSet(uint8_t bSPNo, uint8_t bAddress, tpSRule pSRule)
{
  if ((bSPNo && (bSPNo <= SIGNAL_PROGRAMS_MAX))
      && (bAddress && (bAddress == SCP.SConsumed.baRuleTotal[bSPNo - 1] + 1)))                                           /* Add new Rule */
  {
    memcpy(&(SCP.SaSignalPrograms[bSPNo - 1].SaRules[bAddress - 1]),
           pSRule,
           sizeof(tSRule));
    SCP.SConsumed.baRuleTotal[bSPNo - 1]++;

    return TRUE;
  }
  else if ((bSPNo && (bSPNo <= SIGNAL_PROGRAMS_MAX))
           && (bAddress && (bAddress <= SCP.SConsumed.baRuleTotal[bSPNo - 1]))) /* Edit already */
  /* added Rule */
  {
    memcpy(&(SCP.SaSignalPrograms[bSPNo - 1].SaRules[bAddress - 1]),
           pSRule,
           sizeof(tSRule));
    SCP.SChecksum.baRules[bSPNo] = ByteChecksum(&SCP.SaSignalPrograms[bSPNo
                                                                      - 1].
                                                SaRules,
                                                sizeof(tSRule));

    return TRUE;
  }

  return FALSE;
}

uint8_t RuleTotalGet(uint8_t bSPNo)
{
  return SCP.SConsumed.baRuleTotal[bSPNo - 1];
}

void RuleGet(uint8_t bSPNo, uint8_t bAddress, tpSRule pSRule)
{
  memcpy(pSRule,
         &(SCP.SaSignalPrograms[bSPNo - 1].SaRules[bAddress - 1]),
         sizeof(tSRule));
}

uint8_t RuleTOpsStartGet(uint8_t bAddress)
{
  return SCP.SaSignalPrograms[SigProgCurNoGet() - 1].SaRules[bAddress
                                                             - 1].bTOpsStart;
}

uint8_t RuleTOpsEndGet(uint8_t bAddress)
{
  return SCP.SaSignalPrograms[SigProgCurNoGet() - 1].SaRules[bAddress
                                                             - 1].bTOpsEnd;
}

uint8_t RuleFOpsStartGet(uint8_t bAddress)
{
  return SCP.SaSignalPrograms[SigProgCurNoGet() - 1].SaRules[bAddress
                                                             - 1].bFOpsStart;
}

uint8_t RuleFOpsEndGet(uint8_t bAddress)
{
  return SCP.SaSignalPrograms[SigProgCurNoGet() - 1].SaRules[bAddress
                                                             - 1].bFOpsEnd;
}

uint8_t RuleAllocate(void)
{
  uint8_t bIndex;

  for (bIndex = 0; bIndex < RULES_MAX; bIndex++)
  {
    if (SCP.SaSignalPrograms[SigProgCurNoGet() - 1].SaRules[bIndex].sStart == 0)
    {
      return bIndex + 1;
    }
  }

  return 0;
}

long RuleState(uint8_t bAddress)
{
  return OperationState(
    &(SCP.SaSignalPrograms[SigProgCurNoGet()
                           - 1].SaOperations[SCP.SaSignalPrograms[
                                               SigProgCurNoGet()
                                               - 1].SaRules[bAddress
                                                            -
                                                            1]
                                             .sStart
                                             - 1]));
}

long OperationState(tpSOperation pSOperation)
{
  long lVal1; /* result of the first operand */
  long lVal2; /* result of the second operand */
  uint16_t sValue1; /* value in first operand */
  uint16_t sValue2; /* value in second operand */

  /* get values assigned to operands */
  sValue1 = pSOperation->SaOperands[OP_FIR].bValueHigh;
  sValue1 = (sValue1 << 8) + pSOperation->SaOperands[OP_FIR].bValueLow;
  sValue2 = pSOperation->SaOperands[OP_SEC].bValueHigh;
  sValue2 = (sValue2 << 8) + pSOperation->SaOperands[OP_SEC].bValueLow;

  /* calculate first operand */
  if (pSOperation->SaOperands[OP_FIR].bField == OP_FIELD_BLOCK)
  {
    lVal1 = OperationState(&(SCP.SaSignalPrograms[SigProgCurNoGet()
                                                  - 1].SaOperations[sValue1
                                                                    - 1]));
  }
  else
  {
    lVal1 = OperandState(&(pSOperation->SaOperands[OP_FIR]));
  }

  /* calculate second operand */
  if (pSOperation->SaOperands[OP_SEC].bField == OP_FIELD_BLOCK)
  {
    lVal2 = OperationState(&(SCP.SaSignalPrograms[SigProgCurNoGet()
                                                  - 1].SaOperations[sValue2
                                                                    - 1]));
  }
  else
  {
    lVal2 = OperandState(&(pSOperation->SaOperands[OP_SEC]));
  }

  /* return result according to operator */
  switch (pSOperation->bOperator)
  {
      case OPR_EQUAL:
      {
        return lVal1 == lVal2;
      }

      case OPR_NOTEQUAL:
      {
        return lVal1 != lVal2;
      }

      case OPR_LESS:
      {
        return lVal1 < lVal2;
      }

      case OPR_LESSEQUAL:
      {
        return lVal1 <= lVal2;
      }

      case OPR_GREATER:
      {
        return lVal1 > lVal2;
      }

      case OPR_GREATEREQUAL:
      {
        return lVal1 >= lVal2;
      }

      case OPR_ADD:
      {
        return lVal1 + lVal2;
      }

      case OPR_SUB:
      {
        return lVal1 - lVal2;
      }

      case OPR_MUL:
      {
        return lVal1 * lVal2;
      }

      case OPR_DIV:
      {
        return lVal1 / lVal2;
      }

      case OPR_MODULO:
      {
        return lVal1 % lVal2;
      }

      case OPR_AND:
      {
        return lVal1 && lVal2;
      }

      case OPR_OR:
      {
        return lVal1 || lVal2;
      }

      case OPR_GG_CONFLICT:
      {
        return 0;
      }

      case OPR_GY_CONFLICT:
      {
        return 0;
      }

      case OPR_YY_CONFLICT:
      {
        return 0;
      }
  } /* switch */

  return 0;
} /* OperationState */

long OperandState(tpSOperand pSOperand)
{
  uint16_t sValue; /* value in operand */

  /* get values assigned to operands */
  sValue = pSOperand->bValueHigh;
  sValue = (sValue << 8) + pSOperand->bValueLow;

  switch (pSOperand->bField)
  {
      case OP_FIELD_DETECTOR:
      {
        tpSInput pSUnitDef;
        tpSInputRuntime pSUnitRuntime;

        if (sValue)
        {
          if (sValue <= INPUTS_DETECTOR_MAX)
          {
            /* this is detector */
            pSUnitDef = &(SCP.SaDetectorDefs[sValue - 1]);
            pSUnitRuntime = &(SRuntimes.SaDetectorRuntimes[sValue - 1]);
          }
          else
          {
            /* this is input */
            sValue -= INPUTS_DETECTOR_MAX;
            pSUnitDef = &(SCP.SaInputDefs[sValue - 1]);
            pSUnitRuntime = &(SRuntimes.SaInputRuntimes[sValue - 1]);
          }
        }
        else
        {
          return 0;
        }

        switch (pSOperand->bSubField)
        {
            case OP_SUBFIELD_DEMAND_PERIOD:
            {
              return pSUnitRuntime->bDemandCntInPer;
            }

            case OP_SUBFIELD_DEMAND_RED:
            {
              return pSUnitRuntime->bDemandCntInRed;
            }

            case OP_SUBFIELD_DEMAND_GREEN:
            {
              return pSUnitRuntime->bDemandCntInGreen;
            }

            case OP_SUBFIELD_FDEMAND_DUR_PERIOD:
            {
              return pSUnitRuntime->sFDemandDurInPer / 10;
            }

            case OP_SUBFIELD_FDEMAND_DUR_RED:
            {
              return pSUnitRuntime->sFDemandDurInRed / 10;
            }

            case OP_SUBFIELD_OCC_DUR_PERIOD:
            {
              return pSUnitRuntime->sOccDurInPer / 10;
            }

            case OP_SUBFIELD_OCC_DUR_RED:
            {
              return pSUnitRuntime->sOccDurInRed / 10;
            }

            case OP_SUBFIELD_OCC_DUR_GREEN:
            {
              return pSUnitRuntime->sOccDurInGreen / 10;
            }

            case OP_SUBFIELD_GAP_DUR_PERIOD:
            {
              return pSUnitRuntime->sGapDurInPer / 10;
            }

            case OP_SUBFIELD_GAP_DUR_GREEN:
            {
              return pSUnitRuntime->sGapDurInGreen / 10;
            }

            case OP_SUBFIELD_BROKEN_DUR:
            {
              if (pSUnitRuntime->fBroken)
              {
                if (SignalHasRed(SRuntimes.SaSGRuntimes[pSUnitDef->bOwnerSG
                                                        - 1].bCurrentSignal))
                {
                  return SRuntimes.SaSGRuntimes[pSUnitDef->bOwnerSG
                                                - 1].bDuration;
                }
              }

              break;
            }

            case OP_SUBFIELD_IS_BROKEN:
            {
              return pSUnitRuntime->fBroken;
            }

            case OP_SUBFIELD_OWNER_SG:
            {
              return pSUnitDef->bOwnerSG;
            }

            case OP_SUBFIELD_GREEN_DUR_PER_DEMAND:
            {
              return pSUnitDef->bGreenDurPerDemand; /* in seconds */
            }

            case OP_SUBFIELD_RED_DUR_IN_BROKEN:
            {
              return pSUnitDef->bRedDurInBroken; /* in seconds */
            }

            case OP_SUBFIELD_PHASE_IN_BROKEN:
            {
              return pSUnitDef->bPhaseInBroken;
            }
        } /* switch */

        break;
      }

      case OP_FIELD_SG:
      {
        switch (pSOperand->bSubField)
        {
            case OP_SUBFIELD_FDEMAND_DUR_RED_MAX:
            {
              if (sValue)
              {
                return SRuntimes.SaSGIORuntime[sValue - 1].sFDemandDurInRedMax;
              }

              break;
            }

            case OP_SUBFIELD_OCC_DUR_RED_MAX:
            {
              if (sValue)
              {
                return SRuntimes.SaSGIORuntime[sValue - 1].sOccDurInRedMax;
              }

              break;
            }

            case OP_SUBFIELD_OCC_DUR_GREEN_MAX:
            {
              if (sValue)
              {
                return SRuntimes.SaSGIORuntime[sValue - 1].sOccDurInGreenMax;
              }

              break;
            }

            case OP_SUBFIELD_GAP_DUR_GREEN_MIN:
            {
              if (sValue)
              {
                return SRuntimes.SaSGIORuntime[sValue - 1].sGapDurInGreenMin;
              }

              break;
            }

            case OP_SUBFIELD_SG_SIGNAL:
            {
              if (sValue)
              {
                return SRuntimes.SaSGRuntimes[sValue - 1].bCurrentSignal;
              }

              break;
            }

            case OP_SUBFIELD_SG_DURATION:
            {
              if (sValue)
              {
                return SRuntimes.SaSGRuntimes[sValue - 1].bDuration;
              }

              break;
            }

            case OP_SUBFIELD_SG_STATE:
            {
              if (sValue)
              {
                return SRuntimes.SaSGRuntimes[sValue - 1].bState;
              }

              break;
            }

            case OP_SUBFIELD_SG_NO:
            {
              return sValue;
            }
        } /* switch */

        break;
      }

      case OP_FIELD_PHASE:
      {
        switch (pSOperand->bSubField)
        {
            case OP_SUBFIELD_PHASE_MIN_DUR:
            {
              if (sValue)
              {
                return SCP.SaPhaseDefs[sValue - 1].bMinDur;
              }

              break;
            }

            case OP_SUBFIELD_PHASE_MAX_DUR:
            {
              if (sValue)
              {
                return SCP.SaPhaseDefs[sValue - 1].bMaxDur;
              }

              break;
            }

            case OP_SUBFIELD_PHASE_ELAPSED_DUR:
            {
              if (sValue)
              {
                return SRuntimes.SaPhaseRuntimes[sValue - 1].sElapsedDur;
              }

              break;
            }

            case OP_SUBFIELD_PHASE_HAS_RUN:
            {
              if (sValue)
              {
                return SRuntimes.SaPhaseRuntimes[sValue - 1].fRun;
              }

              break;
            }
        } /* switch */

        break;
      }

      case OP_FIELD_SEQ:
      {
        switch (pSOperand->bSubField)
        {
            case OP_SUBFIELD_SEQ_STEP_TOTAL:
            {
              return SeqCurStepNumTotalGet();
            }

            case OP_SUBFIELD_SEQ_CUR_STEP:
            {
              return SRuntimes.SSeqRuntime.bCurrentStep;
            }

            case OP_SUBFIELD_SEQ_CUR_STEP_DUR:
            {
              return SRuntimes.SSeqRuntime.bCurrentStepCurrentDuration;
            }

            case OP_SUBFIELD_SEQ_CUR_DUR:
            {
              return SeqDurCurGet();
            }

            case OP_SUBFIELD_SEQ_IS_ENDED:
            {
              if (SRuntimes.SSeqRuntime.bCurrentStep
                  >= (SeqCurStepNumTotalGet() - 1))
              {
                /* we are in last step */
                if (SRuntimes.SSeqRuntime.bCurrentStepCurrentDuration
                    >= (SCP.SaSeqDefs[bCurrentSeqNo].baDurations[SRuntimes
                                                                 .
                                                                 SSeqRuntime.
                                                                 bCurrentStep]
                        +
                        SeqStepExtDurGet(
                          SRuntimes
                          .SSeqRuntime.bCurrentStep)))
                {
                  return TRUE; /* sequence ends */
                }
              }

              return FALSE;
            }

            case OP_SUBFIELD_SEQ_TOUR:
            {
              return SRuntimes.SSeqRuntime.bTour;
            }
        } /* switch */

        break;
      }

      case OP_FIELD_CONSTANT:
      {
        return sValue;
      }

      case OP_FIELD_VAR:
      {
        switch (pSOperand->bSubField)
        {
            case OP_SUBFIELD_VAR_OPERATION_POOL:
            {
              return OperationState(&(SCP.SaSignalPrograms[SigProgCurNoGet()
                                                           - 1].SaOperations[
                                        sValue - 1]));
            }

            case OP_SUBFIELD_VAR_RULE_POOL:
            {
              return RuleState(sValue - 1);
            }
        }

        break;
      }

      case OP_FIELD_COUNTER:
      {
        switch (pSOperand->bSubField)
        {
            case OP_SUBFIELD_COUNTER_VAL:
            {
              return SaCounters[sValue - 1].lValue;
            }

            case OP_SUBFIELD_COUNTER_IS_ALLOCATED:
            {
              return SaCounters[sValue - 1].fAllocated;
            }

            case OP_SUBFIELD_COUNTER_IS_RUNNING:
            {
              return SaCounters[sValue - 1].fRunning;
            }

            case OP_SUBFIELD_COUNTER_IS_OVERFLOW:
            {
              return SaCounters[sValue - 1].fRunning;
            }
        }

        break;
      }

      case OP_FIELD_WORKPLAN:
      {
        switch (pSOperand->bSubField)
        {
            case OP_SUBFIELD_WORKPLAN_CUR_PHASE_DUR:
            {
              return WorkPlanPhaseDurGet(sValue - 1);
            }
        }

        break;
      }

      case OP_FIELD_TIME:
      {
        switch (pSOperand->bSubField)
        {
            case OP_SUBFIELD_SYNCH_NET:
            {
              return TimeSecondOfDayGet();
            }

            case OP_SUBFIELD_SYNCH_RTC:
            {
              return TimeSecondOfDayGet();
            }

            case OP_SUBFIELD_SYNCH_CENTER:
            {
              return TimeSecondOfDayGet();
            }

            case OP_SUBFIELD_SYNCH_GPS:
            {
              return TimeSecondOfDayGet(); /* gps time is approximately equal to the */
            }/* system time */
        }

        break;
      }

      case OP_FIELD_TIME_VALUE:
      {
        uint8_t bHour = (sValue & OP_FIELD_TIME_VALUE_HOUR) >>
                        OP_FIELD_TIME_VALUE_MINUTE_SIZE;
        uint8_t bMinute = (sValue & OP_FIELD_TIME_VALUE_MINUTE);

        return bHour * 60 * 60 + bMinute * 60;
      }

      case OP_FIELD_USER:
      {
        switch (pSOperand->bSubField)
        {
            case OP_SUBFIELD_STATE_NO_CONTROL:
            {
              if (UserStateReqGet() == STATES_NO_CONTROL)
              {
                return TRUE;
              }

              break;
            }

            case OP_SUBFIELD_STATE_FLASH:
            {
              if (UserStateReqGet() == STATES_FLASH)
              {
                return TRUE;
              }

              break;
            }

            case OP_SUBFIELD_STATE_CLOSED:
            {
              if (UserStateReqGet() == STATES_CLOSED)
              {
                return TRUE;
              }

              break;
            }

            case OP_SUBFIELD_STATE_ANY:
            {
              switch (UserStateReqGet())
              {
                  case STATES_NO_CONTROL:
                  case STATES_FLASH:
                  case STATES_CLOSED:
                  {
                    return TRUE;
                  }
              }

              break;
            }
        } /* switch */

        break;
      }

      case OP_FIELD_TRANSITION:
      {
        switch (pSOperand->bSubField)
        {
            case OP_SUBFIELD_TRANSITION_LAST:
            {
              return ProgramCurrentTransitionGet();
            }
        }

        break;
      }

      case OP_FIELD_TRAFFIC_DATA_SET:
      {
        break;
      }
  } /* switch */

  return 0;
} /* OperandState */

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Data Operations */
void DataInit(uint8_t keepConnectionInfo, uint8_t fInitSOPowers)
{
  uint8_t bIndex = 0;

  RelayStateRequestSet(TRUE);

  memset(&SaSeqExtDur, 0, sizeof(SaSeqExtDur));

  memset(&SCPRuntime.SPeripheralStates, 0,
         sizeof(SCPRuntime.SPeripheralStates));

  if (keepConnectionInfo == FALSE)
  {
    memset(&SCP.SDevInfo, 0, sizeof(SCP.SDevInfo));
  }

  memset(&SCP.SaSignalDefs, 0, sizeof(SCP.SaSignalDefs));
  memset(&SCP.SSignalsDefined, 0, sizeof(SCP.SSignalsDefined));
  memset(&SCP.SaSGDefs, 0, sizeof(SCP.SaSGDefs));
  memset(&SCP.SaSODefs, 0, sizeof(SCP.SaSODefs));
  memset(&SCP.SCVSDef, 0, sizeof(SCP.SCVSDef));
  memset(&SCP.SConflictsEM, 0, sizeof(SCP.SConflictsEM));
  memset(&SCP.SaSeqDefs, 0, sizeof(SCP.SaSeqDefs));
  memset(&SCP.SaPhaseDefs, 0, sizeof(SCP.SaPhaseDefs));
  memset(&SCP.SaDetectorDefs, 0, sizeof(SCP.SaDetectorDefs));
  memset(&SCP.SaInputDefs, 0, sizeof(SCP.SaInputDefs));
  memset(&SCP.SaOutputDefs, 0, sizeof(SCP.SaOutputDefs));
  memset(&SCP.SaWorkPlan, 0, sizeof(SCP.SaWorkPlan));
  memset(&SCP.SaSPPlan, 0, sizeof(SCP.SaSPPlan));
  memset(&SCP.SaSignalPlans, 0, sizeof(SCP.SaSignalPlans));
  memset(&SCP.SaWorkSchedule, 0, sizeof(SCP.SaWorkSchedule));
  memset(&SCP.SaSignalPrograms, 0, sizeof(SCP.SaSignalPrograms));
  memset(&SCP.SConsumed, 0, sizeof(SCP.SConsumed));
  memset(&SCP.SChecksum, 0, sizeof(SCP.SChecksum));
  memset(&SCP.SFlashPeriods, 0, sizeof(SCP.SFlashPeriods));

  memset(&SRuntimes, 0, sizeof(SRuntimes)); /*  Init Runtimes */
  memset(&SMCSTrafficCountsRuntimes, 0, sizeof(SMCSTrafficCountsRuntimes)); /* Init MCS traffic count runtime */
  memset(&SCPRuntime.SUserState, 0, sizeof(tSUserState)); /*  Initialize User State// Initialize Flash Periods */

  /* so powers */
  if (fInitSOPowers)
  {
    InitSOPowers();
  }

  /* io input values */
  memset(&SaPrevCanDigitalIOInputs, 0xFF, sizeof(SaPrevCanDigitalIOInputs));
  memset(&SaPrevCanDetectorIOInputs, 0xFF, sizeof(SaPrevCanDetectorIOInputs));

  /* store last demands from detectors and inputs */
  bLastDetectorDemandIssued = 0;
  bLastInputDemandIssued = 0;

  /* time source */
  TimeSourceSet(TIME_SOURCE_RTC);

  /* ssm test */
  SSSMTest.bSSMTestSource = SSM_TEST_FROM_NONE;
  SSSMTest.bTurnOnSONo = 0;

  SignalPlanCurrentSet(0);
  bCurrentSeqNo = SIGNAL_SEQS_MAX;
  sSGFlashers = 0;

  /* Set Default Flash Periods */
  SCP.SFlashPeriods.sFlashPeriod = FLASH_PERIOD_1000ms;
  SCP.SFlashPeriods.sEmergencyFlashPeriod = FLASH_PERIOD_500ms;

  /* transition lock mechanism init */
  memset(&laTransitonsLock, 0, sizeof(laTransitonsLock));

  /* Init Signal Program */
  SigProgCurClr();

  /* default work plan */
  SCP.SaWorkPlan[WORK_PLAN_DEFAULT][0].bHours = 0;
  SCP.SaWorkPlan[WORK_PLAN_DEFAULT][0].bMinutes = 0;
  SCP.SConsumed.baWPEntriesTotal[WORK_PLAN_DEFAULT] = 1;

  WorkPlanCurNoSet(WORK_PLAN_DEFAULT);

  /* power supply */
  memset(&SaPSMs, 0, sizeof(SaPSMs));

  for (bIndex = 0; bIndex < SIGNAL_OUTPUT_CURRENT_GROUPS_MAX; bIndex++)
  {
    SaCurrents[bIndex].sMin = 1024;
    SaCurrents[bIndex].sNow = 0;
    SaCurrents[bIndex].sMax = 0;
  }

  /* io inputs */
  memset(&SaCanDigitalIOInputs, 0xFF, sizeof(SaCanDigitalIOInputs));
  memset(&SaCanDetectorIOInputs, 0xFF, sizeof(SaCanDetectorIOInputs));

  memset(&baIOMessagePeriodCounter, 0, sizeof(baIOMessagePeriodCounter));
  memset(&baLDMessagePeriodCounter, 0, sizeof(baLDMessagePeriodCounter));

  SCPRuntime.bVoltageState = EVENT_NONE;
  SCPRuntime.bFrequencyState = EVENT_NONE;

  /* Function Conf */
  memset(&SFuncConf, 0, sizeof(tSFuncConf));

  /* Error Info */
  InitErrorInfo();

  /* Traffic Counts Timer */
  SetTrafficCountsTimer(0);

  /* Modules version */
  SetModulesVersion();

  SignalStateRuntimeCurNoSet(SIGNAL_STATE_DEFAULT);

  UserOperationsInit();

  SignalStateRuntimeInit();

  ChannelErrorsInit();

  TRPatternsAndCoordsInit();

  GCInit();
  GTMInit();

  AscPhaseInit();

  UnitInit();

  CoordInit();

  TimebaseAscInit();

  RingInit();

  OverlapInit();

  AscBlockInit();

  PreemptInit();

  ASCCabinetEnvironmentInit();
} /* DataInit */

uint8_t ProgramDataErase(void)
{
  uint32_t lSize = ((sizeof(SCP) + 31) / 32) * 32;

  return DataPersistenceErase(PERSIST_OBJECT_PROGRAM_DATA, 0U, lSize);
}

uint8_t ProgramDataWrite(void)
{
  uint32_t lSize = ((sizeof(SCP) + 31) / 32) * 32;

  return DataPersistenceWrite(PERSIST_OBJECT_PROGRAM_DATA,
                              0U,
                              &SCP,
                              lSize);
}

uint8_t ProgramDataRead(void)
{
  uint32_t lSize = ((sizeof(SCP) + 31) / 32) * 32;

  return DataPersistenceRead(PERSIST_OBJECT_PROGRAM_DATA,
                             0U,
                             &SCP,
                             lSize);
}

uint8_t ProgramDataStartMagicRead(void)
{
  uint32_t lSize = ((sizeof(SCP.laStartMagic) + 31) / 32) * 32;

  return DataPersistenceRead(PERSIST_OBJECT_PROGRAM_DATA,
                             0U,
                             &SCP.laStartMagic,
                             lSize);
}

uint32_t ProgramDataStartMagicGet(uint8_t bIdx)
{
  return SCP.laStartMagic[bIdx];
}

void ProgramDataStartMagicSet(void)
{
  uint8_t bIdx;

  for (bIdx = 0; bIdx < SCP_MAGIC_MAX; bIdx++)
  {
    SCP.laStartMagic[bIdx] = SCP_START_MAGIC;
  }
}

uint8_t ProgramPlanUpdateTimeRead(void)
{
  return DataPersistenceRead(PERSIST_OBJECT_PROGRAM_LAST_CHANGE_TIME,
                             0U,
                             &SRuntimes.lPlanLastChangeTime,
                             sizeof(void *));
}

void ProgramPlanUpdateTimeSet(void)
{
  tSTime STimeNow = { 0 };

  TimeGet(&STimeNow);
  TimeEpochCalculate(&STimeNow, &SRuntimes.lPlanLastChangeTime);
}

uint8_t ProgramPlanUpdateTimeWrite(void)
{
  return DataPersistenceWrite(PERSIST_OBJECT_PROGRAM_LAST_CHANGE_TIME,
                              0U,
                              &SRuntimes.lPlanLastChangeTime,
                              sizeof(void *));
}

uint8_t ProgramDataGet(void)
{
  if (ProgramDataRead() == FALSE)
  {
    LogRequest(LOG_REQ_APPEND_ASYNCH,
               NULL,
               EVENT_MAIN_STORAGE_GET_ERROR,
               0,
               0,
               FLASH_STORAGE_ADDR_SCP,
               0);

    return FALSE;
  }

  ProgramPlanUpdateTimeRead();

  if ((GetSOTotal() > SIGNAL_OUTPUTS_MAX)
      || (SGTotalGet() > SIGNAL_GROUPS_MAX) || (SeqTotalGet() > SIGNAL_SEQS_MAX)
      || (PhaseTotalGet() > PHASES_MAX)
      || (SignalPlanTotalGet() > SIGNAL_PLANS_MAX)
      || (WorkPlanTotalGet() > WORK_PLANS_MAX)
      || (SigProgPlanTotalGet() > SIGNAL_PROGRAM_PLANS_MAX)
      || (SigProgTotalGet() > SIGNAL_PROGRAMS_MAX)
      || (WorkScheduleTotalGet() > WORK_SCHEDULE_ENTRIES_MAX)
      || (InputTotalGet(INPUT_TYPE_DETECTOR) > INPUTS_DETECTOR_MAX)
      || (InputTotalGet(INPUT_TYPE_DIGITAL) > INPUTS_DIGITAL_MAX)
      || (OutputTotalGet() > OUTPUTS_MAX))
  {
    return FALSE;
  }

  if (DataChecksumIsCorrect() == FALSE)
  {
    return FALSE;
  }

  DataChecksumTotalSet(DataChecksumTotalCalculate());

  return TRUE;
}

void ProgramRelativeDataInit(void)
{
  /* The following initialization must be done after reading program from Flash */
  GTMInit();

  GCSetASCModuleID(DataChecksumTotalGet());

  SetRuntimeSSMStatus();

  ASCChannelInit();

  ASCDetectorInit();
}

void DataValidate(void)
{
  #if defined(CHECK_DEVICE_UID)
  if (!CheckDeviceUIDs())
  {
    /* Legacy LCD state machine is removed from the active UI path. */
  }

  #endif

  /* Init User Settings */
  UserSettingsRead();
  if (!IsUserSettingsChanged())
  {
    UserSettingsInit();
  }

  SetExternalBatteryState(UserSettingsStandbyFlagGet());

  UserStateReqRead();
  if (!UserStateIsValid())
  {
    UserStateReqInit();
    UserStateReqWrite();
  }

  /* Init Log Settings */
  LogSettingsRead();
  if (!IsLogSettingsChanged())
  {
    LogSettingsInit();
  }

  ReadFunctionConf();
  if (GetFunctionConf() == UINT8_MAX)
  {
    SetFunctionConf(0);
    WriteFunctionConf();
  }

  /* Check DST Flag */
  ReadDaylightSavingTimeFlag();
  CheckDaylightSavingTimeFlag();

  /* Init System Start Time */
  SystemStartTimeRead();
  if (!IsSystemStartTimeWritten())
  {
    SystemStartTimeInit();
  }

  /* Init Broken Loop Settings */
  BrokenInputSettingsRead();
  if (!IsBrokenInputSettingsSet())
  {
    BrokenInputSettingsInit();
  }

  /* Init Server Settings */
  ServerSettingsRead();
  if (!IsServerSettingsSet())
  {
    ServerSettingsInit();
  }

  /* Set LRLF Detect Time */
  LRLFDetectTimeCheck();

  ReadSOPowers();
} /* DataValidate */

uint8_t ProgramDataSet(void)
{
  DataChecksumCalculate(&(SCP.SChecksum));

  DataChecksumTotalSet(DataChecksumTotalCalculate());

  GCSetASCModuleID(DataChecksumTotalGet());

  if (ProgramDataStartMagicRead())
  {
    if ((ProgramDataStartMagicGet(0) == SCP_START_MAGIC)
        && (ProgramDataStartMagicGet(SCP_MAGIC_MAX - 1) == SCP_START_MAGIC) )
    {
      if (!ProgramDataErase())
      {
        return FALSE;
      }
    }
  }

  ProgramDataStartMagicSet();
  if (!ProgramDataWrite())
  {
    LogRequest(LOG_REQ_APPEND_ASYNCH,
               NULL,
               EVENT_MAIN_STORAGE_SET_ERROR,
               0,
               0,
               FLASH_STORAGE_ADDR_SCP,
               0);

    return FALSE;
  }

  ProgramPlanUpdateTimeSet();

  ProgramPlanUpdateTimeWrite();

  return TRUE;
}

void DataRuntimeInit(void)
{
  uint8_t bSGNo = 0;

  memset(&SRuntimes.SaSGRuntimes, 0, sizeof(SRuntimes.SaSGRuntimes));
  memset(&SRuntimes.SaPhaseRuntimes, 0, sizeof(SRuntimes.SaPhaseRuntimes));
  memset(&SRuntimes.SSeqRuntime, 0, sizeof(SRuntimes.SSeqRuntime));

  for (bSGNo = 0; bSGNo < SIGNAL_GROUPS_MAX; bSGNo++)
  {
    SRuntimes.SaSGRuntimes[bSGNo].bIdx = bSGNo + 1;
    SRuntimes.SaSGRuntimes[bSGNo].USORedFaults.SSORedFaults.fNoFaults = 1;
    SRuntimes.SaSGRuntimes[bSGNo].USOYellowFaults.SSOYellowFaults.fNoFaults = 1;
    SRuntimes.SaSGRuntimes[bSGNo].USOGreenFaults.SSOGreenFaults.fNoFaults = 1;
  }
}

void DataChecksumCalculate(tpSChecksum pSChecksum)
{
  /* only calculate checksum for the meaningful/consumed data */
  pSChecksum->bDeviceInfo = ByteChecksum(&SCP.SDevInfo, sizeof(SCP.SDevInfo));
  pSChecksum->bSignalDefs = ByteChecksum(&SCP.SaSignalDefs,
                                         sizeof(SCP.SaSignalDefs));
  pSChecksum->bSODefs = ByteChecksum(&SCP.SaSODefs, sizeof(SCP.SaSODefs));
  pSChecksum->bSGDefs = ByteChecksum(&SCP.SaSGDefs, sizeof(SCP.SaSGDefs));
  pSChecksum->bConflictsEM = ByteChecksum(&SCP.SConflictsEM,
                                          sizeof(SCP.SConflictsEM));
  pSChecksum->bPhaseDefs = ByteChecksum(&SCP.SaPhaseDefs,
                                        sizeof(SCP.SaPhaseDefs));
  pSChecksum->bDedectors = ByteChecksum(&SCP.SaDetectorDefs,
                                        sizeof(SCP.SaDetectorDefs));
  pSChecksum->bInputs = ByteChecksum(&SCP.SaInputDefs, sizeof(SCP.SaInputDefs));
  pSChecksum->bOutputs = ByteChecksum(&SCP.SaOutputDefs,
                                      sizeof(SCP.SaOutputDefs));
  pSChecksum->bSignalPlans = ByteChecksum(&SCP.SaSignalPlans,
                                          sizeof(SCP.SaSignalPlans));
  pSChecksum->bWSDef = ByteChecksum(&SCP.SaWorkSchedule,
                                    sizeof(SCP.SaWorkSchedule));
  pSChecksum->bCVSDefs = ByteChecksum(&SCP.SCVSDef, sizeof(SCP.SCVSDef));
  pSChecksum->bFlashPeriods = ByteChecksum(&SCP.SFlashPeriods,
                                           sizeof(tSFlashPeriods));
  pSChecksum->bConsumed = ByteChecksum(&SCP.SConsumed, sizeof(SCP.SConsumed));
}

uint16_t DataChecksumTotalCalculate(void)
{
  uint16_t sTotal = 0;

  /* Consumed data checksum total */
  sTotal += SCP.SChecksum.bDeviceInfo;
  sTotal += SCP.SChecksum.bSignalDefs;
  sTotal += SCP.SChecksum.bSODefs;
  sTotal += SCP.SChecksum.bSGDefs;
  sTotal += SCP.SChecksum.bConflictsEM;
  sTotal += SCP.SChecksum.bPhaseDefs;
  sTotal += SCP.SChecksum.bDedectors;
  sTotal += SCP.SChecksum.bInputs;
  sTotal += SCP.SChecksum.bOutputs;
  sTotal += SCP.SChecksum.bSignalPlans;
  sTotal += SCP.SChecksum.bWSDef;
  sTotal += SCP.SChecksum.bCVSDefs;
  sTotal += SCP.SChecksum.bFlashPeriods;
  sTotal += SCP.SChecksum.bConsumed;

  return sTotal;
}

void DataChecksumTotalSet(uint16_t sSum)
{
  SCPRuntime.sDataChecksumTotal = sSum;
}

uint16_t DataChecksumTotalGet(void)
{
  return SCPRuntime.sDataChecksumTotal;
}

void DataChecksumGet(tpSChecksum pSChecksum)
{
  memcpy(pSChecksum, &SCP.SChecksum, sizeof(tSChecksum));
}

int8_t DataChecksumIsCorrect(void)
{
  uint8_t bIndex;
  tSChecksum SChecksumCur;
  uint32_t lLogParam = 0;

  /* read sequences from storage, while reading, cheksum control is already done */
  for (bIndex = 0; bIndex < SeqTotalGet(); bIndex++)
  {
    if (SeqGet(bIndex) == FALSE)
    {
      lLogParam |= EVENT_PARAM_CHECKSUM_FLASH_ERROR_SeqDefs;
      break;
    }
  }

  /* calculate checksum of current data */
  memset(&SChecksumCur, 0, sizeof(SChecksumCur));

  DataChecksumCalculate(&SChecksumCur);

  /* other comparisions */
  if (SChecksumCur.bConflictsEM != SCP.SChecksum.bConflictsEM)
  {
    lLogParam |= EVENT_PARAM_CHECKSUM_FLASH_ERROR_ConflictsEM;
  }

  if (SChecksumCur.bDeviceInfo != SCP.SChecksum.bDeviceInfo)
  {
    lLogParam |= EVENT_PARAM_CHECKSUM_FLASH_ERROR_DeviceInfo;
  }

  if (SChecksumCur.bFlashPeriods != SCP.SChecksum.bFlashPeriods)
  {
    lLogParam |= EVENT_PARAM_CHECKSUM_FLASH_ERROR_FlashPeriods;
  }

  if (SChecksumCur.bPhaseDefs != SCP.SChecksum.bPhaseDefs)
  {
    lLogParam |= EVENT_PARAM_CHECKSUM_FLASH_ERROR_PhaseDefs;
  }

  if (SChecksumCur.bSGDefs != SCP.SChecksum.bSGDefs)
  {
    lLogParam |= EVENT_PARAM_CHECKSUM_FLASH_ERROR_SGDefs;
  }

  if (SChecksumCur.bSignalDefs != SCP.SChecksum.bSignalDefs)
  {
    lLogParam |= EVENT_PARAM_CHECKSUM_FLASH_ERROR_SignalDefs;
  }

  if (SChecksumCur.bSODefs != SCP.SChecksum.bSODefs)
  {
    lLogParam |= EVENT_PARAM_CHECKSUM_FLASH_ERROR_SODefs;
  }

  if (SChecksumCur.bConsumed != SCP.SChecksum.bConsumed)
  {
    lLogParam |= EVENT_PARAM_CHECKSUM_FLASH_ERROR_Consumed;
  }

  if (SChecksumCur.bSignalPlans != SCP.SChecksum.bSignalPlans)
  {
    lLogParam |= EVENT_PARAM_CHECKSUM_FLASH_ERROR_SignalPlans;
  }

  if (SChecksumCur.bWSDef != SCP.SChecksum.bWSDef)
  {
    lLogParam |= EVENT_PARAM_CHECKSUM_FLASH_ERROR_WSDef;
  }

  if (SChecksumCur.bCVSDefs != SCP.SChecksum.bCVSDefs)
  {
    lLogParam |= EVENT_PARAM_CHECKSUM_FLASH_ERROR_CVS;
  }

  if (SChecksumCur.bDedectors != SCP.SChecksum.bDedectors)
  {
    lLogParam |= EVENT_PARAM_CHECKSUM_FLASH_ERROR_Detectors;
  }

  if (SChecksumCur.bInputs != SCP.SChecksum.bInputs)
  {
    lLogParam |= EVENT_PARAM_CHECKSUM_FLASH_ERROR_Inputs;
  }

  if (SChecksumCur.bOutputs != SCP.SChecksum.bOutputs)
  {
    lLogParam |= EVENT_PARAM_CHECKSUM_FLASH_ERROR_Outputs;
  }

  if (lLogParam != 0)
  {
    LogRequest(LOG_REQ_APPEND_ASYNCH,
               NULL,
               EVENT_CHECKSUM_FLASH_ERROR,
               0,
               0,
               lLogParam,
               0);

    return FALSE;
  }

  return TRUE;
} /* DataChecksumIsCorrect */

void ReturnFactorySettings(void)
{
  DataInit(FALSE, TRUE);
  ProgramDataSet();

  (void) UiLanguageServiceSet(&g_uiLanguageService, LANGUAGE_TURKISH);
  (void) UiLanguageServiceSave(&g_uiLanguageService);

  LRLFDetectTimeSet(LRLF_DETECT_TIME_800_MS);
  LRLFDetectTimeWrite();

  SecureSystemReset();
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  MP Events */
void EventMPCont(tpSEvent pSEvent)
{
  switch (pSEvent->bEvent)
  {
      case EVENT_SIGNAL_AT_SG:
      {
        break;
      }

      /* ------------------------------------- Invalid Signal */
      /* ----------------------------------------------------------- */
      case EVENT_INVALID_SIGNAL:
      {
        if (GetInvalidSignalEM())
        {
          SErrInfo.SErrInvalidSignal.fError = TRUE;
          SErrInfo.SErrInvalidSignal.bSGNo = pSEvent->bParam;
          SErrInfo.SErrInvalidSignal.bDisplayedSignal = pSEvent->sParam;

          if (GetBitValue(pSEvent->sParam,
                          SUBSIGNAL_RED) && !GetBitValue(pSEvent->lParam,
                                                         SUBSIGNAL_RED))
          {
            GetSRuntimePtr()->SaChannelErrors[(pSEvent->bParam - 1)
                                              / 8].bUndesiredRedLamp |=
              laValue2Bit[(pSEvent->bParam
                           -
                           1)
                          %
                          8];
          }

          if (GetBitValue(pSEvent->sParam,
                          SUBSIGNAL_YELLOW) && !GetBitValue(pSEvent->lParam,
                                                            SUBSIGNAL_YELLOW))
          {
            GetSRuntimePtr()->SaChannelErrors[(pSEvent->bParam - 1)
                                              / 8].bUndesiredYellowLamp |=
              laValue2Bit[(pSEvent
                           ->
                           bParam
                           -
                           1)
                          %
                          8];
          }

          if (GetBitValue(pSEvent->sParam,
                          SUBSIGNAL_GREEN) && !GetBitValue(pSEvent->lParam,
                                                           SUBSIGNAL_GREEN))
          {
            GetSRuntimePtr()->SaChannelErrors[(pSEvent->bParam - 1)
                                              / 8].bUndesiredGreenLamp |=
              laValue2Bit[(pSEvent
                           ->
                           bParam
                           -
                           1)
                          %
                          8];
          }

          ApplyEMToOneSet(GetSGOwner(pSEvent->bParam - 1), GetInvalidSignalEM(),
                          EVENT_INVALID_SIGNAL,
                          (pSEvent->bParam), (pSEvent->sParam));
          SetInvalidSignalState(GetSGOwner(pSEvent->bParam - 1), TRUE);
        }

        LogRequest(LOG_REQ_APPEND_ASYNCH,
                   NULL,
                   pSEvent->bEvent,
                   pSEvent->bParam,
                   pSEvent->sParam,
                   pSEvent->lParam,
                   0);
        break;
      }

      /* ------------------------------------- Invalid Signal Sequence */
      /* -------------------------------------------------- */

      case EVENT_INVALID_SIGNAL_SEQUENCE:
      {
        if (GetInvalidSignalSequenceEM())
        {
          SErrInfo.SErrInvalidSigSequence.fError = TRUE;
          SErrInfo.SErrInvalidSigSequence.bSGNo = pSEvent->bParam;
          SErrInfo.SErrInvalidSigSequence.bPreviousSignal = pSEvent->sParam;
          SErrInfo.SErrInvalidSigSequence.bDisplayedSignal = pSEvent->lParam;
          ApplyEMToOneSet(GetSGOwner(pSEvent->bParam - 1),
                          GetInvalidSignalSequenceEM(),
                          EVENT_INVALID_SIGNAL_SEQUENCE,
                          (pSEvent->bParam),
                          0);
          SetInvalidSignalSequenceState(GetSGOwner(pSEvent->bParam - 1), TRUE);
        }

        LogRequest(LOG_REQ_APPEND_ASYNCH,
                   NULL,
                   pSEvent->bEvent,
                   pSEvent->bParam,
                   pSEvent->sParam,
                   pSEvent->lParam,
                   0);
        break;
      }

      /* ------------------------------------- so definition update */
      /* ----------------------------------------------------- */

      case EVENT_SO_POWER_RECORD:
      {
        tSSOPowerRecord SSOPowerRecord;

        /* this log is used to update the signal output definition with its */
        /* measured power record */
        SCP.SaSODefs[pSEvent->bParam
                     - 1].sPower[(GetLampDimmingState()) ? 0 : 1] =
          pSEvent->sParam;
        SCP.SaSODefs[pSEvent->bParam
                     - 1].sPowerRecordNet[(GetLampDimmingState()) ? 0 : 1] =
          (uint16_t) pSEvent->lParam;

        if (GetLampDimmingState())
        {
          SCP.SaSODefs[pSEvent->bParam - 1].SFlags.bPowerRecorded0 = TRUE;
        }
        else
        {
          SCP.SaSODefs[pSEvent->bParam - 1].SFlags.bPowerRecorded1 = TRUE;
        }

        SSOPowerRecord.sPower[0] = SCP.SaSODefs[pSEvent->bParam - 1].sPower[0];
        SSOPowerRecord.sPower[1] = SCP.SaSODefs[pSEvent->bParam - 1].sPower[0];
        SSOPowerRecord.sPowerNet[0] = SCP.SaSODefs[pSEvent->bParam
                                                   - 1].sPowerRecordNet[0];
        SSOPowerRecord.sPowerNet[1] = SCP.SaSODefs[pSEvent->bParam
                                                   - 1].sPowerRecordNet[1];

        SSOPowerRecord.SFlags.bPowerRecorded0 = SCP.SaSODefs[pSEvent->bParam
                                                             - 1].SFlags.
                                                bPowerRecorded0;
        SSOPowerRecord.SFlags.bPowerRecorded1 = SCP.SaSODefs[pSEvent->bParam
                                                             - 1].SFlags.
                                                bPowerRecorded1;

        if (!WriteSOPower((pSEvent->bParam - 1), &SSOPowerRecord))
        {
          LogRequest(LOG_REQ_APPEND_ASYNCH,
                     NULL,
                     EVENT_MAIN_STORAGE_SET_ERROR,
                     0,
                     0,
                     EEPROM_STORAGE_ADDR_SO_POWERS
                     + (sizeof(tSSOPowerRecord) * (pSEvent->bParam - 1)),
                     0);
        }

        LogRequest(LOG_REQ_APPEND_ASYNCH,
                   NULL,
                   pSEvent->bEvent,
                   pSEvent->bParam,
                   pSEvent->sParam,
                   pSEvent->lParam,
                   0);
        break;
      }

      /* ------------------------------------- module communication */
      /* ------------------------------------------------------ */

      case EVENT_MODULE_MISSING:
      {
        if (pSEvent->bParam <= SIG_DEV_SSM_MAX)
        {
          uint8_t bSONo = 0;
          uint8_t bFirstOutput = ((pSEvent->bParam - 1)
                                  * SIGNAL_OUTPUTS_PER_SSM);
          uint8_t bLastOutput = ((pSEvent->bParam) * SIGNAL_OUTPUTS_PER_SSM);

          if ((SErrInfo.bErrSSM & laValue2Bit[pSEvent->bParam - 1]) >>
              (pSEvent->bParam - 1) == 0)
          {
            LogRequest(LOG_REQ_APPEND_ASYNCH,
                       NULL,
                       pSEvent->bEvent,
                       pSEvent->bParam,
                       pSEvent->sParam,
                       pSEvent->lParam,
                       0);
            SErrInfo.bErrSSM |= laValue2Bit[pSEvent->bParam - 1];
            SRuntimes.sSSMStatus &= ~(laValue2Bit[pSEvent->bParam - 1]);

            SNMPSendSSMStatusTrap(SRuntimes.sSSMStatus);
          }

          /* all sets that have an output on this ssm will have emergency dark */
          for (bSONo = bFirstOutput; bSONo < bLastOutput; bSONo++)
          {
            if (SCP.SaSODefs[bSONo].bOwner)
            {
              uint8_t bSetNo = SCP.SaSGDefs[(SCP.SaSODefs[bSONo].bOwner
                                             - 1)].bOwner;

              RelayStateRequestSet(FALSE);
              ApplyEMToOneSet(bSetNo,
                              EMERGENCY_METHOD_DARK,
                              EVENT_MODULE_MISSING,
                              pSEvent->bParam,
                              0);
            }
          }
        }
        else if (pSEvent->bParam <= SIG_DEV_PSM_LAST)
        {
          LogRequest(LOG_REQ_APPEND_ASYNCH,
                     NULL,
                     pSEvent->bEvent,
                     pSEvent->bParam,
                     pSEvent->sParam,
                     pSEvent->lParam,
                     0);
          SErrInfo.bErrPSM |= laValue2Bit[pSEvent->bParam - SIG_DEV_PSM_FIRST];
        }

        break;
      }

      case EVENT_MODULE_RESPONDS:
      {
        if ((pSEvent->bParam >= SIG_DEV_SSM_1)
            && (pSEvent->bParam <= SIG_DEV_PSM_LAST) )
        {
          LogRequest(LOG_REQ_APPEND_ASYNCH,
                     NULL,
                     pSEvent->bEvent,
                     pSEvent->bParam,
                     pSEvent->sParam,
                     pSEvent->lParam,
                     0);

          if (pSEvent->bParam <= SIG_DEV_SSM_MAX)
          {
            SErrInfo.bErrSSM &= ~(laValue2Bit[pSEvent->bParam - 1]);
            SRuntimes.sSSMStatus |= laValue2Bit[pSEvent->bParam - 1];

            if ((SErrInfo.bErrSSM & 0xFF) == 0)
            {
              RelayStateRequestSet(TRUE);
              RestartProgram();
            }

            SNMPSendSSMStatusTrap(SRuntimes.sSSMStatus);
          }
          else if (pSEvent->bParam <= SIG_DEV_PSM_LAST)
          {
            SErrInfo.bErrPSM &= ~(laValue2Bit[pSEvent->bParam
                                              - SIG_DEV_PSM_FIRST]);
          }
        }

        break;
      }

      /* -------------------------------------- Conflicts */
      /* ------------------------------------------------------------------- */

      case EVENT_YELLOW_YELLOW_CONFLICT:
      {
        if (GetYYEM())
        {
          SErrInfo.SErrConflict.fError = TRUE;
          SErrInfo.SErrConflict.bType = pSEvent->bEvent
                                        - EVENT_YELLOW_YELLOW_CONFLICT;
          SErrInfo.SErrConflict.bSG1 = pSEvent->bParam;
          SErrInfo.SErrConflict.bSG2 = pSEvent->sParam;

          GetSRuntimePtr()->SaSGRuntimes[pSEvent->bParam
                                         - 1].USOYellowFaults.SSOYellowFaults.
          fNoFaults = 0;
          GetSRuntimePtr()->SaSGRuntimes[pSEvent->bParam
                                         - 1].USOYellowFaults.SSOYellowFaults.
          fConflict = 1;
          GetSRuntimePtr()->SaSGRuntimes[pSEvent->sParam
                                         - 1].USOYellowFaults.SSOYellowFaults.
          fNoFaults = 0;
          GetSRuntimePtr()->SaSGRuntimes[pSEvent->sParam
                                         - 1].USOYellowFaults.SSOYellowFaults.
          fConflict = 1;

          ApplyEMToOneSet(GetSGOwner(pSEvent->bParam - 1), GetYYEM(),
                          EVENT_YELLOW_YELLOW_CONFLICT,
                          (pSEvent->bParam), (pSEvent->sParam));
        }

        LogRequest(LOG_REQ_APPEND_ASYNCH,
                   NULL,
                   pSEvent->bEvent,
                   pSEvent->bParam,
                   pSEvent->sParam,
                   pSEvent->lParam,
                   0);
        break;
      }

      case EVENT_YELLOW_GREEN_CONFLICT:
      {
        if (GetYGEM())
        {
          SErrInfo.SErrConflict.fError = TRUE;
          SErrInfo.SErrConflict.bType = pSEvent->bEvent
                                        - EVENT_YELLOW_YELLOW_CONFLICT;
          SErrInfo.SErrConflict.bSG1 = pSEvent->bParam;
          SErrInfo.SErrConflict.bSG2 = pSEvent->sParam;

          GetSRuntimePtr()->SaSGRuntimes[pSEvent->bParam
                                         - 1].USOYellowFaults.SSOYellowFaults.
          fNoFaults = 0;
          GetSRuntimePtr()->SaSGRuntimes[pSEvent->bParam
                                         - 1].USOYellowFaults.SSOYellowFaults.
          fConflict = 1;
          GetSRuntimePtr()->SaSGRuntimes[pSEvent->sParam
                                         - 1].USOGreenFaults.SSOGreenFaults.
          fNoFaults = 0;
          GetSRuntimePtr()->SaSGRuntimes[pSEvent->sParam
                                         - 1].USOGreenFaults.SSOGreenFaults.
          fConflict = 1;

          ApplyEMToOneSet(GetSGOwner(pSEvent->bParam - 1), GetYGEM(),
                          EVENT_YELLOW_GREEN_CONFLICT,
                          (pSEvent->bParam), (pSEvent->sParam));
        }

        LogRequest(LOG_REQ_APPEND_ASYNCH,
                   NULL,
                   pSEvent->bEvent,
                   pSEvent->bParam,
                   pSEvent->sParam,
                   pSEvent->lParam,
                   0);
        break;
      }

      case EVENT_GREEN_GREEN_CONFLICT:
      {
        if (GetGGEM())
        {
          SErrInfo.SErrConflict.fError = TRUE;
          SErrInfo.SErrConflict.bType = pSEvent->bEvent
                                        - EVENT_YELLOW_YELLOW_CONFLICT;
          SErrInfo.SErrConflict.bSG1 = pSEvent->bParam;
          SErrInfo.SErrConflict.bSG2 = pSEvent->sParam;

          GetSRuntimePtr()->SaSGRuntimes[pSEvent->bParam
                                         - 1].USOGreenFaults.SSOGreenFaults.
          fNoFaults = 0;
          GetSRuntimePtr()->SaSGRuntimes[pSEvent->bParam
                                         - 1].USOGreenFaults.SSOGreenFaults.
          fConflict = 1;
          GetSRuntimePtr()->SaSGRuntimes[pSEvent->sParam
                                         - 1].USOGreenFaults.SSOGreenFaults.
          fNoFaults = 0;
          GetSRuntimePtr()->SaSGRuntimes[pSEvent->sParam
                                         - 1].USOGreenFaults.SSOGreenFaults.
          fConflict = 1;

          ApplyEMToOneSet(GetSGOwner(pSEvent->bParam - 1), GetGGEM(),
                          EVENT_GREEN_GREEN_CONFLICT,
                          (pSEvent->bParam), (pSEvent->sParam));
        }

        LogRequest(LOG_REQ_APPEND_ASYNCH,
                   NULL,
                   pSEvent->bEvent,
                   pSEvent->bParam,
                   pSEvent->sParam,
                   pSEvent->lParam,
                   0);
        break;
      }

      /* ---------------------------------------- Frequency */
      /* -------------------------------------------------------------------- */

      case EVENT_FREQUENCY_VALUE_LOWER_BOUND:
      {
        if (GetFrequencyErrorEM())
        {
          SErrInfo.bErrFreq |= laValue2Bit[0];
          ApplyEMToAllSets(GetFrequencyErrorEM(),
                           EVENT_FREQUENCY_VALUE_LOWER_BOUND);
        }

        SetFrequencyState(EVENT_FREQUENCY_VALUE_LOWER_BOUND);
        LogRequest(LOG_REQ_APPEND_ASYNCH,
                   NULL,
                   pSEvent->bEvent,
                   pSEvent->bParam,
                   pSEvent->sParam,
                   pSEvent->lParam,
                   0);
        break;
      }

      case EVENT_FREQUENCY_VALUE_UPPER_BOUND:
      {
        if (GetFrequencyErrorEM())
        {
          SErrInfo.bErrFreq |= laValue2Bit[1];
          ApplyEMToAllSets(GetFrequencyErrorEM(),
                           EVENT_FREQUENCY_VALUE_UPPER_BOUND);
        }

        SetFrequencyState(EVENT_FREQUENCY_VALUE_UPPER_BOUND);
        LogRequest(LOG_REQ_APPEND_ASYNCH,
                   NULL,
                   pSEvent->bEvent,
                   pSEvent->bParam,
                   pSEvent->sParam,
                   pSEvent->lParam,
                   0);
        break;
      }

      case EVENT_FREQUENCY_VALUE_NORMAL:
      {
        SErrInfo.bErrFreq = 0;

        if (((GetFrequencyState() == EVENT_FREQUENCY_VALUE_LOWER_BOUND)
             || (GetFrequencyState() == EVENT_FREQUENCY_VALUE_UPPER_BOUND) )
            && GetFrequencyErrorEM())
        {
          RestartProgram();
        }

        SetFrequencyState(EVENT_FREQUENCY_VALUE_NORMAL);
        LogRequest(LOG_REQ_APPEND_ASYNCH,
                   NULL,
                   pSEvent->bEvent,
                   pSEvent->bParam,
                   pSEvent->sParam,
                   pSEvent->lParam,
                   0);
        break;
      }

      /* ----------------------------------------- Voltage */
      /* --------------------------------------------------------------------------- */

      case EVENT_VOLTAGE_VALUE_LOWER_BOUND:
      {
        if (GetVoltageLimitsEM())
        {
          SErrInfo.bErrVoltage |= laValue2Bit[0];
          ApplyEMToAllSets(GetVoltageLimitsEM(),
                           EVENT_VOLTAGE_VALUE_LOWER_BOUND);
        }

        SetVoltageState(EVENT_VOLTAGE_VALUE_LOWER_BOUND);
        LogRequest(LOG_REQ_APPEND_ASYNCH,
                   NULL,
                   pSEvent->bEvent,
                   pSEvent->bParam,
                   pSEvent->sParam,
                   pSEvent->lParam,
                   0);
        break;
      }

      case EVENT_VOLTAGE_VALUE_UPPER_BOUND:
      {
        if (GetVoltageLimitsEM())
        {
          SErrInfo.bErrVoltage |= laValue2Bit[1];
          ApplyEMToAllSets(GetVoltageLimitsEM(),
                           EVENT_VOLTAGE_VALUE_UPPER_BOUND);
        }

        SetVoltageState(EVENT_VOLTAGE_VALUE_UPPER_BOUND);
        LogRequest(LOG_REQ_APPEND_ASYNCH,
                   NULL,
                   pSEvent->bEvent,
                   pSEvent->bParam,
                   pSEvent->sParam,
                   pSEvent->lParam,
                   0);
        break;
      }

      case EVENT_VOLTAGE_VALUE_NORMAL:
      {
        SErrInfo.bErrVoltage = 0;

        if (((GetVoltageState() == EVENT_VOLTAGE_VALUE_LOWER_BOUND)
             || (GetVoltageState() == EVENT_VOLTAGE_VALUE_UPPER_BOUND) )
            && GetVoltageLimitsEM())
        {
          RestartProgram();
        }

        SetVoltageState(EVENT_VOLTAGE_VALUE_NORMAL);
        LogRequest(LOG_REQ_APPEND_ASYNCH,
                   NULL,
                   pSEvent->bEvent,
                   pSEvent->bParam,
                   pSEvent->sParam,
                   pSEvent->lParam,
                   0);
        break;
      }

      /* ------------------------------------- lamp failures */
      /* -------------------------------------------------------------- */

      case EVENT_SO_WORKING_LAMP_TOTAL_CHANGE:
      {
        uint8_t bOwnerSG = GetSOOwner(pSEvent->bParam - 1);

        if ((GetSOEM(pSEvent->bParam - 1) != EMERGENCY_METHOD_NONE)
            && (pSEvent->sParam == 0))
        {
          ApplyEMToOneSet(GetSGOwner(bOwnerSG - 1),
                          GetSOEM((pSEvent->bParam - 1)),
                          EVENT_SO_WORKING_LAMP_TOTAL_CHANGE,
                          (pSEvent->bParam), 0);
        }

        /* the first parameter is so number */
        /* the second parameter is the current number of working lamps (sParam) */
        /* the third parameter is the previous number of working lamps (lParam) */
        switch (SCP.SaSODefs[pSEvent->bParam - 1].bType)
        {
            case SIGNAL_OUTPUT_TYPE_RED:
            {
              if (pSEvent->sParam
                  < SCP.SaSODefs[pSEvent->bParam - 1].bNoOfLamps)
              {
                SErrInfo.SErrLampFailRed.fError = TRUE;
                SErrInfo.SErrLampFailRed.bOwnerSG = bOwnerSG;

                GetSRuntimePtr()->SaSGRuntimes[bOwnerSG
                                               - 1].USORedFaults.SSORedFaults.
                fNoFaults = 0;
                GetSRuntimePtr()->SaSGRuntimes[bOwnerSG
                                               - 1].USORedFaults.SSORedFaults.
                fLampBroken = 1;

                GetSRuntimePtr()->SaChannelErrors[(bOwnerSG - 1)
                                                  / 8].bRedLampFailure |=
                  laValue2Bit[(bOwnerSG - 1) % 8];
              }
              else
              {
                SErrInfo.SErrLampFailRed.fError = FALSE;
                SErrInfo.SErrLampFailRed.bOwnerSG = bOwnerSG;

                GetSRuntimePtr()->SaSGRuntimes[bOwnerSG
                                               - 1].USORedFaults.SSORedFaults.
                fLampBroken = 0;

                GetSRuntimePtr()->SaChannelErrors[(bOwnerSG - 1)
                                                  / 8].bRedLampFailure &=
                  ~laValue2Bit[(bOwnerSG - 1) % 8];
              }

              break;
            }

            case SIGNAL_OUTPUT_TYPE_YELLOW:
            {
              if (pSEvent->sParam
                  < SCP.SaSODefs[pSEvent->bParam - 1].bNoOfLamps)
              {
                SErrInfo.SErrLampFailYellow.fError = TRUE;
                SErrInfo.SErrLampFailYellow.bOwnerSG = bOwnerSG;

                GetSRuntimePtr()->SaSGRuntimes[bOwnerSG
                                               - 1].USOYellowFaults.
                SSOYellowFaults.fNoFaults = 0;
                GetSRuntimePtr()->SaSGRuntimes[bOwnerSG
                                               - 1].USOYellowFaults.
                SSOYellowFaults.fLampBroken = 1;

                GetSRuntimePtr()->SaChannelErrors[(bOwnerSG - 1)
                                                  / 8].bYellowLampFailure |=
                  laValue2Bit[(bOwnerSG - 1) % 8];
              }
              else
              {
                SErrInfo.SErrLampFailYellow.fError = FALSE;
                SErrInfo.SErrLampFailYellow.bOwnerSG = bOwnerSG + 1;

                GetSRuntimePtr()->SaSGRuntimes[bOwnerSG
                                               - 1].USOYellowFaults.
                SSOYellowFaults.fLampBroken = 0;

                GetSRuntimePtr()->SaChannelErrors[(bOwnerSG - 1)
                                                  / 8].bYellowLampFailure &=
                  ~laValue2Bit[(bOwnerSG - 1) % 8];
              }

              break;
            }

            case SIGNAL_OUTPUT_TYPE_GREEN:
            {
              if (pSEvent->sParam
                  < SCP.SaSODefs[pSEvent->bParam - 1].bNoOfLamps)
              {
                SErrInfo.SErrLampFailGreen.fError = TRUE;
                SErrInfo.SErrLampFailGreen.bOwnerSG = bOwnerSG;

                GetSRuntimePtr()->SaSGRuntimes[bOwnerSG
                                               - 1].USOGreenFaults.
                SSOGreenFaults.fNoFaults = 0;
                GetSRuntimePtr()->SaSGRuntimes[bOwnerSG
                                               - 1].USOGreenFaults.
                SSOGreenFaults.fLampBroken = 1;

                GetSRuntimePtr()->SaChannelErrors[(bOwnerSG - 1)
                                                  / 8].bGreenLampFailure |=
                  laValue2Bit[(bOwnerSG - 1) % 8];
              }
              else
              {
                SErrInfo.SErrLampFailGreen.fError = FALSE;
                SErrInfo.SErrLampFailGreen.bOwnerSG = bOwnerSG;

                GetSRuntimePtr()->SaSGRuntimes[bOwnerSG
                                               - 1].USOGreenFaults.
                SSOGreenFaults.fLampBroken = 0;

                GetSRuntimePtr()->SaChannelErrors[(bOwnerSG - 1)
                                                  / 8].bGreenLampFailure &=
                  ~laValue2Bit[(bOwnerSG - 1) % 8];
              }

              break;
            }
        } /* switch */

        LogRequest(LOG_REQ_APPEND_ASYNCH,
                   NULL,
                   pSEvent->bEvent,
                   pSEvent->bParam,
                   pSEvent->sParam,
                   pSEvent->lParam,
                   0);
        break;
      }

      case EVENT_SG_NUMBER_OF_RED_LAMPS_FAILURE:
      {
        if (GetSGRedLampFailureNumberEM(pSEvent->bParam - 1))
        {
          SErrInfo.SErrNoOfRedLamps.fError = TRUE;
          SErrInfo.SErrNoOfRedLamps.bOwnerSG = pSEvent->sParam;

          GetSRuntimePtr()->SaSGRuntimes[pSEvent->bParam
                                         - 1].USORedFaults.SSORedFaults.
          fNoFaults = 0;
          GetSRuntimePtr()->SaSGRuntimes[pSEvent->bParam
                                         - 1].USORedFaults.SSORedFaults.
          fLampBroken = 1;
          GetSRuntimePtr()->SaSGRuntimes[pSEvent->bParam
                                         - 1].USORedFaults.SSORedFaults.
          fCriticalLampBroken = 1;

          GetSRuntimePtr()->SaChannelErrors[(pSEvent->bParam - 1)
                                            / 8].bRedLampFailure |=
            laValue2Bit[(pSEvent->bParam - 1) % 8];

          ApplyEMToOneSet(GetSGOwner(pSEvent->bParam - 1),
                          GetSGRedLampFailureNumberEM(pSEvent->bParam - 1),
                          EVENT_SG_NUMBER_OF_RED_LAMPS_FAILURE,
                          (pSEvent->bParam),
                          0);
        }

        LogRequest(LOG_REQ_APPEND_ASYNCH,
                   NULL,
                   pSEvent->bEvent,
                   pSEvent->bParam,
                   pSEvent->sParam,
                   pSEvent->lParam,
                   0);
        break;
      }

      case EVENT_SG_LAST_RED_LAMP_FAILURE:
      {
        if (GetSGLastRedLampFailureEM(pSEvent->sParam - 1))
        {
          SErrInfo.SErrLastRedLamp.fError = TRUE;
          SErrInfo.SErrLastRedLamp.bOwnerSG = pSEvent->sParam;

          GetSRuntimePtr()->SaSGRuntimes[pSEvent->bParam
                                         - 1].USORedFaults.SSORedFaults.
          fNoFaults = 0;
          GetSRuntimePtr()->SaSGRuntimes[pSEvent->bParam
                                         - 1].USORedFaults.SSORedFaults.
          fLampBroken = 1;
          GetSRuntimePtr()->SaSGRuntimes[pSEvent->bParam
                                         - 1].USORedFaults.SSORedFaults.
          fCriticalLampBroken = 1;
          GetSRuntimePtr()->SaSGRuntimes[pSEvent->bParam
                                         - 1].USORedFaults.SSORedFaults.
          fAllLampsBroken = 1;

          GetSRuntimePtr()->SaChannelErrors[(pSEvent->bParam - 1)
                                            / 8].bRedLampFailure |=
            laValue2Bit[(pSEvent->bParam - 1) % 8];

          ApplyEMToOneSet((pSEvent->bParam - 1),
                          GetSGLastRedLampFailureEM(pSEvent->sParam - 1),
                          EVENT_SG_LAST_RED_LAMP_FAILURE,
                          (pSEvent->sParam), 0);
        }

        LogRequest(LOG_REQ_APPEND_ASYNCH,
                   NULL,
                   pSEvent->bEvent,
                   pSEvent->bParam,
                   pSEvent->sParam,
                   pSEvent->lParam,
                   0);
        break;
      }

      case EVENT_SG_ALL_RED_LAMPS_BROKEN:
      {
        /* the first parameter is sg number */
        /* all red lamps are broken at this signal group */
        SErrInfo.SErrLampFailAllRed.fError = TRUE;
        SErrInfo.SErrLampFailAllRed.bOwnerSG = pSEvent->bParam;
        SErrInfo.SErrLampFailRed.fError = TRUE;
        SErrInfo.SErrLampFailRed.bOwnerSG = pSEvent->bParam;

        GetSRuntimePtr()->SaSGRuntimes[pSEvent->bParam
                                       - 1].USORedFaults.SSORedFaults.fNoFaults
          =
            0;
        GetSRuntimePtr()->SaSGRuntimes[pSEvent->bParam
                                       - 1].USORedFaults.SSORedFaults.
        fLampBroken = 1;
        GetSRuntimePtr()->SaSGRuntimes[pSEvent->bParam
                                       - 1].USORedFaults.SSORedFaults.
        fCriticalLampBroken = 1;
        GetSRuntimePtr()->SaSGRuntimes[pSEvent->bParam
                                       - 1].USORedFaults.SSORedFaults.
        fAllLampsBroken = 1;

        GetSRuntimePtr()->SaChannelErrors[(pSEvent->bParam - 1)
                                          / 8].bRedLampFailure |=
          laValue2Bit[(pSEvent->bParam - 1)
                      %
                      8];

        LogRequest(LOG_REQ_APPEND_ASYNCH,
                   NULL,
                   EVENT_SG_ALL_RED_LAMPS_BROKEN,
                   pSEvent->bParam,
                   pSEvent->sParam,
                   pSEvent->lParam,
                   0);
        break;
      }

      case EVENT_SG_ALL_YELLOW_LAMPS_BROKEN:
      {
        /* the first parameter is sg number */
        /* all yellow lamps are broken at this signal group */
        SErrInfo.SErrLampFailAllYellow.fError = TRUE;
        SErrInfo.SErrLampFailAllYellow.bOwnerSG = pSEvent->bParam;
        SErrInfo.SErrLampFailYellow.fError = TRUE;
        SErrInfo.SErrLampFailYellow.bOwnerSG = pSEvent->bParam;

        GetSRuntimePtr()->SaSGRuntimes[pSEvent->bParam
                                       - 1].USOYellowFaults.SSOYellowFaults.
        fNoFaults = 0;
        GetSRuntimePtr()->SaSGRuntimes[pSEvent->bParam
                                       - 1].USOYellowFaults.SSOYellowFaults.
        fLampBroken = 1;
        GetSRuntimePtr()->SaSGRuntimes[pSEvent->bParam
                                       - 1].USOYellowFaults.SSOYellowFaults.
        fCriticalLampBroken = 1;
        GetSRuntimePtr()->SaSGRuntimes[pSEvent->bParam
                                       - 1].USOYellowFaults.SSOYellowFaults.
        fAllLampsBroken = 1;

        GetSRuntimePtr()->SaChannelErrors[(pSEvent->bParam - 1)
                                          / 8].bYellowLampFailure |=
          laValue2Bit[(pSEvent->bParam
                       -
                       1)
                      %
                      8];

        LogRequest(LOG_REQ_APPEND_ASYNCH,
                   NULL,
                   EVENT_SG_ALL_YELLOW_LAMPS_BROKEN,
                   pSEvent->bParam,
                   pSEvent->sParam,
                   pSEvent->lParam,
                   0);
        break;
      }

      case EVENT_SG_ALL_GREEN_LAMPS_BROKEN:
      {
        /* the first parameter is sg number */
        /* all green lamps are broken at this signal group */
        SErrInfo.SErrLampFailAllGreen.fError = TRUE;
        SErrInfo.SErrLampFailAllGreen.bOwnerSG = pSEvent->bParam;
        SErrInfo.SErrLampFailGreen.fError = TRUE;
        SErrInfo.SErrLampFailGreen.bOwnerSG = pSEvent->bParam;

        GetSRuntimePtr()->SaSGRuntimes[pSEvent->bParam
                                       - 1].USOGreenFaults.SSOGreenFaults.
        fNoFaults = 0;
        GetSRuntimePtr()->SaSGRuntimes[pSEvent->bParam
                                       - 1].USOGreenFaults.SSOGreenFaults.
        fLampBroken = 1;
        GetSRuntimePtr()->SaSGRuntimes[pSEvent->bParam
                                       - 1].USOGreenFaults.SSOGreenFaults.
        fCriticalLampBroken = 1;
        GetSRuntimePtr()->SaSGRuntimes[pSEvent->bParam
                                       - 1].USOGreenFaults.SSOGreenFaults.
        fAllLampsBroken = 1;

        GetSRuntimePtr()->SaChannelErrors[(pSEvent->bParam - 1)
                                          / 8].bGreenLampFailure |=
          laValue2Bit[(pSEvent->bParam - 1) % 8];

        LogRequest(LOG_REQ_APPEND_ASYNCH,
                   NULL,
                   EVENT_SG_ALL_GREEN_LAMPS_BROKEN,
                   pSEvent->bParam,
                   pSEvent->sParam,
                   pSEvent->lParam,
                   0);
        break;
      }

      case EVENT_SG_ALL_RED_LAMPS_SAFE:
      {
        /* the first parameter is sg number */
        /* all red lamps are safe at this output */
        SErrInfo.SErrLampFailAllRed.fError = FALSE;
        SErrInfo.SErrLampFailAllRed.bOwnerSG = pSEvent->bParam;
        SErrInfo.SErrLampFailRed.fError = FALSE;
        SErrInfo.SErrLampFailRed.bOwnerSG = pSEvent->bParam;

        GetSRuntimePtr()->SaSGRuntimes[pSEvent->bParam
                                       - 1].USORedFaults.SSORedFaults.
        fLampBroken = 0;
        GetSRuntimePtr()->SaSGRuntimes[pSEvent->bParam
                                       - 1].USORedFaults.SSORedFaults.
        fCriticalLampBroken = 0;
        GetSRuntimePtr()->SaSGRuntimes[pSEvent->bParam
                                       - 1].USORedFaults.SSORedFaults.
        fAllLampsBroken = 0;

        GetSRuntimePtr()->SaChannelErrors[(pSEvent->bParam - 1)
                                          / 8].bRedLampFailure &=
          ~laValue2Bit[(pSEvent->bParam - 1)
                       %
                       8];

        LogRequest(LOG_REQ_APPEND_ASYNCH,
                   NULL,
                   EVENT_SG_ALL_RED_LAMPS_SAFE,
                   pSEvent->bParam,
                   pSEvent->sParam,
                   pSEvent->lParam,
                   0);
        break;
      }

      case EVENT_SG_ALL_YELLOW_LAMPS_SAFE:
      {
        /* the first parameter is sg number */
        /* all yellow lamps are safe at this output */
        SErrInfo.SErrLampFailAllYellow.fError = FALSE;
        SErrInfo.SErrLampFailAllYellow.bOwnerSG = pSEvent->bParam;
        SErrInfo.SErrLampFailYellow.fError = FALSE;
        SErrInfo.SErrLampFailYellow.bOwnerSG = pSEvent->bParam;

        GetSRuntimePtr()->SaSGRuntimes[pSEvent->bParam
                                       - 1].USOYellowFaults.SSOYellowFaults.
        fLampBroken = 0;
        GetSRuntimePtr()->SaSGRuntimes[pSEvent->bParam
                                       - 1].USOYellowFaults.SSOYellowFaults.
        fCriticalLampBroken = 0;
        GetSRuntimePtr()->SaSGRuntimes[pSEvent->bParam
                                       - 1].USOYellowFaults.SSOYellowFaults.
        fAllLampsBroken = 0;

        GetSRuntimePtr()->SaChannelErrors[(pSEvent->bParam - 1)
                                          / 8].bYellowLampFailure &=
          ~laValue2Bit[(pSEvent->bParam
                        -
                        1)
                       %
                       8];

        LogRequest(LOG_REQ_APPEND_ASYNCH,
                   NULL,
                   EVENT_SG_ALL_YELLOW_LAMPS_SAFE,
                   pSEvent->bParam,
                   pSEvent->sParam,
                   pSEvent->lParam,
                   0);
        break;
      }

      case EVENT_SG_ALL_GREEN_LAMPS_SAFE:
      {
        /* the first parameter is sg number */
        /* all green lamps are safe at this output */
        SErrInfo.SErrLampFailAllGreen.fError = FALSE;
        SErrInfo.SErrLampFailAllGreen.bOwnerSG = pSEvent->bParam;
        SErrInfo.SErrLampFailGreen.fError = FALSE;
        SErrInfo.SErrLampFailGreen.bOwnerSG = pSEvent->bParam;

        GetSRuntimePtr()->SaSGRuntimes[pSEvent->bParam
                                       - 1].USOGreenFaults.SSOGreenFaults.
        fLampBroken = 0;
        GetSRuntimePtr()->SaSGRuntimes[pSEvent->bParam
                                       - 1].USOGreenFaults.SSOGreenFaults.
        fCriticalLampBroken = 0;
        GetSRuntimePtr()->SaSGRuntimes[pSEvent->bParam
                                       - 1].USOGreenFaults.SSOGreenFaults.
        fAllLampsBroken = 0;

        GetSRuntimePtr()->SaChannelErrors[(pSEvent->bParam - 1)
                                          / 8].bGreenLampFailure &=
          ~laValue2Bit[(pSEvent->bParam
                        -
                        1)
                       %
                       8];

        LogRequest(LOG_REQ_APPEND_ASYNCH,
                   NULL,
                   EVENT_SG_ALL_GREEN_LAMPS_SAFE,
                   pSEvent->bParam,
                   pSEvent->sParam,
                   pSEvent->lParam,
                   0);
        break;
      }

      case EVENT_DOOR_OPEN:
      {
        SErrInfo.bErrDoor = TRUE;
        GetASCCabinetEnvironmentPtr()->SaaEnvironmentDevices[0][
          CABINET_ENVIRONMENT_DEVICE_TYPE_DOOR - 1].bOnStatus =
          CABINET_ENVIRONMENT_DEVICE_ON_STATUS_TRUE;
        LogRequest(LOG_REQ_APPEND_ASYNCH,
                   NULL,
                   pSEvent->bEvent,
                   pSEvent->bParam,
                   pSEvent->sParam,
                   pSEvent->lParam,
                   0);
        SNMPSendDoorOpenTrap();
        break;
      }

      case EVENT_DOOR_CLOSED:
      {
        SErrInfo.bErrDoor = FALSE;
        GetASCCabinetEnvironmentPtr()->SaaEnvironmentDevices[0][
          CABINET_ENVIRONMENT_DEVICE_TYPE_DOOR - 1].bOnStatus =
          CABINET_ENVIRONMENT_DEVICE_ON_STATUS_FALSE;
        LogRequest(LOG_REQ_APPEND_ASYNCH,
                   NULL,
                   pSEvent->bEvent,
                   pSEvent->bParam,
                   pSEvent->sParam,
                   pSEvent->lParam,
                   0);
        break;
      }

      case EVENT_SO_SWITCH_OPEN_CIRCUIT:
      {
        switch (SCP.SaSODefs[pSEvent->bParam - 1].bType)
        {
            case SIGNAL_OUTPUT_TYPE_RED:
            {
              uint8_t bOwnerSG = GetSOOwner((pSEvent->bParam - 1)) - 1;

              GetSRuntimePtr()->SaSGRuntimes[bOwnerSG].USORedFaults.SSORedFaults
              .fNoFaults = 0;
              GetSRuntimePtr()->SaSGRuntimes[bOwnerSG].USORedFaults.SSORedFaults
              .fOpenCircuit = 1;
              break;
            }

            case SIGNAL_OUTPUT_TYPE_YELLOW:
            {
              uint8_t bOwnerSG = GetSOOwner((pSEvent->bParam - 1)) - 1;

              GetSRuntimePtr()->SaSGRuntimes[bOwnerSG].USOYellowFaults.
              SSOYellowFaults.fNoFaults = 0;
              GetSRuntimePtr()->SaSGRuntimes[bOwnerSG].USOYellowFaults.
              SSOYellowFaults.fOpenCircuit = 1;
              break;
            }

            case SIGNAL_OUTPUT_TYPE_GREEN:
            {
              uint8_t bOwnerSG = GetSOOwner((pSEvent->bParam - 1)) - 1;

              GetSRuntimePtr()->SaSGRuntimes[bOwnerSG].USOGreenFaults.
              SSOGreenFaults.fNoFaults = 0;
              GetSRuntimePtr()->SaSGRuntimes[bOwnerSG].USOGreenFaults.
              SSOGreenFaults.fOpenCircuit = 1;
              break;
            }
        }

        LogRequest(LOG_REQ_APPEND_ASYNCH,
                   NULL,
                   pSEvent->bEvent,
                   pSEvent->bParam,
                   pSEvent->sParam,
                   pSEvent->lParam,
                   0);
        break;
      }

      case EVENT_SO_SWITCH_SHORT_CIRCUIT:
      {
        switch (SCP.SaSODefs[pSEvent->bParam - 1].bType)
        {
            case SIGNAL_OUTPUT_TYPE_RED:
            {
              uint8_t bOwnerSG = GetSOOwner((pSEvent->bParam - 1)) - 1;

              GetSRuntimePtr()->SaSGRuntimes[bOwnerSG].USORedFaults.SSORedFaults
              .fNoFaults = 0;
              GetSRuntimePtr()->SaSGRuntimes[bOwnerSG].USORedFaults.SSORedFaults
              .fShortCircuirt = 1;
              break;
            }

            case SIGNAL_OUTPUT_TYPE_YELLOW:
            {
              uint8_t bOwnerSG = GetSOOwner((pSEvent->bParam - 1)) - 1;

              GetSRuntimePtr()->SaSGRuntimes[bOwnerSG].USOYellowFaults.
              SSOYellowFaults.fNoFaults = 0;
              GetSRuntimePtr()->SaSGRuntimes[bOwnerSG].USOYellowFaults.
              SSOYellowFaults.fShortCircuirt = 1;
              break;
            }

            case SIGNAL_OUTPUT_TYPE_GREEN:
            {
              uint8_t bOwnerSG = GetSOOwner((pSEvent->bParam - 1)) - 1;

              GetSRuntimePtr()->SaSGRuntimes[bOwnerSG].USOGreenFaults.
              SSOGreenFaults.fNoFaults = 0;
              GetSRuntimePtr()->SaSGRuntimes[bOwnerSG].USOGreenFaults.
              SSOGreenFaults.fShortCircuirt = 1;
              break;
            }
        }

        LogRequest(LOG_REQ_APPEND_ASYNCH,
                   NULL,
                   pSEvent->bEvent,
                   pSEvent->bParam,
                   pSEvent->sParam,
                   pSEvent->lParam,
                   0);
        break;
      }

      case EVENT_SO_LAMPS_DRIVEN_EXTERNALLY:
      {
        uint8_t bOwnerSG = GetSOOwner(pSEvent->bParam - 1);

        switch (SCP.SaSODefs[pSEvent->bParam - 1].bType)
        {
            case SIGNAL_OUTPUT_TYPE_RED:
            {
              GetSRuntimePtr()->SaChannelErrors[(bOwnerSG - 1)
                                                / 8].bUndesiredRedLamp |=
                laValue2Bit[(bOwnerSG - 1) % 8];
              break;
            }

            case SIGNAL_OUTPUT_TYPE_YELLOW:
            {
              GetSRuntimePtr()->SaChannelErrors[(bOwnerSG - 1)
                                                / 8].bUndesiredYellowLamp |=
                laValue2Bit[(bOwnerSG - 1) % 8];
              break;
            }

            case SIGNAL_OUTPUT_TYPE_GREEN:
            {
              GetSRuntimePtr()->SaChannelErrors[(bOwnerSG - 1)
                                                / 8].bUndesiredGreenLamp |=
                laValue2Bit[(bOwnerSG - 1) % 8];
              break;
            }
        }

        LogRequest(LOG_REQ_APPEND_ASYNCH,
                   NULL,
                   pSEvent->bEvent,
                   pSEvent->bParam,
                   pSEvent->sParam,
                   pSEvent->lParam,
                   0);
        break;
      }

      default:
      {
        LogRequest(LOG_REQ_APPEND_ASYNCH,
                   NULL,
                   pSEvent->bEvent,
                   pSEvent->bParam,
                   pSEvent->sParam,
                   pSEvent->lParam,
                   0);
        break;
      }
  } /* switch */
} /* EventMPCont */

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Peripheral States */
void GetPeripheralStates(tpSPeripheralStates pSPeripheralStates)
{
  memcpy(pSPeripheralStates, &SCPRuntime.SPeripheralStates,
         sizeof(tSPeripheralStates));
  if (SCPRuntime.SPeripheralStates.fClearSOPowers)
  {
    SCPRuntime.SPeripheralStates.fClearSOPowers =
      FALSE; /* reset clearSOPowers flag if delivered as TRUE */
  }

  if (SCPRuntime.SPeripheralStates.fProgramRestart)
  {
    SCPRuntime.SPeripheralStates.fProgramRestart =
      FALSE; /* reset fProgramRestart flag if delivered as TRUE */
  }
}

uint8_t WriteSOPower(uint8_t bSOIndex, tpSSOPowerRecord ptSOPower)
{
  return DataPersistenceWrite(PERSIST_OBJECT_SIGNAL_OUTPUT_POWERS,
                              bSOIndex * sizeof(tSSOPowerRecord),
                              ptSOPower,
                              sizeof(tSSOPowerRecord));
}

uint8_t ReadSOPower(uint8_t bSOIndex, tpSSOPowerRecord ptSOPower)
{
  return DataPersistenceRead(PERSIST_OBJECT_SIGNAL_OUTPUT_POWERS,
                             bSOIndex * sizeof(tSSOPowerRecord),
                             ptSOPower,
                             sizeof(tSSOPowerRecord));
}

void InitSOPowers(void)
{
  uint8_t bIndex;
  tSSOPowerRecord SSOPowerRecord;

  for (bIndex = 0; bIndex < SIGNAL_OUTPUTS_MAX; bIndex++)
  {
    memset(&SSOPowerRecord, 0, sizeof(tSSOPowerRecord));

    IWDGRefresh();

    WriteSOPower(bIndex, &SSOPowerRecord);
  }
}

uint8_t ReadSOPowers(void)
{
  uint8_t bIndex;
  tSSOPowerRecord SSOPowerRecord;

  for (bIndex = 0; bIndex < GetSOTotal(); bIndex++)
  {
    IWDGRefresh();

    memset(&SSOPowerRecord, 0, sizeof(tSSOPowerRecord));
    if (ReadSOPower(bIndex, &SSOPowerRecord))
    {
      SCP.SaSODefs[bIndex].sPower[0] = SSOPowerRecord.sPower[0];
      SCP.SaSODefs[bIndex].sPowerRecordNet[0] = SSOPowerRecord.sPowerNet[0];
      SCP.SaSODefs[bIndex].SFlags.bPowerRecorded0 =
        SSOPowerRecord.SFlags.bPowerRecorded0;
      SCP.SaSODefs[bIndex].SFlags.bPowerRecorded1 =
        SSOPowerRecord.SFlags.bPowerRecorded1;
    }
    else
    {
      return FALSE;
    }
  }

  return TRUE;
}

void ClearSOPower(uint8_t bSONo)
{
  if (bSONo < GetSOTotal())
  {
    SCP.SaSODefs[bSONo].SFlags.bPowerRecorded0 = FALSE;
    SCP.SaSODefs[bSONo].SFlags.bPowerRecorded1 = FALSE;
  }
}

void ClearSOPowers(void)
{
  uint8_t bSONo;

  for (bSONo = 0; bSONo < GetSOTotal(); bSONo++)
  {
    ClearSOPower(bSONo);
  }

  SCPRuntime.SPeripheralStates.fClearSOPowers = TRUE;
}

void SetPowerRelay(uint8_t fState)
{
  SCPRuntime.SPeripheralStates.fRelay = fState;

  RelaySet(&g_relayPort, fState ? 1U : 0U);
}

uint8_t GetPowerRelay(void)
{
  return SCPRuntime.SPeripheralStates.fRelay;
}

void SetProgramLoading(uint8_t fState)
{
  SCPRuntime.SPeripheralStates.fCPUinProgress = fState;
}

uint8_t GetProgramLoading(void)
{
  return SCPRuntime.SPeripheralStates.fCPUinProgress;
}

void RestartProgram(void)
{
  LoadProgramEnds();
  StateCurrentInit();
  SetProgramRestart(TRUE);
}

uint8_t GetProgramRestart(void)
{
  return SCPRuntime.SPeripheralStates.fProgramRestart;
}

void SetProgramRestart(uint8_t fNewState)
{
  SCPRuntime.SPeripheralStates.fProgramRestart = fNewState;
}

void SetHeaterState(uint8_t fState)
{
  SCPRuntime.SPeripheralStates.fHeater = fState;

  if (fState)
  {
    HeaterEnable(&g_heaterPort);
  }
  else
  {
    HeaterDisable(&g_heaterPort);
  }
}

uint8_t GetHeaterState(void)
{
  SCPRuntime.SPeripheralStates.fHeater = HAL_GPIO_ReadPin(HEAT_GPIO_Port,
                                                          HEAT_Pin)
                                         == GPIO_PIN_SET;

  return SCPRuntime.SPeripheralStates.fHeater;
}

void SetLampDimmingState(uint8_t fState)
{
  SCPRuntime.SPeripheralStates.fLampDimming = fState;
}

uint8_t GetLampDimmingState(void)
{
  SCPRuntime.SPeripheralStates.fLampDimming =
    HAL_GPIO_ReadPin(DIMMING_GPIO_Port,
                     DIMMING_Pin) == GPIO_PIN_SET;

  return SCPRuntime.SPeripheralStates.fLampDimming;
}

void SetExternalBatteryState(uint8_t fState)
{
  SCPRuntime.SPeripheralStates.fExternalBattery = fState;
}

uint8_t GetExternalBatteryState(void)
{
  return SCPRuntime.SPeripheralStates.fExternalBattery;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  User State */
void UserStateReqInit(void)
{
  memset(&SCPRuntime.SUserState, 0, sizeof(SCPRuntime.SUserState));
}

uint8_t UserStateReqRead(void)
{
  return DataPersistenceRead(PERSIST_OBJECT_USER_REQUEST,
                             0U,
                             &SCPRuntime.SUserState,
                             sizeof(SCPRuntime.SUserState));
}

uint8_t UserStateReqWrite(void)
{
  return DataPersistenceWrite(PERSIST_OBJECT_USER_REQUEST,
                              0U,
                              &SCPRuntime.SUserState,
                              sizeof(SCPRuntime.SUserState));
}

uint8_t UserStateReqSet(uint8_t bNewState)
{
  SCPRuntime.SUserState.bStateReq = bNewState;
  if (UserStateReqGet() == STATES_NONE)
  {
    SCPRuntime.SUserState.fRunning = TRUE;
  }
  else
  {
    SCPRuntime.SUserState.bStateCurrent = UserStateReqGet();
    /* keep this information to read during powering on */
    UserStateReqWrite();
  }

  return FALSE;
}

void UserStateReqEnd(void)
{
  SCPRuntime.SUserState.bStateCurrent = UserStateReqGet();
  SCPRuntime.SUserState.bStateReq = STATES_NONE; /* wait for a new request */
}

uint8_t UserStateReqFree(void)
{
  if (UserStateRunning())
  {
    SCPRuntime.SUserState.fRunning = FALSE;
    SCPRuntime.SUserState.bStateReq = STATES_NONE;

    TransitionLockEnd();
    StateCurrentInit();

    return UserStateReqWrite();
  }

  return TRUE;
}

uint8_t UserStateRunning(void)
{
  return SCPRuntime.SUserState.fRunning;
}

uint8_t UserStateCurrentGet(void)
{
  return SCPRuntime.SUserState.bStateCurrent;
}

uint8_t UserStateReqGet(void)
{
  return SCPRuntime.SUserState.bStateReq;
}

uint8_t UserStateIsValid(void)
{
  if ((UserStateRunning() != FALSE) && (UserStateRunning() != TRUE))
  {
    return FALSE;
  }

  if ((UserStateCurrentGet() > STATES_MAX) || (UserStateReqGet() > STATES_MAX))
  {
    return FALSE;
  }

  return TRUE;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Green Wave Synchronization */
void GpsSynchro(void)
{
  if (SeqTotalExtDurGet() != 0) /* Sequence duration extention already determined */
  {
    if (SaSeqExtDur[SeqCurrentGet() - 1].sExtendedDur
        < (SeqTotalExtDurGet() + SeqDurGet(bCurrentSeqNo)))                                               /* Is extended duration less than total to */
    /* be extended duration */
    {
      SaSeqExtDur[SeqCurrentGet() - 1].sExtendedDur++;
      if (SaSeqExtDur[SeqCurrentGet() - 1].sExtendedDur
          >= (SeqTotalExtDurGet() + SeqDurGet(bCurrentSeqNo)))                                               /* Check if total to be extended duration */
      /* reached */
      {
        memset(&SaSeqExtDur[SeqCurrentGet() - 1], 0, sizeof(tSSeqExtension));
        LogRequest(LOG_REQ_APPEND_ASYNCH,
                   NULL,
                   EVENT_GREEN_WAVE_SYNCH_ENDS,
                   0,
                   0,
                   0,
                   0);
      }
    }
  }
  else /* Check if the time is synchronized to sequence */
  {
    uint32_t lPeriod = 0;
    uint8_t bOperationIndex = 0;

    SaSeqExtDur[SeqCurrentGet() - 1].sExtendedDur = 0;

    /* Loop all operations to find period and offset values if defined */
    for (bOperationIndex = 0;
         bOperationIndex < (RULE_OPERATIONS_MAX - 1);
         bOperationIndex++)
    {
      uint8_t OperandIndex = 0;

      memcpy(&SCurrentOperation,
             &(SCP.SaSignalPrograms[SigProgCurNoGet()
                                    - 1].SaOperations[bOperationIndex]),
             sizeof(tSOperation));

      /* Get period and offset values */
      for (OperandIndex = 0; OperandIndex < OP_MAX; OperandIndex++)
      {
        /* Get period */
        if (SCurrentOperation.SaOperands[OperandIndex].bField
            == OP_FIELD_TIME) /* Operand type: Current time */
        {
          if (SCurrentOperation.SaOperands[OperandIndex].bSubField
              == OP_SUBFIELD_SYNCH_GPS) /* Function: Source GPS */
          {
            lPeriod = (uint32_t) ((SCurrentOperation.SaOperands[1
                                                                - OperandIndex].
                                   bValueHigh << 8)
                                  + SCurrentOperation.SaOperands[1
                                                                 - OperandIndex]
                                  .bValueLow);
          }
        }
        else if (SCurrentOperation.SaOperands[OperandIndex].bField
                 == OP_FIELD_VAR) /* Operand type: Constant */
        {
          if (SCurrentOperation.SaOperands[OperandIndex].bSubField
              == OP_SUBFIELD_VAR_OPERATION_POOL)
          {
            uint8_t bOperationIndex2 = 0;

            for (bOperationIndex2 = 0;
                 bOperationIndex2 < OP_MAX;
                 bOperationIndex2++)
            {
              if (SCP.SaSignalPrograms[SigProgCurNoGet()
                                       - 1].SaOperations[SCurrentOperation.
                                                         SaOperands[OperandIndex]
                                                         .
                                                         bValueLow
                                                         - 1].
                  SaOperands[bOperationIndex2].bSubField
                  == OP_SUBFIELD_SYNCH_GPS)
              {
                bOffsetVal = (uint32_t) ((SCurrentOperation.SaOperands[1
                                                                       -
                                                                       OperandIndex]
                                          .bValueHigh << 8)
                                         + SCurrentOperation.SaOperands[1
                                                                        -
                                                                        OperandIndex]
                                         .bValueLow);
              }
            }
          }
        }
      }
    }

    /* period value caught, so controller works for green-wave */
    if ((lPeriod != 0) && (lPeriod == SeqDurGet(bCurrentSeqNo)))
    {
      switch (StateCurrentGet())
      {
          case STATES_SEQ:
          {
            uint8_t bExpectedSeqDur = 0;

            /* Calculate expected current seq elapsed duration */
            uint32_t lSeconds = TimeSecondOfDayGet();

            if ((lSeconds % lPeriod) <= bOffsetVal)
            {
              bExpectedSeqDur = (lSeconds % lPeriod) - bOffsetVal + lPeriod;
            }
            else
            {
              bExpectedSeqDur = (lSeconds % lPeriod) - bOffsetVal;
            }

            if (bExpectedSeqDur > (SeqDurCurGet() + 1)) /* sequence is coming back, so increase sequence duration by */
            /* (T */
            /* - difference) (means extend duration to 2 * T) */
            {
              SeqTotalExtDurSet(lPeriod
                                - (bExpectedSeqDur - (SeqDurCurGet() + 1)));
            }
            else if (bExpectedSeqDur < (SeqDurCurGet() + 1)) /* sequence duration is at front, so increase sequence */
            /* duration by difference */
            {
              SeqTotalExtDurSet((SeqDurCurGet() + 1) - bExpectedSeqDur);
            }
            else
            {
              memset(&SaSeqExtDur[SeqCurrentGet() - 1],
                     0,
                     sizeof(tSSeqExtension));
            }

            if (SeqTotalExtDurGet() != 0)
            {
              uint8_t bSeqStepIndex = 0;
              uint8_t bAddedDur = 0;

              for (bSeqStepIndex = SRuntimes.SSeqRuntime.bCurrentStep;
                   bSeqStepIndex < SeqCurStepNumTotalGet();
                   bSeqStepIndex++)
              {
                if (SeqStepDurGet(bCurrentSeqNo, bSeqStepIndex) > 4) /* if the sequence step is not a transition step */
                {
                  SeqStepExtDurSet(bSeqStepIndex,
                                   (SeqTotalExtDurGet() - bAddedDur));
                  bAddedDur += (SeqTotalExtDurGet() - bAddedDur);

                  if (bAddedDur >= SeqTotalExtDurGet())
                  {
                    SeqExtValidationSet(TRUE);
                    SaSeqExtDur[SeqCurrentGet()
                                - 1].sExtendedDur = SeqDurCurGet() + 1;
                    LogRequest(LOG_REQ_APPEND_ASYNCH,
                               NULL,
                               EVENT_GREEN_WAVE_SYNCH_STARTS,
                               SeqTotalExtDurGet(),
                               lPeriod,
                               bOffsetVal,
                               0);
                    break;
                  }
                }
              }

              /* this is a protection for if the algorithm could not find a seq */
              /* step longer than 3 seconds */
              if (SeqExtValidationGet() == FALSE)
              {
                memset(&SaSeqExtDur[SeqCurrentGet() - 1], 0,
                       sizeof(tSSeqExtension));
              }
            }

            break;
          }
      } /* switch */
    }
  }
} /* GpsSynchro */

uint8_t SeqExtValidationGet(void)
{
  return SaSeqExtDur[SeqCurrentGet() - 1].fIsValid;
}

void SeqExtValidationSet(uint8_t fState)
{
  SaSeqExtDur[SeqCurrentGet() - 1].fIsValid = fState;
}

uint8_t SeqTotalExtDurGet(void)
{
  return SaSeqExtDur[SeqCurrentGet() - 1].bTotalSeqExtDur;
}

void SeqTotalExtDurSet(uint8_t bSeqExtDur)
{
  SaSeqExtDur[SeqCurrentGet() - 1].bTotalSeqExtDur = bSeqExtDur;
}

uint8_t SeqStepExtDurGet(uint8_t bSeqStepNo)
{
  return SaSeqExtDur[SeqCurrentGet() - 1].baSeqStepExtDur[bSeqStepNo];
}

void SeqStepExtDurSet(uint8_t bSeqStepNo, uint8_t bSeqStepExtDur)
{
  SaSeqExtDur[SeqCurrentGet() - 1].baSeqStepExtDur[bSeqStepNo] = bSeqStepExtDur;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Counters */
uint8_t CounterIsValid(uint8_t bAddress)
{
  if (bAddress && (bAddress <= COUNTERS_MAX))
  {
    return TRUE;
  }

  return FALSE;
}

void CounterSet(uint8_t bAddress, tpSCounter pSCounter)
{
  if (CounterIsValid(bAddress))
  {
    memcpy(&(SaCounters[bAddress - 1]), pSCounter, sizeof(tSCounter));
  }
}

void CounterGet(uint8_t bAddress, tpSCounter pSCounter)
{
  if (CounterIsValid(bAddress))
  {
    memcpy(pSCounter, &(SaCounters[bAddress - 1]), sizeof(tSCounter));
  }
}

void CounterValueSet(uint8_t bAddress, uint32_t lValue)
{
  if (CounterIsValid(bAddress))
  {
    SaCounters[bAddress - 1].lValue = lValue;
  }
}

uint32_t CounterValueGet(uint8_t bAddress)
{
  if (CounterIsValid(bAddress))
  {
    return SaCounters[bAddress - 1].lValue;
  }

  return 0;
}

void CounterValueAdd(uint8_t bAddress, uint32_t lValue)
{
  if (CounterIsValid(bAddress))
  {
    if ((SaCounters[bAddress - 1].lValue + lValue)
        < SaCounters[bAddress - 1].lValue)
    {
      SaCounters[bAddress - 1].fOverflow = TRUE;
    }

    SaCounters[bAddress - 1].lValue += lValue;
  }
}

void CounterPeriodSet(uint8_t bAddress, uint16_t sPeriod)
{
  if (CounterIsValid(bAddress))
  {
    SaCounters[bAddress - 1].sPeriod = sPeriod;
  }
}

uint16_t CounterPeriodGet(uint8_t bAddress)
{
  if (CounterIsValid(bAddress))
  {
    return SaCounters[bAddress - 1].sPeriod;
  }

  return 0;
}

void CounterAllocatedSet(uint8_t bAddress, uint8_t fValue)
{
  if (CounterIsValid(bAddress))
  {
    SaCounters[bAddress - 1].fAllocated = fValue;
  }
}

uint8_t CounterAllocatedGet(uint8_t bAddress)
{
  if (CounterIsValid(bAddress))
  {
    return SaCounters[bAddress - 1].fAllocated;
  }

  return FALSE;
}

void CounterAllocate(uint8_t bAddress)
{
  if (CounterIsValid(bAddress))
  {
    SaCounters[bAddress - 1].fAllocated = TRUE;
  }
}

void CounterRunningSet(uint8_t bAddress, uint8_t fValue)
{
  if (CounterIsValid(bAddress))
  {
    SaCounters[bAddress - 1].fRunning = fValue;
  }
}

uint8_t CounterRunningGet(uint8_t bAddress)
{
  if (CounterIsValid(bAddress))
  {
    return SaCounters[bAddress - 1].fRunning;
  }

  return FALSE;
}

void CounterOverflowSet(uint8_t bAddress, uint8_t fValue)
{
  if (CounterIsValid(bAddress))
  {
    SaCounters[bAddress - 1].fOverflow = fValue;
  }
}

uint8_t CounterOverflowGet(uint8_t bAddress)
{
  if (CounterIsValid(bAddress))
  {
    return SaCounters[bAddress - 1].fOverflow;
  }

  return FALSE;
}

/* /////////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Functions Configurations */
uint8_t WriteFunctionConf(void)
{
  if (DataPersistenceWrite(PERSIST_OBJECT_FUNCTION_CONFIG,
                           0U,
                           &SFuncConf,
                           sizeof(tSFuncConf)))
  {
    return TRUE;
  }

  return FALSE;
}

uint8_t ReadFunctionConf(void)
{
  return DataPersistenceRead(PERSIST_OBJECT_FUNCTION_CONFIG,
                             0U,
                             &SFuncConf,
                             sizeof(tSFuncConf));
}

uint8_t GetFunctionConfByIndex(uint8_t bConfIdx)
{
  if (bConfIdx < MAX_BITS_IN_BYTE)
  {
    return (SFuncConf.bConf0 >> bConfIdx) & 1;
  }

  return FALSE;
}

uint8_t GetFunctionConf(void)
{
  return SFuncConf.bConf0;
}

void SetFunctionConf(uint8_t bNewVal)
{
  SFuncConf.bConf0 = bNewVal;
}

void SetFunctionConfByIndex(uint8_t bConfIdx, uint8_t bNewVal)
{
  if (bConfIdx < MAX_BITS_IN_BYTE)
  {
    if (bNewVal)
    {
      SFuncConf.bConf0 |= (uint8_t) (1 << bConfIdx);
    }
    else
    {
      SFuncConf.bConf0 &= ~(uint8_t) (1 << bConfIdx);
    }
  }
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Voltage State */
void SetVoltageState(uint8_t bNewState)
{
  SCPRuntime.bVoltageState = bNewState;
}

uint8_t GetVoltageState(void)
{
  return SCPRuntime.bVoltageState;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Frequency State */
void SetFrequencyState(uint8_t bNewState)
{
  SCPRuntime.bFrequencyState = bNewState;
}

uint8_t GetFrequencyState(void)
{
  return SCPRuntime.bFrequencyState;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  SSM Test */
void StartSSMTest(uint8_t bSource)
{
  SSSMTest.bSSMTestSource = bSource;
  SSSMTest.bTurnOnSONo = 0;
}

uint8_t GetSSMTestSource(void)
{
  return SSSMTest.bSSMTestSource;
}

void StopSSMTest(void)
{
  SSSMTest.bSSMTestSource = SSM_TEST_FROM_NONE;
}

uint8_t TurnOnNextSONo(void)
{
  SSSMTest.bTurnOnSONo++;
  if (SSSMTest.bTurnOnSONo == GetSOTotal())
  {
    SSSMTest.bTurnOnSONo = 0;
  }

  return SSSMTest.bTurnOnSONo;
}

uint8_t TurnOnPreviousSONo(void)
{
  if (SSSMTest.bTurnOnSONo == 0)
  {
    SSSMTest.bTurnOnSONo = (GetSOTotal() - 1);
  }
  else
  {
    SSSMTest.bTurnOnSONo--;
  }

  return SSSMTest.bTurnOnSONo;
}

uint8_t GetOnSONo(void)
{
  return SSSMTest.bTurnOnSONo;
}

void SetOnSONo(uint8_t bSONo)
{
  SSSMTest.bTurnOnSONo = bSONo;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Power Supply */
uint16_t GetPowerSupplyNet(uint8_t bPSMNo)
{
  return SaPSMs[bPSMNo].sNet;
}

uint16_t GetPowerSupplyFreq(uint8_t bPSMNo)
{
  return SaPSMs[bPSMNo].sFrequency / 2;
}

uint16_t GetPowerSupply24V1(uint8_t bPSMNo)
{
  return SaPSMs[bPSMNo].s24V1;
}

uint16_t GetPowerSupply5V1(uint8_t bPSMNo)
{
  return SaPSMs[bPSMNo].s5V1;
}

uint16_t GetPowerSupply24V2(uint8_t bPSMNo)
{
  return SaPSMs[bPSMNo].s24V2;
}

uint16_t GetPowerSupply5V2(uint8_t bPSMNo)
{
  return SaPSMs[bPSMNo].s5V2;
}

uint8_t Set24V1(uint8_t bPSMNo, uint16_t sNewVoltage)
{
  if (bPSMNo < PSMS_MAX)
  {
    if (sNewVoltage < VOLTAGE_VALUE_MAX)
    {
      SaPSMs[bPSMNo].s24V1 = sNewVoltage;

      return TRUE;
    }
  }

  return FALSE;
}

uint8_t Set5V1(uint8_t bPSMNo, uint16_t sNewVoltage)
{
  if (bPSMNo < PSMS_MAX)
  {
    if (sNewVoltage < VOLTAGE_VALUE_MAX)
    {
      SaPSMs[bPSMNo].s5V1 = sNewVoltage;

      return TRUE;
    }
  }

  return FALSE;
}

uint8_t Set24V2(uint8_t bPSMNo, uint16_t sNewVoltage)
{
  if (bPSMNo < PSMS_MAX)
  {
    if (sNewVoltage < VOLTAGE_VALUE_MAX)
    {
      SaPSMs[bPSMNo].s24V2 = sNewVoltage;

      return TRUE;
    }
  }

  return FALSE;
}

uint8_t Set5V2(uint8_t bPSMNo, uint16_t sNewVoltage)
{
  if (bPSMNo < PSMS_MAX)
  {
    if (sNewVoltage < VOLTAGE_VALUE_MAX)
    {
      SaPSMs[bPSMNo].s5V2 = sNewVoltage;

      return TRUE;
    }
  }

  return FALSE;
}

uint8_t SetNetFrequency(uint8_t bPSMNo, uint16_t sNewFrequency)
{
  if (bPSMNo < PSMS_MAX)
  {
    if (sNewFrequency < FREQUENCY_VALUE_MAX)
    {
      SaPSMs[bPSMNo].sPrevFrequency = SaPSMs[bPSMNo].sFrequency;
      SaPSMs[bPSMNo].sFrequency = sNewFrequency;

      return TRUE;
    }
  }

  return FALSE;
}

uint8_t SetNetVoltages(uint8_t bPSMNo, uint16_t sNewVoltage)
{
  if (bPSMNo < PSMS_MAX)
  {
    if (sNewVoltage < VOLTAGE_VALUE_MAX)
    {
      SaPSMs[bPSMNo].sPrevNet = SaPSMs[bPSMNo].sNet;
      SaPSMs[bPSMNo].sNet = sNewVoltage;

      return TRUE;
    }
  }

  return FALSE;
}

uint8_t SetIsolatedVoltageState(uint8_t bPSMNo, uint8_t fNewState)
{
  if (bPSMNo < PSMS_MAX)
  {
    SaPSMs[bPSMNo].SFlags.fIsolatedVoltage = fNewState;

    return TRUE;
  }

  return FALSE;
}

uint8_t GetPowerSupplyIsolatedVoltage(uint8_t bPSMNo)
{
  return SaPSMs[bPSMNo].SFlags.fIsolatedVoltage;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Currents */
void GetSGCurrentMeasurement(uint8_t bSGNo,
                             tpSCurrentMeasurement pSCurrentMeasurement)
{
  memcpy(pSCurrentMeasurement, &SaCurrents[bSGNo],
         sizeof(tSCurrentMeasurement));
}

uint16_t GetCurrentMeasurement(uint8_t bCurrentGroupNo, uint8_t bOption)
{
  if (bCurrentGroupNo < SIGNAL_OUTPUT_CURRENT_GROUPS_MAX)
  {
    switch (bOption)
    {
        case CURRENT_MIN:
        {
          return SaCurrents[bCurrentGroupNo].sMin;
        }

        case CURRENT_NOW:
        {
          return SaCurrents[bCurrentGroupNo].sNow;
        }

        case CURRENT_MAX:
        {
          return SaCurrents[bCurrentGroupNo].sMax;
        }

        case CURRENT_PREV:
        {
          return SaCurrents[bCurrentGroupNo].sPrev;
        }

        default:
        {
          return 0;
        }
    }
  }
  else
  {
    return 0;
  }
}

void SetCurrentMeasurement(uint8_t bCurrentGroupNo, uint16_t sNewValue)
{
  if (bCurrentGroupNo < SIGNAL_OUTPUT_CURRENT_GROUPS_MAX)
  {
    SaCurrents[bCurrentGroupNo].sPrev = SaCurrents[bCurrentGroupNo].sNow;
    SaCurrents[bCurrentGroupNo].sNow = sNewValue;

    if (sNewValue < SaCurrents[bCurrentGroupNo].sMin)
    {
      SaCurrents[bCurrentGroupNo].sMin = sNewValue;
    }

    if (sNewValue > SaCurrents[bCurrentGroupNo].sMax)
    {
      SaCurrents[bCurrentGroupNo].sMax = sNewValue;
    }
  }
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Log Settings */
uint8_t GetLogSettingsByEventID(uint8_t bEventID)
{
  uint8_t bByte = bEventID / 8;
  uint8_t bBit = bEventID % 8;
  uint8_t bMask = 1 << bBit;

  return (SLogSettings.baLogSettings[bByte] & bMask) ? TRUE : FALSE;
}

void LogSettingsSet(tpSLogSettings pSLogSettings)
{
  memcpy(&SLogSettings, pSLogSettings, sizeof(tSLogSettings));
}

uint8_t LogSettingsSave(void)
{
  return DataPersistenceWrite(PERSIST_OBJECT_LOG_SETTINGS,
                              0U,
                              &SLogSettings,
                              sizeof(tSLogSettings));
}

uint8_t LogSettingsRead(void)
{
  return DataPersistenceRead(PERSIST_OBJECT_LOG_SETTINGS,
                             0U,
                             &SLogSettings,
                             sizeof(tSLogSettings));
}

void LogSettingsGet(tpSLogSettings pSLogSettings)
{
  memcpy(pSLogSettings, &SLogSettings, sizeof(tSLogSettings));
}

void LogSettingsInit(void)
{
  tSLogSettings SLLogSettings;

  memset(&SLLogSettings, 0xFF, sizeof(tSLogSettings));

  SLLogSettings.fSettingsChanged = LOG_SETTINGS_CHANGE_CONTROL_VLAUE;

  LogSettingsSet(&SLLogSettings);
  if (LogSettingsSave())
  {
    LogSettingsRead();
  }
}

uint8_t IsLogSettingsChanged(void)
{
  return SLogSettings.fSettingsChanged == LOG_SETTINGS_CHANGE_CONTROL_VLAUE;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  System Start Time */
void SystemStartTimeInit(void)
{
  memset(&SSystemStartTime, 0, sizeof(tSSystemStartTime));
  bSystemUpHours = 0;
}

void SystemStartTimeSet(tpSSystemStartTime pSSystemStartTime)
{
  memcpy(&SSystemStartTime, pSSystemStartTime, sizeof(tSSystemStartTime));
}

uint8_t SystemStartTimeSave(void)
{
  return DataPersistenceWrite(PERSIST_OBJECT_SYSTEM_START_TIME,
                              0U,
                              &SSystemStartTime,
                              sizeof(tSSystemStartTime));
}

uint8_t SystemStartTimeRead(void)
{
  return DataPersistenceRead(PERSIST_OBJECT_SYSTEM_START_TIME,
                             0U,
                             &SSystemStartTime,
                             sizeof(tSSystemStartTime));
}

void SystemStartTimeGet(tpSSystemStartTime pSSystemStartTime)
{
  memcpy(pSSystemStartTime, &SSystemStartTime, sizeof(tSSystemStartTime));
}

void SystemStartTimeStart(void)
{
  tSTime STime;
  tSSystemStartTime SLSystemStartTime;

  memset(&SLSystemStartTime, 0, sizeof(tSSystemStartTime));
  memset(&STime, 0, sizeof(STime));
  bSystemUpHours = 0;

  TimeGet(&STime);

  SLSystemStartTime.bAlreadyWritten = SYSTEM_START_TIME_ALREADY_WRITTEN;
  SLSystemStartTime.bMonthDay = STime.SCurrentDate.Date;
  SLSystemStartTime.bMonth = STime.SCurrentDate.Month;
  SLSystemStartTime.sYear = STime.SCurrentDate.Year;

  SystemStartTimeSet(&SLSystemStartTime);
  if (SystemStartTimeSave())
  {
    SystemStartTimeRead();
  }
}

uint8_t IsSystemStartTimeWritten(void)
{
  return SSystemStartTime.bAlreadyWritten
         == SYSTEM_START_TIME_ALREADY_WRITTEN;
}

void SystemStartTimeSetMinUpHours(uint8_t bMinUpHours)
{
  bMinSystemUpHours = bMinUpHours;
}

uint8_t SystemStartTimeGetMinUpHours(void)
{
  return bMinSystemUpHours;
}

void SystemStartTimeSetUpHours(uint8_t bUpHours)
{
  bSystemUpHours = bUpHours;
}

void SystemStartTimeIncUpHours(void)
{
  bSystemUpHours++;
}

uint8_t SystemStartTimeGetUpHours(void)
{
  return bSystemUpHours;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  LRLF Time Settings */
uint8_t LRLFDetectTimeWrite(void)
{
  return DataPersistenceWrite(PERSIST_OBJECT_LRLF_DETECT_TIME,
                              0U,
                              &bLRLFDetectTime,
                              sizeof(bLRLFDetectTime));
}

uint8_t LRLFDetectTimeRead(void)
{
  return DataPersistenceRead(PERSIST_OBJECT_LRLF_DETECT_TIME,
                             0U,
                             &bLRLFDetectTime,
                             sizeof(bLRLFDetectTime));
}

void LRLFDetectTimeSet(uint8_t bTime)
{
  bLRLFDetectTime = bTime;
}

uint8_t LRLFDetectTimeGet(void)
{
  return bLRLFDetectTime;
}

void LRLFDetectTimeCheck(void)
{
  uint8_t bTime = LRLF_DETECT_TIME_800_MS;

  if (!LRLFDetectTimeRead()
      || (LRLFDetectTimeGet() < LRLF_DETECT_TIME_300_MS)
      || (LRLFDetectTimeGet() > LRLF_DETECT_TIME_MAX) )
  {
    LRLFDetectTimeSet(LRLF_DETECT_TIME_800_MS);
    LRLFDetectTimeWrite();
  }

  bTime = LRLFDetectTimeGet();
  if ((bTime <= LRLF_DETECT_TIME_NONE)
      || (LRLFDetectTimeGet() > LRLF_DETECT_TIME_MAX) )
  {
    LRLFDetectTimeSet(LRLF_DETECT_TIME_800_MS);
    LRLFDetectTimeWrite();
  }

  CANTxRequest(1, CAN_ID_TYPE_STD, CAN_LRLF_DETECT_TIME, &bTime);
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Secure Transition */
void ApplySecureTransition(void)
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
          }while (StateCurrentGet() == STATES_SECURE_TRANSITION);

          break;
        }

        default:
        {
          break;
        }
    }
  }
}

void SecureSystemReset(void)
{
  ApplySecureTransition();
  osDelay(1000);
  SystemReset();
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/* Error Info */
void InitErrorInfo(void)
{
  memset(&SErrInfo, 0, sizeof(tSErrInfo));
}

tpSErrInfo GetErrorInfoPtr(void)
{
  return &SErrInfo;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/* Traffic Counts Timer */
void IncTrafficCountsTimer(void)
{
  sTrafficCntTimer++;
}

uint16_t GetTrafficCountsTimer(void)
{
  return sTrafficCntTimer;
}

void SetTrafficCountsTimer(uint16_t sTimer)
{
  sTrafficCntTimer = sTimer;
}

void GetMCSTrafficCountsDigital(void *pData)
{
  memcpy(pData, SMCSTrafficCountsRuntimes.SaMCSDigitalInputRuntimes,
         sizeof(SMCSTrafficCountsRuntimes.SaMCSDigitalInputRuntimes));
}

void GetMCSTrafficCountsDetector(void *pData)
{
  memcpy(pData, SMCSTrafficCountsRuntimes.SaMCSDetectorInputRuntimes,
         sizeof(SMCSTrafficCountsRuntimes.SaMCSDetectorInputRuntimes));
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/* Device UID */
void InitDeviceUIDs(void)
{
  memset(&SDeviceUID, 0, sizeof(SDeviceUID));
  memset(&SEEPROMDeviceUID, 0, sizeof(SEEPROMDeviceUID));
}

void ReadCPUDeviceUID(void)
{
  memset(&SDeviceUID, 0, sizeof(SDeviceUID));

  SDeviceUID.ulaUID[0] = HAL_GetUIDw0();
  SDeviceUID.ulaUID[1] = HAL_GetUIDw1();
  SDeviceUID.ulaUID[2] = HAL_GetUIDw2();
}

tpSDeviceUID GetCPUDeviceUID(void)
{
  return &SDeviceUID;
}

uint8_t ReadEEPROMDeviceUID(void)
{
  return DataPersistenceRead(PERSIST_OBJECT_DEVICE_UID,
                             0U,
                             &SEEPROMDeviceUID,
                             sizeof(SEEPROMDeviceUID));
}

uint8_t WriteEEPROMDeviceUID(void)
{
  return DataPersistenceWrite(PERSIST_OBJECT_DEVICE_UID,
                              0U,
                              &SDeviceUID,
                              sizeof(SDeviceUID));
}

#ifdef DEBUG
uint8_t SetDeviceUID(void)
{
  ReadCPUDeviceUID();

  return WriteEEPROMDeviceUID();
}

uint8_t ClearDeviceUID(void)
{
  InitDeviceUIDs();

  return WriteEEPROMDeviceUID();
}

#endif /* ifdef DEBUG */

uint8_t CheckDeviceUIDs(void)
{
  InitDeviceUIDs();

  if (!ReadEEPROMDeviceUID())
  {
    return FALSE;
  }

  ReadCPUDeviceUID();

  if (memcmp(&SEEPROMDeviceUID, &SDeviceUID, sizeof(SDeviceUID)) != 0)
  {
    return FALSE;
  }

  return TRUE;
}

void SignalMaintenanceTask(uint32_t ulSignal)
{
  if (MaintenanceEventHandle != NULL)
  {
    osEventFlagsSet(MaintenanceEventHandle,
                    ulSignal);
  }
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/* CP - UCM */
tpSCPRuntime GetSCPRuntimePtr(void)
{
  return &SCPRuntime;
}

tpSRuntimes GetSRuntimePtr(void)
{
  return &SRuntimes;
}

tpSCP GetSCPPtr(void)
{
  return &SCP;
}

tpSErrInfo GetSErrInfoPtr(void)
{
  return &SErrInfo;
}

tpSCanDetectorIOInputs GetSaCanDetectorIOInputsPtr(void)
{
  return &SaCanDetectorIOInputs[0];
}

tpSCanDigitalIOInputs GetSaCanDigitalIOInputsPtr(void)
{
  return &SaCanDigitalIOInputs[0];
}

tpSCurrentMeasurement GetSaCurrentsPtr(void)
{
  return &SaCurrents[0];
}

tpSPowerSupply GetSaPSMsPtr(void)
{
  return &SaPSMs[0];
}

tpSSeqExtension GetSaSeqExtDurPtr(void)
{
  return &SaSeqExtDur[0];
}

tpSUserOperations GetSUserOperationsPtr(void)
{
  return &SUseroperations;
}

tpSGlobalConfiguration GetSGlobalConfigurationPtr(void)
{
  return &SGlobalConfiguration;
}

tpSGlobalDbManagement GetSGlobalDbManagementPtr(void)
{
  return &SGlobalDbMangement;
}

tpSGlobalTimeManagement GetSGlobalTimeManagementPtr(void)
{
  return &SGlobalTimeManagement;
}

tpSTRPatternsAndCoords GetSTRPatternsAndCoordsPtr(void)
{
  return &STRPatternsAndCoords;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/* User Operations */
void UserOperationsInit(void)
{
  uint8_t bIdx = 0;

  memset(&SUseroperations, 0, sizeof(SUseroperations));
  for (bIdx = 0; bIdx < USER_OPERATIONS_MAX; bIdx++)
  {
    SUseroperations.SaOperations[bIdx].bIdx = bIdx + 1;
  }
}

void UserOperationsAdd(uint8_t bType)
{
  tSTime STimeNow;

  memset(&STimeNow, 0, sizeof(STimeNow));

  memset(&SUseroperations.SaOperations[SUseroperations.bLastOperation], 0,
         sizeof(SUseroperations.SaOperations[SUseroperations.bLastOperation]));

  TimeGet(&STimeNow);

  SUseroperations.SaOperations[SUseroperations.bLastOperation].bType = bType;
  TimeEpochCalculate(&STimeNow,
                     &SUseroperations.SaOperations[SUseroperations.
                                                   bLastOperation].lTime);
  snprintf(
    SUseroperations.SaOperations[SUseroperations.bLastOperation].strUsername,
    sizeof(SUseroperations.SaOperations[SUseroperations.bLastOperation].
           strUsername),
    "%d",
    sAdminUsername);

  SUseroperations.bLastOperation++;
  SUseroperations.bLastOperation %= USER_OPERATIONS_MAX;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/* Signal State Runtime */
void SignalStateRuntimeInit(void)
{
  uint8_t bIdx = 0;

  memset(&SRuntimes.SaSignalStateRuntimes, 0,
         sizeof(SRuntimes.SaSignalStateRuntimes));
  for (bIdx = 0; bIdx < SIGNAL_STATES_MAX; bIdx++)
  {
    SRuntimes.SaSignalStateRuntimes[bIdx].bNumber = bIdx + 1;
    SRuntimes.SaSignalStateRuntimes[bIdx].bSetNo = 1;
  }
}

void SignalStateRuntimeCurNoSet(uint8_t bState)
{
  SCPRuntime.bCurSignalState = bState;
}

uint8_t SignalStateRuntimeCurNoGet(void)
{
  return SCPRuntime.bCurSignalState;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/* Channel Errors */
void ChannelErrorsInit(void)
{
  uint8_t bIdx = 0;

  memset(&SRuntimes.SaChannelErrors, 0, sizeof(SRuntimes.SaChannelErrors));
  for (bIdx = 0; bIdx < CHANNEL_ERROR_FLAGS_MAX ; bIdx++)
  {
    SRuntimes.SaChannelErrors[bIdx].bNumber = bIdx + 1;
  }
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  TR Pattern */
void TRPatternsAndCoordsInit(void)
{
  uint8_t bIdx1 = 0, bIdx2 = 0;

  memset(&STRPatternsAndCoords, 0, sizeof(STRPatternsAndCoords));

  for (bIdx1 = 0; bIdx1 < TR_PATTERNS_SUBJUNCTIONS_MAX; bIdx1++)
  {
    for (bIdx2 = 0; bIdx2 < TR_PATERNS_MAX; bIdx2++)
    {
      STRPatternsAndCoords.SaaPatterns[bIdx1][bIdx2].bSubjunctionNo = bIdx1 + 1;
      STRPatternsAndCoords.SaaPatterns[bIdx1][bIdx2].bPatternNo = bIdx2 + 1;
      STRPatternsAndCoords.SaaPatterns[bIdx1][bIdx2].bSpecialParamTableOIDLength
        =
          sizeof(STRPatternsAndCoords
                 .
                 SaaPatterns[bIdx1][bIdx2].laSpecialParamTablOID);
    }
  }

  for (bIdx1 = 0; bIdx1 < TR_COORDS_MAX; bIdx1++)
  {
    STRPatternsAndCoords.SaCoords[bIdx1].bIdx = bIdx1 + 1;
  }
}

uint8_t TRPatternsAndCoordsGetCurJunctionNo(void)
{
  return SUBJUNCTIONS_DEFAULT_JUNCTION;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Global Configuration */
void GCInit(void)
{
  memset(&SGlobalConfiguration, 0, sizeof(SGlobalConfiguration));

  SGlobalConfiguration.sSetIDParameter = SCPRuntime.sDataChecksumTotal;

  SGlobalConfiguration.SaModules[GLOBAL_CONFIG_ASC_CP_HARDWARE_MODULE_INDEX].
  bNumber = 1;
  SGlobalConfiguration.SaModules[GLOBAL_CONFIG_ASC_CP_HARDWARE_MODULE_INDEX].
  bType = GLOBAL_CONFIG_MODULE_TYPE_HARDWARE;
  memcpy(
    SGlobalConfiguration.SaModules[GLOBAL_CONFIG_ASC_CP_HARDWARE_MODULE_INDEX].
    laDeviceNode,
    baAscModuleDeviceNode,
    sizeof(baAscModuleDeviceNode));
  strcpy(
    SGlobalConfiguration.SaModules[GLOBAL_CONFIG_ASC_CP_HARDWARE_MODULE_INDEX].
    strMake,
    GLOBAL_CONFIG_ASC_MODULE_MAKE);
  strcpy(
    SGlobalConfiguration.SaModules[GLOBAL_CONFIG_ASC_CP_HARDWARE_MODULE_INDEX].
    strModel,
    GLOBAL_CONFIG_ASC_CP_HARDWARE_MODULE_MODEL);
  strcpy(
    SGlobalConfiguration.SaModules[GLOBAL_CONFIG_ASC_CP_HARDWARE_MODULE_INDEX].
    strVersion,
    GLOBAL_CONFIG_ASC_CP_HARDWARE_MODULE_VERSION);

  SGlobalConfiguration.SaModules[GLOBAL_CONFIG_ASC_CP_SOFTWARE_MODULE_INDEX].
  bNumber = 2;
  SGlobalConfiguration.SaModules[GLOBAL_CONFIG_ASC_CP_SOFTWARE_MODULE_INDEX].
  bType = GLOBAL_CONFIG_MODULE_TYPE_SOFTWARE;
  memcpy(
    SGlobalConfiguration.SaModules[GLOBAL_CONFIG_ASC_CP_SOFTWARE_MODULE_INDEX].
    laDeviceNode,
    baAscModuleDeviceNode,
    sizeof(baAscModuleDeviceNode));
  strcpy(
    SGlobalConfiguration.SaModules[GLOBAL_CONFIG_ASC_CP_SOFTWARE_MODULE_INDEX].
    strMake,
    GLOBAL_CONFIG_ASC_MODULE_MAKE);
  strcpy(
    SGlobalConfiguration.SaModules[GLOBAL_CONFIG_ASC_CP_SOFTWARE_MODULE_INDEX].
    strModel,
    GLOBAL_CONFIG_ASC_CP_SOFTWARE_MODULE_MODEL);
  strcpy(
    SGlobalConfiguration.SaModules[GLOBAL_CONFIG_ASC_CP_SOFTWARE_MODULE_INDEX].
    strVersion,
    GLOBAL_CONFIG_ASC_CP_SOFTWARE_MODULE_VERSION);

  SGlobalConfiguration.SaModules[GLOBAL_CONFIG_ASC_MP_SOFTWARE_MODULE_INDEX].
  bNumber = 3;
  SGlobalConfiguration.SaModules[GLOBAL_CONFIG_ASC_MP_SOFTWARE_MODULE_INDEX].
  bType = GLOBAL_CONFIG_MODULE_TYPE_SOFTWARE;
  memcpy(
    SGlobalConfiguration.SaModules[GLOBAL_CONFIG_ASC_MP_SOFTWARE_MODULE_INDEX].
    laDeviceNode,
    baAscModuleDeviceNode,
    sizeof(baAscModuleDeviceNode));
  strcpy(
    SGlobalConfiguration.SaModules[GLOBAL_CONFIG_ASC_MP_SOFTWARE_MODULE_INDEX].
    strMake,
    GLOBAL_CONFIG_ASC_MODULE_MAKE);
  strcpy(
    SGlobalConfiguration.SaModules[GLOBAL_CONFIG_ASC_MP_SOFTWARE_MODULE_INDEX].
    strModel,
    GLOBAL_CONFIG_ASC_MP_SOFTWARE_MODULE_MODEL);
  strcpy(
    SGlobalConfiguration.SaModules[GLOBAL_CONFIG_ASC_MP_SOFTWARE_MODULE_INDEX].
    strVersion,
    GLOBAL_CONFIG_ASC_MP_SOFTWARE_MODULE_VERSION);

  strcpy(SGlobalConfiguration.strControllerBaseStandards,
         GLOBAL_CONFIG_CONTROLLER_BASED_STANDARDS);
} /* GCInit */

void GCSetASCModuleID(uint16_t sSum)
{
  SGlobalConfiguration.sSetIDParameter = sSum;
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Global Time Management */
void GTMInit(void)
{
  uint8_t bIdx1 = 0, bIdx2 = 0;

  memset(&SGlobalTimeManagement, 0, sizeof(SGlobalTimeManagement));

  SGlobalTimeManagement.lGlobalLocalTimeDifferential = GetDeviceTimeZone()
                                                       * 3600;
  if (IsDaylightSavingTimeFlagSet())
  {
    if (TimeDstGet())
    {
      SGlobalTimeManagement.lGlobalLocalTimeDifferential -= 3600;
    }
    else
    {
      SGlobalTimeManagement.lGlobalLocalTimeDifferential += 3600;
    }
  }

  SGlobalTimeManagement.lControllerStandardTimeZone = GetDeviceTimeZone()
                                                      * 3600;

  for (bIdx1 = 0; bIdx1 < GTM_TIME_BASE_MAX_SCHEDULES; bIdx1++)
  {
    SGlobalTimeManagement.STimebase.SaSchedules[bIdx1].bNumber = bIdx1 + 1;
  }

  for (bIdx1 = 0; bIdx1 < GTM_TIME_BASE_MAX_DAY_PLANS; bIdx1++)
  {
    for (bIdx2 = 0; bIdx2 < GTM_TIME_BASE_MAX_DAY_PLAN_EVENTS; bIdx2++)
    {
      SGlobalTimeManagement.STimebase.SaaDayPlans[bIdx1][bIdx2].bNumber =
        bIdx1 + 1;
      SGlobalTimeManagement.STimebase.SaaDayPlans[bIdx1][bIdx2].bEventNumber =
        bIdx2 + 1;
      SGlobalTimeManagement.STimebase.SaaDayPlans[bIdx1][bIdx2].
      bActionNumberOIDLength = sizeof(SGlobalTimeManagement
                                      .
                                      STimebase.SaaDayPlans[bIdx1][bIdx2].
                                      laActionNumberOID);
    }
  }

  if (IsDaylightSavingTimeFlagSet())
  {
    SGlobalTimeManagement.bDaylightSaving = GTM_DAYLIGHT_SAVING_OTHER;
  }
  else
  {
    SGlobalTimeManagement.bDaylightSaving = GTM_DAYLIGHT_SAVING_DISABLE;
  }

  for (bIdx1 = 0; bIdx1 < GTM_DAYLIGHT_SAVINGS_MAX; bIdx1++)
  {
    SGlobalTimeManagement.SaDaylightSavings[bIdx1].bNumber = bIdx1 + 1;
    SGlobalTimeManagement.SaDaylightSavings[bIdx1].bBeginMonth =
      GTM_DAYLIGHT_SAVING_MONTH_MAR;
    SGlobalTimeManagement.SaDaylightSavings[bIdx1].bBeginOccurences =
      GTM_DAYLIGHT_SAVING_OCCURRENCES_FOURTH;
    SGlobalTimeManagement.SaDaylightSavings[bIdx1].bBeginDayOfWeek =
      GTM_DAYLIGHT_SAVING_DAY_OF_WEEK_SUN;
    SGlobalTimeManagement.SaDaylightSavings[bIdx1].bBeginDayOfMonth = 1;
    SGlobalTimeManagement.SaDaylightSavings[bIdx1].lBeginSecondToTransition = 0;
    SGlobalTimeManagement.SaDaylightSavings[bIdx1].bEndMonth =
      GTM_DAYLIGHT_SAVING_MONTH_OCT;
    SGlobalTimeManagement.SaDaylightSavings[bIdx1].bEndOccurences =
      GTM_DAYLIGHT_SAVING_OCCURRENCES_FOURTH;
    SGlobalTimeManagement.SaDaylightSavings[bIdx1].bEndDayOfWeek =
      GTM_DAYLIGHT_SAVING_DAY_OF_WEEK_SUN;
    SGlobalTimeManagement.SaDaylightSavings[bIdx1].bEndDayOfMonth = 1;
    SGlobalTimeManagement.SaDaylightSavings[bIdx1].lEndSecondToTransition = 0;
    SGlobalTimeManagement.SaDaylightSavings[bIdx1].sSecondsToAdjust = 3600;
  }
} /* GTMInit */

uint8_t GTMGetGlobalTime(uint32_t *ulEpoch)
{
  tSTime STimeNow;

  TimeGet(&STimeNow);

  TimeUTCCalculate(&STimeNow);

  return TimeEpochCalculate(&STimeNow, ulEpoch);
}

uint8_t GTMSetGlobalTime(uint32_t ulEpoch)
{
  tSTime STimeNow;

  memset(&STimeNow, 0, sizeof(STimeNow));

  if (TimeCalculate(ulEpoch, &STimeNow))
  {
    if (TimeIsValid(&STimeNow))
    {
      GpsTimeAdjust(&STimeNow);
      TimeSet(&STimeNow);

      return TRUE;
    }
  }

  return FALSE;
}

uint8_t GTMGetControllerLocalTime(uint32_t *ulEpoch)
{
  tSTime STimeNow;

  TimeGet(&STimeNow);

  return TimeEpochCalculate(&STimeNow, ulEpoch);
}

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  ASC Channel */
tpSASCChannel GetASCChannelPtr(void)
{
  return &SASCChannel;
}

void ASCChannelInit(void)
{
  uint8_t bIdx = 0;

  memset(&SASCChannel, 0, sizeof(SASCChannel));

  for (bIdx = 0; bIdx < SIGNAL_GROUPS_MAX; bIdx++)
  {
    SASCChannel.SaChannels[bIdx].bNumber = bIdx + 1;

    uint8_t bPhaseIdx = 0;

    for (bPhaseIdx = 0; bPhaseIdx < PhaseTotalGet(); bPhaseIdx++)
    {
      if (PhaseHasSG(bPhaseIdx, bIdx))
      {
        SASCChannel.SaChannels[bIdx].bControlSource = bPhaseIdx + 1;
      }
    }

    uint8_t bSGType = SGTypeGet(bIdx);

    if ((bSGType == SIGNAL_GROUP_TYPE_VEHICLE_MAINWAY)
        || (bSGType == SIGNAL_GROUP_TYPE_VEHICLE_SUBWAY) )
    {
      SASCChannel.SaChannels[bIdx].bControlType =
        CHANNEL_CONTROL_TYPE_PHASE_VEHICLE;
    }
    else if (bSGType == SIGNAL_GROUP_TYPE_PEDESTRIAN)
    {
      SASCChannel.SaChannels[bIdx].bControlType =
        CHANNEL_CONTROL_TYPE_PHASE_PEDESTRAIN;
    }
    else
    {
      SASCChannel.SaChannels[bIdx].bControlType = CHANNEL_CONTROL_TYPE_OTHER;
    }

    if (SignalHasYellow(SGFlashSignalGet(bIdx)))
    {
      SASCChannel.SaChannels[bIdx].USFlash.SFlash.fYellow = 1;
    }
    else if (SignalHasRed(SGFlashSignalGet(bIdx)))
    {
      SASCChannel.SaChannels[bIdx].USFlash.SFlash.fRed = 1;
    }

    SASCChannel.SaChannels[bIdx].bGreenType = CHANNEL_GREEN_TYPE_OTHER;

    SASCChannel.SaChannels[bIdx].sIntersectionId = MCSGetSNMPDeviceID();
  }

  for (bIdx = 0; bIdx < CHANNEL_STATUS_GROUPS_MAX; bIdx++)
  {
    SASCChannel.SaStatues[bIdx].bNumber = bIdx + 1;
  }
} /* ASCChannelInit */

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  ASC Dector */
tpSASCDetector GetASCDetector(void)
{
  return &SASCDetector;
}

void ASCDetectorInit(void)
{
  uint8_t bLDIdx = 0, bIOIdx = 0;

  memset(&SASCDetector, 0, sizeof(SASCDetector));

  for (bLDIdx = 0; bLDIdx < INPUTS_DETECTOR_MAX; bLDIdx++)
  {
    SASCDetector.SaVehicleDetectors[bLDIdx].bNumber = bLDIdx + 1;

    SASCDetector.SaVehicleDetectors[bLDIdx].USOptions.SOptions.fVolumeDetector =
      1;
    SASCDetector.SaVehicleDetectors[bLDIdx].USOptions.SOptions.
    fOccupancyDetector = 1;

    uint8_t bPhaseIdx = 0;
    uint8_t bOwnerSG = InputOwnerSGGet(INPUT_TYPE_DETECTOR, bLDIdx);

    for (bPhaseIdx = 0; bPhaseIdx < PhaseTotalGet(); bPhaseIdx++)
    {
      if (PhaseHasSG(bPhaseIdx, bOwnerSG))
      {
        SASCDetector.SaVehicleDetectors[bLDIdx].bCallPhase = bPhaseIdx + 1;
      }
    }
  }

  for (bIOIdx = 0; bIOIdx < INPUTS_DIGITAL_MAX; bIOIdx++)
  {
    SASCDetector.SaPedestrianDetectors[bIOIdx].bNumber = bIOIdx + 1;

    uint8_t bPhaseIdx = 0;
    uint8_t bOwnerSG = InputOwnerSGGet(INPUT_TYPE_DIGITAL, bIOIdx);

    for (bPhaseIdx = 0; bPhaseIdx < PhaseTotalGet(); bPhaseIdx++)
    {
      if (PhaseHasSG(bPhaseIdx, bOwnerSG))
      {
        SASCDetector.SaPedestrianDetectors[bIOIdx].bCallPhase = bPhaseIdx + 1;
      }
    }
  }
} /* ASCDetectorInit */

/* Cabinet Environment Devices */
tpSASCCabinetEnvironment GetASCCabinetEnvironmentPtr(void)
{
  return &SASCCabinetEnvironment;
}

void ASCCabinetEnvironmentInit(void)
{
  memset(&SASCCabinetEnvironment, 0, sizeof(SASCCabinetEnvironment));

  SASCCabinetEnvironment.bATCCLEDMode = ATCC_LED_MODE_OTHER;

  SASCCabinetEnvironment.SaaEnvironmentDevices[0][0].bNumber = 1;
  SASCCabinetEnvironment.SaaEnvironmentDevices[0][0].bIndex = 1;
  SASCCabinetEnvironment.SaaEnvironmentDevices[0][0].bType =
    CABINET_ENVIRONMENT_DEVICE_TYPE_DOOR;
  strcpy(SASCCabinetEnvironment.SaaEnvironmentDevices[0][0].strDescription,
         "DOOR");
  SASCCabinetEnvironment.SaaEnvironmentDevices[0][0].bOnStatus =
    CABINET_ENVIRONMENT_DEVICE_ON_STATUS_FALSE;
  SASCCabinetEnvironment.SaaEnvironmentDevices[0][0].bErrorStatus =
    CABINET_ENVIRONMENT_DEVICE_ERROR_STATUS_NO_ERROR;

  SASCCabinetEnvironment.SaaEnvironmentDevices[1][0].bNumber = 2;
  SASCCabinetEnvironment.SaaEnvironmentDevices[1][0].bIndex = 1;
  SASCCabinetEnvironment.SaaEnvironmentDevices[1][0].bType =
    CABINET_ENVIRONMENT_DEVICE_TYPE_FAN;
  strcpy(SASCCabinetEnvironment.SaaEnvironmentDevices[1][0].strDescription,
         "FAN");
  SASCCabinetEnvironment.SaaEnvironmentDevices[1][0].bOnStatus =
    CABINET_ENVIRONMENT_DEVICE_ON_STATUS_FALSE;
  SASCCabinetEnvironment.SaaEnvironmentDevices[1][0].bErrorStatus =
    CABINET_ENVIRONMENT_DEVICE_ERROR_STATUS_NOT_MONITORED;

  SASCCabinetEnvironment.SaaEnvironmentDevices[2][0].bNumber = 3;
  SASCCabinetEnvironment.SaaEnvironmentDevices[2][0].bIndex = 1;
  SASCCabinetEnvironment.SaaEnvironmentDevices[2][0].bType =
    CABINET_ENVIRONMENT_DEVICE_TYPE_HEATER;
  strcpy(SASCCabinetEnvironment.SaaEnvironmentDevices[2][0].strDescription,
         "HEATER");
  SASCCabinetEnvironment.SaaEnvironmentDevices[2][0].bOnStatus =
    CABINET_ENVIRONMENT_DEVICE_ON_STATUS_FALSE;
  SASCCabinetEnvironment.SaaEnvironmentDevices[2][0].bErrorStatus =
    CABINET_ENVIRONMENT_DEVICE_ERROR_STATUS_NOT_MONITORED;

  SASCCabinetEnvironment.SaTempSensorStatuses[0].bNumber = 1;
  strcpy(SASCCabinetEnvironment.SaTempSensorStatuses[0].strDescription,
         "TEMP SENSOR");
  SASCCabinetEnvironment.SaTempSensorStatuses[0].bStatus =
    CABINET_TEMP_SENSOR_STATUS_FAIL;

  SASCCabinetEnvironment.SaHumiditySensorStatuses[0].bNumber = 1;
  strcpy(SASCCabinetEnvironment.SaHumiditySensorStatuses[0].strDescription,
         "HUMIDITY SENSOR");
  SASCCabinetEnvironment.SaHumiditySensorStatuses[0].bStatus =
    CABINET_HUMIDITY_SENSOR_STATUS_FAIL;
} /* ASCCabinetEnvironmentInit */

/* ASC Unit */
tpSAscUnit GetUnitPtr(void)
{
  return &SAscUnit;
}

tpSAscClock GetUnitAscClockPtr(void)
{
  return &SAscUnit.SAscClock;
}

void UnitInit(void)
{
  uint8_t bIdx = 0;

  memset(&SAscUnit, 0, sizeof(SAscUnit));

  SAscUnit.SAscClock.SaTimeTable[0].bNumber = 1;
  SAscUnit.SAscClock.SaTimeTable[0].bSourceAvailable =
    UNIT_TIME_SOURCE_AVILABLE_RTC_SQWR;
  SAscUnit.SAscClock.SaTimeTable[1].bNumber = 2;
  SAscUnit.SAscClock.SaTimeTable[1].bSourceAvailable =
    UNIT_TIME_SOURCE_AVILABLE_GNSS;
  SAscUnit.SAscClock.SaTimeTable[2].bNumber = 3;
  SAscUnit.SAscClock.SaTimeTable[2].bSourceAvailable =
    UNIT_TIME_SOURCE_AVILABLE_LINE_SYNC;
  SAscUnit.SAscClock.SaTimeTable[3].bNumber = 4;
  SAscUnit.SAscClock.SaTimeTable[3].bSourceAvailable =
    UNIT_TIME_SOURCE_AVILABLE_NTP;

  switch (TimeSourceGet())
  {
      case TIME_SOURCE_RTC:
      {
        SAscUnit.SAscClock.bSourceCommanded =
          UNIT_TIME_SOURCE_AVILABLE_RTC_SQWR;
        break;
      }

      case TIME_SOURCE_GPS:
      {
        SAscUnit.SAscClock.bSourceCommanded = UNIT_TIME_SOURCE_AVILABLE_GNSS;
        break;
      }

      default:
      {
        SAscUnit.SAscClock.bSourceCommanded = UNIT_TIME_SOURCE_AVILABLE_OTHER;
        break;
      }
  }

  for (bIdx = 0; bIdx < UNIT_ALARM_GROUPS_MAX; bIdx++)
  {
    SAscUnit.SaAlarmGroups[bIdx].bNumber = bIdx + 1;
  }
} /* UnitInit */

/* ASC Coord */
tpSAscCoord GetCoordPtr(void)
{
  return &SAscCoord;
}

void CoordInit(void)
{
  uint8_t bPatternIdx = 0, bSplitIdx = 0, bPhaseIdx = 0;

  memset(&SAscCoord, 0, sizeof(SAscCoord));

  for (bPatternIdx = 0; bPatternIdx < ASC_PATTERNS_MAX; bPatternIdx++)
  {
    SAscCoord.SaPatterns[bPatternIdx].bNumber = bPatternIdx + 1;
  }

  for (bSplitIdx = 0; bSplitIdx < ASC_SPLITS_MAX; bSplitIdx++)
  {
    for (bPhaseIdx = 0; bPhaseIdx < PHASES_MAX; bPhaseIdx++)
    {
      SAscCoord.SaaSplits[bSplitIdx][bPhaseIdx].bNumber = bSplitIdx + 1;
      SAscCoord.SaaSplits[bSplitIdx][bPhaseIdx].bPhase = bPhaseIdx + 1;
    }
  }
}

void CoordSplitTimeSet(void)
{
  uint8_t bEntryIdx = 0, bPhaseIdx = 0;

  for (bEntryIdx = 0;
       bEntryIdx < SCP.SConsumed.baWPEntriesTotal[WorkPlanCurNoGet()];
       bEntryIdx++)
  {
    for (bPhaseIdx = 0; bPhaseIdx < PhaseTotalGet(); bPhaseIdx++)
    {
      SAscCoord.SaaSplits[bEntryIdx][bPhaseIdx].bTime =
        SCP.SaWorkPlan[WorkPlanCurNoGet()][bEntryIdx].baPhaseDur[bPhaseIdx]
        + PhaseExtDurGet(bPhaseIdx);
    }
  }
}

/* Timebase ASC */
tpSTimebaseAsc GetTimebaseAscPtr(void)
{
  return &STimebaseAsc;
}

void TimebaseAscInit(void)
{
  uint8_t bIdx = 0;

  memset(&STimebaseAsc, 0, sizeof(STimebaseAsc));

  for (bIdx = 0; bIdx < TIMEBASE_ASC_ACTIONS_MAX; bIdx++)
  {
    STimebaseAsc.SaActions[bIdx].bNumber = bIdx + 1;
  }
}

/* ASC Ring */
tpSAscRing GetRingPtr(void)
{
  return &SAscRing;
}

void RingInit(void)
{
  uint8_t bIdx1 = 0, bIdx2 = 0;

  memset(&SAscRing, 0, sizeof(SAscRing));

  for (bIdx1 = 0; bIdx1 < SEQUENCE_PLANS_MAX; bIdx1++)
  {
    for (bIdx2 = 0; bIdx2 < RINGS_MAX; bIdx2++)
    {
      SAscRing.SaaSequencePlans[bIdx1][bIdx2].bNumber = bIdx1 + 1;
      SAscRing.SaaSequencePlans[bIdx1][bIdx2].bRingNumber = bIdx2 + 1;
    }
  }

  for (bIdx1 = 0; bIdx1 < RING_CONTROL_GROUPS_MAX; bIdx1++)
  {
    SAscRing.SaControlGroups[bIdx1].bNumber = bIdx1 + 1;
  }
}

/* Overlap */
tpSOverlap GetOverlapPtr(void)
{
  return &SOverlap;
}

void OverlapInit(void)
{
  uint8_t bIdx = 0;

  memset(&SOverlap, 0, sizeof(SOverlap));

  for (bIdx = 0; bIdx < OVERLAPS_MAX; bIdx++)
  {
    SOverlap.SaOverlaps[bIdx].bNumber = bIdx + 1;
  }

  for (bIdx = 0; bIdx < OVERLAP_STATUS_GROUPS_MAX; bIdx++)
  {
    SOverlap.SaStatues[bIdx].bNumber = bIdx + 1;
  }
}

/* ASC Block */
tpSAscBlock GetAscBlockPtr(void)
{
  return &SAscBlock;
}

void AscBlockInit(void)
{
  memset(&SAscBlock, 0, sizeof(SAscBlock));
}

/* Preempt */
tpSPreempt GetPreemptPtr(void)
{
  return &SPreempt;
}

void PreemptInit(void)
{
  uint8_t bIdx1 = 0, bIdx2 = 0;

  memset(&SPreempt, 0, sizeof(SPreempt));

  for (bIdx1 = 0; bIdx1 < PREEMPTS_MAX; bIdx1++)
  {
    SPreempt.SaPreempts[bIdx1].bNumber = bIdx1 + 1;
    SPreempt.SaPreempts[bIdx1].bExitType = PREEMPT_EXIT_PHASES;
    SPreempt.SaPreempts[bIdx1].bState = PREEMPT_STATE_NOT_ACTIVE;
  }

  for (bIdx1 = 0; bIdx1 < PREEMPTS_MAX; bIdx1++)
  {
    SPreempt.SaPreemptControls[bIdx1].bNumber = bIdx1 + 1;
  }

  for (bIdx1 = 0; bIdx1 < PREEMPT_GROUPS_MAX; bIdx1++)
  {
    SPreempt.SaStatusGroups[bIdx1].bNumber = bIdx1 + 1;
  }

  for (bIdx1 = 0; bIdx1 < PREEMPTS_MAX; bIdx1++)
  {
    for (bIdx2 = 0; bIdx2 < INPUTS_DETECTOR_MAX; bIdx2++)
    {
      SPreempt.SaaQueueDelays[bIdx1][bIdx2].bNumber = bIdx1 + 1;
      SPreempt.SaaQueueDelays[bIdx1][bIdx2].bVehDetectorNumber = bIdx2 + 1;
    }
  }

  for (bIdx1 = 0; bIdx1 < PREEMPT_GATES_MAX; bIdx1++)
  {
    SPreempt.SaGates[bIdx1].bNumber = bIdx1 + 1;
  }
}

/* ASC Block */
tpSAscPhase GetAscPhasePtr(void)
{
  return &SAscPhase;
}

void AscPhaseInit(void)
{
  uint8_t bIdx = 0;

  memset(&SAscPhase, 0, sizeof(SAscPhase));

  for (bIdx = 0; bIdx < PHASES_MAX; bIdx++)
  {
    SAscPhase.SaPhases[bIdx].bNumber = bIdx + 1;
  }

  for (bIdx = 0; bIdx < PHASE_STATUS_GROUPS_MAX; bIdx++)
  {
    SAscPhase.SaStatuses[bIdx].bNumber = bIdx + 1;
  }

  for (bIdx = 0; bIdx < PHASE_CONTROL_GROUPS_MAX; bIdx++)
  {
    SAscPhase.SaControlGroups[bIdx].bNumber = bIdx + 1;
  }
}
