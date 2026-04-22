/* App/Domain/Lcd/LcdPage_Logs.c
 */
#include "LcdPage.h"
#include "LcdEngine.h"
#include "LcdLanguage.h"
#include "LcdPageRegistry.h"
#include "LcdServiceRegistry.h"
#include "Domain/Services/MmiEventLogService.h"
#include "Ports/ISystemPort.h"
#include "Ports/UserInputTypes.h"
#include <stdio.h>
#include <string.h>

typedef struct
{
  const LcdServiceRegistry_t *services;
  const LcdPageRegistry_t    *pages;
  uint16_t logIndex;
  uint16_t tempIndex;
  uint8_t digitIndex;
  uint8_t inputActive;
} LogsCtx_t;

static uint8_t GetLanguage(const LcdServiceRegistry_t *services)
{
  if ((services == NULL) || (services->system == NULL))
  {
    return LANGUAGE_ENGLISH;
  }

  return ISystemPort_GetLanguage(services->system);
}

static uint16_t GetLatestLogIndex(const LcdServiceRegistry_t *services)
{
  uint16_t latestIndex = MMI_PROTOCOL_V2_EVENT_CURSOR_NONE;

  if ((services == NULL) || (services->eventLogService == NULL))
  {
    return MMI_PROTOCOL_V2_EVENT_CURSOR_NONE;
  }

  if (MmiEventLogServiceGetLatestIndex(services->eventLogService,
                                       &latestIndex) == 0U)
  {
    return MMI_PROTOCOL_V2_EVENT_CURSOR_NONE;
  }

  return latestIndex;
}

static uint8_t IsLogIndexValid(const LcdServiceRegistry_t *services,
                               uint16_t index)
{
  if ((services == NULL) || (services->eventLogService == NULL))
  {
    return 0U;
  }

  return MmiEventLogServiceIsIndexValid(services->eventLogService, index);
}

static uint8_t ReadLogRecord(const LcdServiceRegistry_t *services,
                             uint16_t index,
                             MmiEventRecord_t *record)
{
  if ((services == NULL) || (services->eventLogService == NULL)
      || (record == NULL))
  {
    return 0U;
  }

  return MmiEventLogServiceReadRecord(services->eventLogService,
                                      index,
                                      record);
}

static void DaysToCivil(int64_t days, uint8_t *month, uint8_t *date)
{
  int64_t z = days + 719468LL;
  int64_t era = (z >= 0LL) ? (z / 146097LL) : ((z - 146096LL) / 146097LL);
  uint32_t dayOfEra = (uint32_t) (z - (era * 146097LL));
  uint32_t yearOfEra = (dayOfEra - (dayOfEra / 1460U)
                        + (dayOfEra / 36524U) - (dayOfEra / 146096U))
                       / 365U;
  uint32_t dayOfYear = dayOfEra - ((365U * yearOfEra) + (yearOfEra / 4U)
                                   - (yearOfEra / 100U));
  uint32_t monthPrime = ((5U * dayOfYear) + 2U) / 153U;
  uint32_t day = dayOfYear - ((153U * monthPrime) + 2U) / 5U + 1U;
  uint32_t monthValue = (monthPrime < 10U) ? (monthPrime + 3U)
                        : (monthPrime - 9U);

  if (month != NULL)
  {
    *month = (uint8_t) monthValue;
  }

  if (date != NULL)
  {
    *date = (uint8_t) day;
  }
}

static void FormatTimestamp(const MmiEventRecord_t *record,
                            char *buffer,
                            uint32_t bufferSize)
{
  uint32_t secondsOfDay;
  uint8_t month = 0U;
  uint8_t date = 0U;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;

  if ((record == NULL) || (buffer == NULL) || (bufferSize == 0U))
  {
    return;
  }

  secondsOfDay = record->eventTime % 86400UL;
  DaysToCivil((int64_t) (record->eventTime / 86400UL), &month, &date);
  hour = (uint8_t) (secondsOfDay / 3600UL);
  minute = (uint8_t) ((secondsOfDay % 3600UL) / 60UL);
  second = (uint8_t) (secondsOfDay % 60UL);

  (void) snprintf(buffer,
                  bufferSize,
                  "%02u/%02u %02u:%02u:%02u.%03u",
                  (unsigned int) date,
                  (unsigned int) month,
                  (unsigned int) hour,
                  (unsigned int) minute,
                  (unsigned int) second,
                  (unsigned int) record->eventTimeMilliseconds);
}

static void FormatValuePreview(const MmiEventRecord_t *record,
                               char *buffer,
                               uint32_t bufferSize)
{
  uint8_t previewCount;
  uint8_t index;
  uint32_t offset = 0U;
  int written;

  if ((record == NULL) || (buffer == NULL) || (bufferSize == 0U))
  {
    return;
  }

  if (record->valueLength == 0U)
  {
    (void) snprintf(buffer, bufferSize, "V:-");
    return;
  }

  previewCount = record->valueLength;
  if (previewCount > 5U)
  {
    previewCount = 5U;
  }

  written = snprintf(buffer, bufferSize, "V:");
  if (written > 0)
  {
    offset = (uint32_t) written;
  }

  for (index = 0U; index < previewCount; index++)
  {
    if (offset >= (bufferSize - 1U))
    {
      break;
    }

    written = snprintf(&buffer[offset],
                       bufferSize - offset,
                       (index == 0U) ? "%02X" : " %02X",
                       (unsigned int) record->value[index]);
    if (written < 0)
    {
      break;
    }

    if ((uint32_t) written >= (bufferSize - offset))
    {
      offset = bufferSize - 1U;
      break;
    }

    offset += (uint32_t) written;
  }

  if ((record->valueLength > previewCount) && (offset < (bufferSize - 1U)))
  {
    (void) snprintf(&buffer[offset], bufferSize - offset, "...");
  }
}

static void OnEnter(void *ctx, LcdEngine_t *e)
{
  LogsCtx_t *c = (LogsCtx_t *) ctx;

  (void) e;
  c->logIndex = GetLatestLogIndex(c->services);
  c->digitIndex = 0;
  c->inputActive = 0;
}

static void OnDraw(void *ctx, LcdEngine_t *e, IDisplayPort_t *display)
{
  LogsCtx_t *c = (LogsCtx_t *) ctx;
  char buf[21];
  MmiEventRecord_t record;
  uint8_t lang = GetLanguage(c->services);

  (void) e;
  DisplayClear(display);

  if (c->logIndex == MMI_PROTOCOL_V2_EVENT_CURSOR_NONE)
  {
    const char *message = Lcd_GetNoLogStr(lang);

    DisplayWrite(display, 0, 0, message, (uint8_t) strlen(message));

    return;
  }

  /* Line 1: Index */
  (void) snprintf(buf,
                  sizeof(buf),
                  "<%04u>",
                  (unsigned int) (c->inputActive ? c->tempIndex : c->logIndex));
  DisplayWrite(display, 0, 0, buf, (uint8_t) strlen(buf));

  if (ReadLogRecord(c->services, c->logIndex, &record) != 0U)
  {
    FormatTimestamp(&record, buf, sizeof(buf));
    DisplayWrite(display, 1, 0, buf, (uint8_t) strlen(buf));

    /* Line 3: Class / event number / event id */
    (void) snprintf(buf,
                    sizeof(buf),
                    "C:%u N:%u ID:%u",
                    (unsigned int) record.eventClass,
                    (unsigned int) record.eventNumber,
                    (unsigned int) record.eventId);
    DisplayWrite(display, 2, 0, buf, (uint8_t) strlen(buf));

    /* Line 4: BER value preview */
    FormatValuePreview(&record, buf, sizeof(buf));
    DisplayWrite(display, 3, 0, buf, (uint8_t) strlen(buf));
  }
  else
  {
    const char *message = Lcd_GetErrorStr(lang);

    DisplayWrite(display, 1, 0, message, (uint8_t) strlen(message));
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
      if (IsLogIndexValid(c->services, c->tempIndex) != 0U)
      {
        c->logIndex = c->tempIndex;
      }

      c->inputActive = 0;
    }
  }
  else if (key == KEY_LEFT)
  {
    if ((c->logIndex != MMI_PROTOCOL_V2_EVENT_CURSOR_NONE) && (c->logIndex > 0U)
        && (IsLogIndexValid(c->services, (uint16_t) (c->logIndex - 1U)) != 0U))
    {
      c->logIndex--;
    }

    c->inputActive = 0;
  }
  else if (key == KEY_RIGHT)
  {
    if ((c->logIndex != MMI_PROTOCOL_V2_EVENT_CURSOR_NONE)
        && (c->logIndex < UINT16_MAX)
        && (IsLogIndexValid(c->services, (uint16_t) (c->logIndex + 1U)) != 0U))
    {
      c->logIndex++;
    }

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
