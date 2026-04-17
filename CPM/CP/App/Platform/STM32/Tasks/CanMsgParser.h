#ifndef __CANMSGPARSER_H__
#define __CANMSGPARSER_H__

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  include files */
#include "fdcan.h"

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  definitions */

/* //////////////////////////////////////////////////////////////////// */
/*  CP Tx Std Id Defs */
#define CAN_TX_CP_EXT_ID_SIGNAL_DEFS_0 0x1000
#define CAN_TX_CP_EXT_ID_SIGNAL_DEFS_1 0x1100
#define CAN_TX_CP_EXT_ID_SIGNALS_DEFINED 0x1200
#define CAN_TX_CP_EXT_ID_SG_DEFS_0 0x1300
#define CAN_TX_CP_EXT_ID_SG_DEFS_1 0x1400
#define CAN_TX_CP_EXT_ID_SG_DEFS_2 0x1500
#define CAN_TX_CP_EXT_ID_SG_DEFS_3 0x1600
#define CAN_TX_CP_EXT_ID_SG_DEFS_4 0x1700
#define CAN_TX_CP_EXT_ID_SG_DEFS_5 0x1800
#define CAN_TX_CP_EXT_ID_SO_DEFS_0 0x1900
#define CAN_TX_CP_EXT_ID_SO_DEFS_1 0x1A00
#define CAN_TX_CP_EXT_ID_CVS_DEFS_0 0x1B00
#define CAN_TX_CP_EXT_ID_CVS_DEFS_1 0x1C00
#define CAN_TX_CP_EXT_ID_CVS_DEFS_2 0x1D00
#define CAN_TX_CP_EXT_ID_CVS_DEFS_3 0x1E00
#define CAN_TX_CP_EXT_ID_CVS_DEFS_4 0x1F00
#define CAN_TX_CP_EXT_ID_CONFLICTS_EM 0x2000
#define CAN_TX_CP_EXT_ID_FLASH_CFG 0x2100
#define CAN_TX_CP_EXT_ID_DEFAULT 0x2200
#define CAN_TX_CP_EXT_ID_EVENT 0x2300
#define CAN_TX_CP_EXT_ID_PERIPHERAL_STATE 0x2400
#define CAN_TX_CP_EXT_ID_RESET 0x2500
#define CAN_TX_CP_EXT_ID_MP_VERSION 0x2600

/*  MP Tx Std Id Defs */
#define CAN_TX_MP_EXT_ID_SIGNAL_DEFS_0 0x3000
#define CAN_TX_MP_EXT_ID_SIGNAL_DEFS_1 0x3100
#define CAN_TX_MP_EXT_ID_SIGNALS_DEFINED 0x3200
#define CAN_TX_MP_EXT_ID_SG_DEFS_0 0x3300
#define CAN_TX_MP_EXT_ID_SG_DEFS_1 0x3400
#define CAN_TX_MP_EXT_ID_SG_DEFS_2 0x3500
#define CAN_TX_MP_EXT_ID_SG_DEFS_3 0x3600
#define CAN_TX_MP_EXT_ID_SG_DEFS_4 0x3700
#define CAN_TX_MP_EXT_ID_SG_DEFS_5 0x3800
#define CAN_TX_MP_EXT_ID_SO_DEFS_0 0x3900
#define CAN_TX_MP_EXT_ID_SO_DEFS_1 0x3A00
#define CAN_TX_MP_EXT_ID_CVS_DEFS_0 0x3B00
#define CAN_TX_MP_EXT_ID_CVS_DEFS_1 0x3C00
#define CAN_TX_MP_EXT_ID_CVS_DEFS_2 0x3D00
#define CAN_TX_MP_EXT_ID_CVS_DEFS_3 0x3E00
#define CAN_TX_MP_EXT_ID_CVS_DEFS_4 0x3F00
#define CAN_TX_MP_EXT_ID_CONFLICTS_EM 0x4000
#define CAN_TX_MP_EXT_ID_FLASH_CFG 0x4100
#define CAN_TX_MP_EXT_ID_DEFAULT 0x4200
#define CAN_TX_MP_EXT_ID_EVENT 0x4300
#define CAN_TX_MP_EXT_ID_PERIPHERAL_STATE 0x4400
#define CAN_TX_MP_EXT_ID_RESET 0x4500
#define CAN_TX_MP_EXT_ID_MP_VERSION 0x4600
#define CAN_TX_MP_EXT_ID_SG_CONFLICT 0x4700

/* Message ID(MID) definitions */
#define CAN_ID_TYPE_STD 0x01
#define CAN_ID_TYPE_EXT 0x02

/* sender: cpu, receiver: ssm and psm */
#define CAN_MID_VERSION 0x0F

/* sender: cpu, receiver: loop dedector */
#define CAN_MID_LOOP_DEDECTOR_ENTER_OPERATIONAL_MODE 0x000

#define CAN_MID_LOOP_DETECTOR_STATUS_REQUEST0 0x601
#define CAN_MID_LOOP_DETECTOR_STATUS_REQUEST1 0x602
#define CAN_MID_LOOP_DETECTOR_STATUS_REQUEST2 0x603
#define CAN_MID_LOOP_DETECTOR_STATUS_REQUEST3 0x604
#define CAN_MID_LOOP_DETECTOR_STATUS_REQUEST4 0x605
#define CAN_MID_LOOP_DETECTOR_STATUS_REQUEST5 0x606
#define CAN_MID_LOOP_DETECTOR_STATUS_REQUEST6 0x607
#define CAN_MID_LOOP_DETECTOR_STATUS_REQUEST7 0x608

/* sender: ssm, receiver: cpu */
#define CAN_MID_SSM_VOLT_CURRENT_MEASUREMENTS0 0x050
#define CAN_MID_SSM_VOLT_CURRENT_MEASUREMENTS1 0x051
#define CAN_MID_SSM_VOLT_CURRENT_MEASUREMENTS2 0x052
#define CAN_MID_SSM_VOLT_CURRENT_MEASUREMENTS3 0x053
#define CAN_MID_SSM_VOLT_CURRENT_MEASUREMENTS4 0x054
#define CAN_MID_SSM_VOLT_CURRENT_MEASUREMENTS5 0x055
#define CAN_MID_SSM_VOLT_CURRENT_MEASUREMENTS6 0x056
#define CAN_MID_SSM_VOLT_CURRENT_MEASUREMENTS7 0x057

/* sender: psm, receiver: cpu */
#define CAN_MID_PSM_VOLT_MEASUREMENTS0 0x05A
#define CAN_MID_PSM_VOLT_MEASUREMENTS1 0x05B

/* sender: i/o, receiver: cpu */
#define CAN_MID_IO_INPUTS0 0x080
#define CAN_MID_IO_INPUTS1 0x081

#define CAN_MID_IO_INPUTS_DATA_LEN 0x02

/* sender: cpu, receiver: ssm */
#define CAN_MID_CPU_SO0 0x040
#define CAN_MID_CPU_SO1 0x041

/* sender: cpu, receiver: i/o */
#define CAN_MID_CPU_IO_OUTPUTS 0x042

/* sender: cpu, receiver: ssm, psm, i/o */
#define CAN_MID_CPU_MODULE_SECURITY_QUERY 0x120

/* sender: ssm, psm, i/o, receiver: cpu */
#define CAN_MID_MODULE_SECURITY_RESPONSE 0x121

/* sender: cpu, pc, receiver: cpu, psm, ssm */
#define CAN_MID_CPU_DATE_TIME 0x100

/* sender: ld, receiver: cpu */
#define CAN_MID_LD_START 0x580
#define CAN_MID_LD_DATA0 0x581
#define CAN_MID_LD_DATA1 0x582
#define CAN_MID_LD_DATA2 0x583
#define CAN_MID_LD_DATA3 0x584
#define CAN_MID_LD_DATA4 0x585
#define CAN_MID_LD_DATA5 0x586
#define CAN_MID_LD_DATA6 0x587
#define CAN_MID_LD_DATA7 0x588

/* sender: cpu, receiver: pc */
#define CAN_MID_CPU_EVENT 0x200

/* sender: ssm, receiver: cpu */
#define CAN_MID_SSM_EVENT0 0x210
#define CAN_MID_SSM_EVENT1 0x211
#define CAN_MID_SSM_EVENT2 0x212
#define CAN_MID_SSM_EVENT3 0x213
#define CAN_MID_SSM_EVENT4 0x214
#define CAN_MID_SSM_EVENT5 0x215
#define CAN_MID_SSM_EVENT6 0x216
#define CAN_MID_SSM_EVENT7 0x217

/* sender: psm, receiver: cpu */
#define CAN_MID_PSM_EVENT0 0x220
#define CAN_MID_PSM_EVENT1 0x221

/* sender: i/o, receiver: cpu */
#define CAN_MID_IO_EVENT0 0x230
#define CAN_MID_IO_EVENT1 0x231

/* sender: cpu, receiver: psm */
#define CAN_CPU_OFFSET_CONFIG 0x240

/* sender: cpu, pc, receiver: cpu, ssm, psm */
#define CAN_MID_MODULE_CONTROL 0x0A0

/* sender: cpu, ssm, psm, receiver: cpu, pc */
#define CAN_MID_MODULE_INFO 0x0A1

/* sender: cpu, receiver: ssm */
#define CAN_MID_CPU_FLASH_SIGNALS0 0x0B0
#define CAN_MID_CPU_FLASH_SIGNALS1 0x0B1

/* message buffer(MB) sizes that will be used */
#define CAN_DLC_VALUES_MB_ALL 8

/* so messages */
#define CAN_SO_MSG_MAX 2
#define CAN_SO_MSG_FIR 0
#define CAN_SO_MSG_SEC 1

/* periods */
#define CAN_CPU_DATE_TIME_FLASH_SIGNALS_PERIOD 25 /* in terms of 20 ms */
#define CAN_LOOP_DEDECTOR_OP_MODE_ENTRY_PERIOD 25
#define CAN_LOOP_STATUS_REQUEST_PERIOD 10
#define CAN_MMI_SIGNALS_STREAM_PERIOD 50
#define CAN_LOOP_DETECTOR_MAX_COM_ERRORS 10
#define CAN_INPUT_STATUS_CHECK_PERIOD 10
#define CAN_INPUT_MAX_COM_ERRORS 10

/* sender: cp, receiver: mp */
#define CAN_LRLF_DETECT_TIME 0x0DA

/* //////////////////////////////////////////////////////////// */
/* messages */
typedef struct _SCanSSMVoltCurMeas
{
  union _USOVoltages
  {
    struct _SfVoltages
    {
      uint8_t fSOVoltage0 : 1; /* exists or doesn't exist */
      uint8_t fSOVoltage1 : 1;
      uint8_t fSOVoltage2 : 1;
      uint8_t fSOVoltage3 : 1;
      uint8_t fSOVoltage4 : 1;
      uint8_t fSOVoltage5 : 1;
      uint8_t fSOVoltage6 : 1;
      uint8_t fSOVoltage7 : 1;
      uint8_t fSOVoltage8 : 1;
      uint8_t fSOVoltage9 : 1;
      uint8_t fSOVoltage10 : 1;
      uint8_t fSOVoltage11 : 1;
      uint8_t bReserved : 4;
    } __attribute__((packed)) SfVoltages;

    uint16_t sVoltages;
  } __attribute__((packed)) USOVoltages;

  union _USOCurrentsL
  {
    struct _SbCurrentsL
    {
      uint8_t bSOCurrent0_2_L;
      uint8_t bSOCurrent3_5_L;
      uint8_t bSOCurrent6_8_L;
      uint8_t bSOCurrent9_11_L;
    } __attribute__((packed)) SbCurrentsL;

    uint8_t baCurrentsL[4];
  } __attribute__((packed)) USOCurrentsL;

  union _USOCurrentsH
  {
    struct _SbCurrentsH
    {
      uint8_t bSOCurrent0_2_H : 2;
      uint8_t bSOCurrent3_5_H : 2;
      uint8_t bSOCurrent6_8_H : 2;
      uint8_t bSOCurrent9_11_H : 2;
    } __attribute__((packed)) SbCurrentsH;

    uint8_t bCurrentsH;
  } __attribute__((packed)) USOCurrentsH;

  uint8_t bReserved2;
} __attribute__((packed)) tSCanSSMVoltCurMeas, *tpSCanSSMVoltCurMeas;

typedef struct _SCanPSMVoltMeas
{
  uint8_t bNetVoltageL;
  uint8_t b24V1L;
  uint8_t b24V2L;
  uint8_t b5V1L;
  uint8_t b5V2L;
  uint8_t bNetVoltageH : 2;
  uint8_t b24V1H : 2;
  uint8_t b24V2H : 2;
  uint8_t b5V1H : 2;
  uint8_t b5V2H : 2;
  uint8_t fIsolatedVoltage : 1;
  uint8_t bReserved : 5;
  uint8_t bNetFrequency;
} __attribute__((packed)) tSCanPSMVoltMeas, *tpSCanPSMVoltMeas;

typedef struct _SCanCpuSO
{
  union _USOOn0
  {
    struct _SOOn0
    {
      uint8_t fSOOn0 : 1; /* on or off */
      uint8_t fSOOn1 : 1;
      uint8_t fSOOn2 : 1;
      uint8_t fSOOn3 : 1;
      uint8_t fSOOn4 : 1;
      uint8_t fSOOn5 : 1;
      uint8_t fSOOn6 : 1;
      uint8_t fSOOn7 : 1;
      uint8_t fSOOn8 : 1;
      uint8_t fSOOn9 : 1;
      uint8_t fSOOn10 : 1;
      uint8_t fSOOn11 : 1;
      uint8_t fSOOn12 : 1;
      uint8_t fSOOn13 : 1;
      uint8_t fSOOn14 : 1;
      uint8_t fSOOn15 : 1;
      uint8_t fSOOn16 : 1;
      uint8_t fSOOn17 : 1;
      uint8_t fSOOn18 : 1;
      uint8_t fSOOn19 : 1;
      uint8_t fSOOn20 : 1;
      uint8_t fSOOn21 : 1;
      uint8_t fSOOn22 : 1;
      uint8_t fSOOn23 : 1;
      uint8_t fSOOn24 : 1;
      uint8_t fSOOn25 : 1;
      uint8_t fSOOn26 : 1;
      uint8_t fSOOn27 : 1;
      uint8_t fSOOn28 : 1;
      uint8_t fSOOn29 : 1;
      uint8_t fSOOn30 : 1;
      uint8_t fSOOn31 : 1;
    } __attribute__((packed)) SOOn;

    uint32_t lSOOn;
  } __attribute__((packed)) USOOn0;

  union _USOOn1
  {
    struct _SOOn1
    {
      uint8_t fSOOn32 : 1;
      uint8_t fSOOn33 : 1;
      uint8_t fSOOn34 : 1;
      uint8_t fSOOn35 : 1;
      uint8_t fSOOn36 : 1;
      uint8_t fSOOn37 : 1;
      uint8_t fSOOn38 : 1;
      uint8_t fSOOn39 : 1;
      uint8_t fSOOn40 : 1;
      uint8_t fSOOn41 : 1;
      uint8_t fSOOn42 : 1;
      uint8_t fSOOn43 : 1;
      uint8_t fSOOn44 : 1;
      uint8_t fSOOn45 : 1;
      uint8_t fSOOn46 : 1;
      uint8_t fSOOn47 : 1;
      uint16_t sReserved;
    } __attribute__((packed)) SOOn;

    uint32_t lSOOn;
  } __attribute__((packed)) USOOn1;
} __attribute__((packed)) tSCanCpuSO, *tpSCanCpuSO;

typedef struct _SCanCpuDateTime
{
  uint8_t bSecond : 6;
  uint8_t bReserved : 2;
  uint8_t bMinute : 6;
  uint8_t bReserved2 : 2;
  uint8_t bHour : 5;
  uint8_t bReserved3 : 3;
  uint8_t bDay : 5;
  uint8_t bReserved4 : 3;
  uint8_t bMonth : 4;
  uint8_t bReserved5 : 4;
  uint8_t bYear : 8;
  uint16_t sReserved6;
} __attribute__((packed)) tSCanCpuDateTime, *tpSCanCpuDateTime;

/* if SO will flash in flash mode */
typedef struct _SCanCpuFlashSignals
{
  union _UFlashSignals0
  {
    struct _SFlashSignals0
    {
      uint8_t fFlashOutput0 : 1;
      uint8_t fFlashOutput1 : 1;
      uint8_t fFlashOutput2 : 1;
      uint8_t fFlashOutput3 : 1;
      uint8_t fFlashOutput4 : 1;
      uint8_t fFlashOutput5 : 1;
      uint8_t fFlashOutput6 : 1;
      uint8_t fFlashOutput7 : 1;
      uint8_t fFlashOutput8 : 1;
      uint8_t fFlashOutput9 : 1;
      uint8_t fFlashOutput10 : 1;
      uint8_t fFlashOutput11 : 1;
      uint8_t fFlashOutput12 : 1;
      uint8_t fFlashOutput13 : 1;
      uint8_t fFlashOutput14 : 1;
      uint8_t fFlashOutput15 : 1;
      uint8_t fFlashOutput16 : 1;
      uint8_t fFlashOutput17 : 1;
      uint8_t fFlashOutput18 : 1;
      uint8_t fFlashOutput19 : 1;
      uint8_t fFlashOutput20 : 1;
      uint8_t fFlashOutput21 : 1;
      uint8_t fFlashOutput22 : 1;
      uint8_t fFlashOutput23 : 1;
      uint8_t fFlashOutput24 : 1;
      uint8_t fFlashOutput25 : 1;
      uint8_t fFlashOutput26 : 1;
      uint8_t fFlashOutput27 : 1;
      uint8_t fFlashOutput28 : 1;
      uint8_t fFlashOutput29 : 1;
      uint8_t fFlashOutput30 : 1;
      uint8_t fFlashOutput31 : 1;
    } __attribute__((packed)) SFlashSignals;

    uint32_t lFlashSignals;
  } __attribute__((packed)) UFlashSignals0;

  union _UFlashSignals1
  {
    struct _SFlashSignals1
    {
      uint8_t fFlashOutput32 : 1;
      uint8_t fFlashOutput33 : 1;
      uint8_t fFlashOutput34 : 1;
      uint8_t fFlashOutput35 : 1;
      uint8_t fFlashOutput36 : 1;
      uint8_t fFlashOutput37 : 1;
      uint8_t fFlashOutput38 : 1;
      uint8_t fFlashOutput39 : 1;
      uint8_t fFlashOutput40 : 1;
      uint8_t fFlashOutput41 : 1;
      uint8_t fFlashOutput42 : 1;
      uint8_t fFlashOutput43 : 1;
      uint8_t fFlashOutput44 : 1;
      uint8_t fFlashOutput45 : 1;
      uint8_t fFlashOutput46 : 1;
      uint8_t fFlashOutput47 : 1;
      uint8_t bFlashPeriod0 : 6; /* flash period is (bFlashPeriod0 * 100ms) */
      uint8_t bReserved : 2;
      uint8_t bReserved2;
    } __attribute__((packed)) SFlashSignals;

    uint32_t lFlashSignals;
  } __attribute__((packed)) UFlashSignals1;
} __attribute__((packed)) tSCanCpuFlashSignals, *tpSCanCpuFlashSignals;

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  public methods */
extern uint8_t CANGetRxDataLength(uint32_t lCode);
extern void CANRxRequest(tpSFDCANRxMsg pSRxMsg);
extern void CANTxRequest(uint8_t bDLC, uint8_t bIdType, uint16_t sMID,
                         uint8_t *baData);

#endif /* ifndef __CANMSGPARSER_H__ */
