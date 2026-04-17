/* App/Adapters/STM32/UBloxModemAdapter.c
 *
 * IModemPort concrete implementation for the u-blox LEON G100 GPRS
 * module (9600 baud, TCP direct-link mode).
 *
 * All modem-type-specific knowledge lives here: AT command strings,
 * per-state response keywords and timeouts, state-transition logic,
 * and disconnect-keyword detection.  The MCS coordinator drives the
 * state machine through the IModemPort_t vtable without knowing which
 * module type is connected.
 */
#include "UBloxModemAdapter.h"

#include <stdio.h>
#include <string.h>

#include "cmsis_os.h" /* osDelay — post-reset stabilisation wait */
#include "MCS.h"
#include "MCSAsynch.h"

/* ------------------------------------------------------------------
 * Private state enum — authoritative state enum for uBlox LEON G100 adapter
 * (same numeric values, same order).
 * ------------------------------------------------------------------ */
typedef enum
{
  UBLOX_CMD_MODE       = 0,
  UBLOX_RESET          = 1,
  UBLOX_AT_CMD         = 2,
  UBLOX_MANUFACTURER   = 3,
  UBLOX_GET_IMEI       = 4,
  UBLOX_CHECK_SIM      = 5,
  UBLOX_CHECK_SIG_QUA  = 6,
  UBLOX_CHECK_GSM_NET  = 7,
  UBLOX_CHECK_GPRS_NET = 8,
  UBLOX_GET_OPERATOR   = 9,
  UBLOX_APN_SET_UP     = 10,
  UBLOX_DYNAMIC_IP     = 11,
  UBLOX_ACTIVATE_GPRS  = 12,
  UBLOX_CREATE_SOCKET  = 13,
  UBLOX_CONFIG_SOCKET  = 14,
  UBLOX_CONNECT        = 15,
  UBLOX_DIRECT_LINK    = 16,
  UBLOX_STATE_TOTAL    = 17
} UBloxState_t;

/* ------------------------------------------------------------------
 * Per-state wait timeout constants (milliseconds).
 * ------------------------------------------------------------------ */
#define UBLOX_TIMEOUT_1S   1000U
#define UBLOX_TIMEOUT_5S   5000U
#define UBLOX_TIMEOUT_10S  10000U
#define UBLOX_TIMEOUT_15S  15000U

/* ------------------------------------------------------------------
 * Retry limit constants (must match the legacy MCS.c #defines).
 * ------------------------------------------------------------------ */
#define UBLOX_MAX_CMD_TRIES  2U   /* MCS_MAX_COMMAND_TRY                  */
#define UBLOX_MAX_SIM_TRIES  5U   /* MAX_GPRS_CHECK_SIM_TRY               */
#define UBLOX_MAX_SIG_TRIES  5U   /* MAX_GPRS_CHECK_SIG_QUA_TRY           */
#define UBLOX_MAX_NET_TRIES  60U  /* MAX_GPRS_CHECK_GSM_GPRS_NET_TRY      */

/* AT+CSQ: value 99 = "not known or not detectable". */
#define UBLOX_SIGNAL_ERR  99U
#define UBLOX_SIGNAL_MAX  31U  /* highest valid RSSI bucket */
#define UBLOX_SIGNAL_MIN  0U

/* Manufacturer ID string returned by AT+CGMI. */
#define UBLOX_MANUFACTURER_ID "u-blox"

/* ------------------------------------------------------------------
 * State label table — indexed by UBloxState_t.
 * Entries match MCS.c pStrUBLOX[] exactly.
 * ------------------------------------------------------------------ */
static const char *const s_labels[UBLOX_STATE_TOTAL] =
{
  /* 0  UBLOX_CMD_MODE       */ "COMMAND MODE",
  /* 1  UBLOX_RESET          */ "RESET MODEM",
  /* 2  UBLOX_AT_CMD         */ "AT COMMAND",
  /* 3  UBLOX_MANUFACTURER   */ "MANUFACTURER ID",
  /* 4  UBLOX_GET_IMEI       */ "GET IMEI",
  /* 5  UBLOX_CHECK_SIM      */ "CHECK SIM",
  /* 6  UBLOX_CHECK_SIG_QUA  */ "CHECK SIGNAL Q.",
  /* 7  UBLOX_CHECK_GSM_NET  */ "CHECK GSM N.",
  /* 8  UBLOX_CHECK_GPRS_NET */ "CHECK GPRS N.",
  /* 9  UBLOX_GET_OPERATOR   */ "GET OPERATOR",
  /* 10 UBLOX_APN_SET_UP     */ "SETUP APN",
  /* 11 UBLOX_DYNAMIC_IP     */ "ASSIGN DYNAMIC IP",
  /* 12 UBLOX_ACTIVATE_GPRS  */ "ACTIVATE GPRS",
  /* 13 UBLOX_CREATE_SOCKET  */ "CREATE SOCKET",
  /* 14 UBLOX_CONFIG_SOCKET  */ "CONFIG SOCKET",
  /* 15 UBLOX_CONNECT        */ "CONNECT SOCKET",
  /* 16 UBLOX_DIRECT_LINK    */ "DIRECT LINK MODE",
};

/* ------------------------------------------------------------------
 * Private helpers
 * ------------------------------------------------------------------ */

/* Parse a 1- or 2-digit ASCII string to uint8_t signal quality. */
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

/* Extract quoted operator name from a +COPS response string such as
 * "0,0,\"Turk Telekom\",7".  Writes at most (maxLen - 1) chars plus
 * NUL into strOut.  Writes "UNKNOWN" if no quoted string is found. */
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

/* ------------------------------------------------------------------
 * IModemPort_t function implementations (static — file-private)
 * ------------------------------------------------------------------ */

static void AdapterOnInit(void *ctx, ISerialPort_t *serialPort)
{
  UBloxModemAdapterCtx_t *c = (UBloxModemAdapterCtx_t *) ctx;

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

  return (uint8_t) UBLOX_CMD_MODE;
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

  switch ((UBloxState_t) state)
  {
      case UBLOX_CMD_MODE:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "+++");

        return 1U;
      }

      case UBLOX_RESET:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+cfun=16\r");

        return 1U;
      }

      case UBLOX_AT_CMD:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "ate0\r");

        return 1U;
      }

      case UBLOX_MANUFACTURER:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+cgmi\r");

        return 1U;
      }

      case UBLOX_GET_IMEI:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+cgsn\r");

        return 1U;
      }

      case UBLOX_CHECK_SIM:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+cpin?\r");

        return 1U;
      }

      case UBLOX_CHECK_SIG_QUA:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+csq\r");

        return 1U;
      }

      case UBLOX_CHECK_GSM_NET:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+creg?\r");

        return 1U;
      }

      case UBLOX_CHECK_GPRS_NET:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+cgreg?\r");

        return 1U;
      }

      case UBLOX_GET_OPERATOR:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+cops?\r");

        return 1U;
      }

      case UBLOX_APN_SET_UP:
      {
        (void) snprintf(outBuf, (size_t) maxLen,
                        "at+upsd=0,1,\"%s\"\r", apn);

        return 1U;
      }

      case UBLOX_DYNAMIC_IP:
      {
        (void) snprintf(outBuf, (size_t) maxLen,
                        "at+upsd=0,7,\"0.0.0.0\"\r");

        return 1U;
      }

      case UBLOX_ACTIVATE_GPRS:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+upsda=0,3\r");

        return 1U;
      }

      case UBLOX_CREATE_SOCKET:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+usocr=6\r");

        return 1U;
      }

      case UBLOX_CONFIG_SOCKET:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+usoso=0,6,1,0\r");

        return 1U;
      }

      case UBLOX_CONNECT:
      {
        (void) snprintf(outBuf, (size_t) maxLen,
                        "at+usoco=0,\"%s\",%u\r",
                        host, (unsigned int) serverPort);

        return 1U;
      }

      case UBLOX_DIRECT_LINK:
      {
        (void) snprintf(outBuf, (size_t) maxLen, "at+usodl=0\r");

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

  /* Sensible defaults; overridden per state below. */
  *sc = '\0';
  *mr = UBLOX_MAX_CMD_TRIES;
  *kw = "OK";
  *tms = UBLOX_TIMEOUT_1S;

  switch ((UBloxState_t) state)
  {
      case UBLOX_CMD_MODE:
      {
        /* "+++" escape — short wait; handler always advances to RESET. */
        *mr = 1U;
        break;
      }

      case UBLOX_RESET:
      {
        *tms = UBLOX_TIMEOUT_10S;
        break;
      }

      case UBLOX_AT_CMD:
      case UBLOX_APN_SET_UP:
      case UBLOX_CREATE_SOCKET:
      case UBLOX_CONFIG_SOCKET:
      {
        break; /* default: "OK", 1 s, 2 retries */
      }

      case UBLOX_MANUFACTURER:
      case UBLOX_GET_IMEI:
      {
        *kw = "\r\n";
        *sc = '\r';
        break;
      }

      case UBLOX_CHECK_SIM:
      {
        *kw = "+CPIN: ";
        *sc = '\r';
        *mr = UBLOX_MAX_SIM_TRIES;
        break;
      }

      case UBLOX_CHECK_SIG_QUA:
      {
        *kw = "+CSQ: ";
        *sc = ',';
        *mr = UBLOX_MAX_SIG_TRIES;
        break;
      }

      case UBLOX_CHECK_GSM_NET:
      {
        *kw = "+CREG: ";
        *sc = '\r';
        *mr = UBLOX_MAX_NET_TRIES;
        break;
      }

      case UBLOX_CHECK_GPRS_NET:
      {
        *kw = "+CGREG: ";
        *sc = '\r';
        *mr = UBLOX_MAX_NET_TRIES;
        break;
      }

      case UBLOX_GET_OPERATOR:
      {
        *kw = "+COPS: ";
        *sc = '\r';
        break;
      }

      case UBLOX_DYNAMIC_IP:
      case UBLOX_ACTIVATE_GPRS:
      {
        *tms = UBLOX_TIMEOUT_5S;
        break;
      }

      case UBLOX_CONNECT:
      {
        *tms = UBLOX_TIMEOUT_15S;
        break;
      }

      case UBLOX_DIRECT_LINK:
      {
        *kw = "CONNECT";
        *tms = UBLOX_TIMEOUT_15S;
        break;
      }

      default:
      {
        break;
      }
  } /* switch */
} /* AdapterGetWaitParams */

/* Handle a successful (responseOk = TRUE) response for network
 * registration states CHECK_GSM_NET and CHECK_GPRS_NET. */
static uint8_t HandleNetworkResponse(UBloxModemAdapterCtx_t *ctx,
                                     UBloxState_t state,
                                     const char             *response,
                                     ModemInfo_t            *pInfo)
{
  UBloxState_t nextState = (state == UBLOX_CHECK_GSM_NET)
                           ? UBLOX_CHECK_GPRS_NET
                           : UBLOX_APN_SET_UP;

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

    return (uint8_t) UBLOX_CMD_MODE;
  }

  /* "0,0" or "0,2": not registered, still searching. */
  ctx->bNetRetries++;
  (void) snprintf(pInfo->strJobLabel, sizeof(pInfo->strJobLabel),
                  "NOT REG. SEARCH. %u",
                  (unsigned int) ctx->bNetRetries);

  if (ctx->bNetRetries > UBLOX_MAX_NET_TRIES)
  {
    ctx->bNetRetries = 0U;

    return (uint8_t) nextState;
  }

  return (uint8_t) state;
} /* HandleNetworkResponse */

static uint8_t AdapterHandleResponse(void         *ctx,
                                     uint8_t state,
                                     const char   *response,
                                     uint8_t responseOk,
                                     ModemInfo_t  *pInfo)
{
  UBloxModemAdapterCtx_t *c = (UBloxModemAdapterCtx_t *) ctx;

  /* ----------------------------------------------------------------
   * Timeout / no-response path (responseOk == FALSE).
   * Only CMD_MODE is allowed to silently advance; all other states
   * treat timeout-exhausted as a hardware fault.
   * ---------------------------------------------------------------- */
  if (responseOk == 0U)
  {
    (void) strncpy(pInfo->strJobLabel, "NO RESPONSE",
                   sizeof(pInfo->strJobLabel) - 1U);
    pInfo->strJobLabel[sizeof(pInfo->strJobLabel) - 1U] = '\0';

    if ((UBloxState_t) state == UBLOX_CMD_MODE)
    {
      /* "+++" may or may not produce "OK"; always proceed to RESET. */
      return (uint8_t) UBLOX_RESET;
    }

    return MODEM_STATE_FAILED;
  }

  /* ----------------------------------------------------------------
   * Success path (responseOk == TRUE).
   * ---------------------------------------------------------------- */
  switch ((UBloxState_t) state)
  {
      case UBLOX_CMD_MODE:
      {
        /* Got "OK" from "+++"; modem is in command mode. */
        return (uint8_t) UBLOX_RESET;
      }

      case UBLOX_RESET:
      {
        /* Post-reset stabilisation: modem needs ~10 s before AT cmds. */
        osDelay(UBLOX_TIMEOUT_10S);

        return (uint8_t) UBLOX_AT_CMD;
      }

      case UBLOX_AT_CMD:
      {
        return (uint8_t) UBLOX_MANUFACTURER;
      }

      case UBLOX_MANUFACTURER:
      {
        uint8_t alive = (strcmp(response, UBLOX_MANUFACTURER_ID) == 0)
                        ? 1U : 0U;

        pInfo->bModemAlive = alive;
        pInfo->bModemAliveValid = 1U;
        if (alive != 0U)
        {
          (void) strncpy(pInfo->strJobLabel, "U-BLOX",
                         sizeof(pInfo->strJobLabel) - 1U);
          pInfo->strJobLabel[sizeof(pInfo->strJobLabel) - 1U] = '\0';
        }

        return (uint8_t) UBLOX_GET_IMEI;
      }

      case UBLOX_GET_IMEI:
      {
        (void) strncpy(pInfo->strIMEI, response,
                       sizeof(pInfo->strIMEI) - 1U);
        pInfo->strIMEI[sizeof(pInfo->strIMEI) - 1U] = '\0';

        return (uint8_t) UBLOX_CHECK_SIM;
      }

      case UBLOX_CHECK_SIM:
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

          return (uint8_t) UBLOX_CHECK_SIG_QUA;
        }

        c->bSimRetries++;
        if (c->bSimRetries > UBLOX_MAX_SIM_TRIES)
        {
          c->bSimRetries = 0U;

          return (uint8_t) UBLOX_CHECK_SIG_QUA; /* advance anyway */
        }

        return (uint8_t) UBLOX_CHECK_SIM; /* retry */
      }

      case UBLOX_CHECK_SIG_QUA:
      {
        uint8_t quality = ParseSignalQuality(response);

        (void) strncpy(pInfo->strJobLabel, response,
                       sizeof(pInfo->strJobLabel) - 1U);
        pInfo->strJobLabel[sizeof(pInfo->strJobLabel) - 1U] = '\0';

        if (quality != UBLOX_SIGNAL_ERR)
        {
          if (quality > UBLOX_SIGNAL_MAX)
          {
            quality = UBLOX_SIGNAL_MAX;
          }

          pInfo->bSignalQuality = quality;
          pInfo->bSignalQualityValid = 1U;
          c->bSigQuaRetries = 0U;

          return (uint8_t) UBLOX_CHECK_GSM_NET;
        }

        c->bSigQuaRetries++;
        if (c->bSigQuaRetries > UBLOX_MAX_SIG_TRIES)
        {
          pInfo->bSignalQuality = UBLOX_SIGNAL_MIN;
          pInfo->bSignalQualityValid = 1U;
          c->bSigQuaRetries = 0U;

          return (uint8_t) UBLOX_CHECK_GSM_NET; /* advance with min quality */
        }

        return (uint8_t) UBLOX_CHECK_SIG_QUA; /* retry */
      }

      case UBLOX_CHECK_GSM_NET:
      case UBLOX_CHECK_GPRS_NET:
      {
        return HandleNetworkResponse(c, (UBloxState_t) state,
                                     response, pInfo);
      }

      case UBLOX_GET_OPERATOR:
      {
        ExtractOperator(response, pInfo->strOperator,
                        (uint8_t) sizeof(pInfo->strOperator));
        (void) strncpy(pInfo->strJobLabel, pInfo->strOperator,
                       sizeof(pInfo->strJobLabel) - 1U);
        pInfo->strJobLabel[sizeof(pInfo->strJobLabel) - 1U] = '\0';

        return (uint8_t) UBLOX_APN_SET_UP;
      }

      case UBLOX_APN_SET_UP:
      case UBLOX_DYNAMIC_IP:
      case UBLOX_ACTIVATE_GPRS:
      case UBLOX_CREATE_SOCKET:
      case UBLOX_CONFIG_SOCKET:
      case UBLOX_CONNECT:
      {
        return (uint8_t) ((uint8_t) state + 1U);
      }

      case UBLOX_DIRECT_LINK:
      {
        /* "CONNECT" received — TCP direct-link mode established. */
        return MODEM_STATE_CONNECTED;
      }

      default:
      {
        return (uint8_t) UBLOX_CMD_MODE;
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
  UBloxModemAdapterCtx_t *c = (UBloxModemAdapterCtx_t *) ctx;

  return (SerialSend(c->serialPort, data, len, 1000) == len) ? 1U : 0U;
}

static const char *AdapterGetStateLabel(void *ctx, uint8_t state)
{
  (void) ctx;
  if (state < (uint8_t) UBLOX_STATE_TOTAL)
  {
    return s_labels[state];
  }

  return "UBLOX ?";
}

static uint8_t AdapterIsDisconnected(void       *ctx,
                                     const char *data,
                                     uint16_t len)
{
  (void) ctx;
  (void) len;

  return ((strstr(data, "NO CARRIER")  != NULL)
          || (strstr(data, "CLOSED")   != NULL)
          || (strstr(data, "+UUSOCL: 0") != NULL)
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

void UBloxModemAdapterInit(UBloxModemAdapterCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
}

IModemPort_t UBloxModemAdapterCreatePort(UBloxModemAdapterCtx_t *ctx)
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
