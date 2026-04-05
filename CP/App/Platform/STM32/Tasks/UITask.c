/*
 * Platform/STM32/Tasks/UITask.c
 *
 * FreeRTOS task that scans the keypad matrix, translates pressed keys to
 * UserCommand_t values, and renders the operator LCD menu.
 *
 * Priority : osPriorityLow
 * Period   : 100 ms
 * Argument : UITaskArgs_t*  (injected from main_stm32.c)
 *
 * Keypad GPIO EXTI callbacks call KeypadAdapter_PushCommand() directly;
 * this task's scan loop provides a debounce fallback for long-press keys.
 *
 * LCD rendering calls Display_Clear() / Display_PrintAt() / Display_Flush()
 * on the IDisplayPort_t.  Actual screen content depends on the active menu
 * state (TODO: implement menu state machine).
 */
#include "Tasks.h"
#include "Adapters/STM32/KeypadAdapter.h"
#include "Adapters/STM32/LCDAdapter.h"
#include "Ports/IDisplayPort.h"
#include "Ports/IUserInputPort.h"
#include "Domain/Intersection/Program.h"

/* Task argument struct injected from main_stm32.c. */
typedef struct
{
  IDisplayPort_t         *display;     /* LCD adapter port            */
  IUserInputPort_t       *input;       /* Keypad adapter port         */
  const ProgramCtx_t     *Program;     /* Read-only access to state   */
} UITaskArgs_t;

/* Refresh period in milliseconds. */
#define UI_REFRESH_MS  100U

void UITask(void *argument)
{
  UITaskArgs_t *args = (UITaskArgs_t *) argument;

  for (;;)
  {
    osDelay(UI_REFRESH_MS);

    if (args == NULL)
    {
      continue;
    }

    /* --- Keypad scan (GPIO debounce fallback) --- */

    /* TODO: Scan keypad matrix GPIO rows/columns here for long-press.
     * EXTI-driven pushes are handled in the GPIO callback.
     *
     * uint8_t row, col;
     * if (Keypad_ScanMatrix(&row, &col)) {
     *     UserCommand_t cmd = Keypad_MapToCommand(row, col);
     *     KeypadAdapter_PushCommand(keypadCtx, cmd);
     * }
     */

    /* --- LCD render --- */
    if ((args->display != NULL) && (args->Program != NULL) )
    {
      IDisplayPort_t *disp = args->display;
      char lineBuf[LCD_COLS_MAX + 1U];

      Display_Clear(disp);

      /* Line 0: Controller state */
      ControllerState_t state = ProgramGetState(args->Program);
      const char *stateStr = "UNKNOWN";

      switch (state)
      {
          case CTRL_STATE_PHASE:
          { stateStr = "PHASE";      break; }

          case CTRL_STATE_PHASE_TRANSITION:
          { stateStr = "TRANSIT";    break; }

          case CTRL_STATE_ALL_RED:
          { stateStr = "ALL RED";    break; }

          case CTRL_STATE_FLASH:
          { stateStr = "FLASH";      break; }

          case CTRL_STATE_DARK:
          { stateStr = "DARK";       break; }

          case CTRL_STATE_SEQUENCE:
          { stateStr = "SEQUENCE";   break; }

          default:
          { stateStr = "INIT";       break; }
      }

      /* Build "STATE: <state>" in lineBuf — no sprintf to avoid libc pull-in. */
      const char *prefix = "ST:";
      uint8_t pos = 0U;

      for (; prefix[pos] != '\0' && pos < LCD_COLS_MAX; pos++)
      {
        lineBuf[pos] = prefix[pos];
      }

      for (uint8_t i = 0U;
           stateStr[i] != '\0' && pos < LCD_COLS_MAX;
           i++, pos++)
      {
        lineBuf[pos] = stateStr[i];
      }

      lineBuf[pos] = '\0';
      Display_PrintAt(disp, 0U, 0U, lineBuf);

      /* Line 1: Active phase index */
      /* TODO: Display more status once menu state machine is implemented. */
      Display_PrintAt(disp, 0U, 1U, "PH:-- TK:------");

      Display_Flush(disp);
    }
  }
} /* UITask */
