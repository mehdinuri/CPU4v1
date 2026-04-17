/**
 ******************************************************************************
 * @file           : tcp_client.h
 * @brief          : Header for tcp_client.c file.
 *                   This file contains the common defines for tcp client app.
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TCP_CLIENT_H__
#define __TCP_CLIENT_H__

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdint.h>
#include "cmsis_os.h"
#include "lwip/tcp.h"
#include "lwip/memp.h"
#include "MCSAsynch.h"
/* Private define ------------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/

/* Private types -------------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Public define ------------------------------------------------------------*/
#define TCP_CLIENT_TIMEOUT_CONNECTION (uint32_t) (10000)
#define TCP_CLIENT_TIMEOUT_CLOSE  (uint32_t) (60000)
#define TCP_CLIENT_TIMEOUT_DISCONNECTION  (uint32_t) (TCP_CLIENT_TIMEOUT_CLOSE \
                                                      * 2)

/* Public macros ------------------------------------------------------------*/

/* Public types -------------------------------------------------------------*/
typedef enum
{
  TCP_CLIENT_STATE_NOT_CONNECTED = 0,
  TCP_CLIENT_STATE_CONNECTING,
  TCP_CLIENT_STATE_CONNECTED,
  TCP_CLIENT_STATE_ABORTED,
  TCP_CLIENT_STATE_CLOSING,
  TCP_CLIENT_STATE_CLOSED
} tETCPClientStates;

typedef struct _tSTCPClient
{
  tETCPClientStates EState;
  struct tcp_pcb *pSClientPCB;
  struct pbuf *pSBuf;
} tSTCPClient, *tpSTCPClient;

/* Public function prototypes -----------------------------------------------*/
extern uint8_t TCPClientIsConnected(void);
extern tETCPClientStates TCPClientGetState(void);
char *TCPClientGetLocalIPv4Str(void);
char *TCPClientGetRemoteIPv4Str(void);
extern uint8_t TCPClientConnect(const ip_addr_t *pSDstIp, uint16_t sDstPort);
extern void TCPClientDisconnect(void);
extern void TCPClientSendData(tpSMCSAsynchRxTxMsg pSTxMsg);

#endif /* __TCP_CLIENT_H__ */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
