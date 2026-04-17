/* App/Adapters/STM32/MmuAdapter.h
 *
 * Conservative MMU filter adapter. Until the dedicated MMU module-bus
 * contract is implemented, this adapter either passes the requested image
 * through unchanged or forces an all-red safe state.
 */
#ifndef MMU_ADAPTER_H
#define MMU_ADAPTER_H

#include <stdint.h>

#include "Ports/IMmuPort.h"

typedef struct
{
  OutputDriverImage_t lastRequestedImage;
  OutputDriverImage_t lastApprovedImage;
  uint8_t forceAllRed;
} MmuAdapterCtx_t;

void MmuAdapterInit(MmuAdapterCtx_t *ctx);
void MmuAdapterSetForceAllRed(MmuAdapterCtx_t *ctx, uint8_t forceAllRed);
IMmuPort_t MmuAdapterCreatePort(MmuAdapterCtx_t *ctx);

#endif /* MMU_ADAPTER_H */
