/* App/Domain/Intersection/CpMpLinkService.h
 *
 * Owns the clean CP-side back-channel to MP. It publishes heartbeats and the
 * canonical MMU config image and consumes MP safety / malfunction status.
 */
#ifndef CPMP_LINK_SERVICE_H
#define CPMP_LINK_SERVICE_H

#include <stdint.h>

#include "Domain/Intersection/ConfigurationService.h"
#include "Domain/Intersection/CpMpProtocol.h"
#include "Domain/Intersection/IntersectionController.h"
#include "Ports/IControlBusPort.h"

typedef enum
{
  CPMP_TX_STATE_IDLE = 0U,
  CPMP_TX_STATE_BEGIN = 1U,
  CPMP_TX_STATE_CHUNKS = 2U,
  CPMP_TX_STATE_COMMIT = 3U
} CpMpConfigTxState_t;

typedef struct
{
  IControlBusPort_t *controlBusPort;
  ConfigurationService_t *configurationService;
  IntersectionController_t *controller;
  CpMpMmuConfigImageV1_t configImage;
  CpMpFaultStatusImageV1_t lastFaultStatus;
  uint32_t tickCount;
  uint32_t configGeneration;
  uint16_t configSetId;
  uint16_t lastMpConfigSetId;
  uint32_t lastMpConfigGeneration;
  uint32_t lastMpHeartbeatTick;
  CpMpConfigState_t lastMpConfigState;
  uint8_t nextChunkIndex;
  uint8_t totalChunks;
  uint8_t registeredRxCallback;
  uint8_t lastMpHeartbeatSeen;
  CpMpConfigTxState_t txState;
  CpMpSafetyAction_t lastSafetyAction;
  uint8_t lastSafetyReasonCode;
  uint8_t lastFaultStatusValid;
} CpMpLinkService_t;

void CpMpLinkServiceInit(CpMpLinkService_t *service,
                         IControlBusPort_t *controlBusPort,
                         ConfigurationService_t *configurationService,
                         IntersectionController_t *controller);
void CpMpLinkServiceStep(CpMpLinkService_t *service);
uint8_t CpMpLinkServicePeerHealthy(const CpMpLinkService_t *service);
uint8_t CpMpLinkServiceAuthorityReady(const CpMpLinkService_t *service);
CpMpSafetyAction_t CpMpLinkServiceGetEffectiveSafetyAction(
  const CpMpLinkService_t *service);
uint8_t CpMpLinkServiceGetLastSafetyReasonCode(
  const CpMpLinkService_t *service);
uint8_t CpMpLinkServiceGetFaultStatus(const CpMpLinkService_t *service,
                                      CpMpFaultStatusImageV1_t *faultStatus);

#endif /* CPMP_LINK_SERVICE_H */
