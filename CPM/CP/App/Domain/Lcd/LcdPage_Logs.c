/* App/Domain/Lcd/LcdPage_Logs.c
 */
#include "LcdPage.h"
#include "LcdEngine.h"
#include "LcdLanguage.h"
#include "LcdPageRegistry.h"
#include "LcdServiceRegistry.h"
#include "Ports/ISystemPort.h"
#include <stdio.h>
#include <string.h>

/* Dependencies */
#include "Ports/ILogRepositoryPort.h"
#include "Ports/UserInputTypes.h"

typedef struct
{
  uint8_t bMonth;
  uint8_t bMonthDay;
  uint8_t bHours;
  uint8_t bMinutes;
  uint8_t bSeconds;

  struct
  {
    uint8_t bEvent;
    uint8_t bParam;
    uint16_t sParam;
    uint32_t lParam;
  } SEvent;
} LcdLogRecord_t;

typedef struct
{
  const LcdServiceRegistry_t *services;
  const LcdPageRegistry_t    *pages;
  uint16_t logIndex;
  uint16_t tempIndex;
  uint8_t digitIndex;
  uint8_t inputActive;
} LogsCtx_t;

static void OnEnter(void *ctx, LcdEngine_t *e)
{
  LogsCtx_t *c = (LogsCtx_t *) ctx;

  (void) e;
  if (LogRepositoryExists(c->services->logs))
  {
    c->logIndex = LogRepositoryGetWriteIndex(c->services->logs);
  }
  else
  {
    c->logIndex = 0xFFFFU; /* LOG_NO_NEW_LOG equivalent */
  }

  c->digitIndex = 0;
  c->inputActive = 0;
}

static void OnDraw(void *ctx, LcdEngine_t *e, IDisplayPort_t *display)
{
  LogsCtx_t *c = (LogsCtx_t *) ctx;
  char buf[32];
  LcdLogRecord_t record;
  uint8_t lang = ISystemPort_GetLanguage(c->services->system);

  (void) e;
  DisplayClear(display);

  if (c->logIndex == 0xFFFFU)
  {
    DisplayWrite(display, 0, 0, Lcd_GetNoLogStr(lang), 20);

    return;
  }

  /* Line 1: Index */
  sprintf(buf, "<%04d>", c->inputActive ? c->tempIndex : c->logIndex);
  DisplayWrite(display, 0, 0, buf, (uint8_t) strlen(buf));

  if (LogRepositoryRead(c->services->logs, c->logIndex, &record,
                        sizeof(record)))
  {
    /* Line 2: Date/Time */
    sprintf(buf, "%02d/%02d %02d:%02d:%02d",
            record.bMonthDay, record.bMonth,
            record.bHours, record.bMinutes, record.bSeconds);
    DisplayWrite(display, 1, 0, buf, (uint8_t) strlen(buf));

    /* Line 3: Event Name */
    const char *eventStr = Lcd_GetEventStr(record.SEvent.bEvent, lang, 1);

    DisplayWrite(display, 2, 0, eventStr, (uint8_t) strlen(eventStr));

    /* Line 4: Params */
    (void) snprintf(buf,
                    sizeof(buf),
                    "%d, %d, %lu",
                    (int) record.SEvent.bParam,
                    (int) record.SEvent.sParam,
                    (unsigned long) record.SEvent.lParam);
    DisplayWrite(display, 3, 0, buf, (uint8_t) strlen(buf));
  }
  else
  {
    DisplayWrite(display, 1, 0, Lcd_GetErrorStr(lang), 20);
  }
} /* OnDraw */

static void OnInput(void *ctx, LcdEngine_t *e, uint8_t key)
{
  LogsCtx_t *c = (LogsCtx_t *) ctx;

  if (key <= KEY_9)
  {
    if (!c->inputActive)
    {
      c->inputActive = 1;
      c->tempIndex = 0;
      c->digitIndex = 0;
    }

    if (c->digitIndex < 4)
    {
      c->tempIndex = (uint16_t) (c->tempIndex * 10 + (uint16_t) key);
      c->digitIndex++;
    }
  }
  else if (key == KEY_ENTER)
  {
    if (c->inputActive)
    {
      if (LogRepositoryIsIndexValid(c->services->logs, c->tempIndex))
      {
        c->logIndex = c->tempIndex;
      }

      c->inputActive = 0;
    }
  }
  else if (key == KEY_LEFT)
  {
    if (c->logIndex > 0)
    {
      c->logIndex--;
    }

    c->inputActive = 0;
  }
  else if (key == KEY_RIGHT)
  {
    c->logIndex++;
    c->inputActive = 0;
  }
  else if (key == KEY_CLEAR)
  {
    LcdEngine_SwitchPage(e, c->pages->menu);
  }
} /* OnInput */

static LogsCtx_t s_logsCtx;
LcdPage_t LcdPage_Logs = {
  .ctx = &s_logsCtx,
  .OnEnter = OnEnter,
  .OnUpdate = NULL,
  .OnInput = OnInput,
  .OnDraw = OnDraw,
  .OnExit = NULL
};

void LcdPage_Logs_Init(LogsCtx_t *ctx,
                       const LcdServiceRegistry_t *services,
                       const LcdPageRegistry_t *pages)
{
  LcdPage_Logs.ctx = ctx;
  ctx->services = services;
  ctx->pages = pages;
}
