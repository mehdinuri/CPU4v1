/* App/Domain/Lcd/LcdPage.h
 *
 * Interface for an LCD menu page.
 * Every screen (Home, Login, Settings, etc.) implements this vtable.
 */
#ifndef LCD_PAGE_H
#define LCD_PAGE_H

#include <stdint.h>
#include "Ports/IDisplayPort.h"

typedef struct LcdPage LcdPage_t;
typedef struct LcdEngine LcdEngine_t;

#include <stdint.h>
#include "Ports/IDisplayPort.h"

struct LcdPage
{
  void *ctx;

  /* Called once when the page becomes active. */
  void (*OnEnter)(void *ctx, LcdEngine_t *engine);

  /* Called periodically (every 100ms).
   * tickMs: milliseconds elapsed since last call. */
  void (*OnUpdate)(void *ctx, LcdEngine_t *engine, uint32_t tickMs);

  /* Called when a key is pressed.
   * key: logical key code (KEY_0, KEY_ENTER, etc.). */
  void (*OnInput)(void *ctx, LcdEngine_t *engine, uint8_t key);

  /* Called to render the page to the display. */
  void (*OnDraw)(void *ctx, LcdEngine_t *engine, IDisplayPort_t *display);

  /* Called once before switching to a different page. */
  void (*OnExit)(void *ctx, LcdEngine_t *engine);
};

#endif /* LCD_PAGE_H */
