/* App/Ports/IModemPort.h
 *
 * Port interface for a modem-type driver.
 * One implementation per connection module type: uBlox LEON G100, Telit
 * GL865, Quectel M95, Quectel EG915U (PPP/NTCIP), USR TCP232-200,
 * Ethernet NTCIP.
 *
 * The MCS coordinator calls the inline helpers; adapters implement the
 * function pointers.  No HAL, FreeRTOS, or modem-type enum appears here.
 */
#ifndef IMODEM_PORT_H
#define IMODEM_PORT_H

#include <stdint.h>
#include "ISerialPort.h"

/* ------------------------------------------------------------------
 * Sentinel return values for HandleResponse()
 * ------------------------------------------------------------------ */
#define MODEM_STATE_CONNECTED  0xFEU
#define MODEM_STATE_FAILED     0xFFU

/* ------------------------------------------------------------------
 * Maestro greeting type constants for GetGreetingType() /
 * MCSAsynchStart().  Three logical categories cover all six module
 * types without exposing modem-type enum values to MCSAsynch.c.
 * ------------------------------------------------------------------ */
#define MODEM_GREETING_IMEI    0U   /* GPRS: uBlox, Telit, Quectel, QuectelNTCIP */
#define MODEM_GREETING_USR_MAC 1U   /* USR TCP232-200                             */
#define MODEM_GREETING_ETH_MAC 2U   /* Ethernet NTCIP                             */

/* ------------------------------------------------------------------
 * ModemInfo_t — data the driver extracts and writes during
 * HandleResponse().  The coordinator copies updated fields into its
 * runtime struct after each call.  All char fields are NUL-terminated.
 * *Valid flags are 0 when the corresponding field was not updated.
 * ------------------------------------------------------------------ */
typedef struct
{
  char strIMEI[16];           /* NUL-terminated IMEI from GET_IMEI state     */
  char strMAC[13];            /* NUL-terminated MAC: "XXXXXXXXXXXX" format   */
  char strOperator[21];       /* NUL-terminated GSM operator name            */
  /* Dynamic job label for MCSJobAdd(); empty string = use GetStateLabel(). */
  char strJobLabel[21];
  uint8_t bSignalQuality;
  uint8_t bSignalQualityValid; /* TRUE when bSignalQuality was set this cycle */
  uint8_t bModemAlive;         /* 0 = not alive, 1 = alive                   */
  uint8_t bModemAliveValid;    /* TRUE when bModemAlive was set this cycle    */
  uint8_t bSimReady;           /* 0 = not ready, 1 = ready                   */
  uint8_t bSimReadyValid;      /* TRUE when bSimReady was set this cycle      */
  /* DHCP/static IP address populated by Ethernet NTCIP adapter.             */
  char strLocalIPv4[16];       /* NUL-terminated local IPv4 address           */
  uint8_t bLocalIPv4Valid;     /* TRUE when strLocalIPv4 was set this cycle   */
} ModemInfo_t;

/* ------------------------------------------------------------------
 * IModemPort_t — vtable for a single modem-type driver.
 * All function pointers must be non-NULL after AdapterCreatePort().
 * ------------------------------------------------------------------ */
typedef struct
{
  void    *ctx;

  /* One-time modem/stack setup called from MCSInit().
   * GPRS adapters: no-op.
   * EthernetNtcip: calls MX_LWIP_Init().
   * QuectelNtcip: calls MX_LWIP_Init() + PPPOSAsynchCreate(serialPort). */
  void (*OnInit)(void *ctx, ISerialPort_t *serialPort);

  /* Baud rate for SerialSetBaudRate(); 0 = no serial (Ethernet NTCIP). */
  uint32_t (*GetBaudRate)(void *ctx);

  /* Starting state value for the coordinator's bState variable. */
  uint8_t (*GetInitialState)(void *ctx);

  /* Write the AT command for 'state' into outBuf (NUL-terminated,
   * maxLen includes the NUL terminator).
   * apn, host, serverPort come from the persisted connection config.
   * Returns TRUE (1) when there is a command to transmit; FALSE (0) for
   * receive-only states or modem types that need no AT commands. */
  uint8_t (*PrepareCommand)(void          *ctx,
                            uint8_t state,
                            const char    *apn,
                            const char    *host,
                            uint16_t serverPort,
                            char          *outBuf,
                            uint16_t maxLen);

  /* Fill *outKeyword with the response keyword for MCSWaitKey(),
   * *outStopChar with an optional stop character (0 = none),
   * *outTimeoutMs with the per-state wait timeout, and
   * *outMaxRetries with the retry count before declaring failure. */
  void (*GetWaitParams)(void          *ctx,
                        uint8_t state,
                        const char   **outKeyword,
                        char          *outStopChar,
                        uint32_t      *timeoutMs,
                        uint8_t       *maxRetries);

  /* Parse 'response' for 'state'; populate pInfo with any extracted data.
   * responseOk = FALSE (0) when the coordinator timed out with no data.
   * Returns the next state value, MODEM_STATE_CONNECTED, or
   * MODEM_STATE_FAILED. */
  uint8_t (*HandleResponse)(void          *ctx,
                            uint8_t state,
                            const char    *response,
                            uint8_t responseOk,
                            ModemInfo_t   *pInfo);

  /* Short label string for MCSJobAdd(), indexed by the driver's own
   * state enum.  Returns a pointer to a string literal; never NULL. */
  const char *(*GetStateLabel)(void *ctx, uint8_t state);

  /* Returns TRUE (1) if 'data' contains a remote-disconnect keyword.
   * Used by MCSAsynchIsRemoteEndClosed(). */
  uint8_t (*IsDisconnected)(void *ctx, const char *data, uint16_t len);

  /* Which Maestro greeting to send after connect (MODEM_GREETING_*). */
  uint8_t (*GetGreetingType)(void *ctx);

  /* Attempt to establish higher-level connection (e.g., MCS asynch,
   * TCP/SNMP/UDP start). Called from MCSTryConnect().
   * Returns TRUE (1) on success, FALSE (0) on error. */
  uint8_t (*OnConnect)(void *ctx);

  /* Periodic maintenance logic (e.g., maintain TCP/PPP/Asynch connection).
   * Called from MCSTaskFunc() loop. */
  void (*OnMaintain)(void *ctx);

  /* Process raw data received from the serial port.
   * If the adapter handles RX itself, it can route it to ring buffer
   * or asynch/PPP as needed. */
  void (*OnRx)(void *ctx, const uint8_t *data, uint16_t len);

  /* Request immediate disconnection of all active sessions.
   * Called when connection info changes or a hard reset is needed. */
  void (*OnDisconnect)(void *ctx);

  /* Transmit asynch data message to the remote server.
   * GPRS/USR: SerialSend(p->port, ...)
   * NTCIP: TCPClientSendData(...)
   * Returns TRUE (1) on success. */
  uint8_t (*Send)(void *ctx, const uint8_t *data, uint16_t len);
} IModemPort_t;

/* ------------------------------------------------------------------
 * Inline dispatch helpers (zero overhead at -O2)
 * ------------------------------------------------------------------ */
static inline void ModemOnInit(IModemPort_t *p, ISerialPort_t *serialPort)
{
  p->OnInit(p->ctx, serialPort);
}

static inline uint32_t ModemGetBaudRate(IModemPort_t *p)
{
  return p->GetBaudRate(p->ctx);
}

static inline uint8_t ModemGetInitialState(IModemPort_t *p)
{
  return p->GetInitialState(p->ctx);
}

static inline uint8_t ModemPrepareCommand(IModemPort_t *p,
                                          uint8_t state,
                                          const char   *apn,
                                          const char   *host,
                                          uint16_t serverPort,
                                          char         *buf,
                                          uint16_t maxLen)
{
  return p->PrepareCommand(p->ctx, state, apn, host, serverPort, buf, maxLen);
}

static inline void ModemGetWaitParams(IModemPort_t  *p,
                                      uint8_t state,
                                      const char   **kw,
                                      char          *sc,
                                      uint32_t      *tms,
                                      uint8_t       *mr)
{
  p->GetWaitParams(p->ctx, state, kw, sc, tms, mr);
}

static inline uint8_t ModemHandleResponse(IModemPort_t *p,
                                          uint8_t state,
                                          const char   *resp,
                                          uint8_t ok,
                                          ModemInfo_t  *info)
{
  return p->HandleResponse(p->ctx, state, resp, ok, info);
}

static inline const char *ModemGetStateLabel(IModemPort_t *p, uint8_t state)
{
  return p->GetStateLabel(p->ctx, state);
}

static inline uint8_t ModemIsDisconnected(IModemPort_t *p,
                                          const char   *data,
                                          uint16_t len)
{
  return p->IsDisconnected(p->ctx, data, len);
}

static inline uint8_t ModemGetGreetingType(IModemPort_t *p)
{
  return p->GetGreetingType(p->ctx);
}

static inline uint8_t ModemOnConnect(IModemPort_t *p)
{
  return p->OnConnect(p->ctx);
}

static inline void ModemOnMaintain(IModemPort_t *p)
{
  p->OnMaintain(p->ctx);
}

static inline void ModemOnRx(IModemPort_t *p, const uint8_t *data, uint16_t len)
{
  p->OnRx(p->ctx, data, len);
}

static inline void ModemOnDisconnect(IModemPort_t *p)
{
  p->OnDisconnect(p->ctx);
}

static inline uint8_t ModemSend(IModemPort_t *p,
                                const uint8_t *data,
                                uint16_t len)
{
  return p->Send(p->ctx, data, len);
}

#endif /* IMODEM_PORT_H */
