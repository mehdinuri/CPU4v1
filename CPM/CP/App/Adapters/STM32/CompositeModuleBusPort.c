/* App/Adapters/STM32/CompositeModuleBusPort.c */
#include "CompositeModuleBusPort.h"

#include <string.h>

static void MergeSourceFamily(ModuleBusSnapshot_t *target,
                              const ModuleBusSnapshot_t *source,
                              uint8_t sourceMask)
{
  if ((target == NULL) || (source == NULL))
  {
    return;
  }

  target->validMask = (uint8_t) ((target->validMask & (uint8_t) ~sourceMask)
                                 | (source->validMask & sourceMask));
  target->healthMask = (uint8_t) ((target->healthMask & (uint8_t) ~sourceMask)
                                  | (source->healthMask & sourceMask));
  target->staleMask = (uint8_t) ((target->staleMask & (uint8_t) ~sourceMask)
                                 | (source->staleMask & sourceMask));
  target->contextFaultMask =
    (uint8_t) ((target->contextFaultMask & (uint8_t) ~sourceMask)
               | (source->contextFaultMask & sourceMask));
  target->sequenceFaultMask =
    (uint8_t) ((target->sequenceFaultMask & (uint8_t) ~sourceMask)
               | (source->sequenceFaultMask & sourceMask));
}

static uint8_t ReadSnapshot(void *ctx, ModuleBusSnapshot_t *snapshot)
{
  CompositeModuleBusPortCtx_t *portCtx = (CompositeModuleBusPortCtx_t *) ctx;
  ModuleBusSnapshot_t fieldSnapshot;
  ModuleBusSnapshot_t moduleSnapshot;
  uint8_t haveField = 0U;
  uint8_t haveModule = 0U;

  if ((portCtx == NULL) || (snapshot == NULL))
  {
    return 0U;
  }

  if (portCtx->fieldInputPort != NULL)
  {
    haveField = ModuleBusReadSnapshot(portCtx->fieldInputPort, &fieldSnapshot);
  }

  if (portCtx->moduleBusPort != NULL)
  {
    haveModule = ModuleBusReadSnapshot(portCtx->moduleBusPort, &moduleSnapshot);
  }

  if ((haveField == 0U) && (haveModule == 0U))
  {
    return 0U;
  }

  if (haveField != 0U)
  {
    *snapshot = fieldSnapshot;
  }
  else
  {
    *snapshot = moduleSnapshot;
  }

  if (haveModule != 0U)
  {
    MergeSourceFamily(snapshot,
                      &moduleSnapshot,
                      (uint8_t) (MODULE_BUS_SNAPSHOT_VALID_PREEMPT_INPUTS
                                 | MODULE_BUS_SNAPSHOT_VALID_PREEMPT_CONTROLS
                                 | MODULE_BUS_SNAPSHOT_VALID_MMU_STATUS
                                 | MODULE_BUS_SNAPSHOT_VALID_LOAD_SWITCH));
    snapshot->preemptInputs = moduleSnapshot.preemptInputs;
    snapshot->preemptControls = moduleSnapshot.preemptControls;
    snapshot->mmuStatus = moduleSnapshot.mmuStatus;
    snapshot->loadSwitchReds = moduleSnapshot.loadSwitchReds;
    snapshot->loadSwitchYellows = moduleSnapshot.loadSwitchYellows;
    snapshot->loadSwitchGreens = moduleSnapshot.loadSwitchGreens;
  }

  return 1U;
}

static uint8_t SetConfigEpoch(void *ctx, uint16_t configEpoch)
{
  CompositeModuleBusPortCtx_t *portCtx = (CompositeModuleBusPortCtx_t *) ctx;

  if (portCtx == NULL)
  {
    return 0U;
  }

  if ((portCtx->fieldInputPort != NULL)
      && (ModuleBusSetConfigEpoch(portCtx->fieldInputPort, configEpoch) == 0U))
  {
    return 0U;
  }

  if ((portCtx->moduleBusPort != NULL)
      && (ModuleBusSetConfigEpoch(portCtx->moduleBusPort, configEpoch) == 0U))
  {
    return 0U;
  }

  return 1U;
}

static uint8_t CommandDetectorReset(void *ctx,
                                    ModuleBusDetectorClass_t detectorClass,
                                    uint8_t detectorNumber)
{
  CompositeModuleBusPortCtx_t *portCtx = (CompositeModuleBusPortCtx_t *) ctx;

  if (portCtx == NULL)
  {
    return 0U;
  }

  if ((portCtx->fieldInputPort != NULL)
      && (ModuleBusCommandDetectorReset(portCtx->fieldInputPort,
                                        detectorClass,
                                        detectorNumber)
          != 0U))
  {
    return 1U;
  }

  if (portCtx->moduleBusPort != NULL)
  {
    return ModuleBusCommandDetectorReset(portCtx->moduleBusPort,
                                         detectorClass,
                                         detectorNumber);
  }

  return 0U;
}

void CompositeModuleBusPortInit(CompositeModuleBusPortCtx_t *ctx,
                                IModuleBusPort_t *fieldInputPort,
                                IModuleBusPort_t *moduleBusPort)
{
  if (ctx == NULL)
  {
    return;
  }

  memset(ctx, 0, sizeof(*ctx));
  ctx->fieldInputPort = fieldInputPort;
  ctx->moduleBusPort = moduleBusPort;
}

IModuleBusPort_t CompositeModuleBusPortCreatePort(
  CompositeModuleBusPortCtx_t *ctx)
{
  IModuleBusPort_t port;

  port.ctx = ctx;
  port.ReadSnapshot = ReadSnapshot;
  port.SetConfigEpoch = SetConfigEpoch;
  port.CommandDetectorReset = CommandDetectorReset;

  return port;
}
