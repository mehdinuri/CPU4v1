/* App/Domain/Lcd/LcdPage_ConnectionLogs.c
 */
#include "LcdPage.h"
#include "LcdEngine.h"
#include "LcdLanguage.h"
#include "LcdServiceRegistry.h"
#include "LcdPageRegistry.h"
#include "Ports/ICommsStatusPort.h"
#include "Ports/UserInputTypes.h"
#include <stdio.h>
#include <string.h>

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

  /* Line 1: Identity */
  if (comms.networkType == (uint8_t) UI_COMMS_NETWORK_TYPE_ETHERNET)
  {
    sprintf(buf, "MAC:%s", &comms.ethernetMac[0]);
  }
  else if (comms.networkType == (uint8_t) UI_COMMS_NETWORK_TYPE_QUECTEL)
  {
    sprintf(buf, "IMEI:%s", &comms.imei[0]);
  }
  else
  {
    sprintf(buf, "NET: NONE");
  }

  DisplayWrite(display, 0, 0, buf, (uint8_t) strlen(buf));

  /* Line 2: Signal/Sim Status */
  if (comms.networkType == (uint8_t) UI_COMMS_NETWORK_TYPE_QUECTEL)
  {
    sprintf(buf,
            "SIM:%s SIG:%02u",
            (comms.simReady != 0U) ? "OK" : "--",
            (unsigned int) comms.signalQuality);
  }
  else
  {
    sprintf(buf,
            "NET:%s SN:%s",
            (comms.transportReady != 0U) ? "UP" : "DN",
            (comms.snmpReady != 0U) ? "UP" : "DN");
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
