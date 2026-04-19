/* App/Ports/IBrokenInputSettingsPort.h
 *
 * Port for persisted broken-input filter settings.
 */
#ifndef IBROKEN_INPUT_SETTINGS_PORT_H
#define IBROKEN_INPUT_SETTINGS_PORT_H

#include <stdint.h>

typedef struct
{
  uint8_t loopInputFlag;
  uint8_t digitalInputFlag;
} BrokenInputSettings_t;

typedef struct
{
  void *ctx;

  uint8_t (*Read)(void *ctx);
  void (*Get)(void *ctx, BrokenInputSettings_t *settings);
  void (*Set)(void *ctx, const BrokenInputSettings_t *settings);
  uint8_t (*Save)(void *ctx);
} IBrokenInputSettingsPort_t;

static inline uint8_t BrokenInputSettingsPort_Read(IBrokenInputSettingsPort_t *p)
{
  return p->Read(p->ctx);
}

static inline void BrokenInputSettingsPort_Get(IBrokenInputSettingsPort_t *p,
                                               BrokenInputSettings_t *settings)
{
  p->Get(p->ctx, settings);
}

static inline void BrokenInputSettingsPort_Set(
  IBrokenInputSettingsPort_t *p,
  const BrokenInputSettings_t *settings)
{
  p->Set(p->ctx, settings);
}

static inline uint8_t BrokenInputSettingsPort_Save(IBrokenInputSettingsPort_t *p)
{
  return p->Save(p->ctx);
}

#endif /* IBROKEN_INPUT_SETTINGS_PORT_H */
