/**
 ******************************************************************************
 * @file    pppos_client.h
 * @author  Teknotel Electronics
 * @version V1.0.0
 * @date    06/26/2024
 * @brief  Maestro Point to Point Protocol Header File
 ******************************************************************************
 */

#ifndef __PPPOS_ASYNCH_H__
#define __PPPOS_ASYNCH_H__

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  include files */
#include <stdint.h>
#include "lwip/err.h"
#include "Ports/ISerialPort.h"
/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  Definitions */
#define PPPOS_ASYNCH_MAX_RXTX_PACKET_LENGTH 1024

#define PPPOS_ASYNCH_TIMEOUT_CONNECTION 30000
#define PPPOS_ASYNCH_TIMEOUT_COUNTER_IN_SECONDS 300
#define PPPOS_ASYNCH_TIMEOUT_MSG_BOX 90000

typedef struct _tSPPPOSAsynchRxTxMsg
{
  uint8_t baData[PPPOS_ASYNCH_MAX_RXTX_PACKET_LENGTH];
  uint16_t sDataLen;
} __attribute__((packed)) tSPPPOSAsynchRxTxMsg, *tpSPPPOSAsynchRxTxMsg;

typedef struct _tSPPPOSAsynchRuntime
{
  struct
  {
    uint8_t fDialed : 1;
    uint8_t fConnected : 1;
    uint8_t fReserved : 6;
  } __attribute__((packed)) SFlags;

  uint16_t sConnectionTimeout;
} tSPPPOSAsynchRuntime, *tpSPPPOSAsynchRuntime;

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  public methods */
extern err_t PPPOSAsynchStart(void);
extern void PPPOSAsynchStop(void);
extern struct netif *PPPOSAsynchGetInfterface(void);

extern uint8_t PPPOSAsynchGetConnected(void);
extern uint8_t PPPOSAsynchIsDailed(void);
extern uint8_t PPPOSAsynchGetPhase(void);
extern void PPPOSAsynchResetConnectionTimeout(void);
extern void PPPOSAsynchCheckConnectionTimeout(void);
extern void PPPOSAsynchCreate(ISerialPort_t *port);
extern uint8_t PPPOSAsynchReqRxMsg(uint8_t *pbData, uint16_t sLength);

#endif /* ifndef __PPPOS_ASYNCH_H__ */
