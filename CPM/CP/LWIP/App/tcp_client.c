/**
 ******************************************************************************
 * File Name          : tcp_client.c
 * Description        : Code for tcp client app
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "tcp_client.h"

#include <stdio.h>
#include <string.h>

#include "MCS.h"
#include "MCSAsynch.h"
#include "PPPOSAsynch.h"
#include "eth.h"
#include "main.h"
#include  "lwip/priv/tcp_priv.h"
#include "lwip/tcpip.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
struct tcp_pcb *pSTCPClientPCB = NULL;
tpSTCPClient pSTCPClient = NULL;

/* Private function prototypes -----------------------------------------------*/
err_t TCPClientReceiveCallback(void *pvArg,
                               struct tcp_pcb *pSPCB,
                               struct pbuf *pSBuf,
                               err_t eError);
void TCPClientClose(struct tcp_pcb *pSPCB, tpSTCPClient pSClient);
err_t TCPClientPollCallback(void *pvArg, struct tcp_pcb *pSPCB);
err_t TCPClientSendCallback(void *pvArg, struct tcp_pcb *pSPCB, uint16_t sLen);
err_t TCPClientSend(struct tcp_pcb *pSPCB, tpSTCPClient pSClient);
err_t TCPClientConnectedCallback(void *pvArg,
                                 struct tcp_pcb *pSPCB,
                                 err_t eError);
void TCPClientSetState(tETCPClientStates eState);

/* Public typedef -----------------------------------------------------------*/

/* Public define ------------------------------------------------------------*/

/* Public variables ---------------------------------------------------------*/

/* Public function prototypes -----------------------------------------------*/

/* Private application code --------------------------------------------------*/
err_t TCPClientReceiveCallback(void *pvArg,
                               struct tcp_pcb *pSPCB,
                               struct pbuf *pSBuf,
                               err_t eError)
{
  tpSTCPClient pSClient;
  err_t eRetVal;

  LWIP_ASSERT("arg != NULL", pvArg != NULL);

  pSClient = (tpSTCPClient) pvArg;

  /* if we receive an empty tcp frame from server => close connection */
  if (pSBuf == NULL)
  {
    /* remote host closed connection */
    if (pSClient != NULL)
    {
      TCPClientSetState(TCP_CLIENT_STATE_CLOSING);
      if (pSClient->pSBuf == NULL)
      {
        /* we're done sending, close connection */
        TCPClientClose(pSPCB, pSClient);

        if (MCSAsynchConnectedGet())
        {
          MCSAsynchStop(MCS_ASYNCH_DISC_TYPE_TCP_CLIENT_CLOSED);
        }
      }
      else
      {
        /* send remaining data*/
        TCPClientSend(pSPCB, pSClient);
      }
    }
    else
    {
      /* structure lost, just close pcb */
      tcp_close(pSPCB);
    }

    eRetVal = ERR_OK;
  }
  else if (eError != ERR_OK)
  {
    /* free received pbuf*/
    if (pSBuf != NULL)
    {
      pbuf_free(pSBuf);
    }

    eRetVal = eError;
  }
  else if ((pSClient != NULL)
           && (pSClient->EState == TCP_CLIENT_STATE_CONNECTED) )
  {
    /* acknowledge data reception */
    tcp_recved(pSPCB, pSBuf->tot_len);

    if (MCSGetConnected())
    {
      MCSAsynchReqRxMsg(pSBuf->payload, pSBuf->tot_len);
    }

    pbuf_free(pSBuf);

    eRetVal = ERR_OK;
  }
  else
  {
    /* acknowledge data reception */
    tcp_recved(pSPCB, pSBuf->tot_len);
    /* free pbuf and do nothing */
    pbuf_free(pSBuf);

    eRetVal = ERR_OK;
  }

  return eRetVal;
} /* TCPClientReceiveCallback */

void TCPClientClose(struct tcp_pcb *pSPCB, tpSTCPClient pSClient)
{
  if (pSPCB != NULL)
  {
    /* Remove callbacks */
    tcp_arg(pSPCB, NULL);
    tcp_recv(pSPCB, NULL);
    tcp_sent(pSPCB, NULL);
    tcp_poll(pSPCB, NULL, 0);
    tcp_err(pSPCB, NULL);

    /* attempt to close gracefully */
    if (tcp_close(pSPCB) != ERR_OK)
    {
      /* if close fails (e.g. low mem), abort to force cleanup */
      tcp_abort(pSPCB);
    }

    /* clear global PCB pointer if it matches */
    if (pSPCB == pSTCPClientPCB)
    {
      pSTCPClientPCB = NULL;
    }
  }

  /* cleanup Memory */
  if (pSClient != NULL)
  {
    /* free any pending data that wasn't sent */
    if (pSClient->pSBuf != NULL)
    {
      pbuf_free(pSClient->pSBuf);
      pSClient->pSBuf = NULL;
    }

    if (pSTCPClient == pSClient)
    {
      pSTCPClient = NULL;
    }

    mem_free(pSClient);
  }

  /* notify OS Task */
  if (TCPClientEventHandle != NULL)
  {
    osEventFlagsSet(TCPClientEventHandle, EVENT_FLAGS_TCP_CLIENT_DISCONNECTED);
  }
} /* TCPClientClose */

err_t TCPClientPollCallback(void *pvArg, struct tcp_pcb *pSPCB)
{
  err_t eRetVal;

  tpSTCPClient pSClient = (tpSTCPClient) pvArg;

  if (pSClient != NULL)
  {
    if (pSClient->pSBuf != NULL)
    {
      /* there is a remaining pbuf (chain) , try to send data */
      TCPClientSend(pSPCB, pSClient);
    }
    else
    {
      /* no remaining pbuf (chain)  */
      if (pSClient->EState == TCP_CLIENT_STATE_CLOSING)
      {
        /* close tcp connection */
        TCPClientClose(pSPCB, pSClient);

        if (MCSAsynchConnectedGet())
        {
          MCSAsynchStop(MCS_ASYNCH_DISC_TYPE_TCP_CLIENT_CLOSED);
        }
      }
    }

    eRetVal = ERR_OK;
  }
  else
  {
    /* nothing to be done */
    tcp_abort(pSPCB);
    eRetVal = ERR_ABRT;
  }

  return eRetVal;
}

err_t TCPClientSendCallback(void *pvArg, struct tcp_pcb *pSPCB, uint16_t sLen)
{
  LWIP_UNUSED_ARG(sLen);

  tpSTCPClient pSClient = (tpSTCPClient) pvArg;

  if ((pSClient != NULL) && (pSClient->pSBuf != NULL))
  {
    /* still got pbufs to send */
    TCPClientSend(pSPCB, pSClient);
  }

  return ERR_OK;
}

err_t TCPClientSend(struct tcp_pcb *pSPCB, tpSTCPClient pSClient)
{
  struct pbuf *pSBuf;
  err_t eError = ERR_OK;

  while ((eError == ERR_OK)
         && (pSClient->pSBuf != NULL)
         && (pSClient->pSBuf->len <= tcp_sndbuf(pSPCB)))
  {
    /* get pointer on pbuf from es structure */
    pSBuf = pSClient->pSBuf;

    /* enqueue data for transmission */
    eError = tcp_write(pSPCB, pSBuf->payload, pSBuf->len, 1);

    if (eError == ERR_OK)
    {
      /* continue with next pbuf in chain (if any) */
      pSClient->pSBuf = pSBuf->next;

      if (pSClient->pSBuf != NULL)
      {
        /* increment reference count for es->p */
        pbuf_ref(pSClient->pSBuf);
      }

      /* free pbuf: will free pbufs up to es->p (because es->p has a reference count > 0) */
      pbuf_free(pSBuf);

      /* flash immediately */
      tcp_output(pSPCB);
    }
    else if (eError == ERR_MEM)
    {
      /* we are low on memory, try later, defer to poll */
      pSClient->pSBuf = pSBuf;
    }
  }

  return eError;
}

void TCPClientErrorCallback(void *pvArg, err_t eErr)
{
  tpSTCPClient pSClient = (tpSTCPClient) pvArg;

  if (pSClient != NULL)
  {
    if (eErr == ERR_ABRT)
    {
      pSClient->EState = TCP_CLIENT_STATE_ABORTED;
    }
    else
    {
      pSClient->EState = TCP_CLIENT_STATE_CLOSED;
    }

    /* clean up pbuf if any */
    if (pSClient->pSBuf != NULL)
    {
      pbuf_free(pSClient->pSBuf);
      pSClient->pSBuf = NULL;
    }

    if (pSTCPClient == pSClient)
    {
      pSTCPClient = NULL;
    }

    mem_free(pSClient);
  }

  /* clear global PCB if it matches (it's already invalid) */
  if (pSTCPClientPCB != NULL)
  {
    pSTCPClientPCB = NULL;
  }

  if (MCSAsynchConnectedGet())
  {
    MCSAsynchStop(MCS_ASYNCH_DISC_TYPE_TCP_ERROR);
  }

  if (TCPClientEventHandle != NULL)
  {
    osEventFlagsSet(TCPClientEventHandle, EVENT_FLAGS_TCP_CLIENT_ERROR);
  }
} /* TCPClientErrorCallback */

err_t TCPClientConnectedCallback(void *pvArg,
                                 struct tcp_pcb *pSPCB,
                                 err_t eError)
{
  LWIP_UNUSED_ARG(pvArg);

  if (eError == ERR_OK)
  {
    /* allocate structure es to maintain tcp connection informations */
    pSTCPClient = (tpSTCPClient) mem_malloc(sizeof(tSTCPClient));
    if (pSTCPClient != NULL)
    {
      memset(pSTCPClient, 0, sizeof(tSTCPClient));

      TCPClientSetState(TCP_CLIENT_STATE_CONNECTED);
      pSTCPClient->pSClientPCB = pSPCB;
      pSTCPClient->pSBuf = NULL;

      /* pass newly allocated es structure as argument to tpcb */
      tcp_arg(pSPCB, pSTCPClient);

      /* initialize LwIP tcp_recv callback function */
      tcp_recv(pSPCB, TCPClientReceiveCallback);

      /* initialize LwIP tcp_sent callback function */
      tcp_sent(pSPCB, TCPClientSendCallback);

      /* initialize LwIP tcp_poll callback function to poll every 1 sec */
      tcp_poll(pSPCB, TCPClientPollCallback, 2);

      /* initialize LwIP tcp_err callback function */
      tcp_err(pSPCB, TCPClientErrorCallback);

      /* enable Keepalive */
      ip_set_option(pSPCB, SOF_KEEPALIVE);

      if (TCPClientEventHandle != NULL)
      {
        osEventFlagsSet(TCPClientEventHandle, EVENT_FLAGS_TCP_CLIENT_CONNECTED);
      }

      return ERR_OK;
    }
    else
    {
      /* memory alloc failed, close connection */
      TCPClientClose(pSPCB, pSTCPClient);

      if (MCSAsynchConnectedGet())
      {
        MCSAsynchStop(MCS_ASYNCH_DISC_TYPE_TCP_CLIENT_CLOSED);
      }

      /* return memory allocation error */
      return ERR_MEM;
    }
  }
  else
  {
    /* connection failed */
    TCPClientClose(pSPCB, pSTCPClient);

    if (MCSAsynchConnectedGet())
    {
      MCSAsynchStop(MCS_ASYNCH_DISC_TYPE_TCP_CLIENT_CLOSED);
    }
  }

  return eError;
} /* TCPClientConnectedCallback */

void TCPClientSetState(tETCPClientStates eState)
{
  if (pSTCPClient != NULL)
  {
    pSTCPClient->EState = eState;
  }
}

/* Public application code --------------------------------------------------*/
uint8_t TCPClientIsConnected(void)
{
  return pSTCPClient != NULL
         && pSTCPClient->EState == TCP_CLIENT_STATE_CONNECTED;
}

tETCPClientStates TCPClientGetState(void)
{
  if (pSTCPClient != NULL)
  {
    return pSTCPClient->EState;
  }

  return TCP_CLIENT_STATE_NOT_CONNECTED;
}

char *TCPClientGetLocalIPv4Str(void)
{
  if ((pSTCPClient != NULL) && (pSTCPClient->pSClientPCB != NULL))
  {
    return ipaddr_ntoa(&pSTCPClient->pSClientPCB->local_ip);
  }

  return "";
}

char *TCPClientGetRemoteIPv4Str(void)
{
  if ((pSTCPClient != NULL) && (pSTCPClient->pSClientPCB != NULL))
  {
    return ipaddr_ntoa(&pSTCPClient->pSClientPCB->remote_ip);
  }

  return "";
}

void TCPClientDisconnect(void)
{
  if (TCPClientIsConnected())
  {
    TCPClientSetState(TCP_CLIENT_STATE_CLOSING);
    osEventFlagsWait(TCPClientEventHandle,
                     EVENT_FLAGS_TCP_CLIENT_DISCONNECTED,
                     osFlagsWaitAll,
                     osWaitForever);
  }
  else
  {
    /* if not connected, clean up any lingering PCB */
    if (pSTCPClientPCB)
    {
      tcp_abort(pSTCPClientPCB);
      pSTCPClientPCB = NULL;
    }
  }
}

uint8_t TCPClientConnect(const ip_addr_t *pSDstIp, uint16_t sDstPort)
{
  /* clean up existing struct */
  if (pSTCPClient != NULL)
  {
    if (pSTCPClient->pSBuf != NULL)
    {
      pbuf_free(pSTCPClient->pSBuf);
    }

    mem_free(pSTCPClient);
    pSTCPClient = NULL;
  }

  /* clean up existing PCB */
  if (pSTCPClientPCB != NULL)
  {
    tcp_abort(pSTCPClientPCB);
    pSTCPClientPCB = NULL;
  }

  pSTCPClientPCB = tcp_new();
  if (pSTCPClientPCB != NULL)
  {
    tcp_err(pSTCPClientPCB, TCPClientErrorCallback);

    return tcp_connect(pSTCPClientPCB,
                       pSDstIp,
                       sDstPort,
                       TCPClientConnectedCallback) == ERR_OK;
  }

  return FALSE;
}

static void TCPClientSendDataInternal(void *pvArg)
{
  tpSMCSAsynchRxTxMsg pSTxMsg = (tpSMCSAsynchRxTxMsg) pvArg;

  if (pSTxMsg != NULL)
  {
    if ((pSTCPClient != NULL) && (pSTCPClient->pSClientPCB != NULL)
        && (pSTCPClient->EState == TCP_CLIENT_STATE_CONNECTED) )
    {
      struct pbuf *pSBuf = pbuf_alloc(PBUF_TRANSPORT,
                                      pSTxMsg->sDataLen,
                                      PBUF_POOL);

      if (pSBuf != NULL)
      {
        if (pbuf_take(pSBuf, pSTxMsg->baData, pSTxMsg->sDataLen) == ERR_OK)
        {
          /* chain pbufs to avoid memory leak! */
          if (pSTCPClient->pSBuf == NULL)
          {
            pSTCPClient->pSBuf = pSBuf;
          }
          else
          {
            pbuf_cat(pSTCPClient->pSBuf, pSBuf);
          }

          TCPClientSend(pSTCPClient->pSClientPCB, pSTCPClient);
        }
        else
        {
          pbuf_free(pSBuf);
        }
      }
    }
  }
}

void TCPClientSendData(tpSMCSAsynchRxTxMsg pSTxMsg)
{
  if (pSTxMsg->sDataLen == 0)
  {
    return;
  }

  tcpip_callback(TCPClientSendDataInternal, (void *) pSTxMsg);
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
