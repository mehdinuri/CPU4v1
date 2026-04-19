/* App/Adapters/STM32/ControllerModeControlAdapter.c */
#include "ControllerModeControlAdapter.h"

#include <string.h>

#include "data.h"

enum
{
  CONTROLLER_MODE_REQUEST_ALL_RED = 1U,
  CONTROLLER_MODE_REQUEST_DARK = 2U,
  CONTROLLER_MODE_REQUEST_FLASH = 3U,
  CONTROLLER_MODE_REQUEST_PLAN_RETURN = 4U
};

static uint8_t RequestModeControl(void *ctx, uint8_t requestedControl)
{
  (void) ctx;

  switch (requestedControl)
  {
      case CONTROLLER_MODE_REQUEST_ALL_RED:
      {
        UserStateReqSet(STATES_CLOSED);
        return 1U;
      }

      case CONTROLLER_MODE_REQUEST_DARK:
      {
        UserStateReqSet(STATES_NO_CONTROL);
        return 1U;
      }

      case CONTROLLER_MODE_REQUEST_FLASH:
      {
        UserStateReqSet(STATES_FLASH);
        return 1U;
      }

      case CONTROLLER_MODE_REQUEST_PLAN_RETURN:
      {
        UserStateReqFree();
        return 1U;
      }

      default:
      {
        return 0U;
      }
  }
}

void ControllerModeControlAdapterInit(ControllerModeControlAdapterCtx_t *ctx)
{
  if (ctx != NULL)
  {
    (void) memset(ctx, 0, sizeof(*ctx));
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
