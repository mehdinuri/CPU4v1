/* App/Ports/ISystemPort.h
 */
#ifndef ISYSTEM_PORT_H
#define ISYSTEM_PORT_H

#include <stdint.h>

typedef struct
{
  void *ctx;

  uint16_t (*GetMainVoltage)(void *ctx);
  uint8_t (*GetTimeSource)(void *ctx);
  uint8_t (*GetLanguage)(void *ctx);
  void (*SetLanguage)(void *ctx, uint8_t lang);
} ISystemPort_t;

static inline uint16_t ISystemPort_GetMainVoltage(ISystemPort_t *p)
{
  return p->GetMainVoltage(p->ctx);
}

static inline uint8_t ISystemPort_GetTimeSource(ISystemPort_t *p)
{
  return p->GetTimeSource(p->ctx);
}

static inline uint8_t ISystemPort_GetLanguage(ISystemPort_t *p)
{
  return p->GetLanguage(p->ctx);
}

static inline void ISystemPort_SetLanguage(ISystemPort_t *p, uint8_t lang)
{
  p->SetLanguage(p->ctx, lang);
}

#endif /* ISYSTEM_PORT_H */
