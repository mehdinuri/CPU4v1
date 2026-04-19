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
  uint32_t tickCount;
  uint32_t lastCpHeartbeatTick;
  uint32_t expectedGeneration;
  uint32_t appliedGeneration;
  uint32_t loadingGeneration;
  uint16_t expectedSetId;
  uint16_t appliedSetId;
  uint16_t loadingSetId;
  uint16_t expectedImageBytes;
  uint8_t totalChunks;
  uint8_t receivedChunkMask;
  uint8_t registeredRxCallback;
  CpMpConfigState_t configState;
  CpMpMmuConfigImageV1_t configImage;
} CpMpLinkService_t;

void CpMpLinkServiceInit(CpMpLinkService_t *service,
                         IControlBusPort_t *controlBusPort,
                         ConfigurationService_t *configurationService,
                         SafetyDecisionService_t *safetyDecisionService,
                         FaultMonitorService_t *faultMonitorService);
void CpMpLinkServiceStep(CpMpLinkService_t *service);

#endif /* MP_CPMP_LINK_SERVICE_H */
