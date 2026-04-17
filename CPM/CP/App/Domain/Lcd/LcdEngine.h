/* App/Domain/Lcd/LcdEngine.h
 *
 * Core UI state machine engine.
 * Manages page transitions and orchestrates input/rendering.
 */
#ifndef LCD_ENGINE_H
#define LCD_ENGINE_H

#include "LcdPage.h"
#include "Ports/IDisplayPort.h"
#include "Ports/IUserInputPort.h"

typedef struct LcdEngine LcdEngine_t;

#include "LcdPage.h"
#include "Ports/IDisplayPort.h"
#include "Ports/IUserInputPort.h"

struct LcdEngine
{
  IDisplayPort_t  *display;
  IUserInputPort_t *keypad;
  LcdPage_t       *currentPage;
  uint32_t lastTickMs;
  KeypadSnapshot_t lastSnapshot;
};

/* Initialize the engine with hardware ports. */
void LcdEngine_Init(LcdEngine_t *e,
                    IDisplayPort_t *display,
                    IUserInputPort_t *keypad);

/* Switch to a new page. */
void LcdEngine_SwitchPage(LcdEngine_t *e, LcdPage_t *newPage);

/* Tick the engine (call every 100ms). */
void LcdEngine_Tick(LcdEngine_t *e, uint32_t currentTickMs);

#endif /* LCD_ENGINE_H */
