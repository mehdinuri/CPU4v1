/* App/Domain/FaultMonitor/FaultMonitorStatus.h
 *
 * TS2 Port-1-style structured status image. Mirrors the NEMA TS2
 * MMU fault status surface: per-channel fail bits plus intersection-
 * global flags. CP reads this image over FDCAN1 and projects it
 * into NTCIP 1202 faultMonitor* OIDs without MP knowing about SNMP.
 *
 * All values are application-layer booleans (0 / 1). Wire encoding
 * is the adapter's concern.
 */
#ifndef FAULT_MONITOR_STATUS_H
#define FAULT_MONITOR_STATUS_H

#include <stdint.h>

#include "Intersection/ChannelStateResolver.h"

typedef struct
{
  uint8_t conflict;
  uint8_t dualIndication;
  uint8_t redFail;
  uint8_t dark;
  uint8_t minYellowShort;
  uint8_t clearanceShort;
  uint8_t signalSequence;
  uint8_t lampOpen;
  uint8_t lampExternallyDriven;
} FaultMonitorChannelFlags_t;

typedef struct
{
  uint8_t acLineFault;
  uint8_t rail24VFault;
  uint8_t rail5VFault;
  uint8_t cpMissing;
  uint8_t psmMissing;
  uint8_t ssmMissing;
  uint8_t watchdog;
  uint8_t relayFeedbackMismatch;
  uint8_t configInvalid;
  uint8_t localFlashActive;
  uint8_t startupFlashActive;
  uint8_t mmuFlashActive;
} FaultMonitorGlobalFlags_t;

typedef struct
{
  FaultMonitorChannelFlags_t channels[MP_CHANNEL_COUNT_MAX];
  FaultMonitorGlobalFlags_t global;
  uint32_t lastUpdateTicks;
  uint32_t sequence;
} FaultMonitorStatus_t;

void FaultMonitorStatusClear(FaultMonitorStatus_t *status);

#endif /* FAULT_MONITOR_STATUS_H */
