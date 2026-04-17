/* App/Adapters/STM32/TelitModemAdapter.c
 *
 * IModemPort concrete implementation for the Telit GL865 Dual GPRS
 * module (9600 baud, TCP socket via AT#SD command).
 *
 * All Telit-specific knowledge lives here: AT command strings, per-state
 * response keywords and timeouts, state-transition logic, and
 * disconnect-keyword detection.
 */
#include "TelitModemAdapter.h"

#include <stdio.h>
#include <string.h>

#include "cmsis_os.h" /* osDelay — post-reset stabilisation wait */
#include "MCS.h"
#include "MCSAsynch.h"

/* ------------------------------------------------------------------
 * Private state enum — authoritative state enum for Telit GL865 adapter
 * (same numeric values, same order).
 * ------------------------------------------------------------------ */
typedef enum
{
  TELIT_CMD_MODE        = 0,
  TELIT_RESET_MODEM     = 1,
  TELIT_AT_CMD          = 2,
  TELIT_MANUFACTURER    = 3,
  TELIT_GET_IMEI        = 4,
  TELIT_CHECK_SIG_QUA   = 5,
  TELIT_CHECK_SIM       = 6,
  TELIT_CHECK_SOCKET    = 7,
  TELIT_CHECK_CONTEXT   = 8,
  TELIT_SHUTDOWN_SOCKET = 9,
  TELIT_SETUP_APN       = 10,
  TELIT_CHECK_GSM_NET   = 11,
  TELIT_CHECK_GPRS_NET  = 12,
  TELIT_GET_OPERATOR    = 13,
  TELIT_CONFIG_SOCKET   = 14,
  TELIT_ACTIVATE_CTX    = 15,
  TELIT_CONNECT_SOCKET  = 16,
  TELIT_STATE_TOTAL     = 17
} TelitState_t;

/* ------------------------------------------------------------------
 * Per-state wait timeout constants (milliseconds).
 * ------------------------------------------------------------------ */
#define TELIT_TIMEOUT_1S   1000U
#define TELIT_TIMEOUT_10S  10000U
#define TELIT_TIMEOUT_15S  15000U

/* ------------------------------------------------------------------
 * Retry limit constants.
 * ------------------------------------------------------------------ */
#define TELIT_MAX_CMD_TRIES  2U   /* MCS_MAX_COMMAND_TRY                 */
#define TELIT_MAX_SIM_TRIES  5U   /* MAX_GPRS_CHECK_SIM_TRY              */
#define TELIT_MAX_SIG_TRIES  5U   /* MAX_GPRS_CHECK_SIG_QUA_TRY          */
#define TELIT_MAX_NET_TRIES  60U  /* MAX_GPRS_CHECK_GSM_GPRS_NET_TRY     */

/* AT+CSQ: value 99 = "not known or not detectable". */
#define TELIT_SIGNAL_ERR  99U
#define TELIT_SIGNAL_MAX  31U
#define TELIT_SIGNAL_MIN  0U

#define TELIT_MANUFACTURER_ID "Telit"

/* ------------------------------------------------------------------
 * State label table — indexed by TelitState_t.
 * Entries match MCS.c pStrTELIT[] exactly.
 * ------------------------------------------------------------------ */
static const char *const s_labels[TELIT_STATE_TOTAL] =
{
  /* 0  TELIT_CMD_MODE        */ "COMMAND MODE",
  /* 1  TELIT_RESET_MODEM     */ "RESET MODEM",
  /* 2  TELIT_AT_CMD          */ "AT COMMAND",
  /* 3  TELIT_MANUFACTURER    */ "MANUFACTURER ID",
  /* 4  TELIT_GET_IMEI        */ "GET IMEI",
  /* 5  TELIT_CHECK_SIG_QUA   */ "CHECK SIGNAL Q.",
  /* 6  TELIT_CHECK_SIM       */ "CHECK SIM",
  /* 7  TELIT_CHECK_SOCKET    */ "CHECK SOCKET",
  /* 8  TELIT_CHECK_CONTEXT   */ "CHECK CONTEXT",
  /* 9  TELIT_SHUTDOWN_SOCKET */ "SHUTDOWN SOCKET",
  /* 10 TELIT_SETUP_APN       */ "SETUP APN",
  /* 11 TELIT_CHECK_GSM_NET   */ "CHECK GSM N.",
  /* 12 TELIT_CHECK_GPRS_NET  */ "CHECK GPRS N.",
  /* 13 TELIT_GET_OPERATOR    */ "GET OPERATOR",
  /* 14 TELIT_CONFIG_SOCKET   */ "CONFIG SOCKET",
  /* 15 TELIT_ACTIVATE_CTX    */ "CONTEXT ACTIVATION",
  /* 16 TELIT_CONNECT_SOCKET  */ "CONNECT SOCKET",
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

static uint8_t HandleNetworkResponse(TelitModemAdapterCtx_t *ctx,
                                     TelitState_t state,
                                     const char             *response,
                                     ModemInfo_t            *pInfo)
{
  TelitState_t nextState = (state == TELIT_CHECK_GSM_NET)
                           ? TELIT_CHECK_GPRS_NET
                           : TELIT_SETUP_APN;

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

    return (uint8_t) TELIT_CMD_MODE;
  }

  /* "0,0" or "0,2": not registered, still searching. */
  ctx->bNetRetries++;
  (void) snprintf(pInfo->strJobLabel, sizeof(pInfo->strJobLabel),
                  "NOT REG. SEARCH. %u",
                  (unsigned int) ctx->bNetRetries);

  if (ctx->bNetRetries > TELIT_MAX_NET_TRIES)
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
  TelitModemAdapterCtx_t *c = (TelitModemAdapterCtx_t *) ctx;

  c->serialPort = serialPort;
}

static uint32_t AdapterGetBaudRate(void *ctx)
{
  (void) ctx;

  return 9600U;
}

static uint8_t AdapterGetInitialState(void *ctx)
{
  (void) ctx;

  return (uint8_t) TELIT_CMD_MODE;
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

  switch ((TelitState_t) state)
  {
      case TELIT_CMD_MODE:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "+++");

        return 1U;
      }

      case TELIT_RESET_MODEM:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+cfun=1,1\r");

        return 1U;
      }

      case TELIT_AT_CMD:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "ate0\r");

        return 1U;
      }

      case TELIT_MANUFACTURER:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+cgmi\r");

        return 1U;
      }

      case TELIT_GET_IMEI:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+cgsn\r");

        return 1U;
      }

      case TELIT_CHECK_SIG_QUA:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+csq\r");

        return 1U;
      }

      case TELIT_CHECK_SIM:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+cpin?\r");

        return 1U;
      }

      case TELIT_CHECK_SOCKET:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at#ss=1\r");

        return 1U;
      }

      case TELIT_CHECK_CONTEXT:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at#sgact?\r");

        return 1U;
      }

      case TELIT_SHUTDOWN_SOCKET:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at#sh=1\r");

        return 1U;
      }

      case TELIT_SETUP_APN:
      {
        (void) snprintf(outBuf, (size_t) maxLen,
                        "at+cgdcont=1,\"IP\",\"%s\",\"0.0.0.0\",0,0\r", apn);

        return 1U;
      }

      case TELIT_CHECK_GSM_NET:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+creg?\r");

        return 1U;
      }

      case TELIT_CHECK_GPRS_NET:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+cgreg?\r");

        return 1U;
      }

      case TELIT_GET_OPERATOR:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+cops?\r");

        return 1U;
      }

      case TELIT_CONFIG_SOCKET:
      {
        (void) snprintf(outBuf, (size_t) maxLen,
                        "at#scfg=1,1,0,180,600,5\r");

        return 1U;
      }

      case TELIT_ACTIVATE_CTX:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at#sgact=1,1\r");

        return 1U;
      }

      case TELIT_CONNECT_SOCKET:
      {
        (void) snprintf(outBuf, (size_t) maxLen,
                        "at#sd=1,0,%u,\"%s\"\r",
                        (unsigned int) serverPort, host);

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
  *mr = TELIT_MAX_CMD_TRIES;
  *kw = "OK";
  *tms = TELIT_TIMEOUT_1S;

  switch ((TelitState_t) state)
  {
      case TELIT_CMD_MODE:
      {
        *mr = 1U;
        break;
      }

      case TELIT_RESET_MODEM:
      {
        *tms = TELIT_TIMEOUT_10S;
        break;
      }

      case TELIT_AT_CMD:
      case TELIT_SETUP_APN:
      case TELIT_CONFIG_SOCKET:
      case TELIT_SHUTDOWN_SOCKET:
      {
        break; /* default: "OK", 1 s, 2 retries */
      }

      case TELIT_MANUFACTURER:
      case TELIT_GET_IMEI:
      {
        *kw = "\r\n";
        *sc = '\r';
        break;
      }

      case TELIT_CHECK_SIG_QUA:
      {
        *kw = "+CSQ: ";
        *sc = ',';
        *mr = TELIT_MAX_SIG_TRIES;
        break;
      }

      case TELIT_CHECK_SIM:
      {
        *kw = "+CPIN: ";
        *sc = '\r';
        *mr = TELIT_MAX_SIM_TRIES;
        break;
      }

      case TELIT_CHECK_SOCKET:
      {
        *kw = "#SS: ";
        *sc = '\r';
        break;
      }

      case TELIT_CHECK_CONTEXT:
      {
        *kw = "#SGACT: ";
        *sc = '\r';
        break;
      }

      case TELIT_CHECK_GSM_NET:
      {
        *kw = "+CREG: ";
        *sc = '\r';
        *mr = TELIT_MAX_NET_TRIES;
        break;
      }

      case TELIT_CHECK_GPRS_NET:
      {
        *kw = "+CGREG: ";
        *sc = '\r';
        *mr = TELIT_MAX_NET_TRIES;
        break;
      }

      case TELIT_GET_OPERATOR:
      {
        *kw = "+COPS: ";
        *sc = '\r';
        break;
      }

      case TELIT_ACTIVATE_CTX:
      {
        *tms = TELIT_TIMEOUT_15S;
        break;
      }

      case TELIT_CONNECT_SOCKET:
      {
        *kw = "CONNECT";
        *tms = TELIT_TIMEOUT_15S;
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
  TelitModemAdapterCtx_t *c = (TelitModemAdapterCtx_t *) ctx;

  /* CMD_MODE always advances to RESET regardless of response. */
  if ((TelitState_t) state == TELIT_CMD_MODE)
  {
    pInfo->bModemAlive = 0U;
    pInfo->bModemAliveValid = 1U;

    return (uint8_t) TELIT_RESET_MODEM;
  }

  if (responseOk == 0U)
  {
    (void) strncpy(pInfo->strJobLabel, "NO RESPONSE",
                   sizeof(pInfo->strJobLabel) - 1U);
    pInfo->strJobLabel[sizeof(pInfo->strJobLabel) - 1U] = '\0';

    return MODEM_STATE_FAILED;
  }

  switch ((TelitState_t) state)
  {
      case TELIT_RESET_MODEM:
      {
        osDelay(TELIT_TIMEOUT_10S);

        return (uint8_t) TELIT_AT_CMD;
      }

      case TELIT_AT_CMD:
      case TELIT_SHUTDOWN_SOCKET:
      case TELIT_SETUP_APN:
      case TELIT_CONFIG_SOCKET:
      {
        return (uint8_t) ((uint8_t) state + 1U);
      }

      case TELIT_MANUFACTURER:
      {
        uint8_t alive = (strcmp(response, TELIT_MANUFACTURER_ID) == 0)
                        ? 1U : 0U;

        pInfo->bModemAlive = alive;
        pInfo->bModemAliveValid = 1U;
        if (alive != 0U)
        {
          (void) strncpy(pInfo->strJobLabel, "TELIT",
                         sizeof(pInfo->strJobLabel) - 1U);
          pInfo->strJobLabel[sizeof(pInfo->strJobLabel) - 1U] = '\0';
        }

        return (uint8_t) TELIT_GET_IMEI;
      }

      case TELIT_GET_IMEI:
      {
        (void) strncpy(pInfo->strIMEI, response,
                       sizeof(pInfo->strIMEI) - 1U);
        pInfo->strIMEI[sizeof(pInfo->strIMEI) - 1U] = '\0';

        return (uint8_t) TELIT_CHECK_SIG_QUA;
      }

      case TELIT_CHECK_SIG_QUA:
      {
        uint8_t quality = ParseSignalQuality(response);

        (void) strncpy(pInfo->strJobLabel, response,
                       sizeof(pInfo->strJobLabel) - 1U);
        pInfo->strJobLabel[sizeof(pInfo->strJobLabel) - 1U] = '\0';

        if (quality != TELIT_SIGNAL_ERR)
        {
          if (quality > TELIT_SIGNAL_MAX)
          {
            quality = TELIT_SIGNAL_MAX;
          }

          pInfo->bSignalQuality = quality;
          pInfo->bSignalQualityValid = 1U;
          c->bSigQuaRetries = 0U;

          return (uint8_t) TELIT_CHECK_SIM;
        }

        c->bSigQuaRetries++;
        if (c->bSigQuaRetries > TELIT_MAX_SIG_TRIES)
        {
          pInfo->bSignalQuality = TELIT_SIGNAL_MIN;
          pInfo->bSignalQualityValid = 1U;
          c->bSigQuaRetries = 0U;

          return (uint8_t) TELIT_CHECK_SIM;
        }

        return (uint8_t) TELIT_CHECK_SIG_QUA;
      }

      case TELIT_CHECK_SIM:
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

          /* Skip CHECK_SOCKET (state 7); go directly to CHECK_CONTEXT (8). */
          return (uint8_t) TELIT_CHECK_CONTEXT;
        }

        c->bSimRetries++;
        if (c->bSimRetries > TELIT_MAX_SIM_TRIES)
        {
          c->bSimRetries = 0U;

          return (uint8_t) TELIT_CHECK_CONTEXT;
        }

        return (uint8_t) TELIT_CHECK_SIM;
      }

      case TELIT_CHECK_SOCKET:
      {
        /* Socket active (1=connected/2=suspended/3=closing) → shut it down. */
        if ((strncmp(response, "1,1", 3U) == 0)
            || (strncmp(response, "1,2", 3U) == 0)
            || (strncmp(response, "1,3", 3U) == 0))
        {
          return (uint8_t) TELIT_SHUTDOWN_SOCKET;
        }

        /* Socket inactive → proceed with setup. */
        return (uint8_t) TELIT_SETUP_APN;
      }

      case TELIT_CHECK_CONTEXT:
      {
        /* Context active → shut down socket first. */
        if (strcmp(response, "1,1") == 0)
        {
          return (uint8_t) TELIT_SHUTDOWN_SOCKET;
        }

        return (uint8_t) TELIT_SETUP_APN;
      }

      case TELIT_CHECK_GSM_NET:
      case TELIT_CHECK_GPRS_NET:
      {
        return HandleNetworkResponse(c, (TelitState_t) state,
                                     response, pInfo);
      }

      case TELIT_GET_OPERATOR:
      {
        ExtractOperator(response, pInfo->strOperator,
                        (uint8_t) sizeof(pInfo->strOperator));
        (void) strncpy(pInfo->strJobLabel, pInfo->strOperator,
                       sizeof(pInfo->strJobLabel) - 1U);
        pInfo->strJobLabel[sizeof(pInfo->strJobLabel) - 1U] = '\0';

        return (uint8_t) TELIT_CONFIG_SOCKET;
      }

      case TELIT_ACTIVATE_CTX:
      {
        return (uint8_t) TELIT_CONNECT_SOCKET;
      }

      case TELIT_CONNECT_SOCKET:
      {
        return MODEM_STATE_CONNECTED;
      }

      default:
      {
        return (uint8_t) TELIT_CMD_MODE;
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
    else
    {
      MCSJobAdd("MCS CON. SUC.");
    }
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
  if (MCSGetConnected())
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

static uint8_t AdapterSend(void *ctx, const uint8_t *data, uint16_t len)
{
  TelitModemAdapterCtx_t *c = (TelitModemAdapterCtx_t *) ctx;

  return (SerialSend(c->serialPort, data, len, 1000) == len) ? 1U : 0U;
}

static const char *AdapterGetStateLabel(void *ctx, uint8_t state)
{
  (void) ctx;
  if (state < (uint8_t) TELIT_STATE_TOTAL)
  {
    return s_labels[state];
  }

  return "TELIT ?";
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

void TelitModemAdapterInit(TelitModemAdapterCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
}

IModemPort_t TelitModemAdapterCreatePort(TelitModemAdapterCtx_t *ctx)
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
