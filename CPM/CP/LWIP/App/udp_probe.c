/*
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Dirk Ziegelmeier <dziegel@gmx.de>
 *
 */

#include "udp_probe.h"
#include "lwip.h"
#include "defs.h"
#include "lwip/ip_addr.h"
#include "lwip/udp.h"
#include "lwip/timeouts.h"
#include "lwip/netif.h"

#include <string.h>

#define UDP_PROBE_CLIENT_PORT  5678
#define UDP_PROBE_SERVER_PORT  5679
#define UDP_PROBE_PAYLOAD      "probe"
#define UDP_ACK_PAYLOAD        "ack"
#define UDP_PROBE_TIMEOUT_MS   10000

typedef enum
{
  PROBE_STATUS_IDLE,
  PROBE_STATUS_IN_FLIGHT,
  PROBE_STATUS_SUCCESS,
  PROBE_STATUS_FAIL
} tEProbeStatus;

static struct udp_pcb *pSUDPClientPCB = NULL;
static struct udp_pcb *pSUDPServerPCB = NULL;
static tEProbeStatus EProbeStatus = PROBE_STATUS_IDLE;

static void (*ProbeResultCb)(uint8_t fSuccess) = NULL;
static tEUDPProbeClientStatus EProbeClientStatus = UDP_PROBE_NONE;

static void ProbeServerReceiveCb(void *arg,
                                 struct udp_pcb *pcb,
                                 struct pbuf *p,
                                 const ip_addr_t *addr,
                                 u16_t port)
{
  if (!p || (p->len <= 0))
  {
    pbuf_free(p);

    return;
  }

  char buf[8] = { 0 };

  pbuf_copy_partial(p, buf, sizeof(buf) - 1, 0);

  if (strcmp(buf, UDP_PROBE_PAYLOAD) == 0)
  {
    struct pbuf *ack = pbuf_alloc(PBUF_TRANSPORT,
                                  strlen(UDP_ACK_PAYLOAD),
                                  PBUF_RAM);

    if (ack)
    {
      memcpy(ack->payload, UDP_ACK_PAYLOAD, strlen(UDP_ACK_PAYLOAD));
      udp_sendto(pSUDPServerPCB, ack, addr, UDP_PROBE_CLIENT_PORT);
      pbuf_free(ack);
    }
  }

  pbuf_free(p);
}

static void ProbeTimeoutCb(void *arg)
{
  if (EProbeStatus == PROBE_STATUS_IN_FLIGHT)
  {
    EProbeStatus = PROBE_STATUS_FAIL;
    if (ProbeResultCb)
    {
      ProbeResultCb(FALSE);
      ProbeResultCb = NULL;
    }
  }
}

static void ProbeClientReceiveCb(void *arg,
                                 struct udp_pcb *pcb,
                                 struct pbuf *p,
                                 const ip_addr_t *addr,
                                 u16_t port)
{
  if (!p || (p->len <= 0))
  {
    pbuf_free(p);

    return;
  }

  char buf[8] = { 0 };

  pbuf_copy_partial(p, buf, sizeof(buf) - 1, 0);

  if (strcmp(buf, UDP_ACK_PAYLOAD) == 0)
  {
    if (EProbeStatus == PROBE_STATUS_IN_FLIGHT)
    {
      EProbeStatus = PROBE_STATUS_SUCCESS;
      sys_untimeout(ProbeTimeoutCb, NULL);
      if (ProbeResultCb)
      {
        ProbeResultCb(TRUE);
        ProbeResultCb = NULL;
      }
    }
  }

  pbuf_free(p);
}

uint8_t UDPProbeStart(void)
{
  err_t err = ERR_OK;

  UDPProbeStop();

  pSUDPServerPCB = udp_new();
  if (!pSUDPServerPCB)
  {
    return FALSE;
  }

  err = udp_bind(pSUDPServerPCB, IP_ADDR_ANY, UDP_PROBE_SERVER_PORT);
  if (err != ERR_OK)
  {
    return FALSE;
  }

  udp_recv(pSUDPServerPCB, ProbeServerReceiveCb, NULL);

  pSUDPClientPCB = udp_new();
  if (!pSUDPClientPCB)
  {
    return FALSE;
  }

  err = udp_bind(pSUDPClientPCB, IP_ADDR_ANY, UDP_PROBE_CLIENT_PORT);
  if (err != ERR_OK)
  {
    return FALSE;
  }

  udp_recv(pSUDPClientPCB, ProbeClientReceiveCb, NULL);

  EProbeClientStatus = UDP_PROBE_STARTED;

  return TRUE;
}

void UDPProbeSend(void (*ResultCb)(uint8_t fSuccess))
{
  if (UDPProbeIsStarted())
  {
    if (!pSUDPClientPCB || ip_addr_isany_val(netif_default->ip_addr))
    {
      return;
    }

    if (EProbeStatus == PROBE_STATUS_IN_FLIGHT)
    {
      return;
    }

    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT,
                                strlen(UDP_PROBE_PAYLOAD),
                                PBUF_RAM);

    if (!p)
    {
      return;
    }

    EProbeStatus = PROBE_STATUS_IN_FLIGHT;
    memcpy(p->payload, UDP_PROBE_PAYLOAD, strlen(UDP_PROBE_PAYLOAD));
    udp_sendto(pSUDPClientPCB, p, &netif_default->ip_addr,
               UDP_PROBE_SERVER_PORT);
    pbuf_free(p);

    ProbeResultCb = ResultCb;

    sys_untimeout(ProbeTimeoutCb, NULL);
    sys_timeout(UDP_PROBE_TIMEOUT_MS, ProbeTimeoutCb, NULL);
  }
}

void UDPProbeStop(void)
{
  if (pSUDPClientPCB != NULL)
  {
    udp_remove(pSUDPClientPCB);
    pSUDPClientPCB = NULL;
  }

  if (pSUDPServerPCB != NULL)
  {
    udp_remove(pSUDPServerPCB);
    pSUDPServerPCB = NULL;
  }

  EProbeStatus = PROBE_STATUS_IDLE;
  EProbeClientStatus = UDP_PROBE_NONE;
}

tEUDPProbeClientStatus UDPProbeClientGetStatus(void)
{
  return EProbeClientStatus;
}

uint8_t UDPProbeIsStarted(void)
{
  return EProbeClientStatus == UDP_PROBE_STARTED;
}
