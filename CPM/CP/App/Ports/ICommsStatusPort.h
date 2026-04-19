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

typedef enum
{
  UI_COMMS_NETWORK_TYPE_NONE = 0,
  UI_COMMS_NETWORK_TYPE_QUECTEL = 1,
  UI_COMMS_NETWORK_TYPE_ETHERNET = 2
} UiCommsNetworkType_t;

typedef struct
{
  uint8_t networkType;
  uint8_t bearerState;
  uint8_t signalQuality;
  uint8_t transportReady;
  uint8_t snmpReady;
  uint8_t modemAlive;
  uint8_t simReady;
  char imei[UI_COMMS_IDENTITY_IMEI_MAX_LEN + 1U];
  char ethernetMac[UI_COMMS_IDENTITY_MAC_MAX_LEN + 1U];
  char operatorName[UI_COMMS_IDENTITY_OPERATOR_MAX_LEN + 1U];
  char localIp[UI_COMMS_IDENTITY_IPV4_MAX_LEN + 1U];
  char managerIp[UI_COMMS_IDENTITY_IPV4_MAX_LEN + 1U];
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
