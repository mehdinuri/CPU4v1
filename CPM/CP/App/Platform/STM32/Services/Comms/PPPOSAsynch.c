/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  include files */
#include "PPPOSAsynch.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "MCS.h"
#include "MCSAsynch.h"
#include "arch.h"
#include "ppp.h"
#include "pppos.h"
#include "snmp_client.h"
#include "tcp_client.h"
#include "udp_probe.h"
#include "lwip/dns.h"
#include "stdio.h"
/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  members */
#define PPPOS_ASYNCH_DMA_TX_TIMEOUT 1000

static ppp_pcb *PSPPPPCB;
struct netif SPPPOSNetif;
static tSPPPOSAsynchRuntime SPPPOSAsynchRuntime;
static ISerialPort_t       *s_port;

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  methods */
static void PPPOSAsynchInit(void)
{
  memset(&SPPPOSAsynchRuntime, 0, sizeof(SPPPOSAsynchRuntime));
}

static void PPPOSAsynchClose(void)
{
  err_t err = ppp_close(PSPPPPCB, 0);

  if (err == ERR_OK)
  {
    ppp_free(PSPPPPCB);
  }
}

static void PPPOSAsynchSetConnected(uint8_t bState)
{
  SPPPOSAsynchRuntime.SFlags.fConnected = bState;
}

static void PPPOSAsynchSetDailed(uint8_t bState)
{
  SPPPOSAsynchRuntime.SFlags.fDialed = bState;
}

static void PPPOSAsynchAddMCSJob(uint8_t bErrorCode)
{
  switch (bErrorCode)
  {
      case PPPERR_NONE:
      {
        MCSJobAdd("PPP ERR. NONE");
        break;
      }

      case PPPERR_PARAM:
      {
        MCSJobAdd("PPP ERR. PARAM");
        break;
      }

      case PPPERR_OPEN:
      {
        MCSJobAdd("PPP ERR. OPEN");
        break;
      }

      case PPPERR_DEVICE:
      {
        MCSJobAdd("PPP ERR. DEVICE");
        break;
      }

      case PPPERR_ALLOC:
      {
        MCSJobAdd("PPP ERR. ALLOC");
        break;
      }

      case PPPERR_USER:
      {
        MCSJobAdd("PPP ERR. USER");
        break;
      }

      case PPPERR_CONNECT:
      {
        MCSJobAdd("PPP ERR. CONNECT");
        break;
      }

      case PPPERR_AUTHFAIL:
      {
        MCSJobAdd("PPP ERR. AUTHFAIL");
        break;
      }

      case PPPERR_PROTOCOL:
      {
        MCSJobAdd("PPP ERR. PROTOCOL");
        break;
      }

      case PPPERR_PEERDEAD:
      {
        MCSJobAdd("PPP. ERR PEER DEAD");
        break;
      }

      case PPPERR_IDLETIMEOUT:
      {
        MCSJobAdd("PPP ERR. IDLE");
        break;
      }

      case PPPERR_CONNECTTIME:
      {
        MCSJobAdd("PPP ERR. CON.TIME");
        break;
      }

      case PPPERR_LOOPBACK:
      {
        MCSJobAdd("PPP ERR. LOOPBACK");
        break;
      }

      default:
      {
        MCSJobAdd("PPP ERR. UNKNOWN");
        break;
      }
  } /* switch */
} /* PPPOSAsynchAddMCSJob */

static void ppp_link_status_cb(ppp_pcb *pcb, int err_code, void *ctx)
{
  struct netif *pppif = ppp_netif(pcb);

  LWIP_UNUSED_ARG(ctx);

  PPPOSAsynchAddMCSJob(err_code);

  switch (err_code)
  {
      case PPPERR_NONE:
      {
        PPPOSAsynchSetConnected(TRUE);

        MCSSetRuntimeLocalIPv4(ip4addr_ntoa(netif_ip4_addr(pppif)));
        ppp_set_default(PSPPPPCB);
        break;
      }

      default:
      {
        PPPOSAsynchInit();
        break;
      }
  }

  if (PPPAsyEventHandle != NULL)
  {
    osEventFlagsSet(PPPAsyEventHandle, EVENT_FLAGS_PPP_ASY_CONNECTED);
  }
}

uint8_t PPPOSAsynchReqTxMsg(uint16_t sLen, const void *pvData)
{
  if (PPPOSAsynchIsDailed())
  {
    tpSPPPOSAsynchRxTxMsg pSTxReq =
      (tpSPPPOSAsynchRxTxMsg) osMemoryPoolAlloc(PPPOSAsyTxReqsMemPoolHandle,
                                                0);

    if (pSTxReq != NULL)
    {
      memset(pSTxReq, 0, sizeof(tSPPPOSAsynchRxTxMsg));

      pSTxReq->sDataLen = sLen;
      memcpy(pSTxReq->baData, pvData, sLen);
      if (osMessageQueuePut(PPPOSAsyTxReqsQueHandle, &pSTxReq, 0, 0) == osOK)
      {
        return TRUE;
      }

      osMemoryPoolFree(PPPOSAsyTxReqsMemPoolHandle, pSTxReq);
    }
  }

  return FALSE;
}

static u32_t ppp_output_cb(ppp_pcb *pcb, const void *data, u32_t len, void *ctx)
{
  LWIP_UNUSED_ARG(pcb);
  LWIP_UNUSED_ARG(ctx);

  if (len > 0)
  {
    PPPOSAsynchReqTxMsg(len, data);
  }

  return len;
}

uint8_t PPPOSAsynchIsDailed(void)
{
  return SPPPOSAsynchRuntime.SFlags.fDialed;
}

uint8_t PPPOSAsynchGetConnected(void)
{
  return SPPPOSAsynchRuntime.SFlags.fConnected;
}

uint8_t PPPOSAsynchGetPhase(void)
{
  return PSPPPPCB->phase;
}

void PPPOSAsynchResetConnectionTimeout(void)
{
  SPPPOSAsynchRuntime.sConnectionTimeout = 0;
}

void PPPOSAsynchCheckConnectionTimeout(void)
{
  if (PPPOSAsynchGetConnected())
  {
    SPPPOSAsynchRuntime.sConnectionTimeout++;
    if (SPPPOSAsynchRuntime.sConnectionTimeout
        >= PPPOS_ASYNCH_TIMEOUT_COUNTER_IN_SECONDS + 1)
    {
      SPPPOSAsynchRuntime.sConnectionTimeout = 0;
      PPPOSAsynchStop();
    }
  }
}

void PPPOSAsynchCreate(ISerialPort_t *port)
{
  s_port = port;
  PPPOSAsynchInit();
  PSPPPPCB = pppos_create(&SPPPOSNetif, ppp_output_cb, ppp_link_status_cb,
                          NULL);
}

err_t PPPOSAsynchStart(void)
{
  err_t err = ERR_OK;

  PPPOSAsynchSetDailed(TRUE);

  ppp_set_usepeerdns(PSPPPPCB, 1);
  ppp_set_auth(PSPPPPCB, PPPAUTHTYPE_ANY, "", "");
  err = ppp_connect(PSPPPPCB, 0);
  if (err != ERR_OK)
  {
    PPPOSAsynchSetDailed(FALSE);

    return err;
  }

  netif_set_default(&SPPPOSNetif);

  return err;
}

void PPPOSAsynchStop(void)
{
  PPPOSAsynchSetConnected(FALSE);

  if (MCSAsynchConnectedGet())
  {
    MCSAsynchStop(MCS_ASYNCH_DISC_TYPE_PPP_ERROR);
  }

  if (SNMPClientIsStarted())
  {
    SNMPClientStop();
  }

  if (UDPProbeIsStarted())
  {
    UDPProbeStop();
  }

  PPPOSAsynchClose();

  PPPOSAsynchSetConnected(FALSE);
  PPPOSAsynchInit();

  PPPOSAsynchSetDailed(FALSE);
  MCSRuntimeInit();
}

struct netif *PPPOSAsynchGetInfterface(void)
{
  return &SPPPOSNetif;
}

static void PPPOSAsynchReceivePacket(tpSPPPOSAsynchRxTxMsg pSRxPacket)
{
  pppos_input(PSPPPPCB, pSRxPacket->baData, pSRxPacket->sDataLen);
}

static void PPPOSAsynchSendMessage(tpSPPPOSAsynchRxTxMsg pSTxMsg)
{
  (void) SerialSend(s_port,
                    (const uint8_t *) pSTxMsg,
                    pSTxMsg->sDataLen,
                    PPPOS_ASYNCH_DMA_TX_TIMEOUT);
}

uint8_t PPPOSAsynchReqRxMsg(uint8_t *pbData, uint16_t sLength)
{
  if (PPPOSAsynchIsDailed())
  {
    tpSPPPOSAsynchRxTxMsg pSRxReq =
      (tpSPPPOSAsynchRxTxMsg) osMemoryPoolAlloc(PPPOSAsyRxReqsMemPoolHandle,
                                                0);

    if (pSRxReq != NULL)
    {
      memset(pSRxReq, 0, sizeof(tSPPPOSAsynchRxTxMsg));

      pSRxReq->sDataLen = sLength;
      memcpy(pSRxReq->baData, pbData, sLength);

      if (osMessageQueuePut(PPPOSAsyRxReqsQueHandle, &pSRxReq, 0, 0) == osOK)
      {
        return TRUE;
      }

      osMemoryPoolFree(PPPOSAsyRxReqsMemPoolHandle, pSRxReq);
    }
  }

  return FALSE;
}

void PPPOSAsyMsgParserTaskFunc(void *argument)
{
  UNUSED(argument);

  tpSPPPOSAsynchRxTxMsg pSRxMsg = NULL;

  while (TRUE)
  {
    if (PPPOSAsynchIsDailed())
    {
      if (osMessageQueueGet(PPPOSAsyRxReqsQueHandle, &pSRxMsg, NULL,
                            PPPOS_ASYNCH_TIMEOUT_MSG_BOX)
          == osOK)
      {
        PPPOSAsynchReceivePacket(pSRxMsg);
        osMemoryPoolFree(PPPOSAsyRxReqsMemPoolHandle, pSRxMsg);
      }
      else
      {
        if (PPPOSAsynchGetConnected())
        {
          PPPOSAsynchStop();
        }
      }
    }
    else
    {
      osDelay(1000);
    }
  }
}

void PPPOSAsyMsgSenderTaskFunc(void *argument)
{
  UNUSED(argument);

  tpSPPPOSAsynchRxTxMsg pSTxMsg = NULL;

  while (TRUE)
  {
    if (osMessageQueueGet(PPPOSAsyTxReqsQueHandle, &pSTxMsg, NULL,
                          osWaitForever)
        == osOK)
    {
      PPPOSAsynchSendMessage(pSTxMsg);
      osMemoryPoolFree(PPPOSAsyTxReqsMemPoolHandle, pSTxMsg);
    }
  }
}
