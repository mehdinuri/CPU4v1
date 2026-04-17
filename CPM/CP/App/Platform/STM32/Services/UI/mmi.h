#ifndef __MMI_H__
#define __MMI_H__

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  include files */
#include "MLM.h"
#include "fdcan.h"

#define CAN_MMI_REQUEST_MAX 10

/*  MMI */
#define CAN_MMI_STD_ID_LAST 0x7FF

/*  Runtime */
typedef struct _tSMMIRuntime
{
  uint8_t bSOTestSONo;

  struct
  {
    uint8_t fSignalStream : 1;
    uint8_t fInputStream : 1;
    uint8_t fSOTestStream : 1;
    uint8_t fReserved : 5;
  } SFlags;
} tSMMIRuntime, *tpSMMIRuntime;

/*  Log */
#define CAN_MMI_LOG_INDEX_MAX_VALUE 1024

#define CAN_MMI_RUNTIME_MEASUREMENT_REQUEST_STD_ID 0x7FF
#define CAN_MMI_RUNTIME_MEASUREMENT_ANSWER_STD_ID 0x7FE
typedef struct _tSMMIMeasurement
{
  uint16_t psm1Voltage;
  uint8_t psm1Frequency;
  uint16_t psm2Voltage;
  uint8_t psm2Frequency;
} __attribute__((packed)) tSMMIMeasurement, *tpSMMIMeasurement;

#define CAN_MMI_RUNTIME_WORKMODE_REQUEST_STD_ID 0x7FD
#define CAN_MMI_RUNTIME_WORKMODE_ANSWER_STD_ID 0x7FC
/*  States */
/*  0: None */
/*  1: Any */
/*  2: No Control */
/*  3: Flash */
/*  4: Closed */
/*  5: Phase */
/*  6: Phase Transition */
/*  7: Sequence */
/*  8: Secure Transition */
/*  9: Program Loading  ->  Arg1  : 1: Just Started */
/*                    2: */
/* Main -> Back up                    3: Main -> Back up Success */
/* 4: Main -> Back up Error                     5: Back up */
/* -> Main                    6: Back up -> Main Success                    7: */
/* Back up -> Main Error                    8: In Progress                    9: */
/* Success                   10: Error                   11: To Main */
/* 12: To Back up */
/*  */
/*  Example1: */
/* Example2: */
/* Example3:  State : Sequence */
/* State  : Phase */
/* State  : Program Loading   Arg1  : Current Sequence */
/* Arg1 : Current Phase         Arg1  : Success   Arg2 */
/* : Number of Sequences */
/* Arg2 : Number of Phases  Arg3  : Current Sequence Step */
/* Arg3 : Min Duration  Arg4  : Number of Sequence Steps */
/* Arg4 : Max Duration  Arg5  : Current Sequence Step Second */
/* Arg5 : Current Duration  Arg6  : Current Sequence Step Duration  Arg7  : */
/* Current Sequence Second  Arg8  : Current Sequence Duration */

#define STATES_PROGRAM_LOAD 9
typedef struct _tSMMIWorkmode
{
  uint8_t bState;
  uint8_t bArg1 : 4; /* (0 - 15) */
  uint8_t bArg2 : 4;
  uint8_t bArg3;
  uint8_t bArg4;
  uint8_t bArg5;
  uint8_t bArg6;
  uint8_t bArg7;
  uint8_t bArg8;
} __attribute__((packed)) tSMMIWorkmode, *tpSMMIWorkmode;

#define CAN_MMI_RUNTIME_TIME_REQUEST_STD_ID 0x7FB
#define CAN_MMI_RUNTIME_TIME_ANSWER_STD_ID 0x7FA
/*  Time Sources */
/*  1: NETWORK */
/*  2: RTC */
/*  3: GPS */
/*  4: CENTRAL SYSTEM */
typedef struct _tSMMITime
{
  uint8_t bTimeSource;
  uint8_t bSeconds;
  uint8_t bMinutes;
  uint8_t bHours;
  uint8_t bDay;
  uint8_t bMonth;
  uint8_t bYear; /* (Year - Century * 100) */
} __attribute__((packed)) tSMMITime, *tpSMMITime;

#define CAN_MMI_MODULE_STATUS_REQUEST_STD_ID 0x7F9
#define CAN_MMI_MODULE_STATUS_ANSWER_STD_ID 0x7F8
typedef struct _tSMMIModule
{
  uint8_t fGPSModemConnected : 1;
  uint8_t fGPSAntennaConnected : 1;
  uint8_t fGPRSModemConnected : 1;
  uint8_t fGPRSCenterConnected : 1;
  uint8_t fIsRelayClosed : 1;
  uint8_t bReserved : 3;

  uint8_t bLastDigitalInputDemand;
  uint8_t bLastLoopDedectorDemand;
  uint8_t bGPSModeType;
} __attribute__((packed)) tSMMIModule, *tpSMMIModule;

#define CAN_MMI_ERROR_REQUEST_STD_ID 0x7F7
#define CAN_MMI_ERROR_ANSWER_STD_ID 0x7F6

typedef struct _tSMMIError
{
  uint8_t bSetNo;
  uint8_t bSignalingMode; /* signaling mode of set */
  uint8_t bSigModeSource; /* cause of the signaling mode above */
  uint8_t bParam1;
  uint8_t bParam2;
} __attribute__((packed)) tSMMIError, *tpSMMIError;

#define CAN_MMI_KEY_REQUEST_STD_ID 0x7F5
#define CAN_MMI_KEY_ANSWER_STD_ID 0x7F4

#define CAN_MMI_CHANGE_MODE_REQUEST_STD_ID 0x7F3
#define CAN_MMI_CHANGE_MODE_ANSWER_STD_ID 0x7F2

typedef struct _tSMMIChangeMode
{
  uint8_t bRequestedMode;
} __attribute__((packed)) tSMMIChangeMode, *tpSMMIChangeMode;

#define CAN_MMI_SET_TIME_REQUEST_STD_ID 0x7F1
#define CAN_MMI_SET_TIME_ANSWER_STD_ID 0x7F0
typedef struct _tSMMISetTime
{
  uint8_t bSecond;
  uint8_t bMinute;
  uint8_t bHour;
  uint8_t bDay;
  uint8_t bMonth;
  uint8_t bYear; /* (Year - Century * 100) */
} __attribute__((packed)) tSMMISetTime, *tpSMMISetTime;

#define CAN_MMI_GET_LOG_REQUEST_STD_ID 0x7EF
#define CAN_MMI_GET_LOG_ANSWER_DATE_TIME_STD_ID 0x7EE
#define CAN_MMI_GET_LOG_ANSWER_CONTENT_STD_ID 0x7ED
typedef struct _tSMMIGetLogRequest
{
  uint16_t sLogIndex;
} tSMMIGetLogRequest, *tpSMMIGetLogRequest;

typedef struct _tSMMILogTime
{
  uint8_t bSecond;
  uint8_t bMinute;
  uint8_t bHour;
  uint8_t bDay;
  uint8_t bMonth;
  uint8_t bYear; /* (Year - Century * 100) */
} __attribute__((packed)) tSMMILogTime, *tpSMMILogTime;

typedef struct _tSMMILogContent
{
  uint8_t bLog;
  uint8_t bParam;
  uint16_t sParam;
  uint32_t lParam;
} __attribute__((packed)) tSMMILogContent, *tpSMMILogContent;

#define CAN_MMI_GET_GPRS_MODEM_LOG_REQUEST_STD_ID 0x7EC
#define CAN_MMI_GET_GPRS_MODEM_LOG_ANSWER_STD_ID 0x7EB
typedef struct _tSMMIGprsLog
{
  uint8_t bModem;
  uint8_t bState;
  uint8_t bSubState;
  uint8_t bSignalQuality; /*  0-5 */
} __attribute__((packed)) tSMMIGprsLog, *tpSMMIGprsLog;

#define CAN_MMI_SET_RELAY_STATE_REQUEST_STD_ID 0x7EA
#define CAN_MMI_SET_RELAY_STATE_ANSWER_STD_ID 0x7E9
typedef struct _tSMMIRelayState
{
  uint8_t bRelayStateRequest;
} __attribute__((packed)) tSMMIRelayState, *tpSMMIRelayState;

#define CAN_MMI_SET_GPS_PORT_REQUEST_STD_ID 0x7E8
#define CAN_MMI_SET_GPS_PORT_ANSWER_STD_ID 0x7E7

typedef struct _tSMMIGpsSettingsPort
{
  uint8_t bGpsPortRequest;
} __attribute__((packed)) tSMMIGpsSettingsPort, *tpSMMIGpsSettingsPort;

/* H&D Commented */

/*
 #define  CAN_MMI_SET_HEATER_SETTINGS_REQUEST_STD_ID    0x7E6
 #define  CAN_MMI_SET_HEATER_SETTINGS_ANSWER_STD_ID     0x7E5
 *  typedef struct _tSMMIHeater
 *  {
 *       uint8_t  bState;
 *       uint8_t  bLogicLevel;
 *
 *  } __attribute__((packed)) tSMMIHeater, *tpSMMIHeater;
 *
 #define  CAN_MMI_SET_DIMMING_SETTINGS_REQUEST_STD_ID   0x7E4
 #define  CAN_MMI_SET_DIMMING_SETTINGS_ANSWER_STD_ID    0x7E3
 *  typedef struct _tSMMIDimming
 *  {
 *       uint8_t  bState;
 *       uint8_t  bLogicLevel;
 *
 *  } __attribute__((packed)) tSMMIDimming, *tpSMMIDimming;
 */

#define CAN_MMI_GET_SIGNALS_REQUEST_STD_ID 0x7E2
#define CAN_MMI_GET_SIGNALS_ANSWER_STD_ID 0x7E1
#define CAN_MMI_MAX_SSM_PER_MSG 4
typedef struct _tSMMISGSignals
{
  uint16_t SMMISSMSignals[CAN_MMI_MAX_SSM_PER_MSG];
} __attribute__((packed)) tSMMISGSignals, *tpSMMISGSignals;

#define CAN_MMI_GET_INPUTS_REQUEST_STD_ID 0x7E0
#define CAN_MMI_GET_INPUTS_ANSWER_STD_ID 0x7DF
typedef struct _tSMMIInputs
{
  uint32_t lLoopDemands;
  uint32_t lDigitalInputDemands;
} __attribute__((packed)) tSMMIInputs, *tpSMMIInputs;

/* H&D Commented */

/*
 #define  CAN_MMI_GET_HEATER_SETTINGS_REQUEST_STD_ID    0x7DE
 #define  CAN_MMI_GET_HEATER_SETTINGS_ANSWER_STD_ID     0x7DD
 *
 #define  CAN_MMI_GET_DIMMING_SETTINGS_REQUEST_STD_ID   0x7DC
 #define  CAN_MMI_GET_DIMMING_SETTINGS_ANSWER_STD_ID    0x7DB
 */

#define CAN_MMI_GET_LAST_LOG_INDEX_REQUEST_STD_ID 0x7DA
#define CAN_MMI_GET_LAST_LOG_INDEX_ANSWER_STD_ID 0x7D9

typedef struct _tSMMILastLogIndex
{
  uint16_t sMMILastLogIndex;
} __attribute__((packed)) tSMMILastLogIndex, *tpSMMILastLogIndex;

#define CAN_MMI_SET_GPRS_MODEM_REQUEST_STD_ID 0x7D8
typedef struct _tSMMISetGprsModem
{
  uint8_t bModemType;
} __attribute__((packed)) tSMMISetGprsModem, *tpSMMISetGprsModem;

#define CAN_MMI_FACTORY_DEFAULTS_REQUEST_STD_ID 0x7D7

#define CAN_MMI_START_SIGNAL_STREAM_STD_ID 0x7D6
#define CAN_MMI_STOP_SIGNAL_STREAM_STD_ID 0x7D5

#define CAN_MMI_START_INPUT_STREAM_STD_ID 0x7D4
#define CAN_MMI_STOP_INPUT_STREAM_STD_ID 0x7D3

/*  SSM Test */
#define CAN_MMI_SO_TEST_START_STD_ID 0x7D2
#define CAN_MMI_SO_TEST_STOP_STD_ID 0x7D1
#define CAN_MMI_SO_TEST_CHANGE_STD_ID 0x7D0
typedef struct _tSMMISOTest
{
  uint8_t bSONo; /* 0 - 95 */
} __attribute__((packed)) tSMMISOTest, *tpSMMISOTest;

/*  SO TEST MMI PRESENTATION */
/*  */
/*  1st MMI Line  %02d            -> bSONo */
/*  2nd MMI Line  %04d,%04d       -> */
/* sPowerNet, sPower  3rd MMI Line  %d,%04d */
/* -> bState, sNet  4th MMI Line  %04d,%04d,%04d  -> sMin, sNow, sMax */
#define CAN_MMI_SO_TEST_STREAM_1_STD_ID 0x7CF
typedef struct _tSMMISOStream1
{
  uint8_t bSONo; /* 1 - 96 */
  uint8_t bState;
  uint16_t sPowerNet;
  uint16_t sPower;
  uint16_t sNet;
} __attribute__((packed)) tSMMISOStream1, *tpSMMISOStream1;

#define CAN_MMI_SO_TEST_STREAM_2_STD_ID 0x7CE
typedef struct _tSMMISOStream2
{
  uint8_t bSONo; /* 1 - 96 */
  uint16_t sNow;
  uint16_t sMin;
  uint16_t sMax;
} __attribute__((packed)) tSMMISOStream2, *tpSMMISOStream2;

/*  Version */
/*  MAESTRO(Arg0) (Arg1).(Arg2).(Arg3)(Arg4) */
/*  Example:  MAESTRO2 2.1.0L */
#define CAN_MMI_VERSTON_REQUEST_STD_ID 0x7CD
#define CAN_MMI_VERSTON_ANSWER_STD_ID 0x7CC
typedef struct _tSMMIVersion
{
  uint8_t bArg0;
  uint8_t bArg1;
  uint8_t bArg2;
  uint8_t bArg3;
  uint8_t bArg4;
} __attribute__((packed)) tSMMIVersion, *tpSMMIVersion;

/*  GPRS Modem IMEI */
/*  IMEI number is 15 digits */
/*  so that it will be carried in two CAN Messages as */
/*  8 digits in Part 1 and 7 digits in Part 2 */
/*  Example: IMEI number is 12345679012345 */
/*  Part1: 12345678 */
/*  Part2: 9012345 */
#define CAN_MMI_STD_ID_GPRS_MODEM_IMEI_PART1 0x7CB
#define CAN_MMI_STD_ID_GPRS_MODEM_IMEI_PART2 0x7CA

/*  Cabinet Door State Change */
#define CAN_MMI_STD_ID_CABINET_DOOR_STATE_CHANGE 0x7C9
typedef struct _tSMMICabinetDoorStateChange
{
  uint8_t fState;
  uint8_t bEvent;
  uint8_t bSeconds;
  uint8_t bMinutes;
  uint8_t bHours;
  uint8_t bDay;
  uint8_t bMonth;
  uint8_t bYear;
} __attribute__((packed)) tSMMICabinetDoorStateChange,
*tpSMMICabinetDoorStateChange;

#define CAN_MMI_STD_ID_OPEN_MMI 0x7C8
#define CAN_MMI_STD_ID_CLOSE_MMI 0x7C7

#define CAN_MMI_STD_ID_LAST_MMI_CLOSE_LOG_REQUEST 0x7C6
#define CAN_MMI_STD_ID_LAST_MMI_CLOSE_LOG_ANSWER 0x7C5

#define CAN_MMI_STD_ID_LAST_MMI_OPEN_LOG_REQUEST 0x7C4
#define CAN_MMI_STD_ID_LAST_MMI_OPEN_LOG_ANSWER 0x7C3

#define CAN_MMI_GET_USER_SETTINGS_REQUEST_STD_ID 0x7C2
#define CAN_MMI_GET_USER_SETTINGS_ANSWER_STD_ID 0x7C1

#define CAN_MMI_SET_USER_SETTINGS_REQUEST_STD_ID 0x7C0
#define CAN_MMI_SET_USER_SETTINGS_ANSWER_STD_ID 0x7BF
typedef struct _tSMMIUserSettings
{
  uint8_t fSettingsChanged;
  uint8_t fConfigFlag;
  uint8_t fLogFlag;
  uint8_t fTrafficCountsFlag;
} __attribute__((packed)) tSMMIUserSettings, *tpSMMIUserSettings;

/* GPS Baud Rate */
#define CAN_MMI_GET_GPS_BAUD_RATE_REQUEST_STD_ID 0x7BE
#define CAN_MMI_GET_GPS_BAUD_RATE_ANSWER_STD_ID 0x7BD

#define CAN_MMI_SET_GPS_BAUD_RATE_REQUEST_STD_ID 0x7BC
#define CAN_MMI_SET_GPS_BAUD_RATE_ANSWER_STD_ID 0x7BB
typedef struct _tSMMIGpsSettingsBaudRateIndex
{
  uint8_t bGpsBaudRateIndexRequest;
} __attribute__((packed)) tSMMIGpsSettingsBaudRateIndex,
*tpSMMIGpsSettingsBaudRateIndex;

/* USR Ethernet Module MAC */
#define CAN_MMI_STD_ID_USR_MAC_PART1 0x7BA
#define CAN_MMI_STD_ID_USR_MAC_PART2 0x7B9

/* IAP Mode */
#define CAN_MMI_IAP_MODE_REQUEST_STD_ID 0x7B8
#define CAN_MMI_IAP_MODE_ANSWER_STD_ID 0x7B2

/* GPRS GSM Operator */
#define CAN_MMI_STD_ID_GPRS_GSM_OPERATOR 0x7B7

/* Username / Password */
#define CAN_MMI_GET_ADMIN_USER_INFO_REQUEST_STD_ID 0x7B6
#define CAN_MMI_GET_ADMIN_USER_INFO_ANSWER_STD_ID 0x7B5

#define CAN_MMI_SET_ADMIN_USER_INFO_REQUEST_STD_ID 0x7B4
#define CAN_MMI_SET_ADMIN_USER_INFO_ANSWER_STD_ID 0x7B3

#define CAN_MMI_GET_USER_SETTINGS_PART2_REQUEST_STD_ID 0x7B2
#define CAN_MMI_GET_USER_SETTINGS_PART2_ANSWER_STD_ID 0x7B1

#define CAN_MMI_SET_USER_SETTINGS_PART2_REQUEST_STD_ID 0x7B0
#define CAN_MMI_SET_USER_SETTINGS_PART2_ANSWER_STD_ID 0x7AF

typedef struct _tSMMIUserSettingsPart2
{
  uint8_t fStandbyInfoFlag : 1;
  uint8_t bReserved : 7;
  uint8_t baReserved[7];
} __attribute__((packed)) tSMMIUserSettingsPart2, *tpSMMIUserSettingsPart2;

#define CAN_MMI_GET_DAYLIGHT_SAVING_TIME_SETTINGS_REQUEST_STD_ID 0x7AE
#define CAN_MMI_GET_DAYLIGHT_SAVING_TIME_SETTINGS_ANSWER_STD_ID 0x7AD

#define CAN_MMI_SET_DAYLIGHT_SAVING_TIME_SETTINGS_REQUEST_STD_ID 0x7AC
#define CAN_MMI_SET_DAYLIGHT_SAVING_TIME_SETTINGS_ANSWER_STD_ID 0x7AB

typedef struct _tSDaylightSavingTimeSettings
{
  uint8_t fDaylightSavingTimeFlag : 1;
  uint8_t bReserved : 7;
  uint8_t baReserved[7];
} __attribute__((packed)) tSDaylightSavingTimeSettings,
*tpSDaylightSavingTimeSettings;

#define CAN_MMI_GET_BROKEN_INPUT_SETTINGS_REQUEST_STD_ID 0x7AA
#define CAN_MMI_GET_BROKEN_INPUT_SETTINGS_ANSWER_STD_ID 0x7A9

#define CAN_MMI_SET_BROKEN_INPUT_SETTINGS_REQUEST_STD_ID 0x7A8
#define CAN_MMI_SET_BROKEN_INPUT_SETTINGS_ANSWER_STD_ID 0x7A7

typedef struct _tSMMIBrokenInputSettings
{
  uint8_t fAlreadySet;
  uint8_t fLoopInputFlag;
  uint8_t fDigitalInputFlag;
} __attribute__((packed)) tSMMIBrokenInputSettings, *tpSSMMIBrokenInputSettings;

#define CAN_MMI_STD_ID_FIRST 0x7A7

extern void OpenMMI(void);
extern void CloseMMI(void);
extern void StreamPSMMeasurements(void);
extern void StreamDateTime(void);
extern void StreamGPRSImei(void);
extern void StreamUSRMAC(void);
extern void StreamEthernetMAC(void);
extern void StreamGsmOperator(void);
extern void StreamGPRSState(void);
extern void StreamOperationRuntime(void);
extern void StreamModuleRuntime(void);
extern void StreamGateStateChanged(uint8_t fState, tpSLogRecord pSLog);
extern void StreamErrorRuntime(void);
extern void StreamSignals(void);
extern void StreamInputs(void);
extern void StreamSOTest(void);
extern void MMIRequest(tpSFDCANRxMsg pSMMIReq);

#endif /* ifndef __MMI_H__ */
