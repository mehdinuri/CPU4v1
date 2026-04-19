/* App/Adapters/STM32/ControllerModeControlAdapter.c */
#include "ControllerModeControlAdapter.h"

#include <string.h>

#include "DomainServices.h"

static uint8_t RequestModeControl(void *ctx, uint8_t requestedControl)
{
  ControllerModeControlAdapterCtx_t *adapterCtx =
    (ControllerModeControlAdapterCtx_t *) ctx;
  uint8_t command = 0U;

  if ((adapterCtx == NULL) || (adapterCtx->engine == NULL))
  {
    return 0U;
  }

  switch (requestedControl)
  {
      case CONTROLLER_MODE_REQUEST_ALL_RED:
      {
        command = INTERSECTION_PATTERN_COMMAND_ALL_RED;
        break;
      }

      case CONTROLLER_MODE_REQUEST_DARK:
      {
        command = INTERSECTION_PATTERN_COMMAND_DARK;
        break;
      }

      case CONTROLLER_MODE_REQUEST_FLASH:
      {
        command = INTERSECTION_PATTERN_COMMAND_FLASH;
        break;
      }

      case CONTROLLER_MODE_REQUEST_PLAN_RETURN:
      {
        command = 0U;
        break;
      }

      default:
      {
        return 0U;
      }
  }

  return IntersectionEngineSetSystemPatternControl(adapterCtx->engine, command);
}

void ControllerModeControlAdapterInit(ControllerModeControlAdapterCtx_t *ctx)
{
  if (ctx != NULL)
  {
    (void) memset(ctx, 0, sizeof(*ctx));
    ctx->engine = &g_intersectionEngine;
  }
}

IControllerModeControlPort_t ControllerModeControlAdapterCreatePort(
  ControllerModeControlAdapterCtx_t *ctx)
{
  IControllerModeControlPort_t port;

  port.ctx = ctx;
  port.RequestModeControl = RequestModeControl;
  return port;
}
