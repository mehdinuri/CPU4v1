/* App/Adapters/STM32/QuectelModemAdapter.c
 *
 * IModemPort concrete implementation for the Quectel M95 GPRS module
 * (115200 baud, TCP socket via AT+QIOPEN command).
 *
 * All Quectel M95-specific knowledge lives here: AT command strings,
 * per-state response keywords and timeouts, state-transition logic.
 */
#include "QuectelModemAdapter.h"

#include <stdio.h>
#include <string.h>

#include "cmsis_os.h" /* osDelay — post-reset stabilisation wait */
#include "MCS.h"
#include "MCSAsynch.h"

/* ------------------------------------------------------------------
 * Private state enum — authoritative for Quectel M95 adapter.
 * ------------------------------------------------------------------ */
typedef enum
{
  QUECTEL_CMD_MODE         = 0,
  QUECTEL_CLOSE_SOCKET     = 1,
  QUECTEL_RESET_MODEM      = 2,
  QUECTEL_AT_COMMAND       = 3,
  QUECTEL_MANUFACTURER     = 4,
  QUECTEL_GET_IMEI         = 5,
  QUECTEL_CHECK_SIG_QUA    = 6,
  QUECTEL_CHECK_SIM        = 7,
  QUECTEL_CHECK_GSM_NET    = 8,
  QUECTEL_CHECK_GPRS_NET   = 9,
  QUECTEL_GET_OPERATOR     = 10,
  QUECTEL_DEACTIVATE_CTX   = 11,
  QUECTEL_SETUP_APN        = 12,
  QUECTEL_ACTIVATE_CTX     = 13,
  QUECTEL_OPEN_SOCKET      = 14,
  QUECTEL_STATE_TOTAL      = 15
} QuectelState_t;

/* ------------------------------------------------------------------
 * Per-state wait timeout constants (milliseconds).
 * ------------------------------------------------------------------ */
#define QUECTEL_TIMEOUT_1S   1000U
#define QUECTEL_TIMEOUT_10S  10000U
#define QUECTEL_TIMEOUT_15S  15000U
#define QUECTEL_TIMEOUT_30S  30000U

/* ------------------------------------------------------------------
 * Retry limit constants.
 * ------------------------------------------------------------------ */
#define QUECTEL_MAX_CMD_TRIES  2U
#define QUECTEL_MAX_SIM_TRIES  5U
#define QUECTEL_MAX_SIG_TRIES  5U
#define QUECTEL_MAX_NET_TRIES  60U

#define QUECTEL_SIGNAL_ERR  99U
#define QUECTEL_SIGNAL_MAX  31U
#define QUECTEL_SIGNAL_MIN  0U

#define QUECTEL_MANUFACTURER_ID1 "Quectel_Ltd"
#define QUECTEL_MANUFACTURER_ID2 "Quectel"

/* ------------------------------------------------------------------
 * State label table — indexed by QuectelState_t.
 * Entries match MCS.c pStrQuectel[] exactly.
 * ------------------------------------------------------------------ */
static const char *const s_labels[QUECTEL_STATE_TOTAL] =
{
  /* 0  QUECTEL_CMD_MODE       */ "COMMAND MODE",
  /* 1  QUECTEL_CLOSE_SOCKET   */ "CLOSE SOCKET",
  /* 2  QUECTEL_RESET_MODEM    */ "RESET MODEM",
  /* 3  QUECTEL_AT_COMMAND     */ "AT COMMAND",
  /* 4  QUECTEL_MANUFACTURER   */ "MANUFACTURER ID",
  /* 5  QUECTEL_GET_IMEI       */ "GET IMEI",
  /* 6  QUECTEL_CHECK_SIG_QUA  */ "CHECK SIGNAL Q.",
  /* 7  QUECTEL_CHECK_SIM      */ "CHECK SIM",
  /* 8  QUECTEL_CHECK_GSM_NET  */ "CHECK GSM N.",
  /* 9  QUECTEL_CHECK_GPRS_NET */ "CHECK GPRS N.",
  /* 10 QUECTEL_GET_OPERATOR   */ "GET OPERATOR",
  /* 11 QUECTEL_DEACTIVATE_CTX */ "DEACT. CONTEXT",
  /* 12 QUECTEL_SETUP_APN      */ "SETUP APN",
  /* 13 QUECTEL_ACTIVATE_CTX   */ "ACT. CONTEXT",
  /* 14 QUECTEL_OPEN_SOCKET    */ "OPEN SOCKET",
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

static uint8_t HandleNetworkResponse(QuectelModemAdapterCtx_t *ctx,
                                     QuectelState_t state,
                                     const char               *response,
                                     ModemInfo_t              *pInfo)
{
  QuectelState_t nextState = (state == QUECTEL_CHECK_GSM_NET)
                             ? QUECTEL_CHECK_GPRS_NET
                             : QUECTEL_GET_OPERATOR;

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

    return (uint8_t) QUECTEL_CMD_MODE;
  }

  ctx->bNetRetries++;
  (void) snprintf(pInfo->strJobLabel, sizeof(pInfo->strJobLabel),
                  "NOT REG. SEARCH. %u",
                  (unsigned int) ctx->bNetRetries);

  if (ctx->bNetRetries > QUECTEL_MAX_NET_TRIES)
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
  QuectelModemAdapterCtx_t *c = (QuectelModemAdapterCtx_t *) ctx;

  c->serialPort = serialPort;
}

static uint32_t AdapterGetBaudRate(void *ctx)
{
  (void) ctx;

  return 115200U;
}

static uint8_t AdapterGetInitialState(void *ctx)
{
  (void) ctx;

  return (uint8_t) QUECTEL_CMD_MODE;
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

  switch ((QuectelState_t) state)
  {
      case QUECTEL_CMD_MODE:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "+++");

        return 1U;
      }

      case QUECTEL_CLOSE_SOCKET:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+qiclose=0\r");

        return 1U;
      }

      case QUECTEL_RESET_MODEM:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+cfun=1,1\r");

        return 1U;
      }

      case QUECTEL_AT_COMMAND:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "ate0\r");

        return 1U;
      }

      case QUECTEL_MANUFACTURER:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+cgmi\r");

        return 1U;
      }

      case QUECTEL_GET_IMEI:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+cgsn\r");

        return 1U;
      }

      case QUECTEL_CHECK_SIG_QUA:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+csq\r");

        return 1U;
      }

      case QUECTEL_CHECK_SIM:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+cpin?\r");

        return 1U;
      }

      case QUECTEL_CHECK_GSM_NET:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+creg?\r");

        return 1U;
      }

      case QUECTEL_CHECK_GPRS_NET:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+cgreg?\r");

        return 1U;
      }

      case QUECTEL_GET_OPERATOR:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+cops?\r");

        return 1U;
      }

      case QUECTEL_DEACTIVATE_CTX:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+qideact=1\r");

        return 1U;
      }

      case QUECTEL_SETUP_APN:
      {
        (void) snprintf(outBuf, (size_t) maxLen,
                        "at+qicsgp=1,1,\"%s\",\"\",\"\",0\r", apn);

        return 1U;
      }

      case QUECTEL_ACTIVATE_CTX:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+qiact=1\r");

        return 1U;
      }

      case QUECTEL_OPEN_SOCKET:
      {
        (void) snprintf(outBuf, (size_t) maxLen,
                        "at+qiopen=1,0,\"TCP\",\"%s\",%u,0,2\r",
                        host, (unsigned int) serverPort);

        return 1U;
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
  *mr = QUECTEL_MAX_CMD_TRIES;
  *kw = "OK";
  *tms = QUECTEL_TIMEOUT_1S;

  switch ((QuectelState_t) state)
  {
      case QUECTEL_CMD_MODE:
      {
        *mr = 1U;
        break;
      }

      case QUECTEL_CLOSE_SOCKET:
      case QUECTEL_RESET_MODEM:
      {
        *tms = QUECTEL_TIMEOUT_10S;
        break;
      }

      case QUECTEL_AT_COMMAND:
      case QUECTEL_SETUP_APN:
      case QUECTEL_DEACTIVATE_CTX:
      {
        break; /* default: "OK", 1 s, 2 retries */
      }

      case QUECTEL_MANUFACTURER:
      case QUECTEL_GET_IMEI:
      {
        *kw = "\r\n";
        *sc = '\r';
        break;
      }

      case QUECTEL_CHECK_SIG_QUA:
      {
        *kw = "+CSQ: ";
        *sc = ',';
        *mr = QUECTEL_MAX_SIG_TRIES;
        break;
      }

      case QUECTEL_CHECK_SIM:
      {
        *kw = "+CPIN: ";
        *sc = '\r';
        *mr = QUECTEL_MAX_SIM_TRIES;
        break;
      }

      case QUECTEL_CHECK_GSM_NET:
      {
        *kw = "+CREG: ";
        *sc = '\r';
        *mr = QUECTEL_MAX_NET_TRIES;
        break;
      }

      case QUECTEL_CHECK_GPRS_NET:
      {
        *kw = "+CGREG: ";
        *sc = '\r';
        *mr = QUECTEL_MAX_NET_TRIES;
        break;
      }

      case QUECTEL_GET_OPERATOR:
      {
        *kw = "+COPS: ";
        *sc = '\r';
        break;
      }

      case QUECTEL_ACTIVATE_CTX:
      {
        *tms = QUECTEL_TIMEOUT_30S;
        break;
      }

      case QUECTEL_OPEN_SOCKET:
      {
        *kw = "CONNECT";
        *tms = QUECTEL_TIMEOUT_15S;
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
  QuectelModemAdapterCtx_t *c = (QuectelModemAdapterCtx_t *) ctx;

  /* CMD_MODE always advances, setting modem-alive to FALSE. */
  if ((QuectelState_t) state == QUECTEL_CMD_MODE)
  {
    pInfo->bModemAlive = 0U;
    pInfo->bModemAliveValid = 1U;

    return (uint8_t) QUECTEL_CLOSE_SOCKET;
  }

  if (responseOk == 0U)
  {
    (void) strncpy(pInfo->strJobLabel, "NO RESPONSE",
                   sizeof(pInfo->strJobLabel) - 1U);
    pInfo->strJobLabel[sizeof(pInfo->strJobLabel) - 1U] = '\0';

    return MODEM_STATE_FAILED;
  }

  switch ((QuectelState_t) state)
  {
      case QUECTEL_CLOSE_SOCKET:
      case QUECTEL_AT_COMMAND:
      case QUECTEL_DEACTIVATE_CTX:
      case QUECTEL_SETUP_APN:
      case QUECTEL_ACTIVATE_CTX:
      {
        return (uint8_t) ((uint8_t) state + 1U);
      }

      case QUECTEL_RESET_MODEM:
      {
        osDelay(QUECTEL_TIMEOUT_10S);

        return (uint8_t) QUECTEL_AT_COMMAND;
      }

      case QUECTEL_MANUFACTURER:
      {
        uint8_t alive = ((strcmp(response, QUECTEL_MANUFACTURER_ID1) == 0)
                         || (strcmp(response, QUECTEL_MANUFACTURER_ID2) == 0))
                        ? 1U : 0U;

        pInfo->bModemAlive = alive;
        pInfo->bModemAliveValid = 1U;
        if (alive != 0U)
        {
          (void) strncpy(pInfo->strJobLabel, "QUECTEL",
                         sizeof(pInfo->strJobLabel) - 1U);
          pInfo->strJobLabel[sizeof(pInfo->strJobLabel) - 1U] = '\0';
        }

        return (uint8_t) QUECTEL_GET_IMEI;
      }

      case QUECTEL_GET_IMEI:
      {
        (void) strncpy(pInfo->strIMEI, response,
                       sizeof(pInfo->strIMEI) - 1U);
        pInfo->strIMEI[sizeof(pInfo->strIMEI) - 1U] = '\0';

        return (uint8_t) QUECTEL_CHECK_SIG_QUA;
      }

      case QUECTEL_CHECK_SIG_QUA:
      {
        uint8_t quality = ParseSignalQuality(response);

        (void) strncpy(pInfo->strJobLabel, response,
                       sizeof(pInfo->strJobLabel) - 1U);
        pInfo->strJobLabel[sizeof(pInfo->strJobLabel) - 1U] = '\0';

        if (quality != QUECTEL_SIGNAL_ERR)
        {
          if (quality > QUECTEL_SIGNAL_MAX)
          {
            quality = QUECTEL_SIGNAL_MAX;
          }

          pInfo->bSignalQuality = quality;
          pInfo->bSignalQualityValid = 1U;
          c->bSigQuaRetries = 0U;

          return (uint8_t) QUECTEL_CHECK_SIM;
        }

        c->bSigQuaRetries++;
        if (c->bSigQuaRetries > QUECTEL_MAX_SIG_TRIES)
        {
          pInfo->bSignalQuality = QUECTEL_SIGNAL_MIN;
          pInfo->bSignalQualityValid = 1U;
          c->bSigQuaRetries = 0U;

          return (uint8_t) QUECTEL_CHECK_SIM;
        }

        return (uint8_t) QUECTEL_CHECK_SIG_QUA;
      }

      case QUECTEL_CHECK_SIM:
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

          return (uint8_t) QUECTEL_CHECK_GSM_NET;
        }

        c->bSimRetries++;
        if (c->bSimRetries > QUECTEL_MAX_SIM_TRIES)
        {
          c->bSimRetries = 0U;

          return (uint8_t) QUECTEL_CHECK_GSM_NET;
        }

        return (uint8_t) QUECTEL_CHECK_SIM;
      }

      case QUECTEL_CHECK_GSM_NET:
      case QUECTEL_CHECK_GPRS_NET:
      {
        return HandleNetworkResponse(c, (QuectelState_t) state,
                                     response, pInfo);
      }

      case QUECTEL_GET_OPERATOR:
      {
        ExtractOperator(response, pInfo->strOperator,
                        (uint8_t) sizeof(pInfo->strOperator));
        (void) strncpy(pInfo->strJobLabel, pInfo->strOperator,
                       sizeof(pInfo->strJobLabel) - 1U);
        pInfo->strJobLabel[sizeof(pInfo->strJobLabel) - 1U] = '\0';

        return (uint8_t) QUECTEL_DEACTIVATE_CTX;
      }

      case QUECTEL_OPEN_SOCKET:
      {
        return MODEM_STATE_CONNECTED;
      }

      default:
      {
        return (uint8_t) QUECTEL_CMD_MODE;
      }
  } /* switch */
} /* AdapterHandleResponse */

static uint8_t AdapterOnConnect(void *ctx)
{
  (void) ctx;
  if (!MCSAsynchConnectedGet())
  {
    if (!MCSAsynchStart(MODEM_GREETING_IMEI))
    {
      MCSJobAdd("MCS CON. ERROR");

      return 0U;
    }

    MCSJobAdd("MCS CON. SUC.");
  }

  return 1U;
}

static void AdapterOnMaintain(void *ctx)
{
  (void) ctx;
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
  if (MCSAsynchConnectedGet())
  {
    (void) MCSAsynchReqRxMsg((uint8_t *) data, len);
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
}

static const char *AdapterGetStateLabel(void *ctx, uint8_t state)
{
  (void) ctx;
  if (state < (uint8_t) QUECTEL_STATE_TOTAL)
  {
    return s_labels[state];
  }

  return "QUECTEL ?";
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

static uint8_t AdapterSend(void *ctx, const uint8_t *data, uint16_t len)
{
  QuectelModemAdapterCtx_t *c = (QuectelModemAdapterCtx_t *) ctx;

  return SerialSend(c->serialPort, data, len, 1000);
}

/* ------------------------------------------------------------------
 * Public API
 * ------------------------------------------------------------------ */

void QuectelModemAdapterInit(QuectelModemAdapterCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
}

IModemPort_t QuectelModemAdapterCreatePort(QuectelModemAdapterCtx_t *ctx)
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
