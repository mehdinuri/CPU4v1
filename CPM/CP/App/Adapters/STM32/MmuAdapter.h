/* App/Adapters/STM32/MmuAdapter.h
 *
 * Conservative MMU filter adapter. Until the dedicated MMU module-bus
 * contract is implemented, this adapter either passes the requested image
 * through unchanged or forces an all-red safe state.
 */
#ifndef MMU_ADAPTER_H
#define MMU_ADAPTER_H

#include <stdint.h>

#include "Domain/Services/RelayControlService.h"
#include "Ports/IMmuPort.h"
#include "Ports/IRelayPort.h"

typedef enum
{
  MMU_RELAY_TOPOLOGY_LEGACY_ACTIVE_HIGH_CLOSE = 0U,
  MMU_RELAY_TOPOLOGY_ECO_ACTIVE_HIGH_TRIP = 1U
} MmuRelayTopology_t;

typedef struct
{
  OutputDriverImage_t lastRequestedImage;
  OutputDriverImage_t lastApprovedImage;
  IRelayPort_t *relayPort;
  uint8_t forceAllRed;
  uint8_t permitOutputPower;
  uint8_t lastRelayDrive;
  MmuControlAction_t safetyAction;
  MmuRelayTopology_t relayTopology;
  RelayControlService_t *relayControlService;
} MmuAdapterCtx_t;

void MmuAdapterInit(MmuAdapterCtx_t *ctx);
void MmuAdapterBindRelayPort(MmuAdapterCtx_t *ctx, IRelayPort_t *relayPort);
void MmuAdapterBindRelayControlService(MmuAdapterCtx_t *ctx,
                                       RelayControlService_t *service);
void MmuAdapterSetRelayTopology(MmuAdapterCtx_t *ctx,
                                MmuRelayTopology_t topology);
void MmuAdapterSetForceAllRed(MmuAdapterCtx_t *ctx, uint8_t forceAllRed);
void MmuAdapterSetSafetyAction(MmuAdapterCtx_t *ctx, MmuControlAction_t action);
uint8_t MmuAdapterGetPermitOutputPower(const MmuAdapterCtx_t *ctx);
uint8_t MmuAdapterGetLastRelayDrive(const MmuAdapterCtx_t *ctx);
IMmuPort_t MmuAdapterCreatePort(MmuAdapterCtx_t *ctx);

#endif /* MMU_ADAPTER_H */
