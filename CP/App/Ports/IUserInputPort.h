#pragma once

/*
 * App/Ports/IUserInputPort.h
 *
 * Parsed user commands from keypad, USB HID, or serial ASCII interface.
 * The concrete adapter scans the keypad matrix / USB queue and translates
 * raw input to typed commands. The Domain polls this port each tick.
 */
#include <stdint.h>
#include <stdbool.h>

/* High-level operator commands — hardware-agnostic. */
typedef enum
{
  USER_CMD_NONE           = 0,
  USER_CMD_PHASE_ADVANCE,       /* Manually advance to the next phase */
  USER_CMD_ENTER_FLASH,         /* Enter flash (yellow flashing) mode */
  USER_CMD_ENTER_ALL_RED,       /* Force all-red hold */
  USER_CMD_ENTER_DARK,          /* Turn off all signal heads */
  USER_CMD_RESUME_NORMAL,       /* Return to normal operation */
  USER_CMD_POLICE_OVERRIDE,     /* Police manual control active */
  USER_CMD_MENU_UP,             /* LCD menu navigation */
  USER_CMD_MENU_DOWN,
  USER_CMD_MENU_SELECT,
  USER_CMD_MENU_BACK,
} UserCommand_t;

typedef struct IUserInputPort
{
  void *ctx;

  /* Return the next pending command (FIFO), or USER_CMD_NONE if empty. */
  UserCommand_t (*dequeue)(void *ctx);

  /* True if at least one command is pending. */
  bool (*hasCommand)(void *ctx);
} IUserInputPort_t;

static inline UserCommand_t UserInput_Dequeue(IUserInputPort_t *p)
{
  return p->dequeue(p->ctx);
}

static inline bool UserInput_HasCommand(IUserInputPort_t *p)
{
  return p->hasCommand(p->ctx);
}
