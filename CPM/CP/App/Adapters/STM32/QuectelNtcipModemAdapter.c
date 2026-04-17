/* App/Adapters/STM32/QuectelNtcipModemAdapter.c
 *
 * IModemPort concrete implementation for the Quectel EG915U NTCIP
 * module (115200 baud, PPP dial-up, LwIP over PPPoS).
 *
 * OnInit() starts LwIP and creates the PPPoS asynch task.
 * The DIAL state calls PPPOSAsynchStart() and polls for connection.
 * The CONNECT state returns MODEM_STATE_CONNECTED so the coordinator
 * triggers MCSTryConnect() for SNMP/TCP client startup.
 */
#include "QuectelNtcipModemAdapter.h"

#include <stdio.h>
#include <string.h>

#include "cmsis_os.h"      /* osDelay, osKernelGetTickCount */
#include "lwip.h"          /* MX_LWIP_Init                  */
#include "PPPOSAsynch.h"   /* PPPOSAsynchCreate, PPPOSAsynchStart, etc. */
#include "MCS.h"
#include "MCSAsynch.h"
#include "Platform/STM32/Services/Control/Legacy/data.h"
#include "snmp_client.h"
#include "udp_probe.h"
#include "tcp_client.h"
#include "program.h"

/* ------------------------------------------------------------------
 * Private state enum — authoritative for Quectel EG915U NTCIP adapter.
 * ------------------------------------------------------------------ */
typedef enum
{
  QNTCIP_CMD_MODE         = 0,
  QNTCIP_RESET_MODEM      = 1,
  QNTCIP_FACTORY_RESET    = 2,
  QNTCIP_AT_COMMAND       = 3,
  QNTCIP_MANUFACTURER     = 4,
  QNTCIP_GET_IMEI         = 5,
  QNTCIP_CHECK_SIG_QUA    = 6,
  QNTCIP_CHECK_SIM        = 7,
  QNTCIP_CHECK_GSM_NET    = 8,
  QNTCIP_CHECK_GPRS_NET   = 9,
  QNTCIP_GET_OPERATOR     = 10,
  QNTCIP_DEACTIVATE_CTX   = 11,
  QNTCIP_CLEAR_CTX        = 12,
  QNTCIP_DETACH_NET       = 13,
  QNTCIP_SETUP_APN        = 14,
  QNTCIP_ATTACH_NET       = 15,
  QNTCIP_ACTIVATE_CTX     = 16,
  QNTCIP_DIAL             = 17,
  QNTCIP_CONNECT          = 18,
  QNTCIP_STATE_TOTAL      = 19
} QNtcipState_t;

/* ------------------------------------------------------------------
 * Per-state wait timeout constants (milliseconds).
 * ------------------------------------------------------------------ */
#define QNTCIP_TIMEOUT_1S   1000U
#define QNTCIP_TIMEOUT_10S  10000U
#define QNTCIP_TIMEOUT_15S  15000U

/* ------------------------------------------------------------------
 * Retry limit constants.
 * ------------------------------------------------------------------ */
#define QNTCIP_MAX_CMD_TRIES  2U
#define QNTCIP_MAX_SIM_TRIES  5U
#define QNTCIP_MAX_SIG_TRIES  5U
#define QNTCIP_MAX_NET_TRIES  60U

#define QNTCIP_SIGNAL_ERR  99U
#define QNTCIP_SIGNAL_MAX  31U
#define QNTCIP_SIGNAL_MIN  0U

#define QNTCIP_MANUFACTURER_ID1 "Quectel_Ltd"
#define QNTCIP_MANUFACTURER_ID2 "Quectel"

/* ------------------------------------------------------------------
 * State label table — indexed by QNtcipState_t.
 * Entries match MCS.c pStrQuectelNTCIP[] exactly.
 * ------------------------------------------------------------------ */
static const char *const s_labels[QNTCIP_STATE_TOTAL] =
{
  /* 0  QNTCIP_CMD_MODE       */ "COMMAND MODE",
  /* 1  QNTCIP_RESET_MODEM    */ "RESET MODEM",
  /* 2  QNTCIP_FACTORY_RESET  */ "FACTORY RESET",
  /* 3  QNTCIP_AT_COMMAND     */ "AT COMMAND",
  /* 4  QNTCIP_MANUFACTURER   */ "MANUFACTURER ID",
  /* 5  QNTCIP_GET_IMEI       */ "GET IMEI",
  /* 6  QNTCIP_CHECK_SIG_QUA  */ "CHECK SIGNAL Q.",
  /* 7  QNTCIP_CHECK_SIM      */ "CHECK SIM",
  /* 8  QNTCIP_CHECK_GSM_NET  */ "CHECK GSM N.",
  /* 9  QNTCIP_CHECK_GPRS_NET */ "CHECK GPRS N.",
  /* 10 QNTCIP_GET_OPERATOR   */ "GET OPERATOR",
  /* 11 QNTCIP_DEACTIVATE_CTX */ "DEACT. CNTXT",
  /* 12 QNTCIP_CLEAR_CTX      */ "CLEAR. CNTXT",
  /* 13 QNTCIP_DETACH_NET     */ "DETACH NET.",
  /* 14 QNTCIP_SETUP_APN      */ "SETUP APN",
  /* 15 QNTCIP_ATTACH_NET     */ "ATTACH NET.",
  /* 16 QNTCIP_ACTIVATE_CTX   */ "ACT. CNTXT",
  /* 17 QNTCIP_DIAL           */ "DIALING...",
  /* 18 QNTCIP_CONNECT        */ "CONNECTING...",
};

/* ------------------------------------------------------------------
 * Private helpers
 * ------------------------------------------------------------------ */

static uint8_t ParseSignalQuality(const char *str)
{
  uint8_t len = (uint8_t) strlen(str);
  uint8_t val = 0U;

  if (len >= 1U)
  {
    val = (uint8_t) (str[0] - '0');
  }

  if (len >= 2U)
  {
    val = (uint8_t) ((val * 10U) + (uint8_t) (str[1] - '0'));
  }

  return val;
}

static void ExtractOperator(const char *strIn,
                            char       *strOut,
                            uint8_t maxLen)
{
  const char *pStart = strchr(strIn, '"');
  uint8_t len = 0U;

  if (pStart != NULL)
  {
    pStart++;
    while ((*pStart != '"') && (*pStart != '\0')
           && (len < (uint8_t) (maxLen - 1U)))
    {
      strOut[len] = *pStart;
      len++;
      pStart++;
    }
  }

  if (len == 0U)
  {
    (void) strncpy(strOut, "UNKNOWN", (size_t) (maxLen - 1U));
    strOut[maxLen - 1U] = '\0';
  }
  else
  {
    strOut[len] = '\0';
  }
}

static uint8_t HandleNetworkResponse(QuectelNtcipModemAdapterCtx_t *ctx,
                                     QNtcipState_t state,
                                     const char                    *response,
                                     ModemInfo_t                   *pInfo)
{
  QNtcipState_t nextState = (state == QNTCIP_CHECK_GSM_NET)
                            ? QNTCIP_CHECK_GPRS_NET
                            : QNTCIP_GET_OPERATOR;

  if ((strcmp(response, "0,1") == 0) || (strcmp(response, "0,5") == 0))
  {
    const char *label = (strcmp(response, "0,1") == 0)
                        ? "REGISTERED HOME"
                        : "REGISTERED ROAMING";

    (void) strncpy(pInfo->strJobLabel, label,
                   sizeof(pInfo->strJobLabel) - 1U);
    pInfo->strJobLabel[sizeof(pInfo->strJobLabel) - 1U] = '\0';
    ctx->bNetRetries = 0U;

    return (uint8_t) nextState;
  }

  if ((strcmp(response, "0,3") == 0) || (strcmp(response, "0,4") == 0))
  {
    const char *label = (strcmp(response, "0,3") == 0)
                        ? "REG. DENIED"
                        : "REG. UNKNOWN";

    (void) strncpy(pInfo->strJobLabel, label,
                   sizeof(pInfo->strJobLabel) - 1U);
    pInfo->strJobLabel[sizeof(pInfo->strJobLabel) - 1U] = '\0';
    ctx->bNetRetries = 0U;

    return (uint8_t) QNTCIP_CMD_MODE;
  }

  ctx->bNetRetries++;
  (void) snprintf(pInfo->strJobLabel, sizeof(pInfo->strJobLabel),
                  "NOT REG. SEARCH. %u",
                  (unsigned int) ctx->bNetRetries);

  if (ctx->bNetRetries > QNTCIP_MAX_NET_TRIES)
  {
    ctx->bNetRetries = 0U;

    return (uint8_t) nextState;
  }

  return (uint8_t) state;
} /* HandleNetworkResponse */

/* ------------------------------------------------------------------
 * IModemPort_t function implementations
 * ------------------------------------------------------------------ */

static void AdapterOnInit(void *ctx, ISerialPort_t *serialPort)
{
  QuectelNtcipModemAdapterCtx_t *c = (QuectelNtcipModemAdapterCtx_t *) ctx;

  c->serialPort = serialPort;
  MX_LWIP_Init();
  PPPOSAsynchCreate(serialPort);
}

static uint32_t AdapterGetBaudRate(void *ctx)
{
  (void) ctx;

  return 115200U;
}

static uint8_t AdapterGetInitialState(void *ctx)
{
  (void) ctx;

  return (uint8_t) QNTCIP_CMD_MODE;
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
  (void) host;
  (void) serverPort;

  switch ((QNtcipState_t) state)
  {
      case QNTCIP_CMD_MODE:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "+++");

        return 1U;
      }

      case QNTCIP_RESET_MODEM:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+cfun=1,1\r");

        return 1U;
      }

      case QNTCIP_FACTORY_RESET:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at&f\r\n");

        return 1U;
      }

      case QNTCIP_AT_COMMAND:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "ate0\r");

        return 1U;
      }

      case QNTCIP_MANUFACTURER:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+cgmi\r");

        return 1U;
      }

      case QNTCIP_GET_IMEI:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+cgsn\r");

        return 1U;
      }

      case QNTCIP_CHECK_SIG_QUA:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+csq\r");

        return 1U;
      }

      case QNTCIP_CHECK_SIM:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+cpin?\r");

        return 1U;
      }

      case QNTCIP_CHECK_GSM_NET:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+creg?\r");

        return 1U;
      }

      case QNTCIP_CHECK_GPRS_NET:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+cgreg?\r");

        return 1U;
      }

      case QNTCIP_GET_OPERATOR:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+cops?\r");

        return 1U;
      }

      case QNTCIP_DEACTIVATE_CTX:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+cgact=0,1\r");

        return 1U;
      }

      case QNTCIP_CLEAR_CTX:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+cgdcont=1\r");

        return 1U;
      }

      case QNTCIP_DETACH_NET:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+cgatt=0\r");

        return 1U;
      }

      case QNTCIP_SETUP_APN:
      {
        (void) snprintf(outBuf, (size_t) maxLen,
                        "at+cgdcont=1,\"PPP\",\"%s\"\r", apn);

        return 1U;
      }

      case QNTCIP_ATTACH_NET:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+cgatt=1\r");

        return 1U;
      }

      case QNTCIP_ACTIVATE_CTX:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+cgact=1,1\r");

        return 1U;
      }

      case QNTCIP_DIAL:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "atd*99#\r");

        return 1U;
      }

      case QNTCIP_CONNECT:
      {
        /* No AT command — just wait for coordinator to advance. */
        outBuf[0] = '\0';

        return 0U;
      }

      default:
      {
        outBuf[0] = '\0';

        return 0U;
      }
  } /* switch */
} /* AdapterPrepareCommand */

static void AdapterGetWaitParams(void          *ctx,
                                 uint8_t state,
                                 const char   **kw,
                                 char          *sc,
                                 uint32_t      *tms,
                                 uint8_t       *mr)
{
  (void) ctx;

  *sc = '\0';
  *mr = QNTCIP_MAX_CMD_TRIES;
  *kw = "OK";
  *tms = QNTCIP_TIMEOUT_1S;

  switch ((QNtcipState_t) state)
  {
      case QNTCIP_CMD_MODE:
      {
        *mr = 1U;
        break;
      }

      case QNTCIP_RESET_MODEM:
      {
        *tms = QNTCIP_TIMEOUT_10S;
        break;
      }

      case QNTCIP_FACTORY_RESET:
      case QNTCIP_AT_COMMAND:
      case QNTCIP_DEACTIVATE_CTX:
      case QNTCIP_CLEAR_CTX:
      case QNTCIP_DETACH_NET:
      case QNTCIP_SETUP_APN:
      case QNTCIP_ATTACH_NET:
      case QNTCIP_ACTIVATE_CTX:
      {
        break; /* default: "OK", 1 s, 2 retries */
      }

      case QNTCIP_MANUFACTURER:
      case QNTCIP_GET_IMEI:
      {
        *kw = "\r\n";
        *sc = '\r';
        break;
      }

      case QNTCIP_CHECK_SIG_QUA:
      {
        *kw = "+CSQ: ";
        *sc = ',';
        *mr = QNTCIP_MAX_SIG_TRIES;
        break;
      }

      case QNTCIP_CHECK_SIM:
      {
        *kw = "+CPIN: ";
        *sc = '\r';
        *mr = QNTCIP_MAX_SIM_TRIES;
        break;
      }

      case QNTCIP_CHECK_GSM_NET:
      {
        *kw = "+CREG: ";
        *sc = '\r';
        *mr = QNTCIP_MAX_NET_TRIES;
        break;
      }

      case QNTCIP_CHECK_GPRS_NET:
      {
        *kw = "+CGREG: ";
        *sc = '\r';
        *mr = QNTCIP_MAX_NET_TRIES;
        break;
      }

      case QNTCIP_GET_OPERATOR:
      {
        *kw = "+COPS: ";
        *sc = '\r';
        break;
      }

      case QNTCIP_DIAL:
      {
        *kw = "CONNECT";
        *tms = QNTCIP_TIMEOUT_15S;
        break;
      }

      case QNTCIP_CONNECT:
      {
        /* HandleResponse is called immediately (no key to wait for). */
        *mr = 1U;
        break;
      }

      default:
      {
        break;
      }
  } /* switch */
} /* AdapterGetWaitParams */

static uint8_t AdapterHandleResponse(void        *ctx,
                                     uint8_t state,
                                     const char  *response,
                                     uint8_t responseOk,
                                     ModemInfo_t *pInfo)
{
  QuectelNtcipModemAdapterCtx_t *c = (QuectelNtcipModemAdapterCtx_t *) ctx;

  /* CMD_MODE always advances. */
  if ((QNtcipState_t) state == QNTCIP_CMD_MODE)
  {
    pInfo->bModemAlive = 0U;
    pInfo->bModemAliveValid = 1U;

    return (uint8_t) QNTCIP_RESET_MODEM;
  }

  /* CONNECT state always returns MODEM_STATE_CONNECTED. */
  if ((QNtcipState_t) state == QNTCIP_CONNECT)
  {
    return MODEM_STATE_CONNECTED;
  }

  if (responseOk == 0U)
  {
    (void) strncpy(pInfo->strJobLabel, "NO RESPONSE",
                   sizeof(pInfo->strJobLabel) - 1U);
    pInfo->strJobLabel[sizeof(pInfo->strJobLabel) - 1U] = '\0';

    return MODEM_STATE_FAILED;
  }

  switch ((QNtcipState_t) state)
  {
      case QNTCIP_FACTORY_RESET:
      case QNTCIP_AT_COMMAND:
      case QNTCIP_DEACTIVATE_CTX:
      case QNTCIP_CLEAR_CTX:
      case QNTCIP_DETACH_NET:
      case QNTCIP_SETUP_APN:
      case QNTCIP_ATTACH_NET:
      case QNTCIP_ACTIVATE_CTX:
      {
        return (uint8_t) ((uint8_t) state + 1U);
      }

      case QNTCIP_RESET_MODEM:
      {
        osDelay(QNTCIP_TIMEOUT_10S);

        return (uint8_t) QNTCIP_FACTORY_RESET;
      }

      case QNTCIP_MANUFACTURER:
      {
        uint8_t alive = ((strcmp(response, QNTCIP_MANUFACTURER_ID1) == 0)
                         || (strcmp(response, QNTCIP_MANUFACTURER_ID2) == 0))
                        ? 1U : 0U;

        pInfo->bModemAlive = alive;
        pInfo->bModemAliveValid = 1U;
        if (alive != 0U)
        {
          (void) strncpy(pInfo->strJobLabel, "QUECTEL",
                         sizeof(pInfo->strJobLabel) - 1U);
          pInfo->strJobLabel[sizeof(pInfo->strJobLabel) - 1U] = '\0';
        }

        return (uint8_t) QNTCIP_GET_IMEI;
      }

      case QNTCIP_GET_IMEI:
      {
        (void) strncpy(pInfo->strIMEI, response,
                       sizeof(pInfo->strIMEI) - 1U);
        pInfo->strIMEI[sizeof(pInfo->strIMEI) - 1U] = '\0';

        return (uint8_t) QNTCIP_CHECK_SIG_QUA;
      }

      case QNTCIP_CHECK_SIG_QUA:
      {
        uint8_t quality = ParseSignalQuality(response);

        (void) strncpy(pInfo->strJobLabel, response,
                       sizeof(pInfo->strJobLabel) - 1U);
        pInfo->strJobLabel[sizeof(pInfo->strJobLabel) - 1U] = '\0';

        if (quality != QNTCIP_SIGNAL_ERR)
        {
          if (quality > QNTCIP_SIGNAL_MAX)
          {
            quality = QNTCIP_SIGNAL_MAX;
          }

          pInfo->bSignalQuality = quality;
          pInfo->bSignalQualityValid = 1U;
          c->bSigQuaRetries = 0U;

          return (uint8_t) QNTCIP_CHECK_SIM;
        }

        c->bSigQuaRetries++;
        if (c->bSigQuaRetries > QNTCIP_MAX_SIG_TRIES)
        {
          pInfo->bSignalQuality = QNTCIP_SIGNAL_MIN;
          pInfo->bSignalQualityValid = 1U;
          c->bSigQuaRetries = 0U;

          return (uint8_t) QNTCIP_CHECK_SIM;
        }

        return (uint8_t) QNTCIP_CHECK_SIG_QUA;
      }

      case QNTCIP_CHECK_SIM:
      {
        uint8_t ready = (strcmp(response, "READY") == 0) ? 1U : 0U;

        pInfo->bSimReady = ready;
        pInfo->bSimReadyValid = 1U;

        if (ready != 0U)
        {
          (void) strncpy(pInfo->strJobLabel, "READY",
                         sizeof(pInfo->strJobLabel) - 1U);
          pInfo->strJobLabel[sizeof(pInfo->strJobLabel) - 1U] = '\0';
          c->bSimRetries = 0U;

          return (uint8_t) QNTCIP_CHECK_GSM_NET;
        }

        c->bSimRetries++;
        if (c->bSimRetries > QNTCIP_MAX_SIM_TRIES)
        {
          c->bSimRetries = 0U;

          return (uint8_t) QNTCIP_CHECK_GSM_NET;
        }

        return (uint8_t) QNTCIP_CHECK_SIM;
      }

      case QNTCIP_CHECK_GSM_NET:
      case QNTCIP_CHECK_GPRS_NET:
      {
        return HandleNetworkResponse(c, (QNtcipState_t) state,
                                     response, pInfo);
      }

      case QNTCIP_GET_OPERATOR:
      {
        ExtractOperator(response, pInfo->strOperator,
                        (uint8_t) sizeof(pInfo->strOperator));
        (void) strncpy(pInfo->strJobLabel, pInfo->strOperator,
                       sizeof(pInfo->strJobLabel) - 1U);
        pInfo->strJobLabel[sizeof(pInfo->strJobLabel) - 1U] = '\0';

        return (uint8_t) QNTCIP_DEACTIVATE_CTX;
      }

      case QNTCIP_DIAL:
      {
        /* "CONNECT" received — attempt PPP negotiation. */
        uint32_t startTick = osKernelGetTickCount();
        uint8_t connected = 0U;
        const char *label = "PPP CON. ERR.";

        if (PPPOSAsynchStart() == 0) /* err_t ERR_OK == 0 */
        {
          while (!PPPOSAsynchGetConnected()
                 && ((osKernelGetTickCount() - startTick)
                     < PPPOS_ASYNCH_TIMEOUT_CONNECTION))
          {
            osDelay(100U);
          }

          if (PPPOSAsynchGetConnected())
          {
            connected = 1U;
            label = "PPP CON. SUC.";
          }
        }

        (void) strncpy(pInfo->strJobLabel, label,
                       sizeof(pInfo->strJobLabel) - 1U);
        pInfo->strJobLabel[sizeof(pInfo->strJobLabel) - 1U] = '\0';

        if (connected != 0U)
        {
          return (uint8_t) QNTCIP_CONNECT;
        }

        PPPOSAsynchStop();

        return (uint8_t) QNTCIP_CMD_MODE;
      }

      default:
      {
        return (uint8_t) QNTCIP_CMD_MODE;
      }
  } /* switch */
} /* AdapterHandleResponse */

static uint8_t AdapterOnConnect(void *ctx)
{
  (void) ctx;

  if (!MCSSNMPClientStart())
  {
    return 0U;
  }

  if (!MCSUDPProbeStart())
  {
    return 0U;
  }

  if (!MCSTCPClientConnect())
  {
    return 0U;
  }

  if (ServerSettingsMCSAvailableGet())
  {
    if (TCPClientIsConnected())
    {
      if (!MCSAsynchConnectedGet())
      {
        MCSSetRuntimeLocalIPv4(TCPClientGetLocalIPv4Str());
        MCSSetRuntimeRemoteIPv4(TCPClientGetRemoteIPv4Str());
        MCSJobAdd(MCSGetRuntimeRemoteIPv4());

        if (!MCSAsynchStart(MODEM_GREETING_IMEI))
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
} /* AdapterOnConnect */

static void AdapterOnMaintain(void *ctx)
{
  (void) ctx;

  if (SNMPClientIsStarted())
  {
    /* SNMP maintenance is handled by LwIP/SNMP stack. */
  }

  if (UDPProbeIsStarted())
  {
    /* UDP probe maintenance. Note: SMCSRuntime usage here might require
     * visibility or moving the counters into the context.
     * For now we use the global MCS helpers if available, or just implement it.
     */
    MCSUDPConnectionMaintain();
  }

  if (PPPOSAsynchGetConnected())
  {
    PPPOSAsynchCheckConnectionTimeout();
  }

  if (MCSAsynchConnectedGet())
  {
    if (MCSAsynchIsRemoteEndClosed())
    {
      MCSAsynchStop(MCS_ASYNCH_DISC_TYPE_REMOTE_END_DISCON);
    }

    MCSAsynchCheckConnectionTimeout();
  }
}

static void AdapterOnRx(void *ctx, const uint8_t *data, uint16_t len)
{
  (void) ctx;
  if (PPPOSAsynchIsDailed())
  {
    (void) PPPOSAsynchReqRxMsg((uint8_t *) data, len);
  }
  else
  {
    MCSRingBufferWrite(data, len);
  }
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

  if (PPPOSAsynchGetConnected())
  {
    PPPOSAsynchStop();
  }
}

static uint8_t AdapterSend(void *ctx, const uint8_t *data, uint16_t len)
{
  (void) ctx;
  (void) len;
  TCPClientSendData((tpSMCSAsynchRxTxMsg) data);

  return 1U;
}

static const char *AdapterGetStateLabel(void *ctx, uint8_t state)
{
  (void) ctx;
  if (state < (uint8_t) QNTCIP_STATE_TOTAL)
  {
    return s_labels[state];
  }

  return "QNTCIP ?";
}

static uint8_t AdapterIsDisconnected(void       *ctx,
                                     const char *data,
                                     uint16_t len)
{
  (void) ctx;
  (void) len;

  return ((strstr(data, "NO CARRIER")   != NULL)
          || (strstr(data, "CLOSED")    != NULL)
          || (strstr(data, "+UUSOCL: 0") != NULL)
          || (strstr(data, "+PDP DEACT") != NULL)
          || (strstr(data, "DISCONNECT") != NULL))
         ? 1U : 0U;
}

static uint8_t AdapterGetGreetingType(void *ctx)
{
  (void) ctx;

  return MODEM_GREETING_IMEI;
}

/* ------------------------------------------------------------------
 * Public API
 * ------------------------------------------------------------------ */

void QuectelNtcipModemAdapterInit(QuectelNtcipModemAdapterCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
}

IModemPort_t QuectelNtcipModemAdapterCreatePort(
  QuectelNtcipModemAdapterCtx_t *ctx)
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
