/* App/Domain/Lcd/LcdEngine.c
 */
#include "LcdEngine.h"
#include <stddef.h>
#include <string.h>

void LcdEngine_Init(LcdEngine_t *e,
                    IDisplayPort_t *display,
                    IUserInputPort_t *keypad)
{
  memset(e, 0, sizeof(*e));
  e->display = display;
  e->keypad = keypad;
  e->lastSnapshot = KEYPAD_SNAPSHOT_NONE;
}

void LcdEngine_SwitchPage(LcdEngine_t *e, LcdPage_t *newPage)
{
  if (e->currentPage != NULL)
  {
    if (e->currentPage->OnExit != NULL)
    {
      e->currentPage->OnExit(e->currentPage->ctx, e);
    }
  }

  e->currentPage = newPage;

  if (e->currentPage != NULL)
  {
    if (e->currentPage->OnEnter != NULL)
    {
      e->currentPage->OnEnter(e->currentPage->ctx, e);
    }

    /* Force a clear and redraw on enter. */
    if (e->display != NULL)
    {
      DisplayClear(e->display);
      if (e->currentPage->OnDraw != NULL)
      {
        e->currentPage->OnDraw(e->currentPage->ctx, e, e->display);
      }
    }
  }
}

void LcdEngine_Tick(LcdEngine_t *e, uint32_t currentTickMs)
{
  uint32_t deltaMs = currentTickMs - e->lastTickMs;

  e->lastTickMs = currentTickMs;

  if (e->currentPage == NULL)
  {
    return;
  }

  /* Handle Update. */
  if (e->currentPage->OnUpdate != NULL)
  {
    e->currentPage->OnUpdate(e->currentPage->ctx, e, deltaMs);
  }

  /* Handle Input (Rising Edge detection). */
  if (e->keypad != NULL)
  {
    KeypadSnapshot_t snapshot = UserInputScanSnapshot(e->keypad);
    KeypadSnapshot_t pressed = snapshot & (~e->lastSnapshot);

    e->lastSnapshot = snapshot;

    if (pressed != KEYPAD_SNAPSHOT_NONE)
    {
      for (uint8_t i = 0; i < KEYPAD_KEY_COUNT; i++)
      {
        if (KeypadSnapshotHasKey(pressed, i))
        {
          if (e->currentPage->OnInput != NULL)
          {
            e->currentPage->OnInput(e->currentPage->ctx, e, i);
          }
        }
      }
    }
  }

  /* Handle Draw. */
  if (e->display != NULL)
  {
    if (e->currentPage->OnDraw != NULL)
    {
      e->currentPage->OnDraw(e->currentPage->ctx, e, e->display);
    }
  }
} /* LcdEngine_Tick */
