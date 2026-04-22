/* App/Domain/Lcd/LcdPage_Home.c */
#include "LcdPage.h"
#include "LcdEngine.h"
#include "LcdEventText.h"
#include "LcdLanguage.h"
#include "LcdPageRegistry.h"
#include "LcdServiceRegistry.h"

#include <stdio.h>
#include <string.h>

#include "Domain/Intersection/IntersectionEngine.h"
#include "Ports/IControllerModeControlPort.h"
#include "Ports/IUserPort.h"
#include "Ports/UserInputTypes.h"

enum
{
  HOME_SHORTCUT_TIMEOUT_MS = 3000U,
  HOME_SHORTCUT_NONE = 0U,
  HOME_SHORTCUT_ALL_RED = 1U,
  HOME_SHORTCUT_DARK = 2U,
  HOME_SHORTCUT_FLASH = 3U,
  HOME_SHORTCUT_OUTPUT_TEST = 9U,
  HOME_SHORTCUT_PLAN_RETURN = 0xFFU
};

typedef struct
{
  const LcdServiceRegistry_t *services;
  const LcdPageRegistry_t *pages;
  uint8_t pendingShortcut;
  uint32_t pendingShortcutMs;
} HomeCtx_t;

static char IntervalToChar(uint8_t interval)
{
  switch ((IntersectionPhaseInterval_t) interval)
  {
      case INTERSECTION_PHASE_INTERVAL_GREEN:
      {
        return 'G';
      }

      case INTERSECTION_PHASE_INTERVAL_YELLOW:
      {
        return 'Y';
      }

      case INTERSECTION_PHASE_INTERVAL_RED:
      {
        return 'R';
      }

      case INTERSECTION_PHASE_INTERVAL_RED_CLEAR:
      {
        return 'r';
      }

      default:
      {
        return '-';
      }
  }
}

static const char *ModeToText(uint8_t lang, uint8_t mode)
{
  switch ((IntersectionControlMode_t) mode)
  {
      case INTERSECTION_CONTROL_MODE_COORDINATED:
      {
        return (lang == LANGUAGE_TURKISH) ? "KOOR" : "COOR";
      }

      case INTERSECTION_CONTROL_MODE_PREEMPT:
      {
        return (lang == LANGUAGE_TURKISH) ? "ONCL" : "PREM";
      }

      case INTERSECTION_CONTROL_MODE_FLASH:
      {
        return "FLSH";
      }

      case INTERSECTION_CONTROL_MODE_ALL_RED:
      {
        return (lang == LANGUAGE_TURKISH) ? "KIRM" : "ARED";
      }

      case INTERSECTION_CONTROL_MODE_DARK:
      {
        return "DARK";
      }

      case INTERSECTION_CONTROL_MODE_FREE:
      default:
      {
        return (lang == LANGUAGE_TURKISH) ? "SERB" : "FREE";
      }
  }
}

static const char *SafetyToText(uint8_t outputTestEnabled,
                                uint8_t safetyAction)
{
  if (outputTestEnabled != 0U)
  {
    return "TEST";
  }

  switch ((CpMpSafetyAction_t) safetyAction)
  {
      case CPMP_SAFETY_ACTION_FLASH:
      {
        return "FLSH";
      }

      case CPMP_SAFETY_ACTION_DARK:
      {
        return "DARK";
      }

      case CPMP_SAFETY_ACTION_NORMAL:
      default:
      {
        return "NORM";
      }
  }
}

static uint8_t CountActiveVehicleDetectors(const MmiSnapshotCache_t *cache)
{
  uint8_t detectorNumber;
  uint8_t activeCount = 0U;
  MmiRuntimeVehicleDetectorRecordV2_t record;

  if (cache == NULL)
  {
    return 0U;
  }

  for (detectorNumber = 1U;
       detectorNumber <= INTERSECTION_VEHICLE_DETECTOR_COUNT_MAX;
       detectorNumber++)
  {
    if ((MmiSnapshotCacheGetVehicleDetectorRecord(cache,
                                                  detectorNumber,
                                                  &record) != 0U)
        && (record.inputActive != 0U))
    {
      activeCount++;
    }
  }

  return activeCount;
}

static uint8_t CountActivePedestrianDetectors(const MmiSnapshotCache_t *cache)
{
  uint8_t detectorNumber;
  uint8_t activeCount = 0U;
  MmiRuntimePedestrianDetectorRecordV2_t record;

  if (cache == NULL)
  {
    return 0U;
  }

  for (detectorNumber = 1U;
       detectorNumber <= INTERSECTION_PED_INPUT_COUNT_MAX;
       detectorNumber++)
  {
    if ((MmiSnapshotCacheGetPedestrianDetectorRecord(cache,
                                                     detectorNumber,
                                                     &record) != 0U)
        && (record.inputActive != 0U))
    {
      activeCount++;
    }
  }

  return activeCount;
}

static uint8_t FindFirstForcedOutput(const MmiRuntimeOutputTestSummaryV2_t *summary,
                                     uint8_t *channelNumber,
                                     const char **aspectText)
{
  uint8_t channelIndex;

  if ((summary == NULL) || (channelNumber == NULL) || (aspectText == NULL))
  {
    return 0U;
  }

  for (channelIndex = 0U; channelIndex < INTERSECTION_CHANNEL_COUNT_MAX;
       channelIndex++)
  {
    uint32_t mask = (uint32_t) (1UL << channelIndex);

    if ((summary->forcedMask & mask) == 0U)
    {
      continue;
    }

    *channelNumber = (uint8_t) (channelIndex + 1U);
    if ((summary->redMask & mask) != 0U)
    {
      *aspectText = "RED";
    }
    else if ((summary->yellowMask & mask) != 0U)
    {
      *aspectText = "YEL";
    }
    else if ((summary->greenMask & mask) != 0U)
    {
      *aspectText = "GRN";
    }
    else
    {
      *aspectText = "DRK";
    }

    return 1U;
  }

  return 0U;
}

static const char *PendingShortcutToText(uint8_t shortcut)
{
  switch (shortcut)
  {
      case HOME_SHORTCUT_ALL_RED:
      {
        return "1";
      }

      case HOME_SHORTCUT_DARK:
      {
        return "2";
      }

      case HOME_SHORTCUT_FLASH:
      {
        return "3";
      }

      case HOME_SHORTCUT_OUTPUT_TEST:
      {
        return "9";
      }

      case HOME_SHORTCUT_PLAN_RETURN:
      {
        return "C";
      }

      default:
      {
        return "-";
      }
  }
}

static void ClearPendingShortcut(HomeCtx_t *ctx)
{
  if (ctx != NULL)
  {
    ctx->pendingShortcut = HOME_SHORTCUT_NONE;
    ctx->pendingShortcutMs = 0U;
  }
}

static void ArmShortcut(HomeCtx_t *ctx, uint8_t shortcut)
{
  if (ctx != NULL)
  {
    ctx->pendingShortcut = shortcut;
    ctx->pendingShortcutMs = HOME_SHORTCUT_TIMEOUT_MS;
  }
}

static void DrawSummaryLine(const HomeCtx_t *ctx,
                            IDisplayPort_t *display,
                            uint8_t lang,
                            const MmiRuntimeSummaryV2_t *summary,
                            const MmiRuntimeOutputTestSummaryV2_t *outputTest)
{
  char buf[21];
  uint8_t actionNumber = 0U;

  if ((summary != NULL) && (summary->timebaseActionStatus != 0U))
  {
    actionNumber = summary->timebaseActionStatus;
  }
  else if ((summary != NULL) && (summary->coordPatternStatus <= 250U))
  {
    actionNumber = summary->coordPatternStatus;
  }
  else if (summary != NULL)
  {
    actionNumber = summary->activeSequenceNumber;
  }

  (void) snprintf(buf,
                  sizeof(buf),
                  "M:%-4s A:%02u S:%-4s",
                  ModeToText(lang, (summary == NULL) ? 0U : summary->mode),
                  actionNumber,
                  SafetyToText((outputTest == NULL) ? 0U : outputTest->enabled,
                               (summary == NULL) ? CPMP_SAFETY_ACTION_NORMAL
                               : summary->safetyAction));
  DisplayWrite(display, 0U, 0U, &buf[0], (uint8_t) strlen(buf));
}

static void DrawRingLine(const MmiSnapshotCache_t *cache,
                         IDisplayPort_t *display,
                         uint8_t row,
                         uint8_t ringNumber,
                         uint8_t firstPhaseNumber)
{
  char buf[21];
  MmiRuntimeRingRecordV2_t ringRecord;
  MmiRuntimePhaseRecordV2_t phaseRecord;
  char phaseChars[5];
  uint8_t phaseOffset;
  unsigned long elapsedSeconds;

  (void) memset(&ringRecord, 0, sizeof(ringRecord));
  (void) memset(&phaseRecord, 0, sizeof(phaseRecord));
  (void) memset(&phaseChars[0], '-', sizeof(phaseChars));
  phaseChars[4] = '\0';

  if (cache != NULL)
  {
    (void) MmiSnapshotCacheGetRingRecord(cache, ringNumber, &ringRecord);
    for (phaseOffset = 0U; phaseOffset < 4U; phaseOffset++)
    {
      if (MmiSnapshotCacheGetPhaseRecord(cache,
                                         (uint8_t) (firstPhaseNumber
                                                    + phaseOffset),
                                         &phaseRecord) != 0U)
      {
        phaseChars[phaseOffset] = IntervalToChar(phaseRecord.interval);
      }
    }
  }

  elapsedSeconds = (unsigned long) (ringRecord.stageElapsedTicks / 100U);
  if (elapsedSeconds > 999UL)
  {
    elapsedSeconds = 999UL;
  }

  (void) snprintf(buf,
                  sizeof(buf),
                  "R%u P%u %s %3lus",
                  ringNumber,
                  ringRecord.activePhaseNumber,
                  &phaseChars[0],
                  elapsedSeconds);
  DisplayWrite(display, row, 0U, &buf[0], (uint8_t) strlen(buf));
}

static void DrawStatusLine(const HomeCtx_t *ctx,
                           IDisplayPort_t *display,
                           uint8_t lang,
                           const MmiRuntimeSummaryV2_t *summary,
                           const MmiRuntimeRelaySummaryV2_t *relay,
                           const MmiRuntimeOutputTestSummaryV2_t *outputTest,
                           const MmiRuntimeDoorSummaryV2_t *door)
{
  char buf[21];
  uint8_t firstForcedChannel = 0U;
  const char *forcedAspect = NULL;

  if ((ctx->pendingShortcut != HOME_SHORTCUT_NONE)
      && (ctx->pendingShortcutMs != 0U))
  {
    (void) snprintf(buf,
                    sizeof(buf),
                    "%s:%s+ENT",
                    (lang == LANGUAGE_TURKISH) ? "KOMUT" : "CMD",
                    PendingShortcutToText(ctx->pendingShortcut));
  }
  else if ((outputTest != NULL) && (outputTest->enabled != 0U))
  {
    if (FindFirstForcedOutput(outputTest,
                              &firstForcedChannel,
                              &forcedAspect) != 0U)
    {
      (void) snprintf(buf,
                      sizeof(buf),
                      "TEST C%02u %s P:%u",
                      firstForcedChannel,
                      forcedAspect,
                      (relay == NULL) ? 0U : relay->permitOutputPower);
    }
    else
    {
      (void) snprintf(buf,
                      sizeof(buf),
                      "TEST READY P:%u",
                      (relay == NULL) ? 0U : relay->permitOutputPower);
    }
  }
  else if ((summary != NULL) && (summary->safetyReasonCode != 0U))
  {
    const char *faultText =
      LcdEventText_GetSafetyReasonShort(summary->safetyReasonCode, lang);

    (void) snprintf(buf,
                    sizeof(buf),
                    "FLT:%-4s D:%c E:%c",
                    faultText,
                    ((door != NULL) && (door->open != 0U)) ? 'O' : 'C',
                    ((relay != NULL) && (relay->permitOutputPower != 0U))
                    ? '1' : '0');
  }
  else
  {
    uint8_t activeVehicleCount =
      CountActiveVehicleDetectors(ctx->services->runtimeCache);
    uint8_t activePedestrianCount =
      CountActivePedestrianDetectors(ctx->services->runtimeCache);

    (void) snprintf(buf,
                    sizeof(buf),
                    "V:%02u P:%02u D:%c E:%c",
                    activeVehicleCount,
                    activePedestrianCount,
                    ((door != NULL) && (door->open != 0U)) ? 'O' : 'C',
                    ((relay != NULL) && (relay->permitOutputPower != 0U))
                    ? '1' : '0');
  }

  DisplayWrite(display, 3U, 0U, &buf[0], (uint8_t) strlen(buf));
}

static void OnDraw(void *ctx, LcdEngine_t *e, IDisplayPort_t *display)
{
  HomeCtx_t *c = (HomeCtx_t *) ctx;
  uint8_t lang = ISystemPort_GetLanguage(c->services->system);
  MmiRuntimeSummaryV2_t summary;
  MmiRuntimeRelaySummaryV2_t relay;
  MmiRuntimeOutputTestSummaryV2_t outputTest;
  MmiRuntimeDoorSummaryV2_t door;

  (void) e;
  (void) memset(&summary, 0, sizeof(summary));
  (void) memset(&relay, 0, sizeof(relay));
  (void) memset(&outputTest, 0, sizeof(outputTest));
  (void) memset(&door, 0, sizeof(door));

  DisplayClear(display);
  if (c->services->runtimeCache != NULL)
  {
    (void) MmiSnapshotCacheGetSummary(c->services->runtimeCache, &summary);
    (void) MmiSnapshotCacheGetRelaySummary(c->services->runtimeCache, &relay);
    (void) MmiSnapshotCacheGetOutputTestSummary(c->services->runtimeCache,
                                                &outputTest);
    (void) MmiSnapshotCacheGetDoorSummary(c->services->runtimeCache, &door);
  }

  DrawSummaryLine(c, display, lang, &summary, &outputTest);
  DrawRingLine(c->services->runtimeCache, display, 1U, 1U, 1U);
  DrawRingLine(c->services->runtimeCache, display, 2U, 2U, 5U);
  DrawStatusLine(c, display, lang, &summary, &relay, &outputTest, &door);
}

static void ExecuteShortcut(HomeCtx_t *ctx, LcdEngine_t *engine)
{
  if ((ctx == NULL) || (ctx->services == NULL) || (ctx->services->maintenance == NULL))
  {
    return;
  }

  switch (ctx->pendingShortcut)
  {
      case HOME_SHORTCUT_ALL_RED:
      {
        (void) MmiMaintenanceServiceRequestModeControl(
          ctx->services->maintenance,
          CONTROLLER_MODE_REQUEST_ALL_RED);
        break;
      }

      case HOME_SHORTCUT_DARK:
      {
        (void) MmiMaintenanceServiceRequestModeControl(
          ctx->services->maintenance,
          CONTROLLER_MODE_REQUEST_DARK);
        break;
      }

      case HOME_SHORTCUT_FLASH:
      {
        (void) MmiMaintenanceServiceRequestModeControl(
          ctx->services->maintenance,
          CONTROLLER_MODE_REQUEST_FLASH);
        break;
      }

      case HOME_SHORTCUT_PLAN_RETURN:
      {
        (void) MmiMaintenanceServiceRequestModeControl(
          ctx->services->maintenance,
          CONTROLLER_MODE_REQUEST_PLAN_RETURN);
        break;
      }

      case HOME_SHORTCUT_OUTPUT_TEST:
      {
        (void) MmiMaintenanceServiceStartOutputTest(ctx->services->maintenance);
        LcdEngine_SwitchPage(engine, ctx->pages->outputTest);
        break;
      }

      default:
      {
        break;
      }
  }

  ClearPendingShortcut(ctx);
}

static void OnUpdate(void *ctx, LcdEngine_t *e, uint32_t tickMs)
{
  HomeCtx_t *c = (HomeCtx_t *) ctx;

  (void) e;
  if ((c->pendingShortcut != HOME_SHORTCUT_NONE) && (c->pendingShortcutMs != 0U))
  {
    if (tickMs >= c->pendingShortcutMs)
    {
      ClearPendingShortcut(c);
    }
    else
    {
      c->pendingShortcutMs -= tickMs;
    }
  }
}

static void OnInput(void *ctx, LcdEngine_t *e, uint8_t key)
{
  HomeCtx_t *c = (HomeCtx_t *) ctx;
  uint8_t isAdmin = UserCanAccessConfiguration(c->services->user);

  if (key == KEY_ENTER)
  {
    if ((isAdmin != 0U) && (c->pendingShortcut != HOME_SHORTCUT_NONE))
    {
      ExecuteShortcut(c, e);
    }
    else if (UserGetActiveRole(c->services->user) == USER_ROLE_NONE)
    {
      LcdEngine_SwitchPage(e, c->pages->login);
    }
    else
    {
      LcdEngine_SwitchPage(e, c->pages->menu);
    }

    return;
  }

  if (isAdmin == 0U)
  {
    ClearPendingShortcut(c);
    return;
  }

  switch (key)
  {
      case KEY_1:
      {
        ArmShortcut(c, HOME_SHORTCUT_ALL_RED);
        break;
      }

      case KEY_2:
      {
        ArmShortcut(c, HOME_SHORTCUT_DARK);
        break;
      }

      case KEY_3:
      {
        ArmShortcut(c, HOME_SHORTCUT_FLASH);
        break;
      }

      case KEY_9:
      {
        ArmShortcut(c, HOME_SHORTCUT_OUTPUT_TEST);
        break;
      }

      case KEY_CLEAR:
      {
        ArmShortcut(c, HOME_SHORTCUT_PLAN_RETURN);
        break;
      }

      default:
      {
        ClearPendingShortcut(c);
        break;
      }
  }
}

static HomeCtx_t s_homeCtx;
LcdPage_t LcdPage_Home = {
  .ctx = &s_homeCtx,
  .OnEnter = NULL,
  .OnUpdate = OnUpdate,
  .OnInput = OnInput,
  .OnDraw = OnDraw,
  .OnExit = NULL
};

void LcdPage_Home_Init(HomeCtx_t *ctx,
                       const LcdServiceRegistry_t *services,
                       const LcdPageRegistry_t *pages,
                       const IntersectionEngine_t *engine)
{
  (void) engine;
  ctx->services = services;
  ctx->pages = pages;
  ClearPendingShortcut(ctx);
  LcdPage_Home.ctx = ctx;
}
