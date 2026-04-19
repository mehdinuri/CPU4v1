/* App/Ports/IUserSettingsPort.h
 *
 * Port for persisted user-visible runtime flags.
 */
#ifndef IUSER_SETTINGS_PORT_H
#define IUSER_SETTINGS_PORT_H

#include <stdint.h>

typedef struct
{
  uint8_t configFlag;
  uint8_t logFlag;
  uint8_t trafficCountsFlag;
  uint8_t standbyInfoFlag;
} UserSettingsFlags_t;

typedef struct
{
  void *ctx;

  uint8_t (*Read)(void *ctx);
  void (*Get)(void *ctx, UserSettingsFlags_t *settings);
  void (*Set)(void *ctx, const UserSettingsFlags_t *settings);
  uint8_t (*Save)(void *ctx);
} IUserSettingsPort_t;

static inline uint8_t UserSettingsPort_Read(IUserSettingsPort_t *p)
{
  return p->Read(p->ctx);
}

static inline void UserSettingsPort_Get(IUserSettingsPort_t *p,
                                        UserSettingsFlags_t *settings)
{
  p->Get(p->ctx, settings);
}

static inline void UserSettingsPort_Set(IUserSettingsPort_t *p,
                                        const UserSettingsFlags_t *settings)
{
  p->Set(p->ctx, settings);
}

static inline uint8_t UserSettingsPort_Save(IUserSettingsPort_t *p)
{
  return p->Save(p->ctx);
}

#endif /* IUSER_SETTINGS_PORT_H */
