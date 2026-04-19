/* App/Domain/Lcd/LcdPage_ConnectionLogs.c
 */
#include "LcdPage.h"
#include "LcdEngine.h"
#include "LcdLanguage.h"
#include "LcdServiceRegistry.h"
#include "LcdPageRegistry.h"
#include <stdio.h>
#include <string.h>

/* Dependencies */
#include "Ports/UserInputTypes.h"

typedef struct
{
  const LcdServiceRegistry_t *services;
  const LcdPageRegistry_t    *pages;
} ConnLogsCtx_t;

static void ReadCommsSnapshot(const LcdServiceRegistry_t *services,
                              CommsStatusSnapshot_t *snapshot)
{
  (void) memset(snapshot, 0, sizeof(*snapshot));
  if ((services != NULL) && (services->comms != NULL))
  {
    (void) CommsStatusReadSnapshot(services->comms, snapshot);
  }
}

static void OnDraw(void *ctx, LcdEngine_t *e, IDisplayPort_t *display)
{
  ConnLogsCtx_t *c = (ConnLogsCtx_t *) ctx;
  char buf[21];
  CommsStatusSnapshot_t comms;
  uint8_t bIndex;

  (void) e;
  DisplayClear(display);
  ReadCommsSnapshot(c->services, &comms);

  /* Line 1: IMEI or MAC */
  uint8_t modemType = comms.modemType;

  /* Use magic numbers from MCS.h for now but ideally should be in a Port Type */
  if ((modemType == 5) || (modemType == 4))   /* ETH_NTCIP=5, USR=4 */
  {
    sprintf(buf,
            "MAC: %s",
            (modemType == 4) ? &comms.usrMac[0] : &comms.ethernetMac[0]);
  }
  else
  {
    sprintf(buf, "IMEI:%s", &comms.imei[0]);
  }

  DisplayWrite(display, 0, 0, buf, (uint8_t) strlen(buf));

  /* Line 2: Signal/Sim Status */
  if (comms.modemAlive != 0U)
  {
    sprintf(buf, "SIM:OK ANT:%d",
            (int) (comms.signalQuality / 8U));
  }
  else
  {
    sprintf(buf, "SIM:-  ANT:-");
  }

  DisplayWrite(display, 1, 0, buf, (uint8_t) strlen(buf));

  /* Lines 3-4: Live Jobs */
  for (bIndex = 0; bIndex < 2; bIndex++)
  {
    memset(buf, 0, sizeof(buf));
    if (comms.jobCurrent[bIndex][0] != '\0')
    {
      (void) strncpy(&buf[0], &comms.jobCurrent[bIndex][0], sizeof(buf) - 1U);
      DisplayWrite(display, 2 + bIndex, 0, buf, (uint8_t) strlen(buf));
    }
  }
} /* OnDraw */

static void OnInput(void *ctx, LcdEngine_t *e, uint8_t key)
{
  ConnLogsCtx_t *c = (ConnLogsCtx_t *) ctx;

  if (key == KEY_CLEAR)
  {
    LcdEngine_SwitchPage(e, c->pages->menu);
  }
}

static ConnLogsCtx_t s_connLogsCtx;
LcdPage_t LcdPage_ConnectionLogs = {
  .ctx = &s_connLogsCtx,
  .OnEnter = NULL,
  .OnUpdate = NULL,
  .OnInput = OnInput,
  .OnDraw = OnDraw,
  .OnExit = NULL
};

void LcdPage_ConnectionLogs_Init(ConnLogsCtx_t *ctx,
                                 const LcdServiceRegistry_t *services,
                                 const LcdPageRegistry_t *pages)
{
  ctx->services = services;
  ctx->pages = pages;
  LcdPage_ConnectionLogs.ctx = ctx;
}
