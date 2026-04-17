/* App/Domain/Lcd/LcdPage_SettingsDateTime.c
 */
#include "LcdPage.h"
#include "LcdEngine.h"
#include "LcdLanguage.h"
#include "LcdPageRegistry.h"
#include "LcdServiceRegistry.h"
#include "Ports/ISystemPort.h"
#include "Ports/IRealtimeClockPort.h"
#include "Ports/UserInputTypes.h"
#include <stdio.h>
#include <string.h>

typedef enum
{
  FIELD_DAY,
  FIELD_MONTH,
  FIELD_YEAR,
  FIELD_HOUR,
  FIELD_MIN,
  FIELD_COUNT
} DateTimeField_t;

typedef struct
{
  const LcdServiceRegistry_t *services;
  const LcdPageRegistry_t    *pages;
  RtcSnapshot_t snapshot;
  DateTimeField_t activeField;
  uint8_t editMode;
} DateTimeCtx_t;

static void OnEnter(void *ctx, LcdEngine_t *e)
{
  DateTimeCtx_t *c = (DateTimeCtx_t *) ctx;

  (void) e;
  RealtimeClockReadSnapshot(c->services->rtc, &c->snapshot);
  c->activeField = FIELD_DAY;
  c->editMode = 0;
}

static void OnDraw(void *ctx, LcdEngine_t *e, IDisplayPort_t *display)
{
  DateTimeCtx_t *c = (DateTimeCtx_t *) ctx;
  char buf[21];

  (void) e;

  DisplayClear(display);

  uint8_t lang = ISystemPort_GetLanguage(c->services->system);

  DisplayWrite(display,
               0,
               0,
               (lang
                == LANGUAGE_TURKISH) ? "TARiH/ZAMAN AYARI"
               : "DATE/TIME SETTING",
               20);

  /* Line 2: DD/MM/YYYY */
  sprintf(buf, "%02d/%02d/20%02d %c",
          c->snapshot.Date, c->snapshot.Month, c->snapshot.Year,
          (c->activeField < FIELD_HOUR) ? '^' : ' ');
  DisplayWrite(display, 1, 0, buf, (uint8_t) strlen(buf));

  /* Line 3: HH:MM */
  sprintf(buf, "%02d:%02d %c",
          c->snapshot.Hours, c->snapshot.Minutes,
          (c->activeField >= FIELD_HOUR) ? '^' : ' ');
  DisplayWrite(display, 2, 0, buf, (uint8_t) strlen(buf));

  if (c->editMode)
  {
    DisplayWrite(display, 3, 0, "EDIT MODE", 10);
  }
}

static void OnInput(void *ctx, LcdEngine_t *e, uint8_t key)
{
  DateTimeCtx_t *c = (DateTimeCtx_t *) ctx;

  if (key == KEY_ENTER)
  {
    if (c->editMode)
    {
      RealtimeClockWriteSnapshot(c->services->rtc, &c->snapshot);
      c->editMode = 0;
    }
    else
    {
      c->editMode = 1;
    }
  }
  else if (key == KEY_RIGHT)
  {
    c->activeField = (c->activeField + 1) % FIELD_COUNT;
  }
  else if (key == KEY_LEFT)
  {
    c->activeField = (c->activeField
                      == 0) ? (FIELD_COUNT - 1) : (c->activeField - 1);
  }
  else if ((key == KEY_UP) && c->editMode)
  {
    switch (c->activeField)
    {
        case FIELD_DAY:
        { if (c->snapshot.Date < 31)
          {
            c->snapshot.Date++;
          }

          break; }

        case FIELD_MONTH:
        { if (c->snapshot.Month < 12)
          {
            c->snapshot.Month++;
          }

          break; }

        case FIELD_YEAR:
        { c->snapshot.Year++; break; }

        case FIELD_HOUR:
        { if (c->snapshot.Hours < 23)
          {
            c->snapshot.Hours++;
          }

          break; }

        case FIELD_MIN:
        { if (c->snapshot.Minutes < 59)
          {
            c->snapshot.Minutes++;
          }

          break; }

        default:
        { break; }
    }
  }
  else if ((key == KEY_DELETE_DOWN) && c->editMode)
  {
    switch (c->activeField)
    {
        case FIELD_DAY:
        { if (c->snapshot.Date > 1)
          {
            c->snapshot.Date--;
          }

          break; }

        case FIELD_MONTH:
        { if (c->snapshot.Month > 1)
          {
            c->snapshot.Month--;
          }

          break; }

        case FIELD_YEAR:
        { if (c->snapshot.Year > 0)
          {
            c->snapshot.Year--;
          }

          break; }

        case FIELD_HOUR:
        { if (c->snapshot.Hours > 0)
          {
            c->snapshot.Hours--;
          }

          break; }

        case FIELD_MIN:
        { if (c->snapshot.Minutes > 0)
          {
            c->snapshot.Minutes--;
          }

          break; }

        default:
        { break; }
    } /* switch */
  }
  else if (key == KEY_CLEAR)
  {
    LcdEngine_SwitchPage(e, c->pages->settings);
  }
} /* OnInput */

static DateTimeCtx_t s_dateTimeCtx;
LcdPage_t LcdPage_SettingsDateTime = {
  .ctx = &s_dateTimeCtx,
  .OnEnter = OnEnter,
  .OnUpdate = NULL,
  .OnInput = OnInput,
  .OnDraw = OnDraw,
  .OnExit = NULL
};

void LcdPage_SettingsDateTime_Init(DateTimeCtx_t *ctx,
                                   const LcdServiceRegistry_t *services,
                                   const LcdPageRegistry_t *pages)
{
  LcdPage_SettingsDateTime.ctx = ctx;
  ctx->services = services;
  ctx->pages = pages;
}
