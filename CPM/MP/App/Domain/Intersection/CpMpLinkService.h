/* App/Domain/Intersection/CpMpLinkService.h
 *
 * Owns the clean MP-side private control link to CP. Receives the canonical
 * MMU config image from CP and publishes MP heartbeat, config state, safety
 * state, and the structured malfunction status image.
 */
#ifndef MP_CPMP_LINK_SERVICE_H
#define MP_CPMP_LINK_SERVICE_H

#include <stdint.h>

#include "CpMpProtocolShared.h"
#include "FaultMonitor/FaultMonitorService.h"
#include "Intersection/ConfigurationService.h"
#include "Malfunction/SafetyDecisionService.h"
#include "Ports/IControlBusPort.h"

typedef struct
{
  IControlBusPort_t *controlBusPort;
  ConfigurationService_t *configurationService;
  SafetyDecisionService_t *safetyDecisionService;
  FaultMonitorService_t *faultMonitorService;
  CpMpFaultEventV1_t eventQueue[FAULT_MONITOR_TRACE_CAPACITY];
  CpMpFaultEventV1_t pendingEvent;
  uint32_t tickCount;
  uint32_t lastCpHeartbeatTick;
  uint32_t expectedGeneration;
  uint32_t appliedGeneration;
  uint32_t loadingGeneration;
  uint32_t nextEventSequenceToQueue;
  uint16_t expectedSetId;
  uint16_t appliedSetId;
  uint16_t loadingSetId;
  uint16_t expectedImageBytes;
  uint8_t totalChunks;
  uint8_t receivedChunks[CPMP_CONFIG_CHUNK_BITMAP_BYTES];
  uint8_t eventQueueHead;
  uint8_t eventQueueTail;
  uint8_t eventQueueCount;
  uint8_t pendingEventValid;
  uint8_t registeredRxCallback;
  uint8_t lastCpPermitOutputPower;
  CpMpConfigState_t configState;
  CpMpMmuConfigImage_t configImage;
} CpMpLinkService_t;

void CpMpLinkServiceInit(CpMpLinkService_t *service,
                         IControlBusPort_t *controlBusPort,
                         ConfigurationService_t *configurationService,
                         SafetyDecisionService_t *safetyDecisionService,
                         FaultMonitorService_t *faultMonitorService);
void CpMpLinkServiceStep(CpMpLinkService_t *service);

#endif /* MP_CPMP_LINK_SERVICE_H */
