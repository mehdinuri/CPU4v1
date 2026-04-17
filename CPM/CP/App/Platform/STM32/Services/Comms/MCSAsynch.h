#ifndef __MCSASYNCH_H__
#define __MCSASYNCH_H__

#include "MLM.h"
#include "Ports/ISerialPort.h"
#include "Ports/IModemPort.h"
#include "program.h"
#include "time.h"

#define MCS_ASYNCH_DATA_PACKET_MAX_LEN 255

#define MCS_ASYNCH_NO_CARRIER_LEN 10
#define MCS_ASYNCH_NO_CARRIER_START 0x4E /* N */

#define MCS_ASYNCH_CLOSED_LEN 6
#define MCS_ASYNCH_CLOSED_START 0x43 /* C */

#define MCS_ASYNCH_PDP_DEACT_LEN 10
#define MCS_ASYNCH_PDP_DEACT_START 0x2B /* + */

#define MCS_ASYNCH_DISCONNECT_LEN 10
#define MCS_ASYNCH_DISCONNECT_START 0x44 /* D */

#define MCS_ASYNCH_IR_MSG_MAX_LEN 58 /* 1 + 2 + 3*3 + 7*5 + 11 */

/* timeouts */
#define MCS_ASYNCH_TIMEOUT_CONNECTION 30000 /* 30 seconds */
#define MCS_ASYNCH_TIMEOUT_MSG_BOX 90000 /* 1.5 minutes */
#define MCS_ASYNCH_TIMEOUT_COUNTER_IN_SECONDS 90 /* 1.5 minutes */

/* periods */
#define MCS_ASYNCH_PERIOD_TRAFFIC_COUNTS 0x5A /* 90 seconds */
#define MCS_ASYNCH_PERIOD_TRAFFIC_COUNTS_DEFAULT 0x0F /* 15 minutes */
#define MCS_ASYNCH_PERIOD_PARTIAL_RUNTIME 0x05 /* 5 seconds */
#define MCS_ASYNCH_PERIOD_ERROR_INFO 60 /* 1 minute */

/*  Message Types */
#define MCS_ASYNCH_HEADER_NONE 0
#define MCS_ASYNCH_HEADER_FIRST 1
#define MCS_ASYNCH_HEADER_IMEI 1
#define MCS_ASYNCH_HEADER_LOG 2
#define MCS_ASYNCH_HEADER_SIGNALS 3

/* #define  MCS_ASYNCH_HEADER_INPUTS */
/* 4 */
typedef struct _tSMCSAsynchInputs
{
  uint8_t baLoopDemandCnt[INPUTS_DETECTOR_MAX]; /* loop demand count */
  uint16_t saLoopOccDur[INPUTS_DETECTOR_MAX]; /* loop occupation duration */
  uint32_t lLoopSafe; /* 1: safe, 0: broken */
  uint8_t baDigDemandCnt[INPUTS_DIGITAL_MAX]; /* digital input demand count */
  uint16_t saDigOccDur[INPUTS_DIGITAL_MAX]; /* digital input occupation duration */
} __attribute__((packed)) tSMCSAsynchInputs, *tpSMCSAsynchInputs;

/* #define  MCS_ASYNCH_HEADER_STATE */
/* 5 */

typedef struct _tSMCSAsynchState
{
  uint8_t bState;

  union
  {
    struct
    {
      uint8_t bCurrentSeq;
      uint8_t bTotalSeq;
      uint8_t bCurrentSeqStep;
      uint8_t bTotalSeqStep;
      uint8_t bCurrentSeqStepDur;
      uint8_t bTotalSeqStepDur;
      uint8_t bCurrentSeqDur;
      uint8_t bTotalSeqDur;
    } __attribute__((packed)) SSequence;

    struct
    {
      uint8_t bCurrentPhase;
      uint8_t bTotalPhase;
      uint8_t bCurrentPhaseDur;
      uint8_t bTotalPhaseDur;
    } __attribute__((packed)) SPhase;

    struct
    {
      uint8_t bPhaseFrom;
      uint8_t bPhaseTo;
    } __attribute__((packed)) STransition;
  } __attribute__((packed)) UStateData;
} __attribute__((packed)) tSMCSAsynchState, *tpSMCSAsynchState;

/* #define  MCS_ASYNCH_HEADER_WORKMODE */
/* 6 */

typedef struct _tSMCSAsynchWorkMode
{
  uint8_t bWorkMode;
  uint8_t bCurrentSignalPlan;
  uint8_t bTotalSignalPlan;

  uint8_t bRelay : 1;
  uint8_t bGate : 1;
  uint8_t bGps : 1;
  uint8_t bReserved : 5;
} __attribute__((packed)) tSMCSAsynchWorkMode, *tpSMCSAsynchWorkMode;

/*
 *       name: workmode indicators used in response packet
 */
#define MCS_WM_FLASH bit0 /* flash */
#define MCS_WM_EMERGENCY_FLASH bit1 /* emergency flash */
#define MCS_WM_USER_REQ bit2 /* user requests via interfaces (LCD etc.) */
#define MCS_WM_WS bit3 /* work schedule */
#define MCS_WM_CENTRAL_DIRECTED bit4 /* center directs running state */
#define MCS_WM_DARK bit5 /* dark */
#define MCS_WM_EMERGENCY_DARK bit6 /* emergency dark */
#define MCS_WM_ALL_SETS_IN_ERROR                                               \
        bit7 /* all sets in intersection has mode of emergency, in other words, they */
             /* are in error */

#define MCS_USER_REQUEST_NONE 0
#define MCS_USER_REQUEST_ALL_RED 1 /* request to assign red to all signal groups */
#define MCS_USER_REQUEST_DARK 2 /* request to assign dark to all signal groups */
#define MCS_USER_REQUEST_FLASH                                                 \
        3 /* request to assign flash signals to all signal groups */
#define MCS_USER_REQUEST_PLAN_RETURN                                           \
        4 /* request to return from user determined states to work schedule */
          /* examination */

#define MCS_ASYNCH_HEADER_DOWNLOAD 7
#define MCS_ASYNCH_HEADER_UPLOAD 8

/* #define  MCS_ASYNCH_HEADER_MEASUREMENTS 9 */

typedef struct _tSMCSAsynchMeasurements
{
  uint16_t sVoltage1;
  uint16_t sVoltage2;
  uint8_t bFrequency1;
  uint8_t bFrequency2;
} __attribute__((packed)) tSMCSAsynchMeasurements, *tpSMCSAsynchMeasurements;

#define MCS_ASYNCH_HEADER_USER_REQUEST 10
#define MCS_ASYNCH_HEADER_RESET 11

/* #define  MCS_ASYNCH_HEADER_POSITION */
/* 12 */
#define MCS_ASYNCH_HEADER_SP_CHANGE 13

/*
 *       expl: active signal plan is decided by a remote location, work schedule
 *  is disabled in this case. Remote location sends the same command
 *  periodically. If this command doesn't exist in the next period, work schedule
 *  is enabled.
 *                               - there are four actions: blocking to signal
 *  plan, running work schedule, requesting unit data, cancelling last command
 */
typedef struct _SMCSAsynchSPChange
{
  uint8_t bSPNo; /* signal plan number */
  uint16_t sStartTime; /* time of day in minutes */
  uint16_t sEndTime; /* time of day in minutes, if 0, it means that always run */
                     /* this signal plan */
  uint8_t fWSRunning : 1;
  uint8_t fReserved : 7;
  uint8_t bChecksum;
} __attribute__((packed)) tSMCSAsynchSPChange, *tpSMCSAsynchSPChange; /* 7 bytes */

/* #define  MCS_ASYNCH_HEADER_CONFIG */
/* 14 #define MCS_ASYNCH_HEADER_ERROR */
/* 15 */

typedef struct _tSMCSAsynchError
{
  uint8_t bErr;
  uint8_t bParam1;
  uint8_t bParam2;
} __attribute__((packed)) tSMCSAsynchError, *tpSMCSAsynchError;

#define MCS_ASYNCH_HEADER_TIME 16

typedef struct _tSMCSAsynchDateTime
{
  uint16_t sMinOfDay;
  uint8_t bDay;
  uint8_t bMonth;
  uint8_t bYear;
} __attribute__((packed)) tSMCSAsynchDateTime, *tpSMCSAsynchDateTime;

/* #define MCS_ASYNCH_HEADER_TRAFFIC_DATA 17 */
#define MCS_ASYNCH_HEADER_IO_AND_LD 18

typedef struct _tSMCSAsynchIOandLDs
{
  uint8_t bNumberOfLoopDedector;
  uint32_t lLoopDedectorDemands; /* LD demands 1: demand, 0: no demand */
  uint8_t bNumberOfDigitalInputs;
  uint32_t lDigitalInputDemands; /* IO demands 1: demand, 0: no demand */
} __attribute__((packed)) tSMCSAsynchIOsAndLDs, *tpSMCSAsynchIOandLDs;

#define MCS_ASYNCH_HEADER_START_WEB_ENGINE 19
#define MCS_ASYNCH_HEADER_TRAFFIC_COUNTS_DIGITAL 20

#define MCS_ASYNCH_HEADER_POWER_RECORDS 21
#define MCS_ASYNCH_HEADER_RUNTIME 22

#define MCS_ASYNCH_OPERATION_MODE_LOCAL 0
#define MCS_ASYNCH_OPERATION_MODE_MANUAL 1
#define MCS_ASYNCH_OPERATION_MODE_CENTRAL 2

#define MCS_ASYNCH_SIGNAL_MODE_DOWNLOAD 9

#define MCS_ASYNCH_MIN_DEVICE_RUNTIME_SIZE 11

typedef struct _tSMCSDeviceRuntime
{
  /*  Operation Modes */
  /*  0 : Local */
  /*  1 : Manual */
  /*  2 : Central */
  uint8_t bOperationMode;

  /*  Signal Modes */
  /*  0 : Offline */
  /*  1 : Any */
  /*  2 : No Control */
  /*  3 : Flash */
  /*  4 : Closed */
  /*  5 : Phase */
  /*  6 : Phase Transition */
  /*  7 : Sequence */
  /*  8 : Emergency Flash */
  /*  9 : Download */
  /*  10  : Emergency Dark */
  uint8_t bSignalMode;

  /*  Operation Modes */
  uint8_t bCurrentSignalPlan;

  uint8_t bRelay : 1;
  uint8_t bGate : 1;
  uint8_t bGps : 1;
  uint8_t bLampDim : 1;
  uint8_t bHeater : 1;
  uint8_t bReserved : 3;

  struct
  {
    uint16_t sVoltage1;
    uint16_t sVoltage2;
  } __attribute__((packed)) SVoltages;

  struct
  {
    uint8_t bFrequency1;
    uint8_t bFrequency2;
  } __attribute__((packed)) SFrequencies;

  union
  {
    uint8_t baData[8];

    struct
    {
      uint8_t bCurrentSeq;
      uint8_t bTotalSeq;
      uint8_t bCurrentSeqStep;
      uint8_t bTotalSeqStep;
      uint8_t bCurrentSeqStepDur;
      uint8_t bTotalSeqStepDur;
      uint8_t bCurrentSeqDur;
      uint8_t bTotalSeqDur;
    } __attribute__((packed)) SSequence;

    struct
    {
      uint8_t bCurrentPhase;
      uint8_t bTotalPhase;
      uint8_t bCurrentPhaseDur;
      uint8_t bTotalPhaseDur;
    } __attribute__((packed)) SPhase;

    struct
    {
      uint8_t bPhaseFrom;
      uint8_t bPhaseTo;
    } __attribute__((packed)) STransition;

    struct
    {
      uint8_t bError;
      uint8_t bParam1;
      uint8_t bParam2;
    } __attribute__((packed)) SError;
  } __attribute__((packed)) UStateData;

  uint8_t bSignalQuality;
} __attribute__((packed)) tSMCSDeviceRuntime, *tpSMCSDeviceRuntime;

typedef struct _tSMCSPartialRuntime
{
  /*  Signal Modes */
  uint8_t bSignalMode;
  /*  Operation Modes */
  uint8_t bCurrentSignalPlan;

  struct
  {
    uint8_t bError;
    uint8_t bParam1;
    uint8_t bParam2;
  } __attribute__((packed)) SError;
} __attribute__((packed)) tSMCSPartialRuntime, *tpSMCSPartialRuntime;

/* H&D Commented */

/*
 #define  MCS_ASYNCH_HEADER_LAMP_DIMM_SET         23
 #define  MCS_ASYNCH_HEADER_HEATER_SET 24
 *
 *  typedef struct _tSMCSHeaterLampDim
 *  {
 *  uint8_t     fLogicLevel;
 *  uint8_t     fState;
 *
 *  } __attribute__((packed)) tSMCSHeaterLampDim, *tpSMCSHeaterLampDim;
 *
 #define  MCS_ASYNCH_HEADER_LAMP_DIMM_GET         25
 #define  MCS_ASYNCH_HEADER_HEATER_GET 26
 */

#define MCS_ASYNCH_HEADER_VERSION_GET 27
#define MCS_ASYNCH_HEADER_VERSION 28

typedef struct _tSMCSVersion
{
  uint8_t bArg0;
  uint8_t bArg1;
  uint8_t bArg2;
  uint8_t bArg3;
  char bArg4;
} __attribute__((packed)) tSMCSVersion, *tpSMCSVersion;

#define MCS_ASYNCH_HEADER_LCD_STREAM_START 29
#define MCS_ASYNCH_HEADER_LCD_STREAM 30

typedef struct _tSMCSLCDStream
{
  char strLCDLines[4][20];
} __attribute__((packed)) tSMCSLCDStream, *tpSMCSLCDStream;

#define MCS_ASYNCH_SO_MEAS_START_OR_STOP 31

typedef struct _tSMCSSOMeas
{
  uint8_t bSSMNo;
  uint8_t fState;
} __attribute__((packed)) tSMCSSOMeas, *tpSMCSSOMeas;

#define MCS_ASYNCH_HEADER_PARTIAL_RUNTIME 32
#define MCS_ASYNCH_HEADER_USER_SETTINGS_SET 33

typedef struct _tSMCSUserSettings
{
  uint8_t fSettingsChanged;
  uint8_t fConfigFlag;
  uint8_t fLogFlag;
  uint8_t fTrafficCountsFlag;
  uint8_t bTrafficCountsPeriod;
  uint8_t fStandbyInfoFlag;
} __attribute__((packed)) tSMCSUserSettings, *tpSMCSUserSettings;

#define MCS_ASYNCH_HEADER_USER_SETTINGS 34

#define MCS_ASYNCH_HEADER_RESUME_CONNECTION 35
#define MCS_ASYNCH_HEADER_START_IAP 36

#define MCS_ASYNCH_HEADER_MAC 37
#define MCS_ASYNCH_HEADER_TRAFFIC_COUNTS_LOOP 38
#define MCS_ASYNCH_HEADER_STOP_MODE 39

typedef struct _tSMCSStopMode
{
  uint8_t fStopMode;
} __attribute__((packed)) tSMCSStopMode, *tpSMCSStopMode;

#define MCS_ASYNCH_HEADER_CONNECTION_CHECK 40
#define MCS_ASYNCH_HEADER_LOG_SETTINGS_SET 41
#define MCS_ASYNCH_HEADER_LOG_SETTINGS 42

typedef struct _tSMCSLogSettings
{
  uint8_t fSettingsChanged;
  uint8_t baLogSettings[17]; /* 17 * 8 = 136 logs, 122 used */
} __attribute__((packed)) tSMCSLogSettings, *tpSMCSLogSettings;

#define MCS_ASYNCH_HEADER_IR_STREAM_START 43
#define MCS_ASYNCH_HEADER_IR_STREAM 44
#define MCS_ASYNCH_HEADER_IR_STREAM_SET 45

typedef struct _tSMCSVirtualInput
{
  uint8_t bType;
  uint8_t bNumber;
  uint8_t bState;
} __attribute__((packed)) tSMCSVirtualInput, *tpSMCSVirtualInput;

#define MCS_ASYNCH_HEADER_ERROR_INFO 46
#define MCS_ASYNCH_HEADER_DST_SET 47
#define MCS_ASYNCH_HEADER_DST 48

#define MCS_ASYNCH_HEADER_BROKEN_INPUT_SETTINGS_SET 49

typedef struct _tSMCSBrokenInputSettings
{
  uint8_t fAlreadySet;
  uint8_t fLoopInputFlag;
  uint8_t fDigitalInputFlag;
} __attribute__((packed)) tSMCSBrokenInputSettings,
*tpSMCSBrokenInputSettings;

#define MCS_ASYNCH_HEADER_BROKEN_INPUT_SETTINGS 50

#define MCS_ASYNCH_HEADER_SERVER_SETTINGS_SET 51

typedef struct _tSMCSServerSettings
{
  uint8_t fMCSAvailable;
  uint8_t fNTCIPAvailable;
} __attribute__((packed)) tSMCSServerSettings, *tpSMCSServerSettings;

#define MCS_ASYNCH_HEADER_SERVER_SETTINGS 52

#define MCS_ASYNCH_HEADER_LAST MCS_ASYNCH_HEADER_SERVER_SETTINGS

typedef union _tUMCSMsgData
{
  uint8_t baData[MCS_ASYNCH_DATA_PACKET_MAX_LEN + 1];
  uint8_t bAckNak;
  char strDownloadUpload[MCS_ASYNCH_DATA_PACKET_MAX_LEN + 1];
  uint8_t bUserRequest;
  tSMCSAsynchSPChange SSPChange; /* 7 bytes */
  tSMCSAsynchDateTime STime; /* 5 bytes */
  tSMCSSOMeas SMCSSOMeas; /* 2 bytes */
  tSMCSUserSettings SMCSUserSettings; /* 6 bytes */
  uint8_t bDSTFlag;
  tSMCSBrokenInputSettings SBrokenInputSettings; /* 3 bytes */
  tSMCSServerSettings SServerSettings;
} __attribute__((packed)) tUMCSMsgData, *tpUMCSMsgData; /* 1342 bytes */

/* ///////////////////////////////////////////////////////// */
/*                    Message */
/* Structure */
typedef struct _tSMCSAsynchMsg
{
  uint8_t bStart;
  uint8_t bHeader;
  uint8_t bLen;
  uint32_t lEpoch;
  tUMCSMsgData UData;
  uint8_t bEnd;
} __attribute__((packed)) tSMCSAsynchMsg, *tpSMCSAsynchMsg; /* 1329 bytes */

#define MCS_ASYNCH_MSG_START 0x02
#define MCS_ASYNCH_MSG_END 0x03
#define MCS_ASYNCH_MSG_ACK 0x06
#define MCS_ASYNCH_MSG_NAK 0x15

#define MCS_ASYNCH_MSG_PROTOCOL_HEAD 3
#define MCS_ASYNCH_MSG_PROTOCOL_TAIL 1
#define MCS_ASYNCH_MSG_PROTOCOL_OVERHEAD                                       \
        MCS_ASYNCH_MSG_PROTOCOL_HEAD + MCS_ASYNCH_MSG_PROTOCOL_TAIL

#define MCS_ASYNCH_RX_TX_MAX_LEN                                               \
        MCS_ASYNCH_MSG_PROTOCOL_OVERHEAD + MCS_ASYNCH_DATA_PACKET_MAX_LEN

#define MCS_ASYNCH_MSG_PROTOCOL_START_INDEX 0
#define MCS_ASYNCH_MSG_PROTOCOL_HEADER_INDEX 1
#define MCS_ASYNCH_MSG_PROTOCOL_LENGTH_INDEX 2
#define MCS_ASYNCH_MSG_PROTOCOL_EPOCH_INDEX 3
#define MCS_ASYNCH_MSG_PROTOCOL_DATA_INDEX 7

/* ///////////////////////////////////////////////////////// */
/*                    Comm */
/* Control */
typedef enum
{
  MCS_ASYNCH_DISC_TYPE_NONE = 0,
  MCS_ASYNCH_DISC_TYPE_TIMEOUT_MSG_BOX,
  MCS_ASYNCH_DISC_TYPE_TIMEOUT_CON_CHECK,
  MCS_ASYNCH_DISC_TYPE_REMOTE_END_DISCON,
  MCS_ASYNCH_DISC_TYPE_CON_INFO_CHANGED,
  MCS_ASYNCH_DISC_TYPE_PWR_NORMAL_STANDBY,
  MCS_ASYNCH_DISC_TYPE_TCP_CLIENT_CLOSED,
  MCS_ASYNCH_DISC_TYPE_TCP_ERROR,
  MCS_ASYNCH_DISC_TYPE_PPP_ERROR,
  MCS_ASYNCH_DISC_TYPE_ETH_LINK_DOWN,
} tMCSAsynchDisconnectionTypes;

typedef enum
{
  MCS_ASYNCH_TXRX_STATE_NONE = 0,
  MCS_ASYNCH_TXRX_STATE_START,
  MCS_ASYNCH_TXRX_STATE_HEADER,
  MCS_ASYNCH_TXRX_STATE_LENGTH,
  MCS_ASYNCH_TXRX_STATE_EPOCH,
  MCS_ASYNCH_TXRX_STATE_DATA,
  MCS_ASYNCH_TXRX_STATE_END,
  MCS_ASYNCH_TXRX_STATE_COMPLETE
} tEMCSAsynchMsgTransferStates;

typedef struct _tSMCSAsynchTransfer
{
  uint8_t bBusy;
  tEMCSAsynchMsgTransferStates eState;
  uint16_t sCurrentByte;
  tSMCSAsynchMsg SMessage; /* 1329 bytes */
} tSMCSAsynchTransfer, *tpSMCSAsynchTransfer;

#define MCS_ASYNCH_STREAM_TIMEOUT 300
typedef struct _tSMCSASynchRuntime
{
  uint8_t bConnected : 1;
  uint8_t bCancelOp : 1;
  uint8_t bMsgRcvd : 1;
  uint8_t bWebEngine : 1;
  uint8_t bLCDStream : 1;
  uint8_t bSOMeasurements : 1;
  uint8_t bIRStream : 1;
  uint8_t bReserved : 1;

  uint16_t bSSMNo;

  uint16_t sWebEngineTimeout;
  uint16_t sLCDStreamTimeout;
  uint16_t sIRStreamTimeout;
  uint16_t sConnectionTimeout;
} tSMCSAsynchRuntime, *tpSMCSAsynchRuntime; /* 7 bytes */

typedef struct _tSMCSAsynchRxTxMsg
{
  uint8_t baData[MCS_ASYNCH_RX_TX_MAX_LEN + 1];
  uint16_t sDataLen;
} tSMCSAsynchRxTxMsg, *tpSMCSAsynchRxTxMsg;

/* ///////////////////////////////////////////////////////// */
/*                    Methods */
extern void MCSAsynchConnectedSet(uint8_t bState);
extern uint8_t MCSAsynchConnectedGet(void);
extern uint8_t MCSAsynchStart(uint8_t bGreetingType);
extern void MCSAsynchStop(uint8_t bDisConType);
extern void MCSAsynchInit(ISerialPort_t *port, IModemPort_t *driver);
extern uint16_t MCSAsynchGeLogReadIndex(void);
extern void MCSAsynchSetLogReadIndex(uint16_t sIndex);
extern void MCSAsynchReadLogReadIndex(void);
extern void MCSAsynchWriteLogReadIndex(void);
extern uint8_t MCSAsynchReqRxMsg(uint8_t *pbData, uint16_t sLength);
extern uint8_t MCSAsynchIsRemoteEndClosed(void);
extern uint8_t MCSAsynchReqTxMsg(uint8_t bHeader, uint8_t bLen, void *pbData);
extern uint8_t MCSAsynchSPNoGet(void);
extern void MCSAsynchLogSeize(void);
extern void MCSAsynchLogRelease(void);
extern uint8_t MCSAsynchStandbyMsgSet(void);
extern void MCSAsynchCheckConnectionTimeout(void);
extern uint32_t MCSAsynchCalculateEpoch(void);

#endif /* ifndef __MCSASYNCH_H__ */
