/* App/Ports/ICommsStatusPort.h
 *
 * Port interface for CP-owned communications identity and runtime status.
 * This is the canonical read seam over MCSTask runtime data for both LCDs.
 */
#ifndef ICOMMS_STATUS_PORT_H
#define ICOMMS_STATUS_PORT_H

#include <stddef.h>
#include <stdint.h>

#define UI_COMMS_IDENTITY_IMEI_MAX_LEN 15U
#define UI_COMMS_IDENTITY_MAC_MAX_LEN 12U
#define UI_COMMS_IDENTITY_OPERATOR_MAX_LEN 20U
#define UI_COMMS_IDENTITY_IPV4_MAX_LEN 15U
#define UI_COMMS_JOB_TEXT_MAX_LEN 20U
#define UI_COMMS_JOB_COUNT 2U

typedef struct
{
  uint8_t modemType;
  uint8_t gprsState;
  uint8_t signalQuality;
  uint8_t connected;
  uint8_t modemAlive;
  uint8_t simReady;
  char imei[UI_COMMS_IDENTITY_IMEI_MAX_LEN + 1U];
  char usrMac[UI_COMMS_IDENTITY_MAC_MAX_LEN + 1U];
  char ethernetMac[UI_COMMS_IDENTITY_MAC_MAX_LEN + 1U];
  char operatorName[UI_COMMS_IDENTITY_OPERATOR_MAX_LEN + 1U];
  char localIp[UI_COMMS_IDENTITY_IPV4_MAX_LEN + 1U];
  char remoteIp[UI_COMMS_IDENTITY_IPV4_MAX_LEN + 1U];
  char jobCurrent[UI_COMMS_JOB_COUNT][UI_COMMS_JOB_TEXT_MAX_LEN + 1U];
} CommsStatusSnapshot_t;

typedef struct
{
  void *ctx;

  uint8_t (*ReadSnapshot)(void *ctx, CommsStatusSnapshot_t *snapshot);
} ICommsStatusPort_t;

static inline uint8_t CommsStatusReadSnapshot(ICommsStatusPort_t *port,
                                              CommsStatusSnapshot_t *snapshot)
{
  if ((port == NULL) || (port->ReadSnapshot == NULL))
  {
    return 0U;
  }

  return port->ReadSnapshot(port->ctx, snapshot);
}

#endif /* ICOMMS_STATUS_PORT_H */
