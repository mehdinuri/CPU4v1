#ifndef __TASK_CPMPCOMM_H__
#define __TASK_CPMPCOMM_H__

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  include files */
#include "data.h"

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  definitions */

/* this file includes the data types and definitions */
/* that are used in CP (controller processor) - MP (monitor processor) */
/* communications */

#define CPMP_ERROR_LIMIT 50

typedef struct _tSCPMPComm
{
  uint8_t bPacketInc;

  uint16_t sPacketId;
  uint16_t sAnsId;
  uint8_t bMsgLen;

  uint8_t bDataNow[8];
  uint8_t bDataPrev[8];

  uint8_t bCPMPState; /*  defines the communication state */
  uint8_t bCPMPSubState;
  uint8_t bPacketNo; /* defines packet number of defs */

  uint8_t *pbDataSrc;
  uint16_t sIndex;
  uint16_t sDataLen;
} tSCPMPComm, *tpSCPMPComm;

/* bPacket Type Definitions */
#define PACKET_TYPE_CP_NONE 0x00
#define PACKET_SUB_TYPE_CP_NONE 0x00
#define PACKET_TYPE_CP_SIGNAL_DEFS 0x01
#define PACKET_SUB_TYPE_CP_SIGNAL_DEFS_1 0x01
#define PACKET_SUB_TYPE_CP_SIGNAL_DEFS_2 0x02
#define PACKET_SUB_TYPE_CP_SIGNAL_DEFS_LAST 0x02
#define PACKET_TYPE_CP_SIGNALS_DEFINED 0x02
#define PACKET_TYPE_CP_SG_DEFS 0x03
#define PACKET_SUB_TYPE_CP_SG_DEFS_1 0x01
#define PACKET_SUB_TYPE_CP_SG_DEFS_2 0x02
#define PACKET_SUB_TYPE_CP_SG_DEFS_3 0x03
#define PACKET_SUB_TYPE_CP_SG_DEFS_4 0x04
#define PACKET_SUB_TYPE_CP_SG_DEFS_5 0x05
#define PACKET_SUB_TYPE_CP_SG_DEFS_6 0x06
#define PACKET_SUB_TYPE_CP_SG_LAST 0x06
#define PACKET_TYPE_CP_SO_DEFS 0x04
#define PACKET_SUB_TYPE_CP_SO_DEFS_1 0x01
#define PACKET_SUB_TYPE_CP_SO_DEFS_2 0x02
#define PACKET_SUB_TYPE_CP_SO_LAST 0x02
#define PACKET_TYPE_CP_CVS_DEFS 0x05
#define PACKET_SUB_TYPE_CP_CVS_DEFS_1 0x01
#define PACKET_SUB_TYPE_CP_CVS_DEFS_2 0x02
#define PACKET_SUB_TYPE_CP_CVS_DEFS_3 0x03
#define PACKET_SUB_TYPE_CP_CVS_DEFS_4 0x04
#define PACKET_SUB_TYPE_CP_CVS_DEFS_5 0x05
#define PACKET_SUB_TYPE_CP_CVS_LAST 0x05
#define PACKET_TYPE_CP_CONFLICTS_EM 0x06
#define PACKET_TYPE_CP_FLASH_CFG 0x07
#define PACKET_TYPE_CP_DEFAULT 0x08
#define PACKET_TOTAL_CNT 0x08

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Public Methods */
extern void SetFlashSignalTransmit(uint8_t fState);
extern uint8_t GetFlashSignalTransmit(void);

/* for cp */
extern void CPMPStateSet(uint8_t nState);
extern uint8_t CPMPStateGet(void);
extern void CPMPSubStateSet(uint8_t bSubState);
extern uint8_t CPMPSubStateGet(void);
extern void CPMPResetErrCounter(void);
extern void CPMPPacketIncrease(void);
extern uint8_t CPMPTxCheck(uint16_t sId, uint8_t *pbData, uint8_t bDataLen);
extern void CPMPCommPacketIncSet(uint8_t bState);
extern uint8_t CPMPCommPacketIncGet(void);

#endif /* ifndef __TASK_CPMPCOMM_H__ */
