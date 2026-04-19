/* App/Ports/IModemPort.h
 *
 * Port interface for a network-bearer driver.
 * One implementation per supported bearer: Quectel PPPoS or Ethernet.
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

  /* One-time bearer/stack setup called from MCSInit(). */
  void (*OnInit)(void *ctx, ISerialPort_t *serialPort);

  /* Baud rate for SerialSetBaudRate(); 0 = no serial (Ethernet). */
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

  /* Returns TRUE (1) when the bearer is ready for SNMP traffic. */
  uint8_t (*IsTransportReady)(void *ctx);

  /* Returns TRUE (1) while the current bearer remains healthy.
   * Idle traffic alone must not drive this to FALSE. */
  uint8_t (*IsTransportHealthy)(void *ctx);

  /* Periodic transport maintenance logic. */
  void (*OnMaintain)(void *ctx);

  /* Process raw data received from the serial port.
   * If the adapter handles RX itself, it can route it to ring buffer
   * or PPP as needed. */
  void (*OnRx)(void *ctx, const uint8_t *data, uint16_t len);

  /* Request immediate bearer disconnection/reset. */
  void (*OnDisconnect)(void *ctx);

  /* Optional raw send hook for adapters that expose one. */
  uint8_t (*Send)(void *ctx, const uint8_t *data, uint16_t len);

  /* Optional disconnect-state hook for adapters that expose one. */
  uint8_t (*IsDisconnected)(void *ctx, const char *data, uint16_t len);
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

static inline uint8_t ModemIsTransportReady(IModemPort_t *p)
{
  return p->IsTransportReady(p->ctx);
}

static inline uint8_t ModemIsTransportHealthy(IModemPort_t *p)
{
  return p->IsTransportHealthy(p->ctx);
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

static inline uint8_t ModemIsDisconnected(IModemPort_t *p,
                                          const char   *data,
                                          uint16_t len)
{
  return p->IsDisconnected(p->ctx, data, len);
}

#endif /* IMODEM_PORT_H */
