/* App/Adapters/STM32/EthernetNtcipModemAdapter.c
 *
 * IModemPort concrete implementation for Ethernet NTCIP (LwIP / DHCP).
 * No serial port is used; GetBaudRate() returns 0 and PrepareCommand()
 * always returns FALSE.
 *
 * INIT state polls EthProcessDHCP() / EthGetDHCPState() until an
 * address is assigned, then advances to CONNECT.
 * CONNECT state returns MODEM_STATE_CONNECTED, triggering the
 * coordinator to call MCSTryConnect() for SNMP/TCP client startup.
 */
#include "EthernetNtcipModemAdapter.h"

#include <stdio.h>
#include <string.h>

#include "eth.h"  /* EthProcessDHCP, EthGetDHCPState, EthGetDHCPIPv4,
                   *  DHCP_* enum, tEEthDHCPStates               */
#include "lwip.h" /* MX_LWIP_Init, LwIPIsNetifUp               */
#include "MCS.h"  /* MCSIsEthernetStaticIP, MCSGetEthernetMACAddress */
#include "MCSAsynch.h"
#include "snmp_client.h"
#include "udp_probe.h"
#include "tcp_client.h"
#include "program.h"

/* ------------------------------------------------------------------
 * Private state enum — authoritative for Ethernet NTCIP adapter.
 * ------------------------------------------------------------------ */
typedef enum
{
  ETH_INIT        = 0,
  ETH_CONNECT     = 1,
  ETH_STATE_TOTAL = 2
} EthState_t;

/* State label table — indexed by EthState_t. */
static const char *const s_labels[ETH_STATE_TOTAL] =
{
  /* 0 ETH_INIT    */ "INIT...",
  /* 1 ETH_CONNECT */ "CONNECTING...",
};

/* ------------------------------------------------------------------
 * Private helper — format a MAC address struct into "XX:XX:XX:XX:XX:XX".
 * ------------------------------------------------------------------ */
/* Format MAC as "XXXXXXXXXXXX" (12 hex chars, no separators) to match
 * the SMCSRuntime.strMAC field format used by MCSSetRuntimeEthernetMAC. */
static void FormatMAC(tpSMCSMACAddress mac, char *out, uint8_t maxLen)
{
  (void) snprintf(out, (size_t) maxLen,
                  "%02X%02X%02X%02X%02X%02X",
                  (unsigned int) mac->bAddress0,
                  (unsigned int) mac->bAddress1,
                  (unsigned int) mac->bAddress2,
                  (unsigned int) mac->bAddress3,
                  (unsigned int) mac->bAddress4,
                  (unsigned int) mac->bAddress5);
}

/* ------------------------------------------------------------------
 * IModemPort_t function implementations
 * ------------------------------------------------------------------ */

static void AdapterOnInit(void *ctx, ISerialPort_t *serialPort)
{
  EthernetNtcipModemAdapterCtx_t *c = (EthernetNtcipModemAdapterCtx_t *) ctx;

  c->serialPort = serialPort;
  MX_LWIP_Init();
}

static uint32_t AdapterGetBaudRate(void *ctx)
{
  (void) ctx;

  return 0U; /* Ethernet — no serial port. */
}

static uint8_t AdapterGetInitialState(void *ctx)
{
  (void) ctx;

  return (uint8_t) ETH_INIT;
}

static uint8_t AdapterPrepareCommand(void       *ctx,
                                     uint8_t state,
                                     const char *apn,
                                     const char *host,
                                     uint16_t serverPort,
                                     char       *outBuf,
                                     uint16_t maxLen)
{
  (void) ctx;
  (void) state;
  (void) apn;
  (void) host;
  (void) serverPort;
  (void) maxLen;

  /* Ethernet NTCIP sends no AT commands. */
  outBuf[0] = '\0';

  return 0U;
}

static void AdapterGetWaitParams(void          *ctx,
                                 uint8_t state,
                                 const char   **kw,
                                 char          *sc,
                                 uint32_t      *tms,
                                 uint8_t       *mr)
{
  (void) ctx;
  (void) state;

  /* Short timeout — HandleResponse polls DHCP state itself. */
  *kw = "OK";
  *sc = '\0';
  *tms = 500U;
  *mr = 1U;
}

static uint8_t AdapterHandleResponse(void        *ctx,
                                     uint8_t state,
                                     const char  *response,
                                     uint8_t responseOk,
                                     ModemInfo_t *pInfo)
{
  (void) ctx;
  (void) response;
  (void) responseOk;

  if ((EthState_t) state == ETH_CONNECT)
  {
    pInfo->bModemAlive = LwIPIsNetifUp();
    pInfo->bModemAliveValid = 1U;

    return MODEM_STATE_CONNECTED;
  }

  /* ETH_INIT — poll DHCP state. */

  if (MCSIsEthernetStaticIP())
  {
    /* Static IP: populate MAC + IP and advance immediately. */
    tpSMCSMACAddress mac = MCSGetEthernetMACAddress();

    FormatMAC(mac, pInfo->strMAC, (uint8_t) sizeof(pInfo->strMAC));

    (void) strncpy(pInfo->strLocalIPv4,
                   EthGetDHCPIPv4(),
                   sizeof(pInfo->strLocalIPv4) - 1U);
    pInfo->strLocalIPv4[sizeof(pInfo->strLocalIPv4) - 1U] = '\0';
    pInfo->bLocalIPv4Valid = 1U;

    (void) strncpy(pInfo->strJobLabel,
                   pInfo->strLocalIPv4,
                   sizeof(pInfo->strJobLabel) - 1U);
    pInfo->strJobLabel[sizeof(pInfo->strJobLabel) - 1U] = '\0';

    return (uint8_t) ETH_CONNECT;
  }

  EthProcessDHCP();

  switch (EthGetDHCPState())
  {
      case DHCP_ADDRESS_ASSIGNED:
      {
        tpSMCSMACAddress mac = MCSGetEthernetMACAddress();

        FormatMAC(mac, pInfo->strMAC, (uint8_t) sizeof(pInfo->strMAC));

        (void) strncpy(pInfo->strLocalIPv4,
                       EthGetDHCPIPv4(),
                       sizeof(pInfo->strLocalIPv4) - 1U);
        pInfo->strLocalIPv4[sizeof(pInfo->strLocalIPv4) - 1U] = '\0';
        pInfo->bLocalIPv4Valid = 1U;

        (void) strncpy(pInfo->strJobLabel, "DHCP ADDR. ASSIGNED",
                       sizeof(pInfo->strJobLabel) - 1U);
        pInfo->strJobLabel[sizeof(pInfo->strJobLabel) - 1U] = '\0';

        return (uint8_t) ETH_CONNECT;
      }

      case DHCP_OFF:
      {
        (void) strncpy(pInfo->strJobLabel, "DHCP OFF",
                       sizeof(pInfo->strJobLabel) - 1U);
        pInfo->strJobLabel[sizeof(pInfo->strJobLabel) - 1U] = '\0';
        break;
      }

      case DHCP_START:
      {
        (void) strncpy(pInfo->strJobLabel, "DHCP STARTING...",
                       sizeof(pInfo->strJobLabel) - 1U);
        pInfo->strJobLabel[sizeof(pInfo->strJobLabel) - 1U] = '\0';
        break;
      }

      case DHCP_WAIT_ADDRESS:
      {
        (void) strncpy(pInfo->strJobLabel, "DHCP WAITING ADDR.",
                       sizeof(pInfo->strJobLabel) - 1U);
        pInfo->strJobLabel[sizeof(pInfo->strJobLabel) - 1U] = '\0';
        break;
      }

      case DHCP_TIMEOUT:
      {
        (void) strncpy(pInfo->strJobLabel, "DHCP TIMEOUT",
                       sizeof(pInfo->strJobLabel) - 1U);
        pInfo->strJobLabel[sizeof(pInfo->strJobLabel) - 1U] = '\0';
        break;
      }

      case DHCP_LINK_DOWN:
      {
        (void) strncpy(pInfo->strJobLabel, "DHCP LINK DOWN",
                       sizeof(pInfo->strJobLabel) - 1U);
        pInfo->strJobLabel[sizeof(pInfo->strJobLabel) - 1U] = '\0';
        break;
      }

      default:
      {
        break;
      }
  } /* switch */

  /* Stay in INIT until DHCP assigns an address. */
  return (uint8_t) ETH_INIT;
} /* AdapterHandleResponse */

static const char *AdapterGetStateLabel(void *ctx, uint8_t state)
{
  (void) ctx;
  if (state < (uint8_t) ETH_STATE_TOTAL)
  {
    return s_labels[state];
  }

  return "ETH ?";
}

static uint8_t AdapterIsDisconnected(void       *ctx,
                                     const char *data,
                                     uint16_t len)
{
  /* Ethernet NTCIP — no disconnect keywords; LwIP handles link loss. */
  (void) ctx;
  (void) data;
  (void) len;

  return 0U;
}

static uint8_t AdapterGetGreetingType(void *ctx)
{
  (void) ctx;

  return MODEM_GREETING_ETH_MAC;
}

static uint8_t AdapterOnConnect(void *ctx)
{
  (void) ctx;
  (void) MCSSNMPClientStart();
  (void) MCSUDPProbeStart();
  (void) MCSTCPClientConnect();

  if (ServerSettingsMCSAvailableGet())
  {
    if (TCPClientIsConnected())
    {
      if (!MCSAsynchConnectedGet())
      {
        MCSSetRuntimeLocalIPv4(TCPClientGetLocalIPv4Str());
        MCSSetRuntimeRemoteIPv4(TCPClientGetRemoteIPv4Str());
        MCSJobAdd(MCSGetRuntimeRemoteIPv4());

        if (!MCSAsynchStart(MODEM_GREETING_ETH_MAC))
        {
          MCSJobAdd("MCS CON. ERROR");

          return 0U;
        }

        MCSJobAdd("MCS CON. SUC.");
      }
    }
    else
    {
      MCSJobAdd("TCP NOT CON.");

      return 0U;
    }
  }

  return 1U;
}

static void AdapterOnMaintain(void *ctx)
{
  (void) ctx;
  MCSUDPConnectionMaintain();
  if (MCSAsynchConnectedGet())
  {
    MCSAsynchCheckConnectionTimeout();
  }
}

static void AdapterOnRx(void *ctx, const uint8_t *data, uint16_t len)
{
  (void) ctx;
  (void) data;
  (void) len;
  /* Ethernet — no serial RX. */
}

static void AdapterOnDisconnect(void *ctx)
{
  (void) ctx;
  if (MCSAsynchConnectedGet())
  {
    MCSAsynchStop(MCS_ASYNCH_DISC_TYPE_CON_INFO_CHANGED);
  }

  if (SNMPClientIsStarted())
  {
    SNMPClientStop();
  }

  if (UDPProbeIsStarted())
  {
    UDPProbeStop();
  }

  if (TCPClientIsConnected())
  {
    TCPClientDisconnect();
  }
}

static uint8_t AdapterSend(void *ctx, const uint8_t *data, uint16_t len)
{
  (void) ctx;
  (void) len;
  TCPClientSendData((tpSMCSAsynchRxTxMsg) data);

  return 1U;
}

/* ------------------------------------------------------------------
 * Public API
 * ------------------------------------------------------------------ */

void EthernetNtcipModemAdapterInit(EthernetNtcipModemAdapterCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
}

IModemPort_t EthernetNtcipModemAdapterCreatePort(
  EthernetNtcipModemAdapterCtx_t *ctx)
{
  IModemPort_t port;

  port.ctx = ctx;
  port.OnInit = AdapterOnInit;
  port.GetBaudRate = AdapterGetBaudRate;
  port.GetInitialState = AdapterGetInitialState;
  port.PrepareCommand = AdapterPrepareCommand;
  port.GetWaitParams = AdapterGetWaitParams;
  port.HandleResponse = AdapterHandleResponse;
  port.GetStateLabel = AdapterGetStateLabel;
  port.IsDisconnected = AdapterIsDisconnected;
  port.GetGreetingType = AdapterGetGreetingType;
  port.OnConnect = AdapterOnConnect;
  port.OnMaintain = AdapterOnMaintain;
  port.OnRx = AdapterOnRx;
  port.OnDisconnect = AdapterOnDisconnect;
  port.Send = AdapterSend;

  return port;
}
