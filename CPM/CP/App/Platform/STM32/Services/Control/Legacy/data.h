#ifndef __DATA_H__
#define __DATA_H__

#include "defs.h"
#include "stm32h7xx_hal.h"

/* Version */
#define MAESTRO_VERSION_ARG0 4
#define MAESTRO_VERSION_ARG1 4
#define MAESTRO_VERSION_ARG2 0
#define MAESTRO_VERSION_ARG3 2
#define MAESTRO_VERSION_ARG4 'T'

/* language support */
#ifndef LANGUAGES_MAX
#define LANGUAGES_MAX 2 /* Turkish, English */
#endif
#ifndef LANGUAGE_TURKISH
#define LANGUAGE_TURKISH 0
#endif
#ifndef LANGUAGE_ENGLISH
#define LANGUAGE_ENGLISH 1
#endif
#define LANGUAGE_NONE 2

/* LRLF Detect Time Settings */
typedef enum
{
  LRLF_DETECT_TIME_NONE = 0,
  LRLF_DETECT_TIME_300_MS,
  LRLF_DETECT_TIME_800_MS,
  LRLF_DETECT_TIME_2_S,
  LRLF_DETECT_TIME_MAX = LRLF_DETECT_TIME_2_S
} tELRLFDetectTime;

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Device Info */
typedef struct _SDeviceInfo
{
  char strCountry[32];
  char strCity[32];
  char strIntersection[32];
  int8_t TimeZone;
  uint8_t bDeviceType;
  uint8_t bCrossNo;
  char strIPNo[15];
  char strDomain[50];
  char strAPN[50];
  char strUsername[30];
  char strPassword[30];
} __attribute__((packed)) tSDeviceInfo, *tpSDeviceInfo;

typedef enum
{
  LAMP_DIMMING_ON = 0x00,
  LAMP_DIMMING_OFF,
} eLampDimmingSource;

typedef enum
{
  POWER_L_SOURCE_LCD = 0x00,
  POWER_L_SOURCE_MCS,
  POWER_L_SOURCE_LAMP_DIMMING,
} ePowerLearningSource;

#define DEVICE_TYPE_NONE 0
#define DEVICE_TYPE_MAESTRO 1

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Signal Group Sets */
#define SIGNAL_SETS_MAX 3

typedef struct _SSetRuntime
{
  uint8_t bSignalingMode; /*  signaling mode of set */
  uint8_t bSigModeSource; /*  cause of the signaling mode above */

  uint8_t bParam1;
  uint8_t bParam2;

  uint8_t fInvalidSignalDetected : 1; /*  is an invalid signal detected in */
                                      /* this set? */
  uint8_t fInvalidSignalSequenceDetected : 1; /*  is an invalid sequence */
                                              /* detected in this set? */
  uint8_t fRestartProgramRequest : 1; /*  has this set requested signal */
                                      /* program restart? */
  uint8_t fValid : 1; /*  is this set valid? */
  uint8_t fReserved : 4;
} tSSetRuntime, *tpSSetRuntime;

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Signaling Modes */
#define SIGNALING_MODE_NONE 0x00
#define SIGNALING_MODE_NORMAL 0x01
#define SIGNALING_MODE_FLASH 0x02

/*
 *       0-8 are used in MCS for program runtime states
 *       9 is used for signaling mode download
 *       10 is used for signaling mode emergency dark
 *       11 is used for signaling mode emergency flash
 *       12 is used in MCS for warning state
 *       13 is used in MCS for stop mode
 */
#define SIGNALING_MODE_EMERGENCY_DARK 0x0A
#define SIGNALING_MODE_EMERGENCY_FLASH 0x0B

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Signal Definitions */

/*(1)
 *       name: signal maximum values
 *       expl: there are three types of subsignal: red, yellow, and green. Below,
 *  referring to subsignals via indexing is provided. You can use these indexing
 *  mechanism while accessing subsignals in signal definition (see (3))
 */
#define SUBSIGNALS_MAX 3 /* red, yellow and green */
#define SUBSIGNAL_RED 0 /*  refer to red subsignal via this index */
#define SUBSIGNAL_YELLOW 1 /* refer to yellow subsignal via this index */
#define SUBSIGNAL_GREEN 2 /*  refer to green subsignal via this index */
#define SIGNALS_MAX 16 /* user can define this number of custom signals */

/*(2)
 *       name: sub-signal definition
 *       expl: sub signals have one of the colors: red, yellow and green.
 */
typedef struct _SSubSignalDef
{
  uint16_t sPeriod; /*  period for the color */
} __attribute__((packed)) tSSubSignalDef, *tpSSubSignalDef;

/*(3)
 *       name: signal definition
 *       expl: A signal is the composition of three subsignals whose colors are
 *  red, yellow and green. All of them has a specific period. A signal may be
 *  valid for flash or emergency flash mode.
 *
 *                       - Examples
 *
 *                               valid        duration
 *  unlimited     min duration    max duration
 *                               ---------    ------------------
 *  -------------   ------------- true
 *  false 5               25 true
 *  true
 *  5               not used false
 *  not used                not used
 *  not used
 *
 *                       - These signals can be called as custom signals. In
 *  other words, they are designed by the MCT user.
 *                       - Follower signals must also be defined by MCT user. The
 *  maximum number of custom signals is 16 at the time of programming. Therefore,
 *  2 bytes (16 bits) are used to show which signals can follow defined signal.
 */
typedef struct _SSignalDef
{
  struct
  {
    uint8_t fValid : 1; /* if it is usable for Maestro product, assign TRUE */
    uint8_t fValidForFlash : 1; /* shows if it is valid for flash mode */
    uint8_t fValidForEmergencyFlash : 1; /* shows if it is valid for emergency */
                                         /* flash mode */
    uint8_t fDurationUnlimited : 1; /* shows if there is no maxmimum duration */
                                    /* limit for this signal */
    uint8_t fReserved : 4;
  } __attribute__((packed)) SFlags;

  uint8_t bDummy; /* 1 byte space for 2 bytes packing here */
  tSSubSignalDef SaSignal[SUBSIGNALS_MAX]; /* all signals is a composition of */
                                           /* three subsignals: red, yellow, */
                                           /* green, use subsignal definitions */
                                           /* to access the subsignal */
  uint16_t sFollowers; /* signals that can follow this signal */
  uint8_t bMinDur; /* this signal cannot have a duration smaller than this - in */
                   /* terms of seconds */
  uint8_t bMaxDuration; /* this signal cannot have a duration greater than this */
                        /* - in terms of seconds */
} __attribute__((packed)) tSSignalDef, *tpSSignalDef;

/*(4)
 *       name: predefined signals
 *       expl: some signals are predefined like red, green, green flash, and
 *  dark.
 */
typedef struct _SSignalsDefined
{
  uint8_t bBlocking : 4; /* red signal */
  uint8_t bFree : 4; /* green signal */
  uint8_t bGreenFlash : 4; /* green flash signal */
  uint8_t bDark : 4; /* dark signal */
} __attribute__((packed)) tSSignalsDefined, *tpSSignalsDefined;

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Current/Voltage/Slope Reference Values */

/*(1)
 *       - For each lamp type, we have reference values which are sent from MCT
 *  user
 *       - At the time of programming, known lamp types are E27, Halogen, and
 *  LED.
 */

#define LAMP_CURRENT_PERCENTAGE (0.70)

/*(2)
 *       name: types of lamps
 *       expl: these values are also used for access via indexing for the arrays
 *  in the following CVS definition.
 */
#define LAMP_TYPE_NONE 255
#define LAMP_TYPE_1 0
#define LAMP_TYPE_2 1
#define LAMP_TYPE_3 2
#define LAMP_TYPE_MAX 3

/*(3)
 *       name: current/voltage/slope reference values for each lamp type
 *       expl:
 */
typedef struct _SCVSDef
{
  uint16_t saCurrents[LAMP_TYPE_MAX];
  uint16_t saVoltages[LAMP_TYPE_MAX];
  double raSlopes[LAMP_TYPE_MAX];
} __attribute__((packed)) tSCVSDef, *tpSCVSDef;

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Signal Outputs */

/*(1)
 *       - An array of signal outputs are defined in data.c. The length of this
 *  array is signal outputs maximum value, 96 at the time of programming. This
 *  array is one dimensional. Besides, signal outputs on SSMs and SSMs have
 *  signal groups on them. An index value for this array refers to a physical
 *  output on the device. Examples are given about physical output access via
 *  this indexing mechanism.
 *
 *               - Examples:
 *
 *                       index        SSM
 *  signal group number on SSM        signal output on signal
 *  group
 *                       -------      ------
 *  --------------------------
 *  ----------------------------- 0 0 0
 *  0 22          1
 *  3
 *  1 27          2
 *  1
 *  0 50          4
 *  0
 *  2 89          8
 *  2
 *  2
 */

/* maximum number of physical outputs on the device */
#define SIGNAL_OUTPUTS_MAX 96

/* signal outputs are grouped on SSMs. Each output group consist of 3 outputs */
/* and these share the same current measurement circuit */
#define SIGNAL_OUTPUTS_PER_SSM 12
#define SIGNAL_OUTPUT_CURRENT_GROUPS_MAX 32
#define SIGNAL_OUTPUT_CURRENT_GROUPS_PER_SSM 4
#define SIGNAL_OUTPUTS_PER_CURRENT_GROUP 3

/*(2)
 *       name:  signal output power record
 */
typedef struct _tSSOPowerRecord
{
  uint16_t sPower[2];
  uint16_t sPowerNet[2];

  struct
  {
    uint8_t bPowerRecorded0 : 1;
    uint8_t bPowerRecorded1 : 1;
    uint8_t fReserved : 6;
  } __attribute__((packed)) SFlags;
} __attribute__((packed)) tSSOPowerRecord, *tpSSOPowerRecord;

/*(3)
 *       name: signal output definition
 *       expl:
 *               - At the time of programming there are 3 types of lamps (See
 *  Current/Voltage/Slope Reference Values (2))
 *               - A signal may have one of the types: red, yellow, green (see
 *  (3))
 *               - A signal output belongs to a signal group
 *               - A signal output has a number of lamps. At the time of
 *  programming, the maximum number of these lamps is 5 (see (4))
 *               - A signal output consumes power and this value must be stored
 *  in its definition for the following comparision done for lamp-broken
 *               decisiob. At any time, MCT or LCD user can request to update
 *  this power value. In this case, device measures power consumed by signal
 *               output and record it again.
 */
typedef struct _SSODef
{
  uint8_t bLampType; /* type of lamps at this output */
  uint8_t bType; /* one of the types defined below */
  uint8_t bOwner; /*  owner signal group of this output (1.. */
                  /* SIGNAL_GROUPS_MAX) */
  uint8_t bNextOutput; /* index of the next output structure that belong */
                       /* to the same signal group */
  uint8_t bNoOfLamps; /*  number of lamps connected to this output */
  uint16_t sPower[2]; /*  total power consumed by the lamps connected to */
                      /* this output // %%&& @160310 */
  uint16_t sPowerRecordNet[2]; /* net voltage when sPower is recorded. Used */
                               /* later */
                               /* when determining lamp failures // %%&& @160310 */
  short sSlope; /*  slope for voltage-current line */

  struct
  {
    uint8_t bSOFailureEM : 2; /*  if bNoOfLamps is 1, use of this */
                              /* emergency method is meaningful */
    uint8_t bEMReserved : 2;
    uint8_t bPowerRecorded0 : 1;
    uint8_t bPowerRecorded1 : 1;
    uint8_t bPowerReserved : 2;
  } __attribute__((packed)) SFlags; /* SEmergencyMethods; */
} __attribute__((packed)) tSSODef, *tpSSODef;

/*(3)
 *       name: signal output types
 *       expl:
 *               - There are three types: red, yellow, green
 *               - Type values are defined so that each output type is
 *  represented with a bit. This makes matching outputs with signal group color
 *               easier
 */
#define SIGNAL_OUTPUT_TYPE_NONE 0
#define SIGNAL_OUTPUT_TYPE_RED 1
#define SIGNAL_OUTPUT_TYPE_YELLOW 2
#define SIGNAL_OUTPUT_TYPE_GREEN 4

/*(5)
 *       name: the maximum number of lamps that can be connected to an output
 *       expl:
 */
#define LAMPS_PER_OUTPUT_MAX 5

/*(6)
 *       name: bit size for the event counters
 *       expl:
 *               - Event counters are used to accept an event as triggered (or
 *  valid).
 *               - When bit size value is modified, structure alignment must be
 *  considered in the following definition. Namely, reserved bits must be
 *  re-considered.
 */
#define EVENT_COUNTER_BIT_SIZE 4

typedef struct _SCurrentMeasurement
{
  uint16_t sMin;
  uint16_t sNow;
  uint16_t sMax;
  uint16_t sPrev;
} tSCurrentMeasurement, *tpSCurrentMeasurement;

#define CURRENT_MIN 0
#define CURRENT_NOW 1
#define CURRENT_MAX 2
#define CURRENT_PREV 3

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Signal Groups */
#define SIGNAL_GROUPS_MAX 32

typedef struct _SSGDef
{
  uint8_t bOwner; /* owner set of this group */
  uint8_t bType;
  uint8_t bOpeningSignal; /* the signal displayed at the signal group while */
                          /* opening (after RED before GREEN) */
  uint8_t bClosingSignal; /* the signal displayed at the signal group while */
                          /* closing (after GREEN before RED) */
  uint8_t bOpeningDuration; /* opening (transition to green) color duration */
  uint8_t bClosingDur; /* closing (transition to red) color duration */
  uint8_t bGreenFlashDur; /* will display GREEN_FLASH this much seconds before */
                          /* closing */
  uint8_t bFlashSignal; /* signal to display in flash mode */
  uint8_t bFailureFlashSignal; /* signal to display in flash mode */
  uint8_t bFirstOutput; /* index of the first output that is in this signal */
                        /* group */
  uint8_t bRedLampFailureNumber; /* a failure is registered if this number of */

  /* red lamps have failed on this signal group */

  struct
  {
    uint8_t fInUse : 1;
    uint8_t bRedLampFailureNumberEM : 2; /* red lamp failure number emergency */
                                         /* method */
    uint8_t bLastRedLampFailureEM : 2; /* last red lamp failure emergency method */
    uint8_t bReserved : 3;
  } __attribute__((packed)) SEmergencyMethods;

  struct
  {
    uint8_t fConflict : 1; /* if true, conflict checking will be made */
    uint8_t bClearance : 7; /* duration needed before the sg related to this */
                            /* struct can finish opening state after conflicting */
                            /* one has finished closing state */
  } __attribute__((
                    packed)) SaConflicts[SIGNAL_GROUPS_MAX]; /* definitions on conflicts with */
                                                             /* other signal groups */
} __attribute__((packed)) tSSGDef, *tpSSGDef;

typedef struct _SSOFaultStatus
{
  uint8_t fNoFaults : 1;
  uint8_t fLampBroken : 1;
  uint8_t fCriticalLampBroken : 1;
  uint8_t fAllLampsBroken : 1;
  uint8_t fConflict : 1;
  uint8_t fShortCircuirt : 1;
  uint8_t fOpenCircuit : 1;
  uint8_t fReserved : 1;
} __attribute__((packed)) tSSOFaultStatus, *tpSSOFaultStatus;

typedef struct _SSGRuntime
{
  uint8_t bIdx;
  uint8_t bCurrentSignal; /* current signal displayed at the signal group */
  uint8_t bDuration; /* counter for the duration in seconds since the group has */
                     /* started displaying current signal */
  uint8_t bState; /* state of the signal group (see below) */

  union
  {
    uint8_t bSORedFaults;
    tSSOFaultStatus SSORedFaults;
  } __attribute__((packed)) USORedFaults;

  union
  {
    uint8_t bSOYellowFaults;
    tSSOFaultStatus SSOYellowFaults;
  } __attribute__((packed)) USOYellowFaults;

  union
  {
    uint8_t bSOGreenFaults;
    tSSOFaultStatus SSOGreenFaults;
  } __attribute__((packed)) USOGreenFaults;
} __attribute__((packed)) tSSGRuntime, *tpSSGRuntime;

/* signal group types */
#define SIGNAL_GROUP_TYPE_NONE 0 /* undefined */
#define SIGNAL_GROUP_TYPE_VEHICLE_MAINWAY 1
#define SIGNAL_GROUP_TYPE_VEHICLE_SUBWAY 2
#define SIGNAL_GROUP_TYPE_PEDESTRIAN 3
#define SIGNAL_GROUP_TYPE_BICYCLE 4
#define SIGNAL_GROUP_TYPE_TRAM 5
#define SIGNAL_GROUP_TYPE_FLASHER 6

/* signal group runtime states */
#define SIGNAL_GROUP_STATE_NONE 0
#define SIGNAL_GROUP_STATE_CLOSING 1
#define SIGNAL_GROUP_STATE_CLOSED 2
#define SIGNAL_GROUP_STATE_OPENING 3
#define SIGNAL_GROUP_STATE_OPEN 4
#define SIGNAL_GROUP_STATE_GREEN_FLASH 5
#define SIGNAL_GROUP_STATE_FLASH 6
#define SIGNAL_GROUP_STATE_FLASHER 7
#define SIGNAL_GROUP_STATE_SEQ_SIGNAL 8

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Conflicts */
typedef struct _SSGConflict
{
  uint8_t bSG1;
  uint8_t bSG2;
  uint8_t bConflict;
} __attribute__((packed)) tSSGConflict, *tpSSGConflict;

typedef struct _SConflictsEM
{
  uint8_t bGreenGreenEM : 2;
  uint8_t bYellowGreenEM : 2;
  uint8_t bYellowYellowEM : 2;
  uint8_t bMalfunctionEM : 2;

  uint8_t bVoltageLimitsEM : 2;
  uint8_t bFrequencyErrorEM : 2;
  uint8_t bInvalidSignalEM : 2;
  uint8_t bInvalidSignalSequenceEM : 2;
} __attribute__((packed)) tSConflictsEM, *tpSConflictsEM;

#define CONFLICT_NONE 0
#define CONFLICT_GREEN_GREEN 1
#define CONFLICT_YELLOW_GREEN 2
#define CONFLICT_YELLOW_YELLOW 3
#define CONFLICT_MALFUNCTION 4
#define CONFLICT_VOLTAGE_LIMIT 5
#define CONFLICT_FREQUENCY_ERROR 6
#define CONFLICT_INVALID_SIGNAL 7
#define CONFLICT_INVALID_SIGNAL_SEQUENCE 8

/* emergency methods */
#define EMERGENCY_METHOD_NONE 0
#define EMERGENCY_METHOD_DARK 1
#define EMERGENCY_METHOD_FLASH 2
#define EMERGENCY_METHODS_MAX 2

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Phases */

/*(1)
 *       -
 */
#define PHASES_MAX 16 /* user can define this maximum number of phases */

/*(2)
 *       name: phase definition
 *       expl: includes groups having green signals at this phase. Also, all
 *  phases a minimum duration which shows that this phase must run at least this
 *  time of duration after it has started running.
 *
 *               - At the time of programming, there are 32 signal groups so it
 *  sufficient to use 4 bytes (32 bits) to show the signal groups this phase
 *  includes. As a result, when the number of signal groups increases, more bits
 *  will be required.
 *
 */
typedef struct _SPhaseDef
{
  uint32_t lGroups; /* if a bit is 1, that signal group is in this phase */
  uint8_t bMinDur; /* phase runs for at least this time of seconds */
  uint8_t bMaxDur; /* phase runs for at most this time of seconds */
} __attribute__((packed)) tSPhaseDef, *tpSPhaseDef;

/*(3)
 *       name: phase runtime data
 *       expl:
 *               - it includes a counter for phase duration. After phase started,
 *  this counter is zerod and incremented per second.
 *               - extension duration can be positive or negative. If it is
 *  negative, it means that phase will be ended this time ago. If it is positive,
 *  phase will be ended after this time takes.
 */
typedef struct _SPhaseRuntime
{
  uint16_t sElapsedDur; /* elapsed duration in the phase */
  uint8_t bRefSec; /* reference second in the phase duration */
  int8_t bExtDur; /* extension duration is added to the phase. It may be */
                  /* negative (=decrease phase duration) */
  uint8_t fRun; /* show if phase has run, set after phase duration has elapsed. */
} tSPhaseRuntime, *tpSPhaseRuntime;

/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
/*                            Sequence */
/* ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

#define SIGNAL_SEQ_STEPS_MAX 48
#define SIGNAL_SEQS_MAX 8

typedef struct _SSeqDef
{
  uint8_t bNoOfSteps; /* number of signal steps contained in this sequence */
  uint8_t baDurations[SIGNAL_SEQ_STEPS_MAX]; /* duration of each step in seconds */
  uint8_t baSignals[SIGNAL_SEQ_STEPS_MAX]
  [SIGNAL_GROUPS_MAX / 2];                  /* signals at each signal group at */
                                            /* each signal step, 2 group signals */
                                            /* in 1 byte */
} __attribute__((packed)) tSSeqDef, *tpSSeqDef;

typedef struct _SSeqRuntime
{
  uint8_t bCurrentStep; /* the active step of the sequence */
  uint8_t bCurrentStepCurrentDuration; /* the ongoing duration of the active */
                                       /* step */
  uint8_t bTour; /* the number of tours, increment at the end of sequence */
} tSSeqRuntime, *tpSSeqRuntime;

typedef struct _tSSeqExtension
{
  uint8_t bTotalSeqExtDur; /* total seconds count to be added to sequence */
  uint16_t sExtendedDur; /* for counting the elapsed extended seconds */
  uint8_t baSeqStepExtDur[SIGNAL_SEQ_STEPS_MAX]; /* seq step extention durations */
  uint8_t fIsValid : 1;
  uint8_t fReserved : 7;
} tSSeqExtension, *tpSSeqExtension;

/* //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
/*                            Flash */
/* Period */
/* // */
/* //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

#define FLASH_PERIOD_CONSTANT 10 /* define in terms of 10ms */
#define FLASH_PERIOD_0ms 0 /* always on */
#define FLASH_PERIOD_500ms 50
#define FLASH_PERIOD_1000ms 100
#define FLASH_PERIOD_2000ms 200
#define FLASH_PERIOD_4000ms 400
#define FLASH_PERIOD_MAX FLASH_PERIOD_4000ms
#define FLASH_PERIOD_INFINITE 1 /* always off */

/* //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
/*                            Peripheral */
/* States */
/* // */
/* //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

/* flags that control some peripherals which are controlled by the cards on the */
/* mp bus */
typedef struct _SPeripheralStates
{
  uint8_t fRelay : 1;
  uint8_t fClearSOPowers : 1;
  uint8_t fCPUinProgress : 1; /* Indicates the download / upload status of CPU */
  uint8_t fProgramRestart : 1;
  uint8_t fSystemInEM : 1;
  uint8_t fHeater : 1;
  uint8_t fLampDimming : 1;
  uint8_t fReserved0 : 1;

  uint8_t fExternalBattery : 1;
  uint8_t fReserved : 7;
} __attribute__((packed)) tSPeripheralStates, *tpSPeripheralStates;

/* //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
/*                            Power */
/* Supply */
/* // */
/* //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

typedef struct _SPowerSupply
{
  uint16_t sPrevNet;
  uint16_t sNet;
  uint16_t s24V1;
  uint16_t s5V1;
  uint16_t s24V2;
  uint16_t s5V2;

  struct _SFlags2
  {
    uint8_t fIsolatedVoltage : 1;
    uint8_t fReserved : 7;
    uint8_t fReserved2;
  } __attribute__((packed)) SFlags;

  uint16_t sPrevFrequency;
  uint16_t sFrequency;
} __attribute__((packed)) tSPowerSupply, *tpSPowerSupply;

#define PSMS_MAX 2
#define VOLTAGE_VALUE_MAX 1024
#define FREQUENCY_VALUE_MAX 1024

#define NET_VOLTAGE_COEFF 0.332

/* net voltage, 220V -> ADC output is 302 */
#define VOLTAGE_VALUE_HYSTERESIS 14 /* 10 V */
#define VOLTAGE_VALUE_LOWER_BOUND 241 /* 176 V */
#define VOLTAGE_VALUE_UPPER_BOUND 364 /* 265 V */

/* net freqeuncy */
#define FREQUENCY_VALUE_HYSTERESIS                                             \
        3 /* when it is exactly 100 hz again restart program */
#define FREQUENCY_VALUE_LOWER_BOUND 95 /* 4% */
#define FREQUENCY_VALUE_UPPER_BOUND 105 /* 4% */

/* ///////////////////////////////////////////////////////// */
/*                    Heater */
/* & Lamp Dimming */
/* H&D Commented */

/*
 *  typedef struct _tSHeaterLampDim
 *  {
 *  uint8_t fLogicLevel;
 *  uint8_t fState;
 *
 *  } __attribute__((packed)) tSHeaterLampDim, *tpSHeaterLampDim;
 */

/* ///////////////////////////////////////////////////////// */
/*                    Security */
/* Settings */
#define USER_SETTINGS_CHANGE_CONTROL_VLAUE 240 /* F0 */

typedef struct _tSUserSettings
{
  uint8_t fSettingsChanged;
  uint8_t fConfigFlag;
  uint8_t fLogFlag;
  uint8_t fTrafficCountsFlag;
  uint8_t bTrafficCountsPeriod;
  uint8_t fStandbyInfoFlag;
} __attribute__((packed)) tSUserSettings, *tpSUserSettings;

/* ///////////////////////////////////////////////////////// */
/*                    Log */
/* Settings */
#define LOG_SETTINGS_CHANGE_CONTROL_VLAUE 170 /* AA */

typedef struct _tLogSettings
{
  uint8_t fSettingsChanged;
  uint8_t baLogSettings[17]; /* for max 17 * 8 = 136 logs, 122 is used */
} __attribute__((packed)) tSLogSettings, *tpSLogSettings;

/* ///////////////////////////////////////////////////////// */
/*                    Virtual */
/* Input */

typedef struct _tVirtualInput
{
  uint8_t bType;
  uint8_t bNumber;
  uint8_t bState;
} __attribute__((packed)) tSVirtualInput, *tpSVirtualInput;

/* ///////////////////////////////////////////////////////// */
/*                    Broken */
/* Input Settings */

#define BROKEN_INPUT_SETTINGS_SET_CONTROL_VLAUE 240 /* F0 */

typedef struct _tSBrokenInputSettings
{
  uint8_t fAlreadySet;

  struct
  {
    uint8_t fLoopBusy : 1;
    uint8_t fDigitalBusy : 1;
    uint8_t fReserved : 6;
  } __attribute__((packed)) SFlags;
} __attribute__((packed)) tSBrokenInputSettings, *tpSBrokenInputSettings;

/* ///////////////////////////////////////////////////////// */
/*                    Server Settings */
#define   SERVER_SETTINGS_SET_CONTROL_VLAUE     240       /* F0 */

typedef struct _tSServerSettings
{
  uint8_t fAlreadySet;

  struct
  {
    uint8_t fMCSAvailable : 1;
    uint8_t fNTCIPAvailable : 1;
    uint8_t fReserved : 6;
  } __attribute__((packed)) SFlags;
} __attribute__((packed)) tSServerSettings, *tpSServerSettings;

/* //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
/*                            Events */
/* // */
/* //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

typedef struct _SEvent
{
  uint8_t bEvent;
  uint8_t bParam;
  uint16_t sParam;
  uint32_t lParam;
} __attribute__((packed)) tSEvent, *tpSEvent;

typedef struct _SLogRecord
{
  uint8_t bSeconds;
  uint8_t bMinutes;
  uint8_t bHours;
  uint8_t bMonthDay;
  uint8_t bMonth;
  uint16_t sYear;
  tSEvent SEvent;
} __attribute__((packed)) tSLogRecord, *tpSLogRecord;

/* event codes in the system */
#define EVENT_NONE 0
#define EVENT_POWER_ON 1 /* power on */
#define EVENT_SO_SWITCH_SHORT_CIRCUIT 2 /* signal outputs */
#define EVENT_SO_SWITCH_OPEN_CIRCUIT 3
#define EVENT_SO_VOLTAGE_SENSOR_FAILURE 4
#define EVENT_SO_LAMPS_DRIVEN_EXTERNALLY 5
#define EVENT_SO_WORKING_LAMP_TOTAL_CHANGE 6
#define EVENT_SIGNAL_AT_SG                                                     \
        7 /* if the dedected signal for an assigned signal changes */
#define EVENT_INVALID_SIGNAL                                                   \
        8 /* if an invalid signal that is not TRUE for intersection is on SO */
#define EVENT_INVALID_SIGNAL_SEQUENCE 9 /* if signal is not in followers */
#define EVENT_SG_RED_LAMP_FAILURE 10 /* when a red lamp failure exist */
#define EVENT_SG_LAST_RED_LAMP_FAILURE 11 /* the last red lamp failure */
#define EVENT_SG_NUMBER_OF_RED_LAMPS_FAILURE                                   \
        12 /* bRedLampFailureNumberTH failure */
#define EVENT_SG_YELLOW_LAMP_FAILURE 13
#define EVENT_SG_GREEN_LAMP_FAILURE 14
#define EVENT_YELLOW_YELLOW_CONFLICT 15 /* conflicts */
#define EVENT_YELLOW_GREEN_CONFLICT 16
#define EVENT_GREEN_GREEN_CONFLICT 17
#define EVENT_SO_POWER_RECORD 18 /* power record */
#define EVENT_MODULE_MISSING 19 /* module missing */
#define EVENT_MODULE_RESPONDS 20 /* module responds */
#define EVENT_INFORMATION 21 /* random information */
#define EVENT_CPMP_COMM_CP_CHECKSUM_ERROR 22 /* cpmp comm checksum errors */
#define EVENT_CPMP_COMM_MP_CHECKSUM_ERROR 23
#define EVENT_CPMP_COMM_CP_RECEIVE_ERROR 24 /* cpmp comm receive/transmit error */
#define EVENT_CPMP_COMM_CP_TRANSMIT_ERROR 25
#define EVENT_CPMP_COMM_MP_RECEIVE_ERROR 26
#define EVENT_CPMP_COMM_MP_TRANSMIT_ERROR 27
#define EVENT_POWER_NORMAL_TO_STAND_BY                                         \
        28 /* switching from normal mode to stand by */
#define EVENT_POWER_STAND_BY_TO_NORMAL                                         \
        29 /* switching from stand by to normal mode */
#define EVENT_CHECKSUM_FLASH_ERROR 30 /* storage error */
#define EVENT_CPMP_COMM_CP_TIMEOUT 31 /* cpmp comm timeouts */
#define EVENT_CPMP_COMM_MP_TIMEOUT 32
#define EVENT_MCT_CONFIGURATION_ERROR 33 /* program loading */
#define EVENT_INVALID_PROGRAM 35
#define EVENT_VOLTAGE_VALUE_LOWER_BOUND 36 /* voltage bounds */
#define EVENT_VOLTAGE_VALUE_UPPER_BOUND 37
#define EVENT_WORK_MODE_CHANGE 38 /* work mode, EVENT_WORK_MODE_CHANGE */
#define EVENT_RESET_WINDOW_WATCHDOG 39 /* reset sources */
#define EVENT_RESET_INDEPENDENT_WATCHDOG 40
#define EVENT_RESET_LOW_POWER 41
#define EVENT_SSM_LOG 42 /* module logs */
#define EVENT_PSM_LOG 43
#define EVENT_IO_LOG 44
#define EVENT_SG_ALL_RED_LAMPS_BROKEN 45 /* lamps broken */
#define EVENT_SG_ALL_YELLOW_LAMPS_BROKEN 46
#define EVENT_SG_ALL_GREEN_LAMPS_BROKEN 47
#define EVENT_SET_SIGNALING_MODE_CHANGE 48 /* signaling mode */
#define EVENT_MAIN_STORAGE_BROKEN 49 /* main and backup storages */
#define EVENT_BACKUP_STORAGE_BROKEN 50
#define EVENT_BACKUP_TO_MAIN_COPY_ERROR 51
#define EVENT_BACKUP_STORAGE_GET_ERROR 52
#define EVENT_BACKUP_STORAGE_SET_ERROR 53
#define EVENT_MAIN_STORAGE_GET_ERROR 54
#define EVENT_MAIN_STORAGE_SET_ERROR 55
#define EVENT_BACKUP_TO_MAIN_COPY_SUCCESS 56
#define EVENT_MAIN_STORAGE_IN_USE 57
#define EVENT_BACKUP_STORAGE_IN_USE 58
#define EVENT_MAIN_TO_BACKUP_COPY_SUCCESS 59
#define EVENT_MAIN_TO_BACKUP_COPY_ERROR 60
#define EVENT_RESET_POWER_ON_CLEAR_CIRCUIT                                     \
        61 /* reset source is power on clear circuit */
#define EVENT_BATTERY_LOW 62 /* battery */
#define EVENT_BATTERY_NORMAL 63
#define EVENT_DOOR_OPEN 64 /* door */
#define EVENT_DOOR_CLOSED 65
#define EVENT_MCT_CONFIGURATION_STARTS 66 /* Maestro Configuration Tool (MCT) */
#define EVENT_MCT_CONFIGURATION_ENDS 67
#define EVENT_DEFAULT_LCD_USER_ADD_SUCCESS 68 /* LCD user */
#define EVENT_DEFAULT_LCD_USER_ADD_ERROR 69
#define EVENT_VOLTAGE_VALUE_NORMAL 70 /* voltage normal value */
#define EVENT_FREQUENCY_VALUE_LOWER_BOUND 71 /* frequency */
#define EVENT_FREQUENCY_VALUE_UPPER_BOUND 72
#define EVENT_FREQUENCY_VALUE_NORMAL 73
#define EVENT_USER_REQ_WORK_MODE_TO_ALL_RED 74 /* user requests */
#define EVENT_USER_REQ_WORK_MODE_TO_DARK 75
#define EVENT_USER_REQ_WORK_MODE_TO_FLASH 76
#define EVENT_USER_REQ_WORK_MODE_TO_WORK_PLAN 77
#define EVENT_USER_REQ_POWER_LEARNING 78
#define EVENT_USER_REQ_SSM_TEST_STARTS 79
#define EVENT_USER_REQ_SSM_TEST_ENDS 80
#define EVENT_USER_REQ_SP_TEST_STARTS 81
#define EVENT_USER_REQ_SP_TEST_ENDS 82
#define EVENT_USER_REQ_TIME_SET 83
#define EVENT_USER_REQ_RELAY_SET_ON 84
#define EVENT_USER_REQ_RELAY_SET_OFF 85
#define EVENT_USER_REQ_LCD_LOG_IN 86 /* user account operations */
#define EVENT_USER_REQ_LCD_LOG_OUT 87
#define EVENT_USER_REQ_LCD_LOG_IN_USERNAME_ERR 88
#define EVENT_USER_REQ_LCD_LOG_IN_PASSWORD_ERR 89
#define EVENT_SIGNAL_DURATION_LESS_THAN_MIN 90 /* signal duration */
#define EVENT_SIGNAL_DURATION_GREATER_THAN_MAX 91
#define EVENT_DETECTOR_BROKEN 92 /* detector states */
#define EVENT_DETECTOR_SAFE 93
#define EVENT_WORK_PLAN_CHANGE 94
#define EVENT_SIGNAL_PROGRAM_PLAN_CHANGE 95
#define EVENT_SIGNAL_PROGRAM_CHANGE 96
#define EVENT_SG_ALL_RED_LAMPS_SAFE 97 /* lamps safe */
#define EVENT_SG_ALL_YELLOW_LAMPS_SAFE 98
#define EVENT_SG_ALL_GREEN_LAMPS_SAFE 99
#define EVENT_RESET_SOFTWARE 100
#define EVENT_RESET_PIN 101
#define EVENT_RESET_PORRST 102
#define EVENT_MCS_ACTIVE 103
#define EVENT_MCS_CONNECTED 104
#define EVENT_MCS_DISCONNECTED 105
#define EVENT_MCS_CONNECTION_TIMEOUT 106
#define EVENT_MCS_USER_REQUEST_SP_CHANGE 107
#define EVENT_MCS_USER_REQUEST_DATE_TIME_ADJUST 108
#define EVENT_MCS_USER_REQUEST_RESET 109
#define EVENT_MCS_USER_REQUEST_DOWNLOAD 110
#define EVENT_MCS_USER_REQUEST_UPLOAD 111
#define EVENT_USER_REQ_PSM_TEST_STARTS 112
#define EVENT_USER_REQ_PSM_TEST_ENDS 113
#define EVENT_GREEN_WAVE_SYNCH_STARTS 114
#define EVENT_GREEN_WAVE_SYNCH_ENDS 115
#define EVENT_MCS_USER_REQ_WORK_MODE_TO_ALL_RED 116
#define EVENT_MCS_USER_REQ_WORK_MODE_TO_DARK 117
#define EVENT_MCS_USER_REQ_WORK_MODE_TO_FLASH 118
#define EVENT_MCS_USER_REQ_WORK_MODE_TO_WORK_PLAN 119
#define EVENT_MCS_RESUMED 121
#define EVENT_USER_REQ_RESET 123
#define EVENT_DIGITAL_INPUT_BROKEN 124 /* digital states */
#define EVENT_DIGITAL_INPUT_SAFE 125
#define EVENT_TASK_NOT_RUNNING 126
#define EVENT_TASK_STACK_OVERFLOW 127

#define EVENT_LAST EVENT_TASK_STACK_OVERFLOW

/* while reading data from flash, data may be harmed, use following parameters */
/* to understand where is harmed, this is for debugging */
#define EVENT_PARAM_CHECKSUM_FLASH_ERROR_ConflictsEM FLAG_BIT_0
#define EVENT_PARAM_CHECKSUM_FLASH_ERROR_DeviceInfo FLAG_BIT_1
#define EVENT_PARAM_CHECKSUM_FLASH_ERROR_FlashPeriods FLAG_BIT_2
#define EVENT_PARAM_CHECKSUM_FLASH_ERROR_PhaseDefs FLAG_BIT_3
#define EVENT_PARAM_CHECKSUM_FLASH_ERROR_SGDefs FLAG_BIT_4
#define EVENT_PARAM_CHECKSUM_FLASH_ERROR_SignalDefs FLAG_BIT_5
#define EVENT_PARAM_CHECKSUM_FLASH_ERROR_SeqDefs FLAG_BIT_6
#define EVENT_PARAM_CHECKSUM_FLASH_ERROR_SODefs FLAG_BIT_7
#define EVENT_PARAM_CHECKSUM_FLASH_ERROR_Consumed FLAG_BIT_8
#define EVENT_PARAM_CHECKSUM_FLASH_ERROR_Transitions FLAG_BIT_9
#define EVENT_PARAM_CHECKSUM_FLASH_ERROR_SignalPlans FLAG_BIT_10
#define EVENT_PARAM_CHECKSUM_FLASH_ERROR_WPDefs FLAG_BIT_1
#define EVENT_PARAM_CHECKSUM_FLASH_ERROR_WSDef FLAG_BIT_12
#define EVENT_PARAM_CHECKSUM_FLASH_ERROR_SPPlans FLAG_BIT_13
#define EVENT_PARAM_CHECKSUM_FLASH_ERROR_CVS FLAG_BIT_14
#define EVENT_PARAM_CHECKSUM_FLASH_ERROR_Detectors FLAG_BIT_15
#define EVENT_PARAM_CHECKSUM_FLASH_ERROR_Inputs FLAG_BIT_16
#define EVENT_PARAM_CHECKSUM_FLASH_ERROR_Outputs FLAG_BIT_17
#define EVENT_PARAM_CHECKSUM_FLASH_ERROR_SP FLAG_BIT_18
#define EVENT_PARAM_CHECKSUM_FLASH_ERROR_Line_Operator FLAG_BIT_19
#define EVENT_PARAM_CHECKSUM_FLASH_ERROR_Pop3 FLAG_BIT_20
#define EVENT_PARAM_CHECKSUM_FLASH_ERROR_Smtp FLAG_BIT_21
#define EVENT_PARAM_CHECKSUM_FLASH_ERROR_SmsUsers FLAG_BIT_22
#define EVENT_PARAM_CHECKSUM_FLASH_ERROR_RemoteConfig FLAG_BIT_23

/* //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
/*                            LCD */
/* // */
/* //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

#define LCD_USERS_MAX 3 /* maximum number of lcd users that can be defined. */
#define LCD_USER_TYPE_NONE 0 /* lcd user types */
#define LCD_USER_TYPE_ADMIN                                                    \
        1 /* admin users have the highes priority in the device */
#define LCD_USER_TYPE_GUEST 2 /* guest users have a limited access to the device */
#define LCD_USER_TYPE_MAX 2 /* current user types are: admin and guest */

/* //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
/*                          Modules */
/* (=Cards) */
/* // */
/* //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

/*(1)
 *       name: cards in device
 *       expl: device may have psm, ssm, io. Also, there are loop detector cards
 *  which are third-party products
 */
#define MODULES_MAX 4 /* CPU, PSM, SSM, IO */
#define MODULES_PU_MAX 2 /* CP & MP */
#define MODULES_PSM_MAX 2
#define MODULES_SSM_MAX 8
#define MODULES_IO_MAX 2

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  IO Module */
#define IO_INPUTS_DIGITAL_MAX 16 /* number of inputs */
#define IO_INPUTS_DETECTOR_MAX 16 /* number of detectors */
#define IO_OUTPUTS_MAX 8 /* number of outputs */

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Hardly Assigned Digital Inputs */
#define HEATER_BUTTON_DIG_INPUT_NO 6 /* digital 6 is hardly assigned for heater */
#define LAMP_DIMMING_BUTTON_DIG_INPUT_NO                                       \
        7 /* digital 7 is hardly assigned for lamp dimming */
#define POLICE_BUTTON_DIG_INPUT_NO 8 /* digital 8 is hardly assigned for police */

/*(1)
 *       name: io packet sent from io card
 *       expl:  these packets are are sent from mp to cp. Mp receives these
 *  packets from io card via CAN at the moment of this comment and CAN packet
 *  size for this packet is 8 byte but last 2 bytes are not used now. At the
 *  moment of programming, we have RAM limitation - its size is 32K for NEC
 *  processor -. As a result, we have commented last 2 bytes and other reserved
 *  bytes at the following structure declarations for not wasting RAM.
 */
#define IO_DRIVEN_PHYSICALLY 1
#define IO_DRIVEN_VIRTUALLY 0
#define IO_MESSAGE_PERIOD 10

typedef struct _SCanDigitalIOInputs
{
  uint16_t sInputStates; /* 1: no demand, 0: there is demand */
  uint16_t sInputSafeStates; /* 1: safe, 0: broken */
  uint8_t fIsPhysicallyDriven : 1; /* 1: driven physically, 0 driven virtually */
  uint8_t bReserved : 7;
} tSCanDigitalIOInputs, *tpSCanDigitalIOInputs;

typedef struct _SCanDetectorIOInputs
{
  uint16_t sLoopSafeStates; /*  1: safe, 0: broken */
  uint16_t sLoopEmptyStates; /* 1: empty, 0: busy */
  uint8_t fIsPhysicallyDriven : 1; /* 1: driven physically, 0 driven virtually */
  uint8_t bReserved : 7;
} tSCanDetectorIOInputs, *tpSCanDetectorIOInputs;

/*(2)
 *       name: io packet sent from cpu card
 *       expl: cp determines output states which are connected to io card.
 *  Determined output states are sent to io card via this packet. Also, unused
 *  bytes are commented for not wasting RAM which is explained in (1).
 */
typedef struct _SCanCpuIOOutputs
{
  uint16_t sOutputStates; /* 1: activate output, 0: deactivate output */
} tSCanCpuIOOutputs, *tpSCanCpuIOOutputs;

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Signal Plan */

/*(1)
 *       - intersection is configured with transition definitions. However,
 *  transition definitions can be adjusted such that signal program can serve as
 *  a signal plan which is a fixed-period working plan
 */

#define SIGNAL_PLANS_MAX 128

/*(2)
 *       name: signal plan
 *       expl: signal plans run conventionally periodically. To be able to use
 *  the following signal plan structure with this aim, workplan and workplan
 *  entry parameters must be given. If the aim is nothing more than a signal
 *  program, these parameters are not used and they are zero.
 */
typedef struct _SSignalPlan
{
  uint8_t bSigProg; /* this signal program will run according to selected work */
                    /* plan and its selected entry */
  uint8_t bWorkPlan : 4; /* use phase durations in this workplan's */
  uint8_t bWorkPlanEntry : 4; /* this workplan entry */
} __attribute__((packed)) tSSignalPlan, *tpSSignalPlan;

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Daily Work Plan */

/*(1)
 */
#define WORK_PLANS_MAX                                                         \
        13 /* 1 of them is default workplan, remanings are user configurable ones */
#define WORK_PLAN_ENTRIES_MAX 16 /* one workplan can have this number of entries */
#define WORK_PLAN_DEFAULT                                                      \
        0 /* if there are no user defined workplan, this will be used */

/*(2)
 *       name: entries for daily work plan
 *       expl: daily workplans have some entries. These entries are used by the
 *  signal program when time constraint is satisfied. Time constraint is given
 *  with hour and minute.
 *
 *               - Example Work Plan
 *
 *                       Entry No Time      Durations:
 *  Phase 1   Phase 2   Phase 3   Phase 4 ...
 *  Phase 16
 *                       -------- ---------
 *  -------   -------   -------   ------- ...
 *  ------- 1     07.00
 *  8     9     10      7
 *  12 2      17.00           6
 *  19      8     7 12
 *                       ...      ...
 *  ...     ...     ...     ...
 *  ...   ... 16      00.00
 *  12      8     10      8
 *  ...   6
 */
typedef struct _SWorkPlanEntryDef
{
  uint8_t bHours;
  uint8_t bMinutes;
  uint8_t baPhaseDur[PHASES_MAX]; /* phase durations for this work plan */
} __attribute__((packed)) tSWorkPlanEntryDef, *tpSWorkPlanEntryDef;

typedef tSWorkPlanEntryDef tSaWorkPlan[WORK_PLAN_ENTRIES_MAX];

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Signal Program */

/*(0)
 *       - a signal program has transitions, rules, operations, operands,
 *  statements and counters. These data are defined separately below. Besides, it
 *  may have additional data.
 */

/*(1)
 *       name: signal program additional data
 *       expl: if signal program has additional data use the following structure
 */
typedef struct _SSigProg
{
  uint8_t bStaStart; /* start index in statement pool for program start */
  uint8_t bStaEnd; /* end index in statement pool for program start */
  uint8_t bEndStart; /* start index in statement pool for program end */
  uint8_t bEndEnd; /* end index in statement pool for program end */
} __attribute__((packed)) tSSigProg, *tpSSigProg;

typedef struct _SSigProgRuntime
{
  uint8_t bCurTimeInPer; /* current time in period */
} tSSigProgRuntime, *tpSSigProgRuntime;

/* //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
/*                            Signal */
/* Program Plan */
/* // */
/* //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

/*(1)
 *       - signal programs consist of transitions. In transition definition, you
 *  can see owner signal program of the transition.
 */
#define SIGNAL_PROGRAM_PLANS_MAX                                               \
        16 /* user can prepare this number of signal program plans */
#define SIGNAL_PROGRAM_ENTRIES_MAX                                             \
        16 /* signal program can have this number of signal program entries */
#define SIGNAL_PROGRAMS_MAX                                                    \
        16 /* user can prepare this number of signal */
           /* programs */
#define SIGNAL_PROGRAM_PLAN_DEFAULT                                            \
        0 /* if there are no user defined signal program plan, this will be used */
#define SIGNAL_PROGRAM_DEFAULT 0 /* with this default signal program */

/*(2)
 *       name: signal program entry
 *       expl: these are the entries for the signal program plan. Before running
 *  signal program, statements given with statement pool indexes must be
 *  executed.
 */
typedef struct _SSPPlanEntry
{
  uint8_t bHours;
  uint8_t bMinutes;
  uint8_t bSigProg; /* this entry has this signal program */
} __attribute__((packed)) tSSPPlanEntry, *tpSSPPlanEntry;

typedef tSSPPlanEntry tSaSPPlan[SIGNAL_PROGRAM_ENTRIES_MAX];

/* //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
/*                            Work */
/* Schedule */
/* // */
/* //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

/*(0)
 */

#define WORK_SCHEDULE_ENTRIES_MAX 16

/*(1)
 *       bDays: 0th bit Monday, 1th bit Tuesday ... 6th bit Sunday
 */
typedef struct _SWorkScheduleEntryDef
{
  uint8_t bDays; /* shows in which days this entry will be active */
  uint8_t bStartDay; /* month day */
  uint8_t bStartMonth; /* month */
  uint8_t bStartYear; /* year */
  uint8_t bEndDay; /* end month day */
  uint8_t bEndMonth; /* end month */
  uint8_t bEndYear; /* end year */
  uint8_t bWorkPlanNo; /* work plan no to work according to within the specified */
                       /* dates (or weekday) */
  uint8_t bSigProgPlan; /* signal program plan to use between these dates */
} __attribute__((packed)) tSWorkScheduleEntryDef, *tpSWorkScheduleEntryDef;

typedef tSWorkScheduleEntryDef tSaWorkSchedule[WORK_SCHEDULE_ENTRIES_MAX];

/* //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
/*                        State */
/* Transitions */
/* // */
/* //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

/*(1)
 *       - transitions occur between states
 */

#define TRANSITIONS_MAX 64

/*(2)
 *       name: transitions between states
 *       expl: device has a number of states.
 *
 *               - Examples:
 *
 *                       from       to
 *  value1              value2
 *                       -------        --------
 *  --------            -------- no control
 *  dark      has no meaning          has no
 *  meaning dark        sequence    has no meaning
 *  sequence no sequence      phase     sequence no
 *  phase no phase        phase     source
 *  phase + sim close   destination phase no + sim open phase
 *  sequence    phase no
 *  sequence no sequence      dark      sequence no
 *  has no meaning sequence     sequence    source
 *  sequence no       destination sequence no any state
 *  sequence    has no meaning          sequence
 *  no any state      phase     has no meaning
 *  phase no sequence (closing) flash     sequence no
 *  has no meaning sequence (closing) closed      has no meaning
 *  has no meaning
 *
 *               - At the time of programming, maximum number of transition rules
 *  is 128 (see Transition Rules (2)). Therefore, related rule for this
 *               transition definition is referred with 1 byte.
 *
 *               - There may be transitions to a state from any state in the case
 *  of related rule is true. Therefore, there must be a state like "state any".
 *
 *               - When we are in a state, many transtions may be valid. Namely,
 *  rules of them may be true at the same time. We need transition priority
 *               values to differentiate them. Transition to the most prioritized
 *  state will be occured.
 */
typedef struct _STransition
{
  uint8_t bFrom; /* source state */
  uint8_t bTo; /* destination state */
  uint8_t bValue1; /* refer to specific data. It is meaningful with source and */
                   /* destinations states */
  uint8_t bValue2; /* refer to specific data. It is meaningful with source and */
                   /* destinations states */
  uint8_t bRule; /* refer to the transition rule related to this transition */
  uint8_t bPriority; /* priority of transition */
} __attribute__((packed)) tSTransition, *tpSTransition;

/* states */
#define STATES_NONE 0
#define STATES_ANY 1 /* one of the states listed below */
#define STATES_NO_CONTROL 2 /* all groups signals are dark */
#define STATES_FLASH 3 /* all groups signals are their flash signals */
#define STATES_CLOSED 4 /* all groups signals are red */
#define STATES_PHASE 5 /* intersection is in a phase */
#define STATES_PHASE_TRANSITION 6 /* intersection is in a phase */
#define STATES_SEQ 7 /* intersection is in a sequence */
#define STATES_SECURE_TRANSITION 8
#define STATES_MAX 8

/*(3)
 *       name: user requests state
 *       expl:
 *               - user may have requests, in this case, work schedule is
 *  discarded
 */
typedef struct _SUserState
{
  uint8_t fRunning; /* is it active? */
  uint8_t bStateCurrent; /* if it is active, device is in this state */
  uint8_t bStateReq; /* it is not active yet but user has a request, store this */
                     /* request here */
} tSUserState, *tpSUserState;

/* access sub-parts of values in transition definition */
#define TRANSITION_VALUE_SIM_GET                                               \
        0x80 /* extract simultaneous closure/opening flag with this */
#define TRANSITION_VALUE_NO_GET                                                \
        0x1F /* use instead of TRANSITION_VALUE1_SEQ and TRANSITION_VALUE2_PHASE */

/* //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
/*                        Transition */
/* Rules */
/* // */
/* //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

/*(1)
 *       i) transition rules are also called as logic blocks or briefly
 *  conditions
 *
 *       ii) a signal program is a state machine that can have a number of
 *  states: dark, transition rules are made up of conditions which are made up of
 *  comparisons
 *
 *       iii) we have already defined runtime values stored in RAM. These are
 *  accessed by the maestro configuration tool user with a user friendly
 *       interface. For example, number of detector demands are stored in a
 *  location in RAM, user can refer to it via provided interface. The following
 *  structral organization and definitions are prepared for that. Besides, user
 *  can define his variables, these variables may be a combination of some
 *  previous variables (like x, y, z...) or they may refer to predefined runtime
 *  value stored in RAM (like DTC3 which means number of detector demands are
 *  stored in a location in RAM)
 *
 *               - Example:
 *
 *                       -- define 3 variables which will be used to prepare
 *  transition rules
 *                       -- in fact, these variables can be called as
 *  sub-conditions which are common between some transition rules
 *                       -- also, user can choose using this ability for the sake
 *  of readibility of his signal program
 *                       -- the following variables are allocated from operation
 *  pool
 *
 *                       x = (DTC3 < 10) && (DTC5 > 4);
 *                       y = (DTC7 < 15) && (DTC8 > 7);
 *                       z = (DTC1 < 6) && (DTC2 > 10);
 *
 *                       -- we have some variables defined above
 *                       -- the following transition rules can be defined by
 *  using them
 *
 *                       t = x || y;
 *                       u = x || z;
 *                       v = y || z;
 *
 *       iv) "(DTC3 < 10) && (DTC5 > 4)" is an example of transition rule. As an
 *  example, its string size is 25.
 *
 *       v) Variables are defined in operation style above. Also, they can be
 *  defined in operand style. In operation style, allocation is extracted from
 *  operation pool. In operand style, allocation is extracted from operand pool.
 *
 *       vi) As a result, there are three types of variable: operand, operation,
 *  rule.
 *
 *               - Examples:
 *
 *                       - operand style:   x = 5;
 *  -- a constant variable
 *                       - operation style:   x = DCT3 > 4;
 *  -- there is only one operation
 *                       - rule style:      x = (DCT3 > 4) && (DCT2
 *  < 10);    -- a rule can have more than one operations, this has 3
 *  operations
 *
 *       vii) Signal programs consist of operands, operations, rules, and
 *  statements.
 */

/*(2)
 *       name: abstract block and operation pool concepts
 *       expl: Let's explain them with an example.
 *                       - Example:
 *                               Given (((DCT3 + DCT4) > 10) && (DCT1 < 5))
 *  transition rule
 *
 *                                       Operation
 *  Abstract Block No         Operation Pool
 *                                       ------------
 *  -------------------         ------------------ (DCT3
 + DCT4)        AB1
 +  (1) used by this example rule (AB1 > 10) AB2
 +  (2) used by this example rule (DCT1 < 5) AB3
 +  (3) used by this example rule (AB2 && AB3)      Result of rule
 +  (true or false)   (4) used by this example rule (5) this is the
 +  first empty operation memory that will be allocated by the next rule
 +
 +                       - In the above example, 4 blocks are used. In fact, all
 +  operations are named with an abstract block. Number of operations in a rule
 +  is equal to number of operators in rule, this can be seen in the above
 +  example.
 +
 +                       - As a result, an operation pool must be allocated - for
 +  this, you can see data.c. User defines his rule by using MCT and device
 +                       partitions his rule into abstract blocks, in other
 +  words, into operations. Then, device extracts an operation memory location
 +  from previously allocated operation pool for each abstract block.
 +
 +                       - Operations are pulled from operation pool sequentially
 +  so a rule can be defined with a start and end index of this allocated array
 +  of operations memory. In the above example, allocation from operation pool
 +  can also be seen. For the example, start index is 1, end index is 4. For the
 +  next rule, start index is 5.
 */

/* transition rules maximum values */
#define RULES_MAX                                                              \
        96 /* user can define this number of transition rules, in other words, this is */
           /* rule pool size */
#define RULE_OPERATIONS_MAX                                                    \
        256 /* operation pool size, this number of operations can be allocated */
#define RULE_OPERANDS_MAX                                                      \
        32 /* operand pool size, this number of operands can be allocated */
#define RULE_STATEMENTS_MAX                                                    \
        128 /* statement pool size, this number of statements can be written by MCT */
            /* user */

/*(3)
 *       name: operands used in operations
 *       expl: structure has three data: field, subfield and value. All of them
 *  is used to refer to a specific data. Meaning of value depends on the field
 *  and subfield.
 *
 *                       field          subfield
 *  value
 *                       --------------     ----------------
 *  ----------------- 1)  detector        do not care
 *  detector no 2)  input         do not care
 *  input no 3) sg            do not care
 *  sg no 4)  phase         do not care
 *  phase no 5) constant        has no meaning
 *  constant value 6) sequence        do not care
 *  sequence no 7)  variable        determines the
 *  pool      index no in the pool 8) block
 *  has no meaning        index no in operation pool 9)
 *  workplan        current phase duration    phase no
 *
 *               - Examples:
 *
 *                       - When field is detector/input, subfield may be demand
 *  in red and value is detector/input number
 *                       - When field is sg, subfield may be signal and value is
 *  signal group number
 *                       - When field is phase, subfield may be phase elapsed
 *  duration, and value is phase number
 *                       - When field is constant, subfield is not used and value
 *  is a number like (0, 1, 100, 300) or a BOOL2ean like (true=1, false=0)
 */
typedef struct _SOperand
{
  uint8_t bField : 5; /* owner of subfield, possible values are listed below */
  uint8_t bValueHigh : 3; /* high byte of specific data */
  uint8_t bSubField; /* refer to information that will be extracted from data */
                     /* referred by field */
  uint8_t bValueLow; /* this value refers to specific data. It is meaningful */
                     /* together with fields and subfields */
} __attribute__((packed)) tSOperand, *tpSOperand;

/*(4)
 *       name: operand field values
 *       expl: these are the owner of the subfields. All fields do not have
 *  subfields. For example, if field is constant, it has not a subfield. Instead,
 *  subfield refers to the value of this constant. Another example is sg no, when
 *  field is sg no, it has not a sub field. Instead, it subfield refers to a sg
 *  no.
 */
#define OP_FIELD_NONE 0
#define OP_FIELD_DETECTOR                                                      \
        1 /* when user wants to access detector related data in subfields, field must */
          /* be this */
#define OP_FIELD_INPUT 1 /* for input related data */
#define OP_FIELD_SG 2 /* for signal group related data */
#define OP_FIELD_PHASE 3 /* for phase related data */
#define OP_FIELD_SEQ 4 /* for sequence related data */
#define OP_FIELD_CONSTANT                                                      \
        5 /* subfield is a number or predefined value: a BOOL2ean value like true or */
          /* false */
#define OP_FIELD_BLOCK 6 /* refer to an abstract block (see (2)) */
#define OP_FIELD_VAR                                                           \
        7 /* user may have defined a variable, he can access it by using this field id */
#define OP_FIELD_COUNTER                                                       \
        8 /* device has counters used for different aims, user can access them by */
          /* using this field id */
#define OP_FIELD_WORKPLAN 9 /* refer to current workplan */
#define OP_FIELD_TIME 10 /* time of the day */
#define OP_FIELD_USER 11 /* user via an interface (lcd or mct for the present) */
#define OP_FIELD_TRANSITION 12 /* data about transitions */
#define OP_FIELD_TIME_VALUE 13 /* time like 23:59, 00:00, 07:01 */
#define OP_FIELD_TRAFFIC_DATA_SET                                              \
        14 /* traffic flow data collected from traffic cams */
#define OP_FIELD_MAX 14

/* operand subfield values */
/* also consider date, time, input falling/rising */
#define OP_SUBFIELD_NO_MEANING 0 /* means subfield has no meaning */
#define OP_SUBFIELD_DEMAND_PERIOD 1 /* number of demands in period */
#define OP_SUBFIELD_DEMAND_RED 2 /* in red */
#define OP_SUBFIELD_DEMAND_GREEN 3 /* in green */
#define OP_SUBFIELD_FDEMAND_DUR_PERIOD                                         \
        4 /* duration taken after first demand occured in period */
#define OP_SUBFIELD_FDEMAND_DUR_RED 5 /* in red */
#define OP_SUBFIELD_OCC_DUR_PERIOD 6 /* occupation duration taken in period */
#define OP_SUBFIELD_OCC_DUR_RED 7 /* in red */
#define OP_SUBFIELD_OCC_DUR_GREEN 8 /* in green */
#define OP_SUBFIELD_GAP_DUR_PERIOD 9 /* gap duration taken in period */
#define OP_SUBFIELD_GAP_DUR_GREEN 10 /* in green */
#define OP_SUBFIELD_BROKEN_DUR                                                 \
        11 /* refer to duration taken after unit is */
           /* broken */
#define OP_SUBFIELD_IS_BROKEN                                                  \
        12 /* info if unit is broken, unit may be a detector or an input */
#define OP_SUBFIELD_OWNER_SG 13 /* refer to owner of unit */
#define OP_SUBFIELD_GREEN_DUR_PER_DEMAND                                       \
        14 /* refer to green duration that will be used to extend green of sg per */
           /* demand */
#define OP_SUBFIELD_RED_DUR_IN_BROKEN                                          \
        15 /* refer to red duration which is red duration of owner sg in case of unit */
           /* broken */
#define OP_SUBFIELD_PHASE_IN_BROKEN                                            \
        16 /* refer to phase to activate sg in case of unit broken */
#define OP_SUBFIELD_FDEMAND_DUR_RED_MAX                                        \
        17 /* duration taken after first demand occured in red for a sg which is not a */
           /* unit like detector or input */
#define OP_SUBFIELD_OCC_DUR_RED_MAX                                            \
        18 /* maximum of occupation durations taken by units belonging to a sg in red */
#define OP_SUBFIELD_OCC_DUR_GREEN_MAX 19 /* in green */
#define OP_SUBFIELD_GAP_DUR_GREEN_MIN                                          \
        20 /* minimum of gap durations taken by units belonging to a sg in green */
#define OP_SUBFIELD_SG_SIGNAL 21 /* sg current signal */
#define OP_SUBFIELD_SG_DURATION 22 /* duration of sg current sg signal */
#define OP_SUBFIELD_SG_STATE                                                   \
        23 /* state of sg, sg may be in state of opening, open, green-flash, closing */
           /* or closed */
#define OP_SUBFIELD_SG_NO 24 /* when operator is conflict, this is meaningful */
#define OP_SUBFIELD_PHASE_MIN_DUR                                              \
        25 /* refer to minimum duration phase should run */
#define OP_SUBFIELD_PHASE_ELAPSED_DUR 26 /* refer to elapsed duration in phase */
#define OP_SUBFIELD_PHASE_HAS_RUN                                              \
        27 /* refer to if phase has run, set after phase duration has elapsed. */
#define OP_SUBFIELD_SEQ_STEP_TOTAL 28 /* refer to total number of steps */
#define OP_SUBFIELD_SEQ_CUR_STEP 29 /* refer to current step */
#define OP_SUBFIELD_SEQ_CUR_STEP_DUR                                           \
        30 /* refer to duration taken in current step */
#define OP_SUBFIELD_SEQ_CUR_DUR 31 /* refer to elapsed duration in sequence */
#define OP_SUBFIELD_SEQ_IS_ENDED 32 /* refer to if sequence duration is elapsed */
#define OP_SUBFIELD_VAR_OPERATION_POOL                                         \
        33 /* refer to a variable from operation pool */
#define OP_SUBFIELD_VAR_OPERAND_POOL 34 /* refer to a variable from operand pool */
#define OP_SUBFIELD_VAR_RULE_POOL 35 /* refer to a variable from rule pool */
#define OP_SUBFIELD_COUNTER_VAL 36 /* refer to value of counter */
#define OP_SUBFIELD_COUNTER_IS_ALLOCATED                                       \
        37 /* refer to counter allocation information */
#define OP_SUBFIELD_COUNTER_IS_RUNNING                                         \
        38 /* refer to counter running */
           /* information */
#define OP_SUBFIELD_COUNTER_IS_OVERFLOW                                        \
        39 /* refer to counter overflow information */
#define OP_SUBFIELD_WORKPLAN_CUR_PHASE_DUR                                     \
        40 /* refer to current phase duration in workplan */
#define OP_SUBFIELD_SYNCH_NET 41 /* synchronization source is net */
#define OP_SUBFIELD_SYNCH_RTC 42 /* rtc */
#define OP_SUBFIELD_SYNCH_CENTER 43 /* traffic center */
#define OP_SUBFIELD_SYNCH_GPS 44 /* global positioning system */
#define OP_SUBFIELD_STATE_NO_CONTROL 45 /* state of the device is no control */
#define OP_SUBFIELD_STATE_FLASH 46 /* flash */
#define OP_SUBFIELD_STATE_CLOSED 47 /* closed */
#define OP_SUBFIELD_STATE_ANY 48 /* no control, flash, closed */
#define OP_SUBFIELD_TRANSITION_LAST 49 /* refer to last transition applied */
#define OP_SUBFIELD_SEQ_TOUR 50 /* refer to tour count */
#define OP_SUBFIELD_PHASE_MAX_DUR                                              \
        51 /* refer to maximum duration phase should run */
#define OP_SUBFIELD_TRAFFIC_AVG_SPEED                                          \
        52 /* traffic data set -> average vehicle speed */
#define OP_SUBFIELD_TRAFFIC_OCCUPANCY 53 /* traffic data set -> occupancy */
#define OP_SUBFIELD_TRAFFIC_VHC_COUNT 54 /* traffic data set -> vehicle count */
#define OP_SUBFIELD_TRAFFIC_VHC_DENSITY                                        \
        55 /* traffic data set -> vehicle */
           /* density */
#define OP_SUBFIELD_MAX 55

/* refering to operands via operand indexing */
#define OP_FIR 0 /* refer to first operand */
#define OP_SEC 1 /* refer to second operand */
#define OP_MAX 2 /* total number of operands */

/* extract time values from operand value field by using the followings */
#define OP_FIELD_TIME_VALUE_HOUR_SIZE 5 /* size is in terms of bits */
#define OP_FIELD_TIME_VALUE_MINUTE_SIZE 6
#define OP_FIELD_TIME_VALUE_HOUR 0x7C0 /* 5 bits from MSB */
#define OP_FIELD_TIME_VALUE_MINUTE 0x03F /* 6 bits from LSB */

/*(5)
 *       name: operation
 *       expl: operations are used to construct a complete rule or a variable
 *  referring a combination of operations in signal programs
 *               - Examples:
 *                       complete rule1 = (DTC1 < 6) && (DTC2 > 10);
 *                       variable1 = (DTC1 < 6) && (DTC2 > 10);
 *                       variable2 = (DTC1 < 6);
 *                       variable3 = (DTC2 > 10);
 *                       complete rule2 = variable2 || variable 2;
 */
typedef struct _SOperation
{
  uint8_t bOperator; /* +, ++, <, <=, ==, etc., operators are listed below */
  tSOperand SaOperands[OP_MAX]; /* there may be operators have single or double */
                                /* operands */
} __attribute__((packed)) tSOperation, *tpSOperation;

/*(6)
 *       name: operators
 *       expl: these operators are used when writing transition rules. There are
 *  traditional operators like add/sub/equal. Also, there are custom operators
 *  which are added for our device.
 *               - Custom Operators:
 *                       - Conflict Operators: They need two operands which are
 *  signal groups. It returns true if they conflict or false.
 */
#define OPR_NONE 0
#define OPR_EQUAL 1
#define OPR_NOTEQUAL 2
#define OPR_LESS 3
#define OPR_LESSEQUAL 4
#define OPR_GREATER 5
#define OPR_GREATEREQUAL 6
#define OPR_ADD 7
#define OPR_SUB 8
#define OPR_MUL 9
#define OPR_DIV 10
#define OPR_MODULO 11
#define OPR_AND 12
#define OPR_OR 13
#define OPR_GG_CONFLICT 14
#define OPR_GY_CONFLICT 15
#define OPR_YY_CONFLICT 16

/*(7)
 *       name: state transition rule
 *       expl: this rule is a bridge between two states defined above. It has
 *  start and end indexes for memory allocation from operation pool. These index
 *  values define an operation array which composes a condition for the rule. You
 *  can find additional explanation on operation pool in (2).
 *
 *               - There is also a transition pool but this is a symbolic pool
 *  because memory locations are allocated from operation pool.
 *
 *               - There may be statements (also called as process block
 *  operations) which can be executed if transition rule is true or false. For
 *               both cases, there are start and end indexes for statement pool.
 */
typedef struct _SRule
{
  uint16_t sStart; /* start index in operation pool */
  uint8_t bTOpsStart; /* start index in statement pool if rule is true (T) */
  uint8_t bTOpsEnd; /* end index in statement pool if rule is true (T) */
  uint8_t bFOpsStart; /* start index in statement pool if rule is false (F) */
  uint8_t bFOpsEnd; /* end index in statement pool if rule is false (F) */
} __attribute__((packed)) tSRule, *tpSRule;

/*(8)
 *       name: statements (also called as commands)
 *       expl: There are commands MCT user wants device execute in some
 *  conditions. These commands may have parameters. The number of parameters
 *               depends on the command type.
 *
 *               - Memory Commands: We can allocate, deallocate, initialize
 *  variables or counters. Also, we can add a value to them. This value may be
 *               positive or negative. Variables may have operand or operation
 *  style.
 *
 *               - Counter Specific Commands: We can run or stop counters.
 *
 *               - Phase Commands: We can execute start/stop/extend/end commands
 *  on phases
 *
 *               - Sequence Commands: We can execute start/stop/add steps/remove
 *  seconds commands on sequences
 *
 *               - In MCT side, user can allocate variables and counters. After
 *  he allocates them, he access them with an id number in MCT side in the
 *  following operations in his signal program. After he prepared his signal
 *  program, he sends it to the device. Device scans the whole signal program.
 *  When device encounters an allocation, it allocates related memory with the
 *  required style (style may be counter, operand or operation). After
 *  allocation, device has an address from related memory pool so device replaces
 *  all references to the allocated memory with this address. In other words,
 *  before allocation and replacement, signal program was using id numbers to
 *  refer to variables or counters but afer allocation and replacement, in device
 *  side, addresses are used to refer them.
 *
 *               - While command is init memory with operand style, firstly, an
 *  operand is allocated from operand pool, this operand is initialized, then
 *  address of this operand is assigned to operand variable. The same case is
 *  valid for operation style allocation. A operation is allocated from operation
 *  pool, then its address is assigned to variable. In all statements, these
 *  addresses are used to access counters and variables.
 *
 *               - Examples
 *
 *                       - Let's allocate a counter: At first, an allocation
 *  request is executed by a statement in a process block. Then, an allocation
 *                       from counter pool is done. If this allocation succeeds,
 *  address of this counter is received. We may use this address value to
 *                       initialize counter to any value at any time. Also, we
 *  can start/stop this counter by using this address. Briefly, all operations on
 *  this counter is applied through its address.
 *
 *                       - Let's allocate an operand style variable: y = DCT3; is
 *  an example. First of all, an operand from operand pool will be allocated.
 *                       After that, we have the address of this operand. This
 *  address value is used for the following operations on this operand.
 *
 *                       - allocate an operand style variable like x = 5;, y =
 *  DCT3;
 *                       - allocate an operation style variable like z = y > 5, t
 *  = DCT1 + DCT5;
 *                       - initialize operand like x = 5;, y = DCT3;
 *                       - initialize operation like z = y > 5, t = DCT1 + DCT5;
 *                       - initialize counter like counter1 = 1;
 *                       - add a value to a operand, add a value to x like x +=
 *  5;, or pseudo demand like DCT3 += 3;
 *                       - add a value to an operation, add a value to t like t
 += DCT6;
 *                       - add a value to a counter, add a value to counter1 like
 *  counter1 += 3;
 *
 *                       Command            param1
 *  param2            param3
 *                       -------------------
 *  --------------------    -------------------
 *  ----------------------- allocate memory       counter
 *  id used in MCT side     initial value allocate memory
 *  operand           id used in MCT side
 *  has no meaning allocate memory        operation
 *  id used in MCT side     has no meaning deallocate memory
 *  memory style        memory address
 *  has no meaning init memory          memory style
 *  memory address        initial value add value to
 *  memory      memory style        memory
 *  address       added value start counter
 *  memory address        has no meaning
 *  has no meaning stop counter       memory address
 *  has no meaning        has no meaning start phase
 *  phase no          start to second
 *  has no meaning stop phase         phase no
 *  has no meaning        has no meaning extend phase
 *  phase no          has no meaning
 *  duration of extension end phase         phase no
 *  has no meaning        duration start sequence
 *  sequence no         start to second
 *  has no meaning stop sequence        sequence no
 *  has no meaning        has no meaning add step
 *  sequence no         signal
 *  duration remove seconds       sequence no
 *  duration          has no meaning user state
 *  request end   has no meaning        has no meaning
 *  has no meaning user state is current state  has no meaning
 *  has no meaning        has no meaning transition lock
 *  first transition      second transition
 *  third transition transition lock end      has no meaning
 *  has no meaning        has no meaning
 *
 *                       - When user starts a phase or sequence, he can specify
 *  the reference second. Therefore, device starts phase or sequence from this
 *  second. For example, if sequence duration is 60 seconds and 'start to second'
 *  is 7th second, device starts sequence from 7th second not 1th second. If
 *  'start to second' is 0th second, device starts sequence from the beginning.
 *
 */
typedef struct _SStatement
{
  uint8_t bCmd; /* the command that will be executed */
  uint8_t bParam1; /* first parameter */
  uint8_t bParam2; /* second parameter */
  signed char bParam3; /* third parameter */
} __attribute__((packed)) tSStatement, *tpSStatement;

/* commands */
#define COMMAND_NONE 0
#define COMMAND_MEMORY_ALLOCATE 1 /* allocation of memory location */
#define COMMAND_MEMORY_DEALLOCATE 2 /* deallocation of memory location */
#define COMMAND_MEMORY_INIT 3 /* initialization of memory location */
#define COMMAND_MEMORY_ADD 4 /* adding a value to memory location */
#define COMMAND_COUNTER_START 5 /* start counter */
#define COMMAND_COUNTER_STOP 6 /* stop counter */
#define COMMAND_PHASE_START 7 /* start phase */
#define COMMAND_PHASE_STOP 8 /* stop phase */
#define COMMAND_PHASE_EXTEND 9 /* extend phase with the given duration */
#define COMMAND_PHASE_END 10 /* end phase a number of seconds ago */
#define COMMAND_SEQ_START 11 /* start sequence */
#define COMMAND_SEQ_STOP 12 /* stop sequence */
#define COMMAND_SEQ_ADD_STEP 13 /* add a step to the end of sequence */
#define COMMAND_SEQ_REMOVE_SECONDS 14 /* remove seconds from the end of sequence */
#define COMMAND_USER_STATE_REQ_END                                             \
        15 /* user request -> user state transition ends */
#define COMMAND_USER_STATE_TO_CURRENT_STATE 16 /* go to user state */
#define COMMAND_TRANSITIONS_LOCK_ADD                                           \
        17 /* look at only these added transitions if their rule is true */
#define COMMAND_TRANSITIONS_LOCK_END 18 /* end transition lock mechanism */
#define COMMAND_SG_ADD_TO_FLASHER_LIST                                         \
        19 /* start these signal groups as flashers */
#define COMMAND_SG_SUB_FROM_FLASHER_LIST                                       \
        20 /* stop these signal groups as flashers */
#define COMMAND_SG_ADD_TO_PHASE 21 /* activate these signal groups in the phase */
#define COMMAND_SG_SUB_FROM_PHASE                                              \
        22 /* stop running of these signal groups in the phase */
#define COMMAND_PHASE_DELETE_RUN_INFO                                          \
        23 /* delete runtime phase run info, this is called when entering main phase */
           /* in TA. */
#define COMMAND_SIG_PROG_PER_RESTART 24 /* restart signal program period counter */

/* predefined parameters */
#define PARAM_NONE 0
#define PARAM_OPERAND 1
#define PARAM_OPERATION 2
#define PARAM_COUNTER 3

/* transition lock mechanism */
#define TRANSITION_LOCK_SIZE 2

/* //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
/*                          Counters */
/* // */
/* //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

#define COUNTERS_MAX 12

/*(1)
 *       i) We have already some counters. These counters are in the definitions
 *  of data in the device.
 *               - signal group current signal duration
 *               - phase elapsed duration
 *               - seqeunce current step duration
 *               - also there are traffic actuated related ones:
 *  gap/occupation/first demand sensed durations
 *
 *       ii) Also, there may be counters defined by MCT user. He must have the
 *  ability of defining/starting/stopping/restarting counters. Also, counters may
 *  be reassigned to a new value. This mechanism must be provided in the device.
 */

/*(2)
 *       name: counter definition
 *       expl: counters have a value and this value is incremented after its
 *  period has taken. First of all, user must allocate the counter before
 *               starting using it. While allocating, counter period must be
 *  parametered. After allocation, user can start the counter. At any time,
 *               counter can be stopped or reinitialized. It is possible that
 *  counter may overflow, this errornous state is stored in a flag.
 */

typedef struct _SCounter
{
  uint32_t lValue; /* current value of counter */
  uint16_t sPeriod; /* counter value is incremented 1 after this period has */
                    /* taken */
  uint8_t fAllocated : 1; /* shows if user has allocated this counter */
  uint8_t fRunning : 1; /* shows if counter is running */
  uint8_t fOverflow : 1; /* shows if counter value is overflow, this is an error */
  uint8_t fReserved : 5;
} tSCounter, *tpSCounter;

/* //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
/*                        Traffic */
/* Actuated */
/* // */
/* //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

/*(1)
 *       - in the following definitions, durations are in terms of 100ms
 *       - runtime values should be stored in ram because these values may be
 *  sent to traffic center deciding the most appropriate signal plan for the
 *  intersection. This decision process is called as Traffic Actuated Signal Plan
 *  Selection (TASS).
 *       - unit may be a detector or an input
 *       - inputs and outputs are connected to io module, however, for detectors,
 *  there are loop detector cards which to loops are connected
 */

/* unit numbers maximum values */
#define INPUTS_DETECTOR_MAX 32
#define INPUTS_DIGITAL_MAX 32
#define INPUTS_MAX 64
#define OUTPUTS_MAX 16

/*(2)
 *       name: input definiton
 *       expl:
 */
typedef struct _SInputs
{
  uint8_t bOwnerSG; /* sg unit belongs to */
  uint8_t bGreenDurPerDemand; /* sg signal will be green at least */
                              /* (bGreenDurPerDemand*sNumberOfDemands) */
  uint8_t bRedDurInBroken; /* if unit is broken, owner sg will have minimum */
                           /* green duration after this red duration */
  uint8_t bPhaseInBroken; /* if unit is broken and red duration has elapsed, go */
                          /* to this phase (min green is given to group with */
                          /* this phase) */
  uint8_t bActiveLevel; /* high or low defined below */
} __attribute__((packed)) tSInput, *tpSInput;

/* input types */
#define INPUT_TYPE_NONE 0
#define INPUT_TYPE_DIGITAL 1
#define INPUT_TYPE_DETECTOR 2

typedef struct _SMCSInputsRuntime
{
  uint16_t sDemandCntInPer; /* number of demands in a period */
  uint16_t sOccDurInPer; /* duration taken when unit is busy in period */
} tSMCSInputRuntime, *tpSMCSInputRuntime;

/*(3)
 *       name: unit runtime data
 *       expl: structure includes some counters and their values may overflow. We
 *  should also store this overflow but this is not considered now. For example,
 *  an overflow flag for each counter may be defined.
 */
typedef struct _SInputsRuntime
{
  uint8_t bDemandCntInPer; /* number of demands in a period */
  uint8_t bDemandCntInRed; /* in red */
  uint8_t bDemandCntInGreen; /* in green */
  uint16_t sFDemandDurInPer; /* duration taken after first demand occured in */
                             /* period, */
  uint16_t sFDemandDurInRed; /* in red */
  uint16_t sOccDurInPer; /* duration taken when unit is busy in period */
  uint16_t sOccDurInRed; /* in red */
  uint16_t sOccDurInGreen; /* in green */
  uint16_t sGapDurInPer; /* duration which in unit has no demand */
  uint16_t sGapDurInGreen; /* green */
  uint16_t sBrokenDur; /* duration taken after unit is broken in red */
  uint8_t fBroken : 1; /* info if it is broken */
  uint8_t fReserved : 7;
} tSInputRuntime, *tpSInputRuntime;

/*(4)
 *       name: signal group traffic actuated related runtime data
 *       expl:
 */
typedef struct _SSGIORuntime
{
  uint16_t bDemandCntInRed; /* sum of number of demands from units sg has in red */
  uint16_t bDemandCntInGreen; /* in green */
  uint16_t sFDemandDurInRedMax; /* duration taken after first demand occured in */
                                /* red for a sg which is not a unit like */
                                /* detector or input, first vehicle waiting time */
                                /* in sg */
  uint16_t sOccDurInRedMax; /* maximum of occupation durations taken by units */
                            /* belonging to a sg in red */
  uint16_t sOccDurInGreenMax; /* in green */
  uint16_t sGapDurInGreenMin; /* minimum of gap durations taken by units */
                              /* belonging to a sg in green */
} tSSGIORuntime, *tpSSGIORuntime;

/*(5)
 *       name: outputs that are connected IO module
 *       expl:
 */
typedef struct _SOutputs
{
  uint8_t bActiveLevel; /* high or low, listed below */
  uint8_t bActiveLevelDur; /* duration of active level */
  uint8_t bInActiveLevelDur; /* duration of inactive level */
} __attribute__((packed)) tSOutputDef, *tpSOutputDef;

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Flash Periods */
typedef struct _SFlashPeriods
{
  uint16_t sFlashPeriod;
  uint16_t sEmergencyFlashPeriod;
} __attribute__((packed)) tSFlashPeriods, *tpSFlashPeriods;

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Checksum */
typedef struct _SChecksum
{
  uint8_t bDeviceInfo;
  uint8_t bSignalDefs;
  uint8_t bSODefs;
  uint8_t bSGDefs;
  uint8_t baSeqDefs[SIGNAL_SEQS_MAX];
  uint8_t bPhaseDefs;
  uint8_t bFlashPeriods;
  uint8_t bConflictsEM;
  uint8_t bConsumed;
  uint8_t bCVSDefs;
  uint8_t bSignalPlans;
  uint8_t baSigProgPlans[SIGNAL_PROGRAM_PLANS_MAX];
  uint8_t baWPDefs[WORK_PLANS_MAX];
  uint8_t baSigProgs[SIGNAL_PROGRAMS_MAX];
  uint8_t baTransitions[SIGNAL_PROGRAMS_MAX];
  uint8_t baRules[SIGNAL_PROGRAMS_MAX];
  uint8_t baOperations[SIGNAL_PROGRAMS_MAX];
  uint8_t baOperands[SIGNAL_PROGRAMS_MAX];
  uint8_t baStatements[SIGNAL_PROGRAMS_MAX];
  uint8_t bWSDef;
  uint8_t bDedectors;
  uint8_t bInputs;
  uint8_t bOutputs;
  uint8_t bRemoteConfig;
  uint8_t bAll;
} __attribute__((packed)) tSChecksum, *tpSChecksum;

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Consumed */
typedef struct _SConsumed
{
  uint8_t bSOTotal;
  uint8_t bSGTotal;
  uint8_t bSeqTotal;
  uint8_t baSeqStepTotal[SIGNAL_SEQS_MAX];
  uint8_t bPhaseTotal;
  uint8_t bSignalTotal;
  uint8_t bSignalPlanTotal; /* total number of signal plans */
  uint8_t bWPTotal; /* work plan */
  uint8_t baWPEntriesTotal[WORK_PLANS_MAX]; /* work plan entries */
  uint8_t bSPPlanTotal; /* signal program plan */
  uint8_t baSPPlanEntriesTotal[SIGNAL_PROGRAM_PLANS_MAX]; /* signal program plan */
                                                          /* entries */
  uint8_t bSPTotal; /* signal program */
  uint8_t baTransitionTotal[SIGNAL_PROGRAMS_MAX]; /* 0th belongs to default */
                                                  /* signal program */
  uint8_t baRuleTotal[SIGNAL_PROGRAMS_MAX]; /* total number of rules used, 0th */
                                            /* belongs to default signal program */
  uint8_t baOperationTotal[SIGNAL_PROGRAMS_MAX]; /* total number of operations */
                                                 /* used, 0th belongs to default */
                                                 /* signal program */
  uint8_t baOperandTotal[SIGNAL_PROGRAMS_MAX]; /* total number of operands used, */
                                               /* 0th belongs to default signal */
                                               /* program */
  uint8_t baStatementTotal[SIGNAL_PROGRAMS_MAX]; /* total number of statements */
                                                 /* used, 0th belongs to default */
                                                 /* signal program */
  uint8_t bWSEntriesTotal; /* work schedule entries */
  uint8_t bDetectorTotal;
  uint8_t bInputDigitalTotal;
  uint8_t bOutputTotal;
  uint8_t bSSMTotal;
} __attribute__((packed)) tSConsumed, *tpSConsumed;

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Device Clock Source */
#define TIME_SOURCE_NONE 0
#define TIME_SOURCE_NET 1 /* net voltage */
#define TIME_SOURCE_RTC 2 /* real time clock */
#define TIME_SOURCE_GPS 3 /* gps anttenna */
#define TIME_SOURCE_CENTER 4 /* traffic center */
#define TIME_SOURCES_MAX 4

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/* Device Test */
#define SSM_TEST_FROM_NONE 0
#define SSM_TEST_FROM_MMI 1
#define SSM_TEST_FROM_UI_RCV_SIGNALS 2

/*
 *       name: general errors collection
 *       expl:
 */
typedef struct
{
  uint8_t fError;
  uint8_t bOwnerSG;
} __attribute__((packed)) tSLampError, *tpSLampError;

typedef struct _SErrInfo
{
  uint8_t bErrSSM : 8;
  uint8_t bErrPSM : 2;
  uint8_t bErrVoltage : 2;
  uint8_t bErrFreq : 2;
  uint8_t bErrDoor : 1;
  uint8_t bReserved : 1;

  struct
  {
    uint8_t fError;
    uint8_t bLDNo;
    uint8_t bSGNo;
  } __attribute__((packed)) SErrLD;

  struct
  {
    uint8_t fError;
    uint8_t bSGNo;
    uint8_t bDisplayedSignal;
  } __attribute__((packed)) SErrInvalidSignal;

  struct
  {
    uint8_t fError;
    uint8_t bSGNo;
    uint8_t bPreviousSignal;
    uint8_t bDisplayedSignal;
  } __attribute__((packed)) SErrInvalidSigSequence;

  tSLampError SErrLastRedLamp;
  tSLampError SErrNoOfRedLamps;
  tSLampError SErrLampFailRed;
  tSLampError SErrLampFailYellow;
  tSLampError SErrLampFailGreen;
  tSLampError SErrLampFailAllRed;
  tSLampError SErrLampFailAllYellow;
  tSLampError SErrLampFailAllGreen;

  struct
  {
    uint8_t fError;
    uint8_t bType;
    uint8_t bSG1;
    uint8_t bSG2;
  } __attribute__((packed)) SErrConflict;

  uint8_t bErrLDM : 8;
  uint8_t bErrIOM : 8;
} __attribute__((packed)) tSErrInfo, *tpSErrInfo;

/*
 *       name: devices
 *       expl:
 */
#define SIG_DEV_SSM_FIRST 0
#define SIG_DEV_SSM_0 0
#define SIG_DEV_SSM_1 1
#define SIG_DEV_SSM_2 2
#define SIG_DEV_SSM_3 3
#define SIG_DEV_SSM_4 4
#define SIG_DEV_SSM_5 5
#define SIG_DEV_SSM_6 6
#define SIG_DEV_SSM_7 7
#define SIG_DEV_SSM_LAST 7
#define SIG_DEV_SSM_MAX 8

#define SIG_DEV_PSM_FIRST 16
#define SIG_DEV_PSM_0 16
#define SIG_DEV_PSM_1 17
#define SIG_DEV_PSM_LAST 17
#define SIG_DEV_PSM_MAX 2

#define SIG_DEV_IO_FIRST 18
#define SIG_DEV_IO_0 18
#define SIG_DEV_IO_1 19
#define SIG_DEV_IO_LAST 19
#define SIG_DEV_IO_MAX 2

#define SIG_DEV_LD_FIRST 20
#define SIG_DEV_LD_0 20
#define SIG_DEV_LD_1 21
#define SIG_DEV_LD_2 22
#define SIG_DEV_LD_3 23
#define SIG_DEV_LD_4 24
#define SIG_DEV_LD_5 25
#define SIG_DEV_LD_6 26
#define SIG_DEV_LD_7 27

#define SIG_DEV_LD_LAST 27
#define SIG_DEV_LD_MAX 8

#define SIG_DEV_MAX 32

/* each 4 loop dedector attached to an i/o card */
#define LD_NUM_FOR_1_IO_CARD 4
#define LOOP_NUM_IN_1_LD 4
#define TOTAL_LOOP_DEDECTOR_NUM 8

/* Signal Program Data Structure */
typedef struct
{
  tSSigProg SSigProg;
  tSTransition SaTransitions[TRANSITIONS_MAX];
  tSOperation SaOperations[RULE_OPERATIONS_MAX];
  tSRule SaRules[RULES_MAX];
  tSStatement SaStatements[RULE_STATEMENTS_MAX];
} __attribute__((packed)) tSSignalPrograms, *tpSSignalPrograms;

#define SEQ_PROC_ADD 0
#define SEQ_PROC_DEL 1
#define SEQ_PROC_UPDATE 2

typedef struct _tSUIRuntime
{
  uint8_t bCheckDownloadTimeout;
  uint8_t bCheckUploadTimeout;

  uint16_t sDownloadDuration;
  uint16_t sUploadDuration;

  uint8_t bMCSDownloadInProg;
  uint8_t bMCSUploadInProg;
} tSUIRuntime, *tpSUIRuntime;

typedef struct _tSMCSTrafficCountsRuntimes
{
  tSMCSInputRuntime SaMCSDetectorInputRuntimes[INPUTS_DETECTOR_MAX];
  tSMCSInputRuntime SaMCSDigitalInputRuntimes[INPUTS_DIGITAL_MAX];
} __attribute__((packed)) tSMCSTrafficCountsRuntimes,
*tpSMCSTrafficCountsRuntimes;

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/* Signal State */
#define SIGNAL_STATES_MAX 1
#define SIGNAL_STATE_DEFAULT 0

#define SIGNAL_STATE_NEXT_PHASES_MAX_LEN 48

typedef enum
{
  SIGNAL_STATE_EXEC_MODE_NONE = 1,
  SIGNAL_STATE_EXEC_MODE_INIT,
  SIGNAL_STATE_EXEC_MODE_INIT_FLASH,
  SIGNAL_STATE_EXEC_MODE_PROGRAM,
  SIGNAL_STATE_EXEC_MODE_FLASH,
  SIGNAL_STATE_EXEC_MODE_FAIL_FLASH,
  SIGNAL_STATE_EXEC_MODE_ALL_YELLOW,
  SIGNAL_STATE_EXEC_MODE_ALL_RED,
  SIGNAL_STATE_EXEC_MODE_ALL_DARK,
  SIGNAL_STATE_EXEC_MODE_TESTING_SOS,
  SIGNAL_STATE_EXEC_MODE_PROGRAM_ERROR,
  SIGNAL_STATE_EXEC_MODE_TEST_SP
} tESignalStateExecutionMode;

typedef enum
{
  SIGNAL_STATE_PLAN_MODE_FIXED_PLAN = 1,
  SIGNAL_STATE_PLAN_MODE_HALF_ACTUATED,
  SIGNAL_STATE_PLAN_MODE_FULLY_ACTUATED,
  SIGNAL_STATE_PLAN_MODE_CENTRAL_ADAPTIVE,
  SIGNAL_STATE_PLAN_MODE_FLASING,
  SIGNAL_STATE_PLAN_MODE_DARK,
  SIGNAL_STATE_PLAN_MODE_LOCAL_ADAPTIVE
} tESignalStatePlanMode;

typedef struct _SSignalStateRuntime
{
  uint8_t bNumber;
  uint8_t bPatternNo;
  uint8_t bExecutionMode;
  uint8_t bPlanMode;
  uint8_t bSetNo;
  uint8_t bPlanNo;
  uint8_t bPhaseNo;
  uint8_t bNextPhase;
  uint8_t bStepNo;
  uint8_t fIsTransitionStep;
  uint8_t bTransitionStepNo;
  uint8_t bElapsedTime;
  uint8_t bRemainingTime;
  uint8_t bStepTime;
  uint8_t bCycleTime;
  uint8_t bTimeToNexCycle;
} tSSignalStateRuntime, *tpSSignalStateRuntime;

#define CHANNEL_ERROR_FLAGS_MAX (uint8_t) (SIGNAL_GROUPS_MAX / 8)
typedef struct _SChannelErrors
{
  uint8_t bNumber;
  uint8_t bRedLampFailure;
  uint8_t bYellowLampFailure;
  uint8_t bGreenLampFailure;

  uint8_t bUndesiredRedLamp;
  uint8_t bUndesiredYellowLamp;
  uint8_t bUndesiredGreenLamp;
} tSSChannelErrors, *tpSSChannelErrors;

typedef struct _tSRuntimes
{
  uint16_t sSSMStatus;
  uint16_t sLDMStatus;
  uint32_t lPlanLastChangeTime;

  tSSetRuntime SaSetRuntime[SIGNAL_SETS_MAX];
  tSSGRuntime SaSGRuntimes[SIGNAL_GROUPS_MAX];
  tSPhaseRuntime SaPhaseRuntimes[PHASES_MAX];
  tSSeqRuntime SSeqRuntime;
  tSSGIORuntime SaSGIORuntime[SIGNAL_GROUPS_MAX];
  tSInputRuntime SaDetectorRuntimes[INPUTS_DETECTOR_MAX];
  tSInputRuntime SaInputRuntimes[INPUTS_DIGITAL_MAX];
  tSUIRuntime SUIRuntime;
  tSSignalStateRuntime SaSignalStateRuntimes[SIGNAL_STATES_MAX];
  tSSChannelErrors SaChannelErrors[CHANNEL_ERROR_FLAGS_MAX];
} tSRuntimes, *tpSRuntimes;

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  GPS Port Configuration */
#define COM_PORT_ASSIGNMENT_NONE 0x00
#define COM_PORT_ASSIGNMENT_INTERNAL 0x01 /* GPS communicates through USART2 */
#define COM_PORT_ASSIGNMENT_EXTERNAL                                           \
        0x02 /* GPS communicates through UART5 (COM2) */
#define COM_PORT_ASSIGNMENT_MAX 0x02

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  GPS Baud Rate Configuration */
#define COM_BAUD_RATE_ASSIGNMENT_MAX 0x0B

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Heater and lamp dimming input state check period */
#define HEATER_AND_LAMP_DIMMING_PERIOD 0x1E
#define DIMMING_CHANGE_COUNT 20

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  SSM Test */
typedef struct _tSSSMTest
{
  uint8_t bSSMTestSource;
  uint8_t bTurnOnSONo;
} tSSSMTest, *tpSSSMTest;

/* Maestro Data Structure */
#define SCP_START_MAGIC 0xAAAA5555
#define SCP_MAGIC_MAX 8

typedef struct _tSCP
{
  uint32_t laStartMagic[SCP_MAGIC_MAX];
  tSFlashPeriods SFlashPeriods;
  tSDeviceInfo SDevInfo;
  tSSignalDef SaSignalDefs[SIGNALS_MAX];
  tSSignalsDefined SSignalsDefined;
  tSSGDef SaSGDefs[SIGNAL_GROUPS_MAX];
  tSSODef SaSODefs[SIGNAL_OUTPUTS_MAX];
  tSCVSDef SCVSDef;
  tSConflictsEM SConflictsEM;
  tSSeqDef SaSeqDefs[SIGNAL_SEQS_MAX];
  tSPhaseDef SaPhaseDefs[PHASES_MAX];
  tSInput SaDetectorDefs[INPUTS_DETECTOR_MAX];
  tSInput SaInputDefs[INPUTS_DIGITAL_MAX];
  tSOutputDef SaOutputDefs[OUTPUTS_MAX];
  tSaWorkPlan SaWorkPlan[WORK_PLANS_MAX];
  tSaSPPlan SaSPPlan[SIGNAL_PROGRAM_PLANS_MAX];
  tSSignalPlan SaSignalPlans[SIGNAL_PLANS_MAX];
  tSaWorkSchedule SaWorkSchedule;
  tSSignalPrograms SaSignalPrograms[SIGNAL_PROGRAMS_MAX];
  tSConsumed SConsumed;
  tSChecksum SChecksum;
} __attribute__((packed)) tSCP, *tpSCP;

/*  CP Runtime */
typedef enum
{
  RESET_SOURCE_NONE = 0,
  RESET_SOURCE_IWDG,
  RESET_SOURCE_WWDG,
  RESET_SOURCE_LOW_POWER,
  RESET_SOURCE_SOFTWARE,
  RESET_SOURCE_PIN,
  RESET_SOURCE_POR,
} tEDeviceResetSource;

typedef struct _tSCPRuntime
{
  uint8_t bResetEvent; /* reset source */
  uint8_t bResetSource; /*  reset source */
  uint8_t bTimeSource; /* clock advance source: net voltage, rtc, gps */
  uint8_t bCurWorkPlan;
  uint8_t bCurSPPlan; /*  id of the running signal program plan */
  uint8_t bCurSignalPlan;
  uint8_t bCurSigProg;
  uint8_t bRunningPhase;
  uint8_t bRelayStateRequest;
  uint16_t sStandbyCntr;
  uint8_t bVoltageState;
  uint8_t bFrequencyState;
  tSPeripheralStates SPeripheralStates;
  tSUserState SUserState;
  /* H&D Commented */

  /*
   *  tSHeaterLampDim       SHeater;
   *  tSHeaterLampDim       SLampDimming;
   */
  tSSigProgRuntime SSigProgRuntime;
  uint8_t bCurSignalState;
  uint8_t fLoadSignalProgram;
  uint16_t sDataChecksumTotal;

  struct
  {
    uint8_t fStandby : 1;
    uint8_t fPoliceButton : 1;
    uint8_t fReserved : 6;
  } SFlags;
} tSCPRuntime, *tpSCPRuntime;

/* Power Measurement */
typedef enum
{
  MEASUREMENT_NONE = 0x00,
  MEASUREMENT_STARTED,
  MEASUREMENT_FINISHED,
  MEASUREMENT_WRITTEN
} tEMeasurementStatus;

#define POWER_MEASUREMENT_WRITE_PERIOD 600 /* 10 Minutes */

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Module Version */
typedef struct _tSVersion
{
  uint8_t bArg1;
  uint8_t bArg2;
  uint8_t bArg3;
  char bArg4;
} __attribute__((packed)) tSVersion, *tpSVersion;

typedef struct _SModuleVersions
{
  tSVersion SCPUVersion;
  tSVersion SPSMVersions[MODULES_PSM_MAX];
  tSVersion SSSMVersions[MODULES_SSM_MAX];
  tSVersion SIOVersions[MODULES_IO_MAX];
} __attribute__((packed)) tSModulesVersion, *tpSModulesVersion;

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/* Function Configurations */
#define LIC_NONE 0
#define LIC_ISSD 1
#define LIC_DEN 2

#define MAX_BITS_IN_BYTE 8

typedef struct _SFuncConf
{
  uint8_t bConf0;

  uint8_t bReserved1;
  uint8_t bReserved2;
  uint8_t bReserved3;
} __attribute__((packed)) tSFuncConf, *tpSFuncConf;

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/* System Start Time */
#define SYSTEM_START_TIME_ALREADY_WRITTEN 0x55
#define SYSTEM_START_TIME_DEF_MIN_SYSTEM_UP_HOURS 24

typedef struct _SSystemStartTime
{
  uint8_t bAlreadyWritten;
  uint8_t bMonthDay;
  uint8_t bMonth;
  uint16_t sYear;
} __attribute__((packed)) tSSystemStartTime, *tpSSystemStartTime;

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/* Device UID */
#define UID_MAX_LENGTH 3

typedef struct _SDeviceUID
{
  uint32_t ulaUID[UID_MAX_LENGTH];
} tSSDeviceUID, *tpSDeviceUID;

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/* User Operations */
#define USER_OPERATIONS_MAX 10
#define USER_OPERATIONS_USERNAME_MAX_LEN 6

typedef enum
{
  USER_OPERATION_NONE = 1,
  USER_OPERATION_USER_ADDED,
  USER_OPERATION_TIME_CHANGED,
  USER_OPERATION_RELAY_ENABLED,
  USER_OPERATION_RELAY_DISABLED,
  USER_OPERATION_USER_LOGIN,
  USER_OPERATION_USER_LOGOUT,
  USER_OPERATION_MODE_CHANGED,
  USER_OPERATION_DEVICE_PARAMS_CHANGED,
} tEUserOperations;

typedef struct _SUserOperation
{
  uint8_t bIdx;
  uint8_t bType;
  uint32_t lTime;
  char strUsername[USER_OPERATIONS_USERNAME_MAX_LEN + 1];
} tSUserOperation, *tpSUserOperation;

typedef struct _SUserOperations
{
  uint8_t bLastOperation;
  uint8_t fIsUpdated;

  tSUserOperation SaOperations[USER_OPERATIONS_MAX];
} tSUserOperations, *tpSUserOperations;

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  TR Pattern */
#define TR_PATTERNS_SUBJUNCTIONS_MAX 1
#define TR_PATERNS_MAX SIGNAL_PROGRAMS_MAX
#define TR_PATERNS_SPECIAL_PARAM_OID_MAX_LEN 32

#define SUBJUNCTIONS_DEFAULT_JUNCTION 0

typedef enum
{
  PATTERN_PLAN_MODE_FIXED_PLAN = 1,
  PATTERN_PLAN_MODE_HALF_ACTUATED,
  PATTERN_PLAN_MODE_FULLY_ACTUATED,
  PATTERN_PLAN_MODE_CENTRAL_ADAPTIVE,
  PATTERN_PLAN_MODE_FLASING,
  PATTERN_PLAN_MODE_DARK,
  PATTERN_PLAN_MODE_LOCAL_ADAPTIVE,
  PATTERN_PLAN_MODE_EXTENDED,
} tEPatternPlanMode;

typedef struct
{
  uint8_t bSubjunctionNo;
  uint8_t bPatternNo;
  uint8_t bPriority;
  uint8_t bPlanMode;
  uint8_t bExtendedMode;
  uint8_t bSequenceNo;
  uint8_t bSplitNo;
  uint8_t fCoordStatus;
  uint8_t fOutputSupervision;
  uint8_t bSpecialParamTableOIDLength;
  uint32_t laSpecialParamTablOID[TR_PATERNS_SPECIAL_PARAM_OID_MAX_LEN];
} tSTRPattern, *tpSTRPattern;

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  TR Pattern */
#define TR_COORDS_MAX 1

typedef struct
{
  uint8_t bIdx;
  uint16_t sCycleTime;
  uint16_t sGeneralOffsetTime;
  uint8_t bPhaseId01;
  uint8_t bPhase01MinDuration;
  uint8_t bOffset01;
  uint8_t bReturnPhaseId01;
  uint8_t bPhaseId02;
  uint8_t bPhase02MinDuration;
  uint8_t bOffset02;
  uint8_t bReturnPhaseId02;
} tSTRCoord, *tpSTRCoord;

typedef struct _STRPatternsAndCoords
{
  uint8_t fSignalPlanUpgrade;
  tSTRPattern SaaPatterns[TR_PATTERNS_SUBJUNCTIONS_MAX][TR_PATERNS_MAX];
  tSTRCoord SaCoords[TR_COORDS_MAX];
} tSTRPatternsAndCoords, *tpSTRPatternsAndCoords;

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/* Global Configuration */
#define GLOBAL_CONFIG_MAX_MODULES 3
#define GLOBAL_CONFIG_ASC_CP_HARDWARE_MODULE_INDEX 0
#define GLOBAL_CONFIG_ASC_CP_SOFTWARE_MODULE_INDEX 1
#define GLOBAL_CONFIG_ASC_MP_SOFTWARE_MODULE_INDEX 2

#define GLOBAL_CONFIG_ASC_MODULE_DEVICE_NODE "1.3.6.1.4.1.59873.4.2.1"
#define GLOBAL_CONFIG_ASC_MODULE_MAKE "Teknotel Elektronik"
#define GLOBAL_CONFIG_CONTROLLER_BASED_STANDARDS                               \
        "NTCIP 1201:v02.19\\r\\nNTCIP 1202:v03A.28\\r\\nKGM v0.4"

#define GLOBAL_CONFIG_ASC_CP_HARDWARE_MODULE_MODEL "CPU4 CP"
#define GLOBAL_CONFIG_ASC_CP_HARDWARE_MODULE_VERSION "4.0.0"
#define GLOBAL_CONFIG_ASC_CP_SOFTWARE_MODULE_MODEL "CPU4 CP"
#define GLOBAL_CONFIG_ASC_CP_SOFTWARE_MODULE_VERSION "20240720-v0.0.1"
#define GLOBAL_CONFIG_ASC_MP_SOFTWARE_MODULE_MODEL "CPU3 MP"
#define GLOBAL_CONFIG_ASC_MP_SOFTWARE_MODULE_VERSION "20240720-v0.0.1"

#define GLOBAL_CONFIG_MODULE_DEVICE_NODE_MAX_LEN 10
#define GLOBAL_CONFIG_MODULE_MAKE_MAX_LEN 20
#define GLOBAL_CONFIG_MODULE_MODEL_MAX_LEN 12
#define GLOBAL_CONFIG_MODULE_VERSION_MAX_LEN 32
#define GLOBAL_CONFIG_MODULE_CTLR_BASE_STANDARDS_MAX_LEN 64

typedef enum
{
  GLOBAL_CONFIG_MODULE_TYPE_OTHER = 1,
  GLOBAL_CONFIG_MODULE_TYPE_HARDWARE,
  GLOBAL_CONFIG_MODULE_TYPE_SOFTWARE
} tEGlobalConfigModuleTypes;

typedef struct _SGlobalModule
{
  uint8_t bNumber;
  uint32_t laDeviceNode[GLOBAL_CONFIG_MODULE_DEVICE_NODE_MAX_LEN];
  char strMake[GLOBAL_CONFIG_MODULE_MAKE_MAX_LEN];
  char strModel[GLOBAL_CONFIG_MODULE_MODEL_MAX_LEN];
  char strVersion[GLOBAL_CONFIG_MODULE_VERSION_MAX_LEN];
  uint8_t bType;
} tSGlobalModule, *tpSGlobalModule;

typedef struct _SGlobalConfiguration
{
  uint16_t sSetIDParameter;
  tSGlobalModule SaModules[GLOBAL_CONFIG_MAX_MODULES];
  char strControllerBaseStandards
  [GLOBAL_CONFIG_MODULE_CTLR_BASE_STANDARDS_MAX_LEN];
} tSGlobalConfiguration, *tpSGlobalConfiguration;

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/* Global DB Management */
typedef enum
{
  DB_TRANSACTION_NORMAL = 1,
  DB_TRANSACTION_TRANSACTION,
  DB_TRANSACTION_VERIFY,
  DB_TRANSACTION_DONE = 6,
} tEDBTransactionType;

typedef enum
{
  DB_TRANSACTION_ERROR_TOO_BIG = 1,
  DB_TRANSACTION_ERROR_NO_SUCH_NAME,
  DB_TRANSACTION_ERROR_BAD_VALUE,
  DB_TRANSACTION_ERROR_READ_ONLY,
  DB_TRANSACTION_ERROR_GEN_ERROR,
  DB_TRANSACTION_ERROR_UPDATE_ERROR,
  DB_TRANSACTION_ERROR_NO_ERROR,
} tEDBTransactionErrorType;

typedef enum
{
  DB_VERIFY_STATUS_NOT_DONE = 1,
  DB_VERIFY_STATUS_DONE_WITH_ERROR,
  DB_VERIFY_STATUS_DONE_WITH_NO_ERROR,
} tEDBVerifyStatus;

typedef struct _SGlobalDbManagement
{
  uint8_t bCreateTransaction;
  uint8_t bErrorType;
  uint32_t baErrorID[256];
  uint8_t bTransactionID;
  uint8_t bMakeID;
  uint8_t bVerifyStatus;
  char strVerifyError[256];
} tSGlobalDbManagement, *tpSGlobalDbManagement;

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/* Global Time Management */
#define GTM_TIME_BASE_MAX_SCHEDULES 6
#define GTM_TIME_BASE_MAX_DAY_PLANS 2
#define GTM_TIME_BASE_MAX_DAY_PLAN_EVENTS 3

#define GTM_TIME_BASE_DAY_PLAN_ACTION_NO_OID_MAX_LEN 32

#define GTM_DAYLIGHT_SAVINGS_MAX 1

typedef enum
{
  GTM_DAYLIGHT_SAVING_OTHER = 1,
  GTM_DAYLIGHT_SAVING_DISABLE,
  GTM_DAYLIGHT_SAVING_ENABLE_US_DST,
  GTM_DAYLIGHT_SAVING_ENABLE_EU_DST,
  GTM_DAYLIGHT_SAVING_ENABLE_AU_DST,
  GTM_DAYLIGHT_SAVING_ENABLE_TS_DST,
  GTM_DAYLIGHT_SAVING_ENABLE_EG_DST,
  GTM_DAYLIGHT_SAVING_ENABLE_NA_DST,
  GTM_DAYLIGHT_SAVING_ENABLE_IQ_DST,
  GTM_DAYLIGHT_SAVING_ENABLE_MN_DST,
  GTM_DAYLIGHT_SAVING_ENABLE_IR_DST,
  GTM_DAYLIGHT_SAVING_ENABLE_FJ_DST,
  GTM_DAYLIGHT_SAVING_ENABLE_NZ_DST,
  GTM_DAYLIGHT_SAVING_ENABLE_TO_DST,
  GTM_DAYLIGHT_SAVING_ENABLE_CU_DST,
  GTM_DAYLIGHT_SAVING_ENABLE_BR_DST,
  GTM_DAYLIGHT_SAVING_ENABLE_CL_DST,
  GTM_DAYLIGHT_SAVING_ENABLE_FK_DST,
  GTM_DAYLIGHT_SAVING_ENABLE_PY_DST,
} tEGTMDaylightSaving;

typedef enum
{
  GTM_DAYLIGHT_SAVING_MONTH_JAN = 1,
  GTM_DAYLIGHT_SAVING_MONTH_FEB,
  GTM_DAYLIGHT_SAVING_MONTH_MAR,
  GTM_DAYLIGHT_SAVING_MONTH_APR,
  GTM_DAYLIGHT_SAVING_MONTH_MAY,
  GTM_DAYLIGHT_SAVING_MONTH_JUN,
  GTM_DAYLIGHT_SAVING_MONTH_JUL,
  GTM_DAYLIGHT_SAVING_MONTH_AUG,
  GTM_DAYLIGHT_SAVING_MONTH_SEP,
  GTM_DAYLIGHT_SAVING_MONTH_OCT,
  GTM_DAYLIGHT_SAVING_MONTH_NOV,
  GTM_DAYLIGHT_SAVING_MONTH_DEC,
  GTM_DAYLIGHT_SAVING_MONTH_ABSOLUTE,
  GTM_DAYLIGHT_SAVING_MONTH_DISABLED,
  GTM_DAYLIGHT_SAVING_MONTH_OTHER,
} tEGTMDaylightSavingMonth;

typedef enum
{
  GTM_DAYLIGHT_SAVING_OCCURRENCES_FIRST = 1,
  GTM_DAYLIGHT_SAVING_OCCURRENCES_SECOND,
  GTM_DAYLIGHT_SAVING_OCCURRENCES_THIRD,
  GTM_DAYLIGHT_SAVING_OCCURRENCES_FOURTH,
  GTM_DAYLIGHT_SAVING_OCCURRENCES_LAST,
  GTM_DAYLIGHT_SAVING_OCCURRENCES_SECOND_LAST,
  GTM_DAYLIGHT_SAVING_OCCURRENCES_THIRD_LAST,
  GTM_DAYLIGHT_SAVING_OCCURRENCES_FOURTH_LAST,
  GTM_DAYLIGHT_SAVING_OCCURRENCES_SPECIFIC_DAY_OF_MONTH,
} tEGTMDaylightSavingOccurrences;

typedef enum
{
  GTM_DAYLIGHT_SAVING_DAY_OF_WEEK_SUN = 1,
  GTM_DAYLIGHT_SAVING_DAY_OF_WEEK_MON,
  GTM_DAYLIGHT_SAVING_DAY_OF_WEEK_TUE,
  GTM_DAYLIGHT_SAVING_DAY_OF_WEEK_WED,
  GTM_DAYLIGHT_SAVING_DAY_OF_WEEK_THU,
  GTM_DAYLIGHT_SAVING_DAY_OF_WEEK_FRI,
  GTM_DAYLIGHT_SAVING_DAY_OF_WEEK_SAT
} tEGTMDaylightSavingDayOfWeek;

typedef struct _STimebaseSchedule
{
  uint8_t bNumber;
  uint16_t sMonth;
  uint8_t bDay;
  uint32_t lDate;
  uint8_t bDayPlan;
} tSTimebaseSchedule, *tpSTimebaseSchedule;

typedef struct _STimebaseDayPlan
{
  uint8_t bNumber;
  uint8_t bEventNumber;
  uint8_t bHour;
  uint8_t bMinute;
  uint8_t bActionNumberOIDLength;
  uint32_t laActionNumberOID[GTM_TIME_BASE_DAY_PLAN_ACTION_NO_OID_MAX_LEN];
} tSTimebaseDayPlan, *tpSTimebaseDayPlan;

typedef struct _STimebase
{
  uint8_t bDayPlanStatus;
  uint16_t sScheduleTableStatus;

  tSTimebaseSchedule SaSchedules[GTM_TIME_BASE_MAX_SCHEDULES];
  tSTimebaseDayPlan SaaDayPlans[GTM_TIME_BASE_MAX_DAY_PLANS]
  [GTM_TIME_BASE_MAX_DAY_PLAN_EVENTS];
} tSTimebase, *tpSTimebase;

typedef struct _SDaylightSaving
{
  uint8_t bNumber;
  uint8_t bBeginMonth;
  uint8_t bBeginOccurences;
  uint8_t bBeginDayOfWeek;
  uint8_t bBeginDayOfMonth;
  uint32_t lBeginSecondToTransition;
  uint8_t bEndMonth;
  uint8_t bEndOccurences;
  uint8_t bEndDayOfWeek;
  uint8_t bEndDayOfMonth;
  uint32_t lEndSecondToTransition;
  uint16_t sSecondsToAdjust;
} tSDaylightSaving, *tpSDaylightSaving;

typedef struct _SGlobalTimeManagement
{
  uint8_t bDaylightSaving;
  int32_t lGlobalLocalTimeDifferential;
  int32_t lControllerStandardTimeZone;
  tSTimebase STimebase;
  tSDaylightSaving SaDaylightSavings[GTM_DAYLIGHT_SAVINGS_MAX];
} tSGlobalTimeManagement, *tpSGlobalTimeManagement;

typedef enum
{
  CHANNEL_CONTROL_TYPE_OTHER = 1,
  CHANNEL_CONTROL_TYPE_PHASE_VEHICLE,
  CHANNEL_CONTROL_TYPE_PHASE_PEDESTRAIN,
  CHANNEL_CONTROL_TYPE_OVERLAP,
  CHANNEL_CONTROL_TYPE_PED_OVERLAP,
  CHANNEL_CONTROL_TYPE_QUEUE_JUMP
} tEChannelControlType;

typedef enum
{
  CHANNEL_GREEN_TYPE_OTHER = 1,
  CHANNEL_GREEN_TYPE_PROTECTED,
  CHANNEL_GREEN_TYPE_PERMISSIVE,
  CHANNEL_GREEN_TYPE_FLASH_YELLOW,
  CHANNEL_GREEN_TYPE_FLASH_RED
} tEChannelGreenType;

typedef struct _SChannelEntry
{
  uint8_t bNumber;
  uint8_t bControlSource;
  uint8_t bControlType;

  union
  {
    uint8_t bFlash;

    struct
    {
      uint8_t fReserved : 1;
      uint8_t fYellow : 1;
      uint8_t fRed : 1;
      uint8_t fAlternateHalfHertz : 1;
      uint8_t fReserved2 : 4;
    } SFlash;
  } USFlash;

  union
  {
    uint8_t bDim;

    struct
    {
      uint8_t fGreen : 1;
      uint8_t fYellow : 1;
      uint8_t fRed : 1;
      uint8_t fAlternateHalfLineCycle : 1;
      uint8_t fReserved2 : 4;
    } SDim;
  } USDim;

  uint8_t bGreenType;
  char strGreenIncluded[20];
  uint16_t sIntersectionId;
} tSChannelEntry, *tpSChannelEntry;

typedef struct _SChannelStatusGroupEntry
{
  uint8_t bNumber;
  uint8_t bReds;
  uint8_t bYellows;
  uint8_t bGreens;
} tSChannelStatusGroupEntry,
*tpSChannelStatusGroupEntry;

#define CHANNEL_STATUS_GROUPS_MAX (SIGNAL_GROUPS_MAX / 8)

typedef struct _SASCChannel
{
  tSChannelEntry SaChannels[SIGNAL_GROUPS_MAX];
  tSChannelStatusGroupEntry SaStatues[CHANNEL_STATUS_GROUPS_MAX];
} tSASCChannel, *tpSASCChannel;

#define INPUTS_DETECTOR_STATUS_GROUPS_MAX (INPUTS_DETECTOR_MAX / 8)
#define INPUTS_DIGITAL_STATUS_GROUPS_MAX (INPUTS_DIGITAL_MAX / 8)
#define INPUTS_DETECTOR_CONTROL_GROUPS_MAX (INPUTS_DETECTOR_MAX / 8)
#define INPUTS_DIGITAL_CONTROL_GROUPS_MAX (INPUTS_DIGITAL_MAX / 8)

typedef enum
{
  SPEED_DETECTOR_DISABLED = 0,
  SPEED_DETECTOR_ENABLED,
} tESpeedDetector;

typedef enum
{
  DETECTOR_SPEED_MODE_CUSTOM = 0,
  DETECTOR_SPEED_MODE_NTCIP,
} tEDetectorSpeedMode;

typedef enum
{
  DETECTOR_PLACEMENT_OPTION_TRAIL = 0,
  DETECTOR_PLACEMENT_OPTION_LEAD,
} tEDetectorPlacementOption;

typedef enum
{
  DETECTOR_TRAVEL_MODE_OTHER = 1,
  DETECTOR_TRAVEL_MODE_VEHICLE,
  DETECTOR_TRAVEL_MODE_TRANSIT,
  DETECTOR_TRAVEL_MODE_BICYCLE,
} tEDetectorTravelMode;

typedef struct _SVehicleDetectorEntry
{
  uint8_t bNumber;

  union
  {
    uint8_t bOptions;

    struct
    {
      uint8_t fVolumeDetector : 1;
      uint8_t fOccupancyDetector : 1;
      uint8_t fYellowLockCall : 1;
      uint8_t fRedLockCall : 1;
      uint8_t fPassage : 1;
      uint8_t fAddedInitial : 1;
      uint8_t fQueue : 1;
      uint8_t fCall : 1;
    } SOptions;
  } USOptions;

  uint8_t bCallPhase;
  uint8_t bSwitchPhase;
  uint16_t sDelay;
  uint8_t bExtend;
  uint8_t bQueueLimit;
  uint8_t bNoActivity;
  uint8_t bMaxPresence;
  uint8_t bErraticCounts;
  uint8_t bFailTime;
  uint8_t bReset;

  union
  {
    uint8_t bOptions2;

    struct
    {
      uint8_t fSpeedDetector : 1;
      uint8_t fPlacementOption : 1;
      uint8_t fSpeedModeOption : 1;
      uint8_t fReserved : 4;
    } SOptions2;
  } USOptions2;

  uint8_t bPairedDetector;
  uint16_t sPairedDetectorSpacing;
  uint16_t sAvgVehicleLength;
  uint16_t sLength;
  uint8_t bTravelMode;
} tSVehicleDetectorEntry, *tpSVehicleDetectorEntry;

typedef struct _SPedestrianDetectorEntry
{
  uint8_t bNumber;
  uint8_t bCallPhase;
  uint8_t bNoActivity;
  uint8_t bMaxPresence;
  uint8_t bErraticCounts;
  uint8_t bReset;
  uint8_t bButtonPushTime;

  union
  {
    uint8_t bOptions;

    struct
    {
      uint8_t fPresence : 1;
      uint8_t fAlternateTiming : 1;
      uint8_t fNonLocking : 1;
      uint8_t fReserved : 4;
    } SOptions;
  } USOptions;
} tSPedestrianDetectorEntry, *tpSPedestrianDetectorEntry;

typedef struct _SASCDetector
{
  tSVehicleDetectorEntry SaVehicleDetectors[INPUTS_DETECTOR_MAX];
  tSPedestrianDetectorEntry SaPedestrianDetectors[INPUTS_DIGITAL_MAX];
} tSASCDetector, *tpSASCDetector;

#define INPUTS_DETECTOR_ALARMS_COMM_FAULT 3
#define INPUTS_DETECTOR_REPORTED_ALARMS_OPEN_WIRE 2
#define INPUTS_DIGITAL_ALARMS_COMM_FAULT 3

#define PHASE_STATUS_GROUPS_MAX (PHASES_MAX / 8)
#define PHASE_CONTROL_GROUPS_MAX (PHASES_MAX / 8)

#define CABINET_ENVIRONMENT_DEVICES_MAX 3
#define CABINET_TEMP_SENSORS_MAX 1
#define CABINET_HUMIDITY_SENSORS_MAX 1

#define CABINET_ENVIRONMENT_DEVICE_DESCRIPTION_MAX 64

typedef struct _SCabinetEnvironmentDevice
{
  uint8_t bNumber;
  uint8_t bType;
  uint8_t bIndex;
  uint8_t bOnStatus;
  char strDescription[CABINET_ENVIRONMENT_DEVICE_DESCRIPTION_MAX];
  uint8_t bErrorStatus;
} tSCabinetEnvironmentDevice, *tpSCabinetEnvironmentDevice;

typedef enum
{
  CABINET_ENVIRONMENT_DEVICE_TYPE_OTHER = 1,
  CABINET_ENVIRONMENT_DEVICE_TYPE_DOOR,
  CABINET_ENVIRONMENT_DEVICE_TYPE_FAN,
  CABINET_ENVIRONMENT_DEVICE_TYPE_HEATER,
  CABINET_ENVIRONMENT_DEVICE_TYPE_FLOAT_SWITCH
} tECabinetEnvironmentDeviceTypes;

typedef enum
{
  CABINET_ENVIRONMENT_DEVICE_ON_STATUS_TRUE = 1,
  CABINET_ENVIRONMENT_DEVICE_ON_STATUS_FALSE,
} tECabinetEnvironmentDevicesOnStatus;

typedef enum
{
  CABINET_ENVIRONMENT_DEVICE_ERROR_STATUS_OTHER = 1,
  CABINET_ENVIRONMENT_DEVICE_ERROR_STATUS_NO_ERROR,
  CABINET_ENVIRONMENT_DEVICE_ERROR_STATUS_FAIL,
  CABINET_ENVIRONMENT_DEVICE_ERROR_STATUS_NOT_MONITORED,
} tECabinetEnvironmentDevicesErrorStatus;

#define CABINET_ENVIRONMENT_DEVICE_TYPE_MAX 2

typedef struct _STempSensorStatus
{
  uint8_t bNumber;
  char strDescription[CABINET_ENVIRONMENT_DEVICE_DESCRIPTION_MAX];
  int8_t bCurrentReading;
  int8_t bHighThreshold;
  int8_t bLowThreshold;
  uint8_t bStatus;
} tSTempSensorStatus, *tpSTempSensorStatus;

typedef enum
{
  CABINET_TEMP_SENSOR_STATUS_OTHER = 1,
  CABINET_TEMP_SENSOR_STATUS_NO_ERROR,
  CABINET_TEMP_SENSOR_STATUS_FAIL,
} tECabinetTempSensorStatus;

typedef struct _SHumiditySensorStatus
{
  uint8_t bNumber;
  char strDescription[CABINET_ENVIRONMENT_DEVICE_DESCRIPTION_MAX];
  uint8_t bCurrentReading;
  uint8_t bThreshold;
  uint8_t bStatus;
} tSHumiditySensorStatus, *tpSHumiditySensorStatus;

typedef enum
{
  CABINET_HUMIDITY_SENSOR_STATUS_OTHER = 1,
  CABINET_HUMIDITY_SENSOR_STATUS_NO_ERROR,
  CABINET_HUMIDITY_SENSOR_STATUS_FAIL,
} tECabinetHumiditySensorStatus;

typedef struct _SASCCabinetEnvironment
{
  uint8_t bATCCLEDMode;
  tSCabinetEnvironmentDevice
    SaaEnvironmentDevices[CABINET_ENVIRONMENT_DEVICES_MAX]
  [CABINET_ENVIRONMENT_DEVICE_TYPE_MAX];
  tSTempSensorStatus SaTempSensorStatuses[CABINET_TEMP_SENSORS_MAX];
  tSHumiditySensorStatus SaHumiditySensorStatuses[CABINET_HUMIDITY_SENSORS_MAX];
} tSASCCabinetEnvironment, *tpSASCCabinetEnvironment;

#define UNIT_TIME_SOURCES_MAX TIME_SOURCES_MAX

typedef enum
{
  UNIT_TIME_SOURCE_AVILABLE_OTHER = 1,
  UNIT_TIME_SOURCE_AVILABLE_LINE_SYNC,
  UNIT_TIME_SOURCE_AVILABLE_RTC_SQWR,
  UNIT_TIME_SOURCE_AVILABLE_CRYSTAL,
  UNIT_TIME_SOURCE_AVILABLE_GNSS,
  UNIT_TIME_SOURCE_AVILABLE_NTP
} tEUnitTimeSourceAvailable;

typedef enum
{
  UNIT_TIME_SOURCE_STATUS_NOT_ACTIVE = 1,
  UNIT_TIME_SOURCE_STATUS_ACTIVE,
  UNIT_TIME_SOURCE_STATUS_DATA_ERROR,
  UNIT_TIME_SOURCE_STATUS_DATA_TIMEOUT_ERROR,
  UNIT_TIME_SOURCE_STATUS_PENDING_UPDATE,
  UNIT_TIME_SOURCE_STATUS_NON_SEQUENTIAL,
} tEUnitTimeSourceStatus;

typedef struct _SUnitTime
{
  uint8_t bNumber;
  uint8_t bSourceAvailable;
} tSUnitTime, *tpSUnitTime;

typedef struct _SAscClock
{
  tSUnitTime SaTimeTable[UNIT_TIME_SOURCES_MAX];
  uint8_t bSourceCommanded;
} tSAscClock, *tpSAscClock;

typedef struct _SAlarmGroup
{
  uint8_t bNumber;
  uint8_t bState;
} tSAlarmGroup, *tpSAlarmGroup;

#define UNIT_ALARM_GROUPS_MAX 1

typedef struct _SAscUnit
{
  uint8_t bStartupFlash;
  uint8_t bAutoPedestrianClear;
  uint16_t sBackupTime;
  uint8_t bRedRevert;
  uint8_t bControlStatus;
  uint8_t bFlashStatus;

  union
  {
    uint8_t bAlarmStatus2;

    struct
    {
      uint8_t fPowerRestart : 1;
      uint8_t fLowBattery : 1;
      uint8_t fResponseFault : 1;
      uint8_t fExternalStart : 1;
      uint8_t fStopTime : 1;
      uint8_t fOffsetTransitioning : 1;
      uint8_t fStallCondition : 1;
      uint8_t fProcessFailure : 1;
    } SAlarmStatus2;
  } USAlarmStatus2;

  union
  {
    uint8_t bAlarmStatus1;

    struct
    {
      uint8_t fCycleFault : 1;
      uint8_t fCoordFault : 1;
      uint8_t fCoordFail : 1;
      uint8_t fCycleFail : 1;
      uint8_t fMMUFlash : 1;
      uint8_t fLocalFlash : 1;
      uint8_t fLocalFree : 1;
      uint8_t fCoordActive : 1;
    } SAlarmStatus1;
  } USAlarmStatus1;

  union
  {
    uint8_t bShortAlarmStatus;

    struct
    {
      uint8_t fPreempt : 1;
      uint8_t fTFFlash : 1;
      uint8_t fLocalCycleZero : 1;
      uint8_t fLocalCycleOverride : 1;
      uint8_t fCoordinationAlarm : 1;
      uint8_t fDetectorFault : 1;
      uint8_t fNonCriticalAlarm : 1;
      uint8_t fCriticalAlarm : 1;
    } SShortAlarmStatus;
  } USShortAlarmStatus;

  union
  {
    uint8_t bControl;

    struct
    {
      uint8_t fReserved : 1;
      uint8_t fDisableRemoteCmds : 1;
      uint8_t fExtMinRecall : 1;
      uint8_t fCallToNonActuated1 : 1;
      uint8_t fCallToNonActuated2 : 1;
      uint8_t fWalkRestModifier : 1;
      uint8_t fInterconnect : 1;
      uint8_t fDimmingEnable : 1;
    } SControl;
  } USControl;

  tSAlarmGroup SaAlarmGroups[UNIT_ALARM_GROUPS_MAX];

  uint8_t bStartupFlashMode;

  tSAscClock SAscClock;
} tSAscUnit, *tpSAscUnit;

#define ASC_PATTERNS_MAX SIGNAL_PROGRAMS_MAX
#define ASC_SPLITS_MAX WORK_PLAN_ENTRIES_MAX

typedef enum
{
  COORD_CORRECTION_MODE_OTHER = 1,
  COORD_CORRECTION_MODE_DWELL,
  COORD_CORRECTION_MODE_SHORT_WAY,
  COORD_CORRECTION_MODE_ADD_ONLY,
  COORD_CORRECTION_MODE_SUBTRACT_ONLY,
} tECoordCorrectionMode;

typedef enum
{
  COORD_MAXIMUM_MODE_OTHER = 1,
  COORD_MAXIMUM_MODE_MAX1,
  COORD_MAXIMUM_MODE_MAX2,
  COORD_MAXIMUM_MODE_MAX_INHIBIT,
  COORD_MAXIMUM_MODE_MAX3,
} tECoordMaximumMode;

typedef enum
{
  COORD_FORCE_MODE_OTHER = 1,
  COORD_FORCE_MODE_FLOATING,
  COORD_FORCE_MODE_FIXED,
} tECoordForceMode;

typedef enum
{
  PATTERN_TABLE_OTHER = 1,
  PATTERN_TABLE_PATTERNS,
  PATTERN_TABLE_OFFSET3,
  PATTERN_TABLE_OFFSET5
} tEPatternTableType;

typedef enum
{
  COORD_SYNC_POINT_OTHER = 1,
  COORD_SYNC_POINT_UNIT,
  COORD_SYNC_POINT_FIRST_COORD_PHASE_GRN_BEGIN,
  COORD_SYNC_POINT_LAST_COORD_PHASE_GRN_BEGIN,
  COORD_SYNC_POINT_FIRST_COORD_PHASE_GRN_END,
  COORD_SYNC_POINT_LAST_COORD_PHASE_GRN_END,
  COORD_SYNC_POINT_FIRST_COORD_PHASE_YEL_END,
  COORD_SYNC_POINT_LAST_COORD_PHASE_YEL_END,
} tECoordSyncPoint;

typedef enum
{
  PATTERN_OPTION_OTHER = 1,
  PATTERN_OPTION_COORD_MAXIMUM_MODE,
  PATTERN_OPTION_MAX_INHIBIT,
  PATTERN_OPTION_MAXIMUM1,
  PATTERN_OPTION_MAXIMUM2,
  PATTERN_OPTION_MAXIMUM3,
} tEPatternOptions;

typedef struct _SAscPattern
{
  uint8_t bNumber;
  uint8_t bCycleTime;
  uint8_t bOffsetTime;
  uint8_t bSplitNumber;
  uint8_t bSequenceNumber;
  uint8_t bCoordSyncPoint;
  uint8_t bOptions;
  char strSplitEnabledLanes[32];
} tSAscPattern, *tpAscPattern;

typedef enum
{
  SPLIT_MODE_OTHER = 1,
  SPLIT_MODE_NONE,
  SPLIT_MODE_MINIMUM_VEHICLE_RECALL,
  SPLIT_MODE_MAXIMUM_VEHICLE_RECALL,
  SPLIT_MODE_PEDESTRIAN_RECALL,
  SPLIT_MODE_MAXIMUM_VEHICLE_AND_PEDESTRIAN_RECALL,
  SPLIT_MODE_PHASE_OMITTED,
  SPLIT_MODE_NON_ACTUATED,
} tESplitMode;

typedef struct _SAscSplit
{
  uint8_t bNumber;
  uint8_t bPhase;
  uint8_t bTime;
  uint8_t bMode;
  uint8_t bCoordPhase;

  union
  {
    uint8_t bOptions;

    struct
    {
      uint8_t fTransitionPhaseOmit : 1;
      uint8_t fRerserved : 7;
    } SOptions;
  } USOptions;
} tSAscSplit, *tpAscSplit;

typedef enum
{
  LOCAL_FREE_STATUS_OTHER = 1,
  LOCAL_FREE_STATUS_NOT_FREE,
  LOCAL_FREE_STATUS_COMMAND_FREE,
  LOCAL_FREE_STATUS_TRANSITION_FREE,
  LOCAL_FREE_STATUS_INPUT_FREE,
  LOCAL_FREE_STATUS_COORD_FREE,
  LOCAL_FREE_STATUS_BAD_PLAN,
  LOCAL_FREE_STATUS_BAD_CYCLE_TIME,
  LOCAL_FREE_STATUS_SPLIT_OVERRUN,
  LOCAL_FREE_STATUS_INVALID_OFFSET,
  LOCAL_FREE_STATUS_FAILED,
} tELcoalFreeStatus;

typedef enum
{
  UNIT_COORD_SYNC_POINT_OTHER = 1,
  UNIT_COORD_SYNC_POINT_FIRST_PHASE_GREEN_BEGIN,
  UNIT_COORD_SYNC_POINT_LAST_PHASE_GREEN_BEGIN,
  UNIT_COORD_SYNC_POINT_FIRST_PHASE_GREEN_END,
  UNIT_COORD_SYNC_POINT_LAST_PHASE_GREEN_END,
  UNIT_COORD_SYNC_POINT_FIRST_PHASE_YELLOW_END,
  UNIT_COORD_SYNC_POINT_LAST_PHASE_YELLOW_END,
} tEUnitCoordSyncPoint;

typedef struct _SAscCoord
{
  uint8_t bOperationalMode;
  uint8_t bCoorectionMode;
  uint8_t bMaximumMode;
  uint8_t bForceMode;
  uint8_t bPatterTableType;

  tSAscPattern SaPatterns[ASC_PATTERNS_MAX];
  tSAscSplit SaaSplits[ASC_SPLITS_MAX][PHASES_MAX];

  uint8_t bPatternStatus;
  uint8_t bLocalFreeStatus;
  uint16_t sCoordCycleStatus;
  uint16_t sCoordSyncStatus;
  uint8_t bSystemPatternControl;
  uint8_t bSystemSyncControl;
  uint8_t bUnitCoordSyncPoint;
} tSAscCoord, *tpSAscCoord;

#define TIMEBASE_ASC_ACTIONS_MAX 1

typedef struct _STimebaseAscAction
{
  uint8_t bNumber;
  uint8_t bPattern;

  union
  {
    uint8_t bAuxiliaryFunction;

    struct
    {
      uint8_t fAuxFunction1 : 1;
      uint8_t fAuxFunction2 : 1;
      uint8_t fAuxFunction3 : 1;
      uint8_t fDimmingEnabled : 1;
      uint8_t fRerserved : 4;
    } SAuxiliaryFunction;
  } UAuxiliaryFunction;

  union
  {
    uint8_t bSpecialFunction;

    struct
    {
      uint8_t fSepcialFunction1 : 1;
      uint8_t fSepcialFunction2 : 1;
      uint8_t fSepcialFunction3 : 1;
      uint8_t fSepcialFunction4 : 1;
      uint8_t fSepcialFunction5 : 1;
      uint8_t fSepcialFunction6 : 1;
      uint8_t fSepcialFunction7 : 1;
      uint8_t fSepcialFunction8 : 1;
    } SSpecialFunction;
  } USpecialFunction;
} tSTimebaseAscAction, *tpSTimebaseAscAction;

typedef struct _STimebaseAsc
{
  uint16_t sPatternSync;
  tSTimebaseAscAction SaActions[TIMEBASE_ASC_ACTIONS_MAX];
  uint8_t bActionStatus;
  uint8_t bActionPlanControl;
} tSTimebaseAsc, *tpSTimebaseAsc;

#define RINGS_MAX 1
#define SEQUENCE_PLANS_MAX 1
#define RING_CONTROL_GROUPS_MAX 1

typedef struct _SSequencePlan
{
  uint8_t bNumber;
  uint8_t bRingNumber;
  char strData[PHASES_MAX];
} tSSequencePlan, *tpSSequencePlan;

typedef struct _SRingControlGroup
{
  uint8_t bNumber;
  uint8_t bStopTime;
  uint8_t bForceOff;
  uint8_t bMax2;
  uint8_t bMaxInhibit;
  uint8_t bPedRecycle;
  uint8_t bRedRest;
  uint8_t bOmitRedClear;
  uint8_t bMax3;
} tSRingControlGroup, *tpSRingControlGroup;

typedef struct _SAscRing
{
  tSSequencePlan SaaSequencePlans[SEQUENCE_PLANS_MAX][RINGS_MAX];
  tSRingControlGroup SaControlGroups[RING_CONTROL_GROUPS_MAX];

  union
  {
    uint8_t bStatus;

    struct
    {
      uint8_t bCodedStatus : 3;
      uint8_t fGapOut : 1;
      uint8_t fMaxOut : 1;
      uint8_t fForceOff : 1;
      uint8_t fReserved : 2;
    } SStatus;
  } UaStatuses[RINGS_MAX];
} tSAscRing, *tpSAscRing;

#define OVERLAPS_MAX 1
#define OVERLAP_STATUS_GROUPS_MAX 1

typedef enum
{
  OVERLAP_TYPE_OTHER = 1,
  OVERLAP_TYPE_NORMAL,
  OVERLAP_TYPE_MINUS_GREEN_YELLOW,
  OVERLAP_TYPE_PEDESTRIAN_NORMAL,
  OVERLAP_TYPE_FYA_THREE_SECTION,
  OVERLAP_TYPE_FYA_FOUR_SECTION,
  OVERLAP_TYPE_FRA_THREE_SECTION,
  OVERLAP_TYPE_FRA_FOUR_SECTION,
  OVERLAP_TYPE_TRANSIT2,
  OVERLAP_TYPE_MINUS_GREEN_YELLOW_ALTERNATE
} tEOverlapType;

typedef struct _SOverlapEntry
{
  uint8_t bNumber;
  uint8_t bType;
  char strIncludedPhases[PHASES_MAX];
  char strModifierPhases[PHASES_MAX];
  uint8_t bTrailGreen;
  uint8_t bTrailYellow;
  uint8_t bTrailRed;
  uint8_t bWalk;
  uint8_t bPedClearance;
  char strConflictingPedPhases[PHASES_MAX];
} tSOverlapEntry, *tpSOverlapEntry;

typedef struct _SOverlapStatusGroup
{
  uint8_t bNumber;
  uint8_t bReds;
  uint8_t bYellows;
  uint8_t bGreens;
} tSOverlapStatusGroup, *tpSOverlapStatusGroup;

typedef struct _SOverlap
{
  tSOverlapEntry SaOverlaps[OVERLAPS_MAX];
  tSOverlapStatusGroup SaStatues[OVERLAP_STATUS_GROUPS_MAX];
} tSOverlap, *tpSOverlap;

typedef struct _SAscBlock
{
  uint8_t baGetControl[12];
  uint8_t baData[127];
  uint16_t sErrorStatus;
} tSAscBlock, *tpSAscBlock;

#define PREEMPTS_MAX 1
#define PREEMPT_GROUPS_MAX 1
#define PREEMPT_GATES_MAX 1

typedef enum
{
  PREEMPT_STATE_OTHER = 1,
  PREEMPT_STATE_NOT_ACTIVE,
  PREEMPT_STATE_NOT_ACTIVE_WITH_CALL,
  PREEMPT_STATE_ENTRY_STARTED,
  PREEMPT_STATE_TRACK_SERVICE,
  PREEMPT_STATE_DWELL,
  PREEMPT_STATE_LINK_ACTIVE,
  PREEMPT_STATE_EXIT_STARTED,
  PREEMPT_STATE_MAX_PRESENCE,
  PREEMPT_STATE_ADVANCED_PREEMPT,
} tEPreemptState;

typedef enum
{
  PREEMPT_EXIT_PHASES = 1,
  PREEMPT_EXIT_QUEUE_DELAY_RECOVERY,
  PREEMPT_EXIT_SHORT_SERVICE,
  PREEMPT_EXIT_COORD,
} tEPreemptExitType;

typedef struct _SPreemptEntry
{
  uint8_t bNumber;

  union
  {
    uint8_t bControl;

    struct
    {
      uint8_t fNonLockingMemory : 1;
      uint8_t fOverrideFlash : 1;
      uint8_t fOverride : 1;
      uint8_t fFlashDwell : 1;
      uint8_t fEnable : 1;
      uint8_t fAllRedFlash : 1;
      uint8_t fReserved : 2;
    } SControl;
  } UControl;

  uint8_t bLink;
  uint16_t sDelay;
  uint16_t sMinDuration;
  uint8_t bMinGreen;
  uint8_t bMinWalk;
  uint8_t bEnterPedClear;
  uint8_t bTrackGreen;
  uint8_t bDwellGreen;
  uint16_t sMaxPresence;
  char strTrackPhase[127];
  char strDwellPhase[127];
  char strDwellPed[127];
  char strExitPhase[127];
  uint8_t bState;
  char strTrackOverlap[127];
  char strDwellOverlap[127];
  char strCyclingPhase[127];
  char strCyclingPed[127];
  char strCyclingOverlap[127];
  uint8_t bEnterYellowChange;
  uint8_t bEnterRedClear;
  uint8_t bTrackYellowChange;
  uint8_t bTrackRedClear;
  uint8_t bSequenceNumber;
  uint8_t bExitType;
} tSPreemptEntry, *tpSPreemptEntry;

typedef struct _SPreemptControlEntry
{
  uint8_t bNumber;
  uint8_t bState;
} tSPreemptControlEntry, *tpSPreemptContryEntry;

typedef struct _SPreemptStatusGroup
{
  uint8_t bNumber;
  uint8_t bStatus;
} tSPreemptStatusGroup, *tpSPreemptStatusGroup;

typedef struct _SPreemptQueueDelay
{
  uint8_t bNumber;
  uint8_t bVehDetectorNumber;
  uint16_t sDetectorWeight;
} tSPreemptQueueDelay, *tpSPreemptQueueDelay;

typedef struct _SPreemptGate
{
  uint8_t bNumber;
  uint8_t bStatus;
  char strDescription[255];
} tSPreemptGate, *tpSPreemptGate;

typedef struct _SPreempt
{
  uint8_t bStatus;
  tSPreemptEntry SaPreempts[PREEMPTS_MAX];
  tSPreemptControlEntry SaPreemptControls[PREEMPTS_MAX];
  tSPreemptStatusGroup SaStatusGroups[PREEMPT_GROUPS_MAX];
  tSPreemptQueueDelay SaaQueueDelays[PREEMPTS_MAX][INPUTS_DETECTOR_MAX];
  tSPreemptGate SaGates[PREEMPT_GATES_MAX];
} tSPreempt, *tpSPreempt;

typedef enum
{
  ASC_POWER_SOURCE_UNKNOWN = 1,
  ASC_POWER_SOURCE_OTHER,
  ASC_POWER_SOURCE_AC_LINE,
  ASC_POWER_SOURCE_GENERATOR,
  ASC_POWER_SOURCE_SOLAR,
  ASC_POWER_SOURCE_BATTERY_UPS,
  ASC_POWER_SOURCE_DC_48V_POWER,
  ASC_POWER_SOURCE_DC_24V_POWER
} tEASCPowerSources;

typedef enum
{
  ATCC_LED_MODE_OTHER,
  ATCC_LED_MODE_ON,
  ATCC_LED_MODE_OFF,
} tEATCCLEDMode;

typedef struct _SAscPhaseEntry
{
  uint8_t bNumber;
  uint8_t bWalk;
  uint8_t bPedestrianClear;
  uint8_t bMinimumGreen;
  uint8_t bPassage;
  uint8_t bMaximum1;
  uint8_t bMaximum2;
  uint8_t bYellowChange;
  uint8_t bRedClear;
  uint8_t bRedRevert;
  uint8_t bAddedInitial;
  uint8_t bMaximumInitial;
  uint8_t bTimeBeforeReduction;
  uint8_t bCarsBeforeReduction;
  uint8_t bTimeToReduce;
  uint8_t bReduceBy;
  uint8_t bMinimumGap;
  uint8_t bDynamicMaxLimit;
  uint8_t bDynamicMaxStep;
  uint8_t bStartup;

  union
  {
    uint16_t sOptions;

    struct
    {
      uint8_t fEnabled : 1;
      uint8_t fAutoFlashEntryPhase : 1;
      uint8_t fAutoFlashExitPhase : 1;
      uint8_t fNonAcutated1 : 1;
      uint8_t fNonAcutated2 : 1;
      uint8_t fNonLockDetectorMemory : 1;
      uint8_t fVehicleRecall : 1;
      uint8_t fMaxVehicleRecall : 1;
      uint8_t fPedRecall : 1;
      uint8_t fSoftVehicleRecall : 1;
      uint8_t fDualEntryPhase : 1;
      uint8_t fSimultaneousGapDisable : 1;
      uint8_t fGuaranteddPassage : 1;
      uint8_t fActuatedRestInWalk : 1;
      uint8_t fConditionalServiceEnable : 1;
      uint8_t fAddedInitialCalculation : 1;
    } SOptions;
  } USOptions;

  uint8_t bRing;
  char strConcurrency[PHASES_MAX];
  uint16_t bMaximum3;
  uint8_t bYelAndRedChangeTimeBeforeEndPedClear;
  uint8_t bPedWalkService;
  uint8_t bDontWalkRevert;
  uint8_t bPedAlternateClearance;
  uint8_t bPedAlternateWalk;
  uint8_t bPedAdvanceWalkTime;
  uint8_t bPedDelayTime;
  uint8_t bAdvWarnGrnStartTime;
  uint8_t bAdvWarnRedStartTime;
  uint8_t bAltMinTimeTransaction;
} tAscPhaseEntry, *tpSAscPhaseEntry;

typedef struct _SAscPhaseStatusGroup
{
  uint8_t bNumber;
  uint8_t bReds;
  uint8_t bYellows;
  uint8_t bGreens;
  uint8_t bDontWalks;
  uint8_t bPedClears;
  uint8_t bWalks;
  uint8_t bVehCalls;
  uint8_t bPedCalls;
  uint8_t bPhaseOns;
  uint8_t bPhaseNexts;
} tSAscPhaseStatusGroup, *tpSAscPhaseStatusGroup;

typedef struct _SAscPhaseControlGroup
{
  uint8_t bNumber;
  uint8_t bOmit;
  uint8_t bPedOmit;
  uint8_t bHold;
  uint8_t bForceOff;
  uint8_t bVehCall;
  uint8_t bPedCall;
} tSAscPhaseControlGroup, *tpSAscPhaseControlGroup;

typedef struct _SAscPhase
{
  tAscPhaseEntry SaPhases[PHASES_MAX];
  tSAscPhaseStatusGroup SaStatuses[PHASE_STATUS_GROUPS_MAX];
  tSAscPhaseControlGroup SaControlGroups[PHASE_CONTROL_GROUPS_MAX];
} tSAscPhase, *tpSAscPhase;

/* Public Data */
extern tSRuntimes SRuntimes;
extern tSCanDigitalIOInputs SaCanDigitalIOInputs[MODULES_IO_MAX];
extern tSCanDetectorIOInputs SaCanDetectorIOInputs[MODULES_IO_MAX];
extern const uint32_t laValue2Bit[32];

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Public Methods */

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  DWT */
extern void DWTInit(void);
extern void DWTDelayuSeconds(volatile uint32_t uSeconds);

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Module Versions */
extern void SetModulesVersion(void);
extern tpSModulesVersion GetModulesVersion(void);

/*  Police Button */
extern uint8_t GetPoliceButtonState(void);
extern void SetPoliceButtonState(uint8_t fState);

/*  Daylight Saving Time */
extern uint8_t WriteDaylightSavingTimeFlag(void);
extern uint8_t ReadDaylightSavingTimeFlag(void);
extern void SetDaylightSavingTimeFlag(uint8_t bNewValue);
extern void GetDaylightSavingTimeFlag(uint8_t *pNewValue);
extern uint8_t IsDaylightSavingTimeFlagSet(void);
extern void CheckDaylightSavingTimeFlag(void);

/* Username Password */
extern int DigitCountsGet(int number);
extern uint8_t GetSuperAdminValidity(void);
extern void SetSuperAdminValidity(uint8_t fValid);
extern uint16_t GetSuperAdminUsername(void);
extern void SetSuperAdminUsername(uint16_t sUsername);
extern uint16_t GetSuperAdminPassword(void);
extern void SetSuperAdminPassword(uint16_t sPassword);
extern uint8_t ReadAdminValidity(void);
extern uint8_t GetAdminValidity(void);
extern uint8_t WriteAdminValidity(void);
extern void SetAdminValidity(uint8_t fValid);
extern uint8_t ReadAdminUsername(void);
extern uint16_t GetAdminUsername(void);
extern uint8_t WriteAdminUsername(void);
extern void SetAdminUsername(uint16_t sUsername);
extern uint8_t ReadAdminPassword(void);
extern uint16_t GetAdminPassword(void);
extern uint8_t WriteAdminPassword(void);
extern void SetAdminPassword(uint16_t sPassword);
extern uint8_t GetGuestValidity(void);
extern void SetGuestValidity(uint8_t fValid);
extern uint16_t GetGuestUsername(void);
extern void SetGuestUsername(uint16_t sUsername);
extern uint16_t GetGuestPassword(void);
extern void SetGuestPassword(uint16_t sPassword);

/*  Heater & Lamp Dimming */
/* H&D Commented */

/*
 *  extern  uint8_t   GetHeaterActiveState(void);
 *  extern  void    SetHeaterActiveState(uint8_t fState);
 *  extern  uint8_t   GetLampDimmingActiveState(void);
 *  extern  void    SetLampDimmingActiveState(uint8_t fState);
 *  extern  uint8_t   HeaterInfoSave(tpSHeaterLampDim pSHeaterLampDim);
 *  extern  uint8_t   HeaterInfoRead(void);
 *  extern  void    HeaterInfoGet(tpSHeaterLampDim pSHeater);
 *  extern  uint8_t   LampDimmingInfoSave(tpSHeaterLampDim pSHeaterLampDim);
 *  extern  uint8_t   LampDimmingInfoRead(void);
 *  extern  void    LampDimmingInfoGet(tpSHeaterLampDim pSLampDimm);
 */
extern void ProcessHeaterAndLampDimRequests(void);

/*  User Settings */
extern uint8_t UserSettingsSave(void);
extern uint8_t UserSettingsRead(void);
extern void UserSettingsInit(void);
extern void UserSettingsSet(tpSUserSettings pSUserSettings);
extern void UserSettingsGet(tpSUserSettings pSUserSettings);
extern uint8_t IsUserSettingsChanged(void);
extern uint8_t UserSettingsConfigFlagGet(void);
extern uint8_t UserSettingsLogFlagGet(void);
extern uint8_t UserSettingsTrafficCountsFlagGet(void);
extern uint8_t UserSettingsTrafficCountsPeriodGet(void);
extern uint8_t UserSettingsStandbyFlagGet(void);

/*  Broken Inputs Settings */
extern uint8_t BrokenInputSettingsSave(void);
extern uint8_t BrokenInputSettingsRead(void);
extern void BrokenInputSettingsInit(void);
extern void BrokenInputSettingsSet(
  tpSBrokenInputSettings pSBrokenInputSettings);
extern void BrokenInputSettingsGet(
  tpSBrokenInputSettings pSBrokenInputSettings);
extern uint8_t IsBrokenInputSettingsSet(void);
extern uint8_t BrokenInputSettingsLoopFlagGet(void);
extern uint8_t BrokenInputSettingsDigitalFlagGet(void);

/*  Server Settings */
extern uint8_t ServerSettingsSave(void);
extern uint8_t ServerSettingsRead(void);
extern void ServerSettingsSet(tpSServerSettings pSServerSettings);
extern void ServerSettingsGet(tpSServerSettings pSServerSettings);
extern void ServerSettingsInit(void);
extern uint8_t IsServerSettingsSet(void);
extern uint8_t ServerSettingsMCSAvailableGet(void);
extern uint8_t ServerSettingsNTCIPAvailableGet(void);

/*  Reset Source */
extern void SetDeviceResetEvent(void);
extern uint8_t GetDeviceResetEvent(void);
extern void SetDeviceResetSource(void);
extern uint8_t GetDeviceResetSource(void);
extern void ClearStandbyFlag(void);
extern void ClearWakeupFlag(void);
extern void ClearResetFlags(void);
extern void ClearAllFlags(void);

/*  Source Of Time */
extern void TimeSourceSet(uint8_t bNewTimeSource);
extern uint8_t TimeSourceGet(void);

/*  Relay State Request */
extern void RelayStateRequestSet(uint8_t bState);
extern uint8_t RelayStateRequestGet(void);

/*    Standby */
extern void EnableDebug(void);
extern void DisableDebug(void);
extern void DisableInterruptRequests(void);
extern void EnterStandbyMode(void);
extern void EnterStandbyModeWithPreparation(uint8_t fPrep);
extern uint8_t GetStandbyState(void);
extern void SetStandbyState(uint8_t fState);
extern void NotifyStandbyState(void);
extern void ExecStandbyInfoOps(void);
extern void CheckWakeupState(void);

/*  Flash */
extern uint16_t FlashPeriodEmergencyGet(void);
extern uint8_t FlashPeriodEmergencySet(uint16_t sNewPeriod);
extern void FlashCntrInc(void);
extern void FlashCntrClear(void);
extern uint8_t FlashOnGet(uint16_t sPeriod);

/*  Device Info */
extern uint8_t SetDeviceInfo(tpSDeviceInfo pSDeviceInfo,
                             uint8_t keepConnectionInfo);
extern void GetDeviceInfo(tpSDeviceInfo pSDeviceInfo);
extern int8_t GetDeviceTimeZone(void);
extern void SetDeviceTimeZone(int8_t bTimeZone);
extern char *GetDeviceDomainName(void);
extern char *GetDeviceInfoAPNName(void);
extern char *GetDeviceInfoUsername(void);
extern char *GetDeviceInfoPassword(void);

/*  Signal Definitions */
extern uint8_t SignalTotalGet(void);
extern uint8_t SetSignalDefs(uint8_t bSignal, tpSSignalDef pSSignalDefBuffer);
extern void GetSignalDefs(uint8_t bSignal, tpSSignalDef pSSignalDefBuffer);
extern uint8_t SignalVoltagesGet(uint8_t bSignal);
extern uint16_t SubSignalHasFlash(uint8_t bSignal, uint8_t bOutputType);
extern uint8_t SignalValidGet(uint8_t bSignal);
extern uint8_t SignalValidForFlashGet(uint8_t bSignal);
extern uint8_t SignalValidForEmergencyFlashGet(uint8_t bSignal);
extern uint8_t SignalDurationUnlimitedGet(uint8_t bSignal);
extern uint8_t SignalMinDurationGet(uint8_t bSignal);
extern uint8_t SignalMaxDurationGet(uint8_t bSignal);
extern uint8_t SignalHasFlash(uint8_t bSignal);
extern uint8_t SignalHasGreen(uint8_t bSignal);
extern uint8_t SignalHasYellow(uint8_t bSignal);
extern uint8_t SignalHasRed(uint8_t bSignal);

/*  Signals Defined */
extern void SetSignalsDefined(tpSSignalsDefined pSSignalsDefined);
extern void GetSignalsDefined(tpSSignalsDefined pSSignalsDefined);
extern uint8_t SignalsDefinedBlockingGet(void);
extern uint8_t SignalsDefinedFreeGet(void);
extern uint8_t SignalsDefinedGreenFlashGet(void);
extern uint8_t SignalsDefinedDarkGet(void);

/*  Signal Group Sets */
extern void SetRuntimeSet(uint8_t bSetNo, tpSSetRuntime pSSetRuntime);
extern void SetRuntimeGet(uint8_t bSetNo, tpSSetRuntime pSSetRuntime);
extern uint8_t IsSetValid(uint8_t bSetNo);
extern uint8_t SetTotalGet(void);
extern uint8_t SetSigModeIsOK(void);
extern uint8_t SetSigModeIsThis(uint8_t bMode);
extern uint8_t SetSigModeGet(uint8_t bSetNo);
extern uint8_t SetSigModeIsEmergent(uint8_t bSetNo);
extern uint8_t SetSigModeSourceGet(uint8_t bSetNo);
extern void SetSigModeSet(uint8_t bSetNo, uint8_t bNewMode);
extern void ApplyEMToOneSet(uint8_t bSetNo, uint8_t bEM, uint8_t bSigModeSource,
                            uint8_t bParam1, uint8_t bParam2);
extern void ApplyEMToAllSets(uint8_t bEM, uint8_t bSigModeSource);
extern void SetInvalidSignalState(uint8_t bSetNo, uint8_t fValue);
extern uint8_t GetInvalidSignalState(uint8_t bSetNo);
extern void SetInvalidSignalSequenceState(uint8_t bSetNo, uint8_t fValue);
extern uint8_t GetInvalidSignalSequenceState(uint8_t bSetNo);

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Signal Switching Modules */
extern uint8_t SSMTotalGet(void);
extern void SSMTotalSet(uint8_t bSSMNo);
extern void SetRuntimeSSMStatus(void);

/*  Signal Groups */
extern uint8_t SetSignalGroups(uint8_t bSGNo, tpSSGDef pSSGBuffer);
extern uint8_t GetSGRedLampFailureNumberEM(uint8_t bSGNo);
extern uint8_t SGGet(uint8_t bSGNo, tpSSGDef pSSGBuffer);
extern uint8_t SGTypeGet(uint8_t bSGNo);
extern uint8_t SGOpeningSignalGet(uint8_t bSGNo);
extern uint8_t SGOpeningDurGet(uint8_t bSGNo);
extern uint8_t SGClosingSignalGet(uint8_t bSGNo);
extern uint8_t SGClosingDurGet(uint8_t bSGNo);
extern uint8_t SGFlashSignalGet(uint8_t bSGNo);
extern uint8_t GetSGFlashFailureSignal(uint8_t bSGNo);
extern uint8_t SGGreenFlashDurGet(uint8_t bSGNo);
extern uint8_t SGConflictGet(uint8_t bSGNo1, uint8_t bSGNo2);
extern uint8_t SGConflictSet(uint8_t bSGNo, uint8_t bTargetSGNo,
                             uint8_t fValue);
extern uint8_t SGClearanceGet(uint8_t bSGNo, uint8_t bConflictSGNo);
extern uint8_t SGClearanceGetbyIndex(uint8_t bSGNo, uint8_t bConflictSGIndex,
                                     uint8_t *pbClearanceDuration,
                                     uint8_t *pbConflictSGNo);
extern uint8_t SGClearanceSet(uint8_t bSGNo, uint8_t bTargetSGNo,
                              uint8_t bClearance);
extern uint8_t SGClearanceTotalGet(uint8_t bSGNo);
extern uint8_t SGFirstOutputGet(uint8_t bSGNo);
extern uint8_t SGIsValid(uint8_t bSGNo);
extern uint8_t GetSGOwner(uint8_t bSGNo);
extern uint8_t GetSGLastRedLampFailureEM(uint8_t bSGNo);
extern uint8_t SGConflictTotalGet(uint8_t bSGNo, uint8_t *pbConflictTotal);

/*  Signal Groups - Runtime */
extern uint8_t SGTotalGet(void);
extern uint8_t SGSignalGet(uint8_t bSGNo);
extern void SGSignalSet(uint8_t bSGNo, uint8_t bSignal);
extern uint8_t SGDurationGet(uint8_t bSGNo);
extern uint8_t SGStateGet(uint8_t bSGNo);
extern void SGRuntimeDataSet(uint8_t bSGNo, uint8_t bSignal, uint8_t bState);
extern void SGDurInc(void);
extern uint8_t SGFlasherAdd(uint8_t bSGNo);
extern uint8_t SGFlasherSub(uint8_t bSGNo);
extern uint8_t SGIsFlasher(uint8_t bSGNo);

/*  Signal Outputs */
extern uint8_t SetSignalOutputs(uint8_t bSSMNo, uint8_t bSONo,
                                tpSSODef pSNewSODef);
extern uint8_t GetSOTotal(void);
extern void GetSODef(uint8_t bSONo, tpSSODef pSSOBuffer);
extern uint8_t GetSODefByIndex(uint8_t bSGNo, uint8_t bIndexInSG,
                               tpSSODef pSSOBuffer, uint8_t *pbIndex);
extern uint16_t GetSOPower(uint8_t bSONo);
extern uint16_t GetSOPowerRecordNet(uint8_t bSONo);
extern uint8_t GetSGNextOutputNo(uint8_t bSONo);
extern uint8_t GetSOEM(uint8_t bSONo);
extern uint8_t GetSOOwner(uint8_t bSONo);
extern uint8_t SGSOTotalGet(uint8_t bSGNo, uint8_t *pbSOTotal);
extern uint8_t GetSOType(uint8_t bSONo);

/*  Current / Voltage / Slope */
extern void GetCVSDef(tpSCVSDef pSCVSDef);
extern uint8_t SetCVS(tpSCVSDef pSCVSDef);

/*  Conflicts EM */
extern void GetConflictsEM(tpSConflictsEM pSConflictsEM);
extern uint8_t SetConflictsEM(tpSConflictsEM pSConflictsEM);
extern uint8_t GetVoltageLimitsEM(void);
extern uint8_t GetFrequencyErrorEM(void);
extern uint8_t GetGGEM(void);
extern uint8_t GetYGEM(void);
extern uint8_t GetYYEM(void);
extern uint8_t GetInvalidSignalSequenceEM(void);
extern uint8_t GetInvalidSignalEM(void);

/*  Sequences */
extern uint8_t SeqTotalGet(void);
extern uint8_t SeqGet(uint8_t bSeqNo);
extern uint8_t SeqRead(uint8_t bSeqNo, tpSSeqDef pSSignalSeqBuffer);
extern uint8_t SeqLoad(uint8_t bSeqNo);
extern uint8_t SeqSet(uint8_t bSeqNo, tpSSeqDef pSSignalSeqBuffer);
extern uint8_t SeqSave(uint8_t bSeqNo, uint8_t bSeqProc, uint8_t LCDReq);
extern void SeqInit(uint8_t bSeqNo);
extern uint8_t SeqStepSGSignalGet(uint8_t bSeqNo, uint8_t bStepNo,
                                  uint8_t bSGNo);
extern uint8_t SeqStepSGSignalSet(uint8_t bSeqNo, uint8_t bStepNo,
                                  uint8_t bSGNo, uint8_t bReceivedSignal);
extern uint8_t SeqStepDurGet(uint8_t bSeqNo, uint8_t bStepNo);
extern uint8_t SeqStepDurSet(uint8_t bSeqNo, uint8_t bStepNo, uint8_t bDur);
extern uint8_t SeqStepInc(uint8_t bSeqNo);
extern uint8_t SeqStepDecr(uint8_t bSeqNo);
extern uint8_t SeqCurStepNumTotalGet(void);
extern uint8_t SeqStepNumTotalGet(uint8_t bSeqNo);

/*  Sequences - Runtime */
extern uint8_t SeqStepGet(uint8_t bSeqNo, uint8_t bReferenceSecond);
extern uint8_t SeqStart(uint8_t bSeqNo, uint8_t bReferenceSecond);
extern uint8_t SeqStop(uint8_t bSeqNo);
extern uint8_t SeqDurInc(void);
extern uint8_t SeqCurrentGet(void);
extern uint8_t SeqCurrentStepGet(void);
extern uint8_t SeqCurrentStepDurationGet(void);
extern uint8_t SeqCurrentStepCurrentDurationGet(void);
extern uint8_t SeqDurCurGet(void);
extern uint8_t SeqDurGet(uint8_t bSeqNo);
extern uint8_t SeqTotalGet(void);
extern uint8_t SeqStepTotalGet(uint8_t bSeqNo);

/*  Phases */
extern uint8_t PhaseGet(uint8_t bPhaseNo, tpSPhaseDef pSPhaseBuffer);
extern uint8_t PhaseSet(uint8_t bPhaseNo, tpSPhaseDef pSPhaseBuffer);
extern uint8_t PhaseMinDurationGet(uint8_t bPhaseNo);
extern uint8_t PhaseMaxDurationGet(uint8_t bPhaseNo);
extern uint8_t PhaseTotalGet(void);
extern uint8_t PhaseHasSG(uint8_t bPhaseNo, uint8_t bSGNo);
extern uint8_t PhaseIsValid(uint8_t bPhaseNo);

/*  Phases - Runtime */
extern uint8_t PhaseStart(uint8_t bPhaseNo, uint8_t bReferenceSecond);
extern uint8_t PhaseStop(uint8_t bPhaseNo);
extern void PhaseRunSet(uint8_t bPhaseNo, uint8_t fValue);
extern uint8_t PhaseRunGet(uint8_t bPhaseNo);
extern void PhaseExtDurSet(uint8_t bPhaseNo, int8_t bExtDur);
extern int8_t PhaseExtDurGet(uint8_t bPhaseNo);
extern uint8_t PhaseDurInc(uint8_t bPhaseNo);
extern uint16_t PhaseElapsedDurGet(uint8_t bPhaseNo);
extern uint16_t PhaseTotalElapsedDurGet(void);
extern uint8_t PhaseCurrentDurGet(uint8_t bPhaseNo);
extern uint8_t PhaseMinDurHasElapsed(uint8_t bPhaseNo);
extern uint8_t PhaseSGAdd(uint8_t bPhaseNo, uint8_t bSGNo);
extern uint8_t PhaseSGSub(uint8_t bPhaseNo, uint8_t bSGNo);

/*  Inputs */
extern uint8_t InputSet(uint8_t bType, uint8_t bInputNo, tpSInput pSInput);
extern uint8_t InputGet(uint8_t bType, uint8_t bInputNo, tpSInput pSInput);
extern uint8_t InputOwnerSGGet(uint8_t bType, uint8_t bInputNo);
extern uint8_t InputGreenDurPerDemandGet(uint8_t bType, uint8_t bInputNo);
extern uint8_t InputRedDurInBrokenGet(uint8_t bType, uint8_t bInputNo);
extern uint8_t InputPhaseInBrokenGet(uint8_t bType, uint8_t bInputNo);
extern uint8_t InputTotalGet(uint8_t bType);

/*  Input Runtime */
extern void SetIOInputs(uint8_t bModuleNo, tpSCanDigitalIOInputs pSCanIOInputs);
extern uint8_t IOInputErrGet(void);
extern void SetLDInputs(uint8_t bLDNo, uint8_t bIOMNo, uint8_t *pData);
extern uint8_t IOLoopErrGet(void);
extern void UseIOValues(void);
extern void IOPerValsInit(void);
extern uint8_t GetLastDetectorDemandIssued(void);
extern uint8_t GetLastInputDemandIssued(void);
extern void UseDigitalIOValues(uint8_t bModuleNo,
                               tpSCanDigitalIOInputs pSCanIOInputs);
extern void UseLDIOValues(uint8_t bModuleNo,
                          tpSCanDetectorIOInputs pSCanIOInputs);

/*  Outputs */
extern uint8_t OutputSet(uint8_t bOutputNo, tpSOutputDef pSOutputDef);
extern uint8_t OutputGet(uint8_t bOutputNo, tpSOutputDef pSOutputDef);
extern uint16_t OutputActiveLevelGet(uint8_t bOutputNo);
extern uint16_t OutputActiveLevelDurGet(uint8_t bOutputNo);
extern uint16_t OutputInActiveLevelDurGet(uint8_t bOutputNo);
extern uint8_t OutputTotalGet(void);

/*  Outputs Runtime */
extern void GetIOOutputs(tpSCanCpuIOOutputs pSCanCpuIOOutputs);
extern void SetIOOutputs(tpSCanCpuIOOutputs pSCanCpuIOOutputs);

/*  Work Plan */
extern void WorkPlanCurNoSet(uint8_t bWPNo);
extern uint8_t WorkPlanCurNoGet(void);
extern uint8_t WorkPlanEntryCurNoGet(void);
extern uint8_t WorkPlanEntrySet(uint8_t bWorkPlan, uint8_t bEntry,
                                tpSWorkPlanEntryDef pSWorkPlanEntryBuffer);
extern uint8_t WorkPlanEntryGet(uint8_t bWorkPlan, uint8_t bEntry,
                                tpSWorkPlanEntryDef pSWorkPlanEntryBuffer);
extern uint8_t WorkPlanEntryPhaseDurGet(uint8_t bWorkPlanEntry,
                                        uint8_t bPhaseNo);
extern uint8_t WorkPlanEntryPhaseDurSet(uint8_t bWorkPlanEntry,
                                        uint8_t bPhaseNo, uint8_t bDur);
extern uint8_t WorkPlanPhaseDurGet(uint8_t bPhaseNo);
extern uint16_t WorkPlanTotalPhaseDurGet(void);
extern uint8_t WorkPlanEntryTotalGet(uint8_t bWorkPlan);
extern uint8_t WorkPlanTotalGet(void);
extern uint8_t WorkPlanIsValid(uint8_t bWorkPlan);

/*  SP Plan */
extern void SigProgPlanCurNoSet(uint8_t bSPPlanNo);
extern uint8_t SigProgPlanCurNoGet(void);
extern void SigProgPlanDefaultActivate(void);
extern uint8_t SigProgPlanEntrySet(uint8_t bSPPlanNo, uint8_t bEntry,
                                   tpSSPPlanEntry pSSPPlanEntry);
extern uint8_t SigProgPlanEntryGet(uint8_t bSPPlanNo, uint8_t bEntry,
                                   tpSSPPlanEntry pSSPPlanEntry);
extern uint8_t SigProgPlanEntryTotalGet(uint8_t bSPPlanNo);
extern uint8_t SigProgPlanGet(uint8_t bSPPlanNo);
extern uint8_t SigProgPlanSigProgGet(void);
extern uint8_t SigProgPlanEntryCurrentGet(void);
extern uint8_t SigProgPlanTotalGet(void);
extern uint8_t SigProgPlanIsValid(uint8_t bSPPlanNo);

/*  Signal Plan */
extern uint8_t SignalPlanSet(uint8_t bPlanNo, tpSSignalPlan pSSignalPlan);
extern uint8_t SignalPlanTotalGet(void);
extern uint8_t SignalPlanGet(uint8_t bPlanNo, tpSSignalPlan pSSignalPlan);
extern uint8_t SignalPlanCurrentSet(uint8_t bPlanNo);
extern uint8_t SignalPlanIsValidGet(uint8_t bPlanNo);
extern uint8_t SignalPlanCurrentGet(void);

/*  Work Schedule */
extern uint8_t WorkScheduleEntrySet(uint8_t bWorkScheduleEntry,
                                    tpSWorkScheduleEntryDef pSWorkScheduleEntry);
extern uint8_t WorkScheduleEntryGet(uint8_t bWorkScheduleEntry,
                                    tpSWorkScheduleEntryDef pSWorkScheduleEntry);
extern void WorkScheduleDefaultSettings(void);
extern uint8_t WorkScheduleTotalGet(void);

/*  Signal Program */
extern uint8_t SigProgSet(uint8_t bSPNo, tpSSigProg pSSigProg);
extern void SigProgGet(uint8_t bSPNo, tpSSigProg pSSigProg);
extern void SigProgCurClr(void);
extern uint8_t SigProgTotalGet(void);
extern uint8_t SigProgIsValid(uint8_t bSPNo);
extern void SigProgCurNoSet(uint8_t bState);
extern uint8_t SigProgCurNoGet(void);
extern uint8_t SigProgCurTimeInPerGet(void);
extern void SigProgCurTimeInPerInc(void);
extern void SigProgCurTimeInPerClr(void);

/*  Transitions */
extern uint8_t TransitionSet(uint8_t bSPNo, uint8_t bTraNo,
                             tpSTransition pSTransitionDef);
extern void TransitionGet(uint8_t bSPNo, uint8_t bTraNo,
                          tpSTransition pSTransitionDef);
extern uint8_t TransitionFromGet(uint8_t bTraNo);
extern uint8_t TransitionToGet(uint8_t bTraNo);
extern uint8_t TransitionFromNoGet(uint8_t bTraNo);
extern uint8_t TransitionSimCloseGet(uint8_t bTraNo);
extern uint8_t TransitionToNoGet(uint8_t bTraNo);
extern uint8_t TransitionSimOpenGet(uint8_t bTraNo);
extern uint8_t TransitionRuleGet(uint8_t bTraNo);
extern uint8_t TransitionPriorityGet(uint8_t bTraNo);
extern uint8_t TransitionTotalGet(uint8_t bSPNo);
extern uint8_t TransitionAllocate(void);
extern uint8_t TransitionIsValid(uint8_t bSPNo, uint8_t bTraNo,
                                 tpSTransition pSTransitionDef);
extern uint8_t TransitionIsCircle(uint8_t bTraNo);

/*  Transition Lock */
extern void TransitionLockSet(uint8_t bTraNo);
extern uint8_t TransitionLockGet(uint8_t bTraNo);
extern uint8_t TransitionLockIsActive(void);
extern void TransitionLockEnd(void);

/*  Statements */
extern uint8_t StatementSet(uint8_t bSPNo, uint8_t bAddress,
                            tpSStatement pSStatement);
extern void StatementGet(uint8_t bSPNo, uint8_t bAddress,
                         tpSStatement pSStatement);
extern uint8_t StatementTotalGet(uint8_t bSPNo);
extern uint8_t StatementCommandGet(uint8_t bAddress);
extern uint8_t StatementExecute(uint8_t bAddress);
extern uint8_t StatementExecuteRange(uint8_t bStartAddr, uint8_t bEndAddr);

/*  Operations */
extern uint8_t OperationSet(uint8_t bSPNo, uint8_t bAddress,
                            tpSOperation pSOperation);
extern void OperationGet(uint8_t bSPNo, uint8_t bAddress,
                         tpSOperation pSOperation);
extern uint8_t OperationAllocate(void);
extern uint8_t OperationTotalGet(uint8_t bSPNo);

/*  Rules */
extern uint8_t RuleSet(uint8_t bSPNo, uint8_t bAddress, tpSRule pSRule);
extern uint8_t RuleTotalGet(uint8_t bSPNo);
extern void RuleGet(uint8_t bSPNo, uint8_t bAddress, tpSRule pSRule);
extern uint8_t RuleTOpsStartGet(uint8_t bAddress);
extern uint8_t RuleTOpsEndGet(uint8_t bAddress);
extern uint8_t RuleFOpsStartGet(uint8_t bAddress);
extern uint8_t RuleFOpsEndGet(uint8_t bAddress);
extern uint8_t RuleAllocate(void);
extern long RuleState(uint8_t bAddress);
extern long OperationState(tpSOperation pSOperation);
extern long OperandState(tpSOperand pSOperand);

/*  Data Operations */
extern void DataRuntimeInit(void);
extern void DataChecksumCalculate(tpSChecksum pSChecksum);
extern uint16_t DataChecksumTotalCalculate(void);
extern void DataChecksumTotalSet(uint16_t sSum);
extern uint16_t DataChecksumTotalGet(void);
extern void DataChecksumGet(tpSChecksum pSChecksum);

/*  MP Events */
extern void EventMPCont(tpSEvent pSEvent);

/*  Peripheral States */
extern void GetPeripheralStates(tpSPeripheralStates pSPeripheralStates);
extern uint8_t WriteSOPower(uint8_t bSOIndex, tpSSOPowerRecord ptSOPower);
extern uint8_t ReadSOPower(uint8_t bSOIndex, tpSSOPowerRecord ptSOPower);
extern void InitSOPowers(void);
extern uint8_t ReadSOPowers(void);
extern void ClearSOPowers(void);
extern void SetPowerRelay(uint8_t fState);
extern uint8_t GetPowerRelay(void);
extern void SetProgramLoading(uint8_t fState);
extern uint8_t GetProgramLoading(void);
extern void RestartProgram(void);
extern uint8_t GetProgramRestart(void);
extern void SetProgramRestart(uint8_t fNewState);
extern void SetHeaterState(uint8_t fState);
extern uint8_t GetHeaterState(void);
extern void SetLampDimmingState(uint8_t fState);
extern uint8_t GetLampDimmingState(void);
extern void SetExternalBatteryState(uint8_t fState);
extern uint8_t GetExternalBatteryState(void);

/*  User State */
extern void UserStateReqInit(void);
extern uint8_t UserStateReqRead(void);
extern uint8_t UserStateReqWrite(void);
extern uint8_t UserStateReqSet(uint8_t bNewState);
extern void UserStateReqEnd(void);
extern uint8_t UserStateReqFree(void);
extern uint8_t UserStateRunning(void);
extern uint8_t UserStateCurrentGet(void);
extern uint8_t UserStateReqGet(void);
extern uint8_t UserStateIsValid(void);

/*  Green Wave Synchronization */
extern void GpsSynchro(void);
extern uint8_t SeqExtValidationGet(void);
extern void SeqExtValidationSet(uint8_t fState);
extern uint8_t SeqTotalExtDurGet(void);
extern void SeqTotalExtDurSet(uint8_t bSeqExtDur);
extern uint8_t SeqStepExtDurGet(uint8_t bSeqStepNo);
extern void SeqStepExtDurSet(uint8_t bSeqStepNo, uint8_t bSeqStepExtDur);

/*  Counters */
extern uint8_t CounterIsValid(uint8_t bAddress);
extern void CounterSet(uint8_t bAddress, tpSCounter pSCounter);
extern void CounterGet(uint8_t bAddress, tpSCounter pSCounter);
extern void CounterValueSet(uint8_t bAddress, uint32_t lValue);
extern uint32_t CounterValueGet(uint8_t bAddress);
extern void CounterValueAdd(uint8_t bAddress, uint32_t lValue);
extern void CounterPeriodSet(uint8_t bAddress, uint16_t sPeriod);
extern uint16_t CounterPeriodGet(uint8_t bAddress);
extern void CounterAllocatedSet(uint8_t bAddress, uint8_t fValue);
extern uint8_t CounterAllocatedGet(uint8_t bAddress);
extern void CounterAllocate(uint8_t bAddress);
extern void CounterRunningSet(uint8_t bAddress, uint8_t fValue);
extern uint8_t CounterRunningGet(uint8_t bAddress);
extern void CounterOverflowSet(uint8_t bAddress, uint8_t fValue);
extern uint8_t CounterOverflowGet(uint8_t bAddress);

/*  Voltage State */
extern void SetVoltageState(uint8_t bNewState);
extern uint8_t GetVoltageState(void);

/*  Frequency State */
extern void SetFrequencyState(uint8_t bNewState);
extern uint8_t GetFrequencyState(void);

/*  SSM Test */
extern void StartSSMTest(uint8_t bSource);
extern uint8_t GetSSMTestSource(void);
extern void StopSSMTest(void);
extern uint8_t TurnOnNextSONo(void);
extern uint8_t TurnOnPreviousSONo(void);
extern uint8_t GetOnSONo(void);
extern void SetOnSONo(uint8_t bSONo);

/*  Power Supply */
extern uint16_t GetPowerSupplyNet(uint8_t bPSMNo);
extern uint16_t GetPowerSupplyFreq(uint8_t bPSMNo);
extern uint16_t GetPowerSupply24V1(uint8_t bPSMNo);
extern uint16_t GetPowerSupply5V1(uint8_t bPSMNo);
extern uint16_t GetPowerSupply24V2(uint8_t bPSMNo);
extern uint16_t GetPowerSupply5V2(uint8_t bPSMNo);
extern uint8_t Set24V1(uint8_t bPSMNo, uint16_t sNewVoltage);
extern uint8_t Set5V1(uint8_t bPSMNo, uint16_t sNewVoltage);
extern uint8_t Set24V2(uint8_t bPSMNo, uint16_t sNewVoltage);
extern uint8_t Set5V2(uint8_t bPSMNo, uint16_t sNewVoltage);
extern uint8_t SetNetFrequency(uint8_t bPSMNo, uint16_t sNewFrequency);
extern uint8_t SetNetVoltages(uint8_t bPSMNo, uint16_t sNewVoltage);
extern uint8_t SetIsolatedVoltageState(uint8_t bPSMNo, uint8_t fNewState);
extern uint8_t GetPowerSupplyIsolatedVoltage(uint8_t bPSMNo);

/*  Currents */
extern void GetSGCurrentMeasurement(uint8_t bSGNo,
                                    tpSCurrentMeasurement pSCurrentMeasurement);
extern uint16_t GetCurrentMeasurement(uint8_t bCurrentGroupNo, uint8_t bOption);
extern void SetCurrentMeasurement(uint8_t bCurrentGroupNo, uint16_t sNewValue);

/* Function Configuration */
extern uint8_t ReadFunctionConf(void);
extern uint8_t WriteFunctionConf(void);
extern uint8_t GetFunctionConf(void);
extern void SetFunctionConf(uint8_t bNewVal);
extern uint8_t GetFunctionConfByIndex(uint8_t bConfIdx);
extern void SetFunctionConfByIndex(uint8_t bConfIdx, uint8_t bNewVal);

/*  Log Settings */
extern uint8_t GetLogSettingsByEventID(uint8_t bEventID);
extern uint8_t LogSettingsSave(void);
extern uint8_t LogSettingsRead(void);
extern void LogSettingsInit(void);
extern void LogSettingsSet(tpSLogSettings pSLogSettings);
extern void LogSettingsGet(tpSLogSettings pSLogSettings);
extern uint8_t IsLogSettingsChanged(void);

/* System Start Time */
extern void SystemStartTimeInit(void);
extern void SystemStartTimeSet(tpSSystemStartTime pSSystemStartTime);
extern uint8_t SystemStartTimeSave(void);
extern uint8_t SystemStartTimeRead(void);
extern void SystemStartTimeGet(tpSSystemStartTime pSSystemStartTime);
extern void SystemStartTimeStart(void);
extern uint8_t IsSystemStartTimeWritten(void);
extern void SystemStartTimeSetMinUpHours(uint8_t bMinUpHours);
extern uint8_t SystemStartTimeGetMinUpHours(void);
extern void SystemStartTimeSetUpHours(uint8_t bUpHours);
extern void SystemStartTimeIncUpHours(void);
extern uint8_t SystemStartTimeGetUpHours(void);

/* LRLF Detect Time Settings */
extern uint8_t LRLFDetectTimeWrite(void);
extern uint8_t LRLFDetectTimeRead(void);
extern void LRLFDetectTimeSet(uint8_t bTime);
extern uint8_t LRLFDetectTimeGet(void);
extern void LRLFDetectTimeCheck(void);

/* Secure Transition */
extern void ApplySecureTransition(void);
extern void SecureSystemReset(void);

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/* Error Info */
extern void InitErrorInfo(void);
extern tpSErrInfo GetErrorInfoPtr(void);

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/* Traffic Counts Timer */
extern void IncTrafficCountsTimer(void);
extern uint16_t GetTrafficCountsTimer(void);
extern void SetTrafficCountsTimer(uint16_t sTimer);
extern void GetMCSTrafficCountsDigital(void *pData);
extern void GetMCSTrafficCountsDetector(void *pData);

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/* Device UID */
extern void InitDeviceUIDs(void);
extern void ReadCPUDeviceUID(void);
extern tpSDeviceUID GetCPUDeviceUID(void);
extern uint8_t WriteEEPROMDeviceUID(void);
extern uint8_t ReadEEPROMDeviceUID(void);
extern uint8_t SetDeviceUID(void);
extern uint8_t ClearDeviceUID(void);
extern uint8_t CheckDeviceUIDs(void);

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/* Maintenance */
extern void SignalMaintenanceTask(uint32_t ulSignal);

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/* CP - UCM */
extern tpSCPRuntime GetSCPRuntimePtr(void);
extern tpSRuntimes GetSRuntimePtr(void);
extern tpSCP GetSCPPtr(void);
extern tpSErrInfo GetSErrInfoPtr(void);
extern tpSCanDetectorIOInputs GetSaCanDetectorIOInputsPtr(void);
extern tpSCanDigitalIOInputs GetSaCanDigitalIOInputsPtr(void);
extern tpSCurrentMeasurement GetSaCurrentsPtr(void);
extern tpSPowerSupply GetSaPSMsPtr(void);
extern tpSSeqExtension GetSaSeqExtDurPtr(void);
extern tpSGlobalConfiguration GetSGlobalConfigurationPtr(void);
extern tpSGlobalDbManagement GetSGlobalDbManagementPtr(void);
extern tpSGlobalTimeManagement GetSGlobalTimeManagementPtr(void);
extern tpSTRPatternsAndCoords GetSTRPatternsAndCoordsPtr(void);

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/* User Operations */
extern tpSUserOperations GetSUserOperationsPtr(void);
extern void UserOperationsInit(void);
extern void UserOperationsAdd(uint8_t bType);

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/* Signal State Runtime */
extern void SignalStateRuntimeInit(void);
extern void SignalStateRuntimeCurNoSet(uint8_t bState);
extern uint8_t SignalStateRuntimeCurNoGet(void);

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/* Channel Errors */
void ChannelErrorsInit(void);

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  TR Pattern */
extern void TRPatternsAndCoordsInit(void);
extern uint8_t TRPatternsAndCoordsGetCurJunctionNo(void);

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Global Configuration */
extern void GCInit(void);
extern void GCSetASCModuleID(uint16_t sSum);

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Global Time Management */
extern void GTMInit(void);
extern uint8_t GTMGetGlobalTime(uint32_t *ulEpoch);
extern uint8_t GTMSetGlobalTime(uint32_t ulEpoch);
extern uint8_t GTMGetControllerLocalTime(uint32_t *ulEpoch);

/* ASC Channel */
extern tpSASCChannel GetASCChannelPtr(void);
extern void ASCChannelInit(void);

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  ASC Dector */
extern tpSASCDetector GetASCDetector(void);
extern void ASCDetectorInit(void);

/* ASC Cabinet Environment */
extern tpSASCCabinetEnvironment GetASCCabinetEnvironmentPtr(void);
extern void ASCCabinetEnvironmentInit(void);

/* ASC Unit */
extern tpSAscUnit GetUnitPtr(void);
extern tpSAscClock GetUnitAscClockPtr(void);
extern void UnitInit(void);

/* ASC Coord */
extern tpSAscCoord GetCoordPtr(void);
extern void CoordInit(void);
extern void CoordSplitTimeSet(void);

/* Timebase ASC */
extern tpSTimebaseAsc GetTimebaseAscPtr(void);
extern void TimebaseAscInit(void);

/* ASC Ring */
extern tpSAscRing GetRingPtr(void);
extern void RingInit(void);

/* Overlap */
extern tpSOverlap GetOverlapPtr(void);
extern void OverlapInit(void);

/* ASC Block */
extern tpSAscBlock GetAscBlockPtr(void);
extern void AscBlockInit(void);

/* Preempt */
extern tpSPreempt GetPreemptPtr(void);
extern void PreemptInit(void);

/* ASC Phase */
extern tpSAscPhase GetAscPhasePtr(void);
extern void AscPhaseInit(void);

#endif /* ifndef __DATA_H__ */
