/* App/Adapters/STM32/MmuAdapter.c
 *
 * CP-side output authority boundary. MP remains the only MMU authority; CP
 * enforces the commanded `NORMAL` / `FLASH` / `DARK` action at the cabinet
 * output boundary while keeping the relay drive and approved image explicit.
 */
#include "MmuAdapter.h"

#include <string.h>

static uint8_t ComputePermitOutputPower(const MmuAdapterCtx_t *ctx)
{
  uint8_t userEnabled = 1U;

  if (ctx == NULL)
  {
    return 0U;
  }

  if (ctx->relayControlService != NULL)
  {
    userEnabled = RelayControlServiceGetUserOutputPowerEnabled(
      ctx->relayControlService);
  }

  return (uint8_t) ((ctx->safetyAction != MMU_CONTROL_ACTION_DARK)
                    && (ctx->configReady != 0U)
                    && (ctx->requiredSsmHealthy != 0U)
                    && (userEnabled != 0U));
}

static void RefreshRelayDrive(MmuAdapterCtx_t *ctx)
{
  uint8_t relayDrive;

  if (ctx == NULL)
  {
    return;
  }

  ctx->permitOutputPower = ComputePermitOutputPower(ctx);
  relayDrive = ctx->permitOutputPower;

  if (ctx->relayTopology == MMU_RELAY_TOPOLOGY_ECO_ACTIVE_HIGH_TRIP)
  {
    relayDrive = (uint8_t) (ctx->permitOutputPower == 0U);
  }

  ctx->lastRelayDrive = relayDrive;

  if (ctx->relayPort != NULL)
  {
    RelaySet(ctx->relayPort, relayDrive);
  }

  if (ctx->relayControlService != NULL)
  {
    RelayControlServiceSetLocalState(ctx->relayControlService,
                                     ctx->permitOutputPower,
                                     relayDrive,
                                     (uint8_t) ctx->relayTopology,
                                     (uint8_t) ctx->safetyAction);
  }
}

static void SetAllRed(OutputDriverImage_t *image)
{
  uint8_t channelIndex;

  if (image == NULL)
  {
    return;
  }

  for (channelIndex = 0U; channelIndex < INTERSECTION_CHANNEL_COUNT_MAX;
       ++channelIndex)
  {
    image->channels[channelIndex] = OUTPUT_DRIVER_ASPECT_RED;
    image->channelDimmed[channelIndex] = 0U;
    image->channelDimAlternateHalfCycle[channelIndex] = 0U;
  }
}

static void SetAllDark(OutputDriverImage_t *image)
{
  uint8_t channelIndex;

  if (image == NULL)
  {
    return;
  }

  for (channelIndex = 0U; channelIndex < INTERSECTION_CHANNEL_COUNT_MAX;
       ++channelIndex)
  {
    image->channels[channelIndex] = OUTPUT_DRIVER_ASPECT_DARK;
    image->channelDimmed[channelIndex] = 0U;
    image->channelDimAlternateHalfCycle[channelIndex] = 0U;
  }
}

static uint8_t FilterOutputImage(void *ctx,
                                 const OutputDriverImage_t *requested,
                                 OutputDriverImage_t *approved)
{
  MmuAdapterCtx_t *adapterCtx = (MmuAdapterCtx_t *) ctx;

  if ((adapterCtx == NULL) || (requested == NULL) || (approved == NULL))
  {
    return 0U;
  }

  adapterCtx->lastRequestedImage = *requested;

  if (adapterCtx->forceAllRed != 0U)
  {
    SetAllRed(approved);
  }
  else if (adapterCtx->safetyAction == MMU_CONTROL_ACTION_DARK)
  {
    SetAllDark(approved);
  }
  else
  {
    *approved = *requested;
  }

  adapterCtx->lastApprovedImage = *approved;

  return 1U;
}

static uint8_t SetForceAllRedPort(void *ctx, uint8_t forceAllRed)
{
  MmuAdapterSetForceAllRed((MmuAdapterCtx_t *) ctx, forceAllRed);

  return 1U;
}

static uint8_t SetSafetyActionPort(void *ctx, MmuControlAction_t action)
{
  MmuAdapterSetSafetyAction((MmuAdapterCtx_t *) ctx, action);

  return 1U;
}

static uint8_t SetConfigReadyPort(void *ctx, uint8_t configReady)
{
  MmuAdapterSetConfigReady((MmuAdapterCtx_t *) ctx, configReady);

  return 1U;
}

static uint8_t SetRequiredSsmHealthyPort(void *ctx, uint8_t requiredSsmHealthy)
{
  MmuAdapterSetRequiredSsmHealthy((MmuAdapterCtx_t *) ctx, requiredSsmHealthy);

  return 1U;
}

void MmuAdapterInit(MmuAdapterCtx_t *ctx)
{
  if (ctx == NULL)
  {
    return;
  }

  memset(ctx, 0, sizeof(*ctx));
  SetAllRed(&ctx->lastRequestedImage);
  SetAllRed(&ctx->lastApprovedImage);
  ctx->safetyAction = MMU_CONTROL_ACTION_DARK;
  ctx->relayTopology = MMU_RELAY_TOPOLOGY_LEGACY_ACTIVE_HIGH_CLOSE;
  RefreshRelayDrive(ctx);
}

void MmuAdapterBindRelayPort(MmuAdapterCtx_t *ctx, IRelayPort_t *relayPort)
{
  if (ctx == NULL)
  {
    return;
  }

  ctx->relayPort = relayPort;
  RefreshRelayDrive(ctx);
}

void MmuAdapterBindRelayControlService(MmuAdapterCtx_t *ctx,
                                       RelayControlService_t *service)
{
  if (ctx == NULL)
  {
    return;
  }

  ctx->relayControlService = service;
  RefreshRelayDrive(ctx);
}

void MmuAdapterSetRelayTopology(MmuAdapterCtx_t *ctx,
                                MmuRelayTopology_t topology)
{
  if (ctx == NULL)
  {
    return;
  }

  ctx->relayTopology = topology;
  RefreshRelayDrive(ctx);
}

void MmuAdapterSetForceAllRed(MmuAdapterCtx_t *ctx, uint8_t forceAllRed)
{
  if (ctx == NULL)
  {
    return;
  }

  ctx->forceAllRed = (uint8_t) (forceAllRed != 0U);
}

void MmuAdapterSetConfigReady(MmuAdapterCtx_t *ctx, uint8_t configReady)
{
  if (ctx == NULL)
  {
    return;
  }

  ctx->configReady = (uint8_t) (configReady != 0U);
  RefreshRelayDrive(ctx);
}

void MmuAdapterSetRequiredSsmHealthy(MmuAdapterCtx_t *ctx,
                                     uint8_t requiredSsmHealthy)
{
  if (ctx == NULL)
  {
    return;
  }

  ctx->requiredSsmHealthy = (uint8_t) (requiredSsmHealthy != 0U);
  RefreshRelayDrive(ctx);
}

void MmuAdapterSetSafetyAction(MmuAdapterCtx_t *ctx, MmuControlAction_t action)
{
  if (ctx == NULL)
  {
    return;
  }

  ctx->safetyAction = action;
  RefreshRelayDrive(ctx);
}

uint8_t MmuAdapterGetPermitOutputPower(const MmuAdapterCtx_t *ctx)
{
  if (ctx == NULL)
  {
    return 0U;
  }

  return ctx->permitOutputPower;
}

uint8_t MmuAdapterGetLastRelayDrive(const MmuAdapterCtx_t *ctx)
{
  if (ctx == NULL)
  {
    return 0U;
  }

  return ctx->lastRelayDrive;
}

IMmuPort_t MmuAdapterCreatePort(MmuAdapterCtx_t *ctx)
{
  IMmuPort_t port;

  port.ctx = ctx;
  port.SetForceAllRed = SetForceAllRedPort;
  port.SetSafetyAction = SetSafetyActionPort;
  port.SetConfigReady = SetConfigReadyPort;
  port.SetRequiredSsmHealthy = SetRequiredSsmHealthyPort;
  port.FilterOutputImage = FilterOutputImage;

  return port;
}
