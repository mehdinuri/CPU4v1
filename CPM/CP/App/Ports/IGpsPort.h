/* App/Ports/IGpsPort.h
 */
#ifndef IGPS_PORT_H
#define IGPS_PORT_H

#include <stdint.h>

typedef struct
{
  void *ctx;

  uint8_t (*GetPortType)(void *ctx);
  void (*SetPortType)(void *ctx, uint8_t type);
  uint8_t (*GetBaudRateIndex)(void *ctx);
  void (*SetBaudRateIndex)(void *ctx, uint8_t index);
  uint32_t (*IndexToBaudRate)(void *ctx, uint8_t index);
  uint8_t (*SaveConfig)(void *ctx);
  uint8_t (*IsValidPortType)(void *ctx, uint8_t type);
  uint8_t (*IsValidBaudRateIndex)(void *ctx, uint8_t index);
} IGpsPort_t;

static inline uint8_t IGpsPort_GetPortType(IGpsPort_t *p)
{
  return p->GetPortType(p->ctx);
}

static inline void IGpsPort_SetPortType(IGpsPort_t *p, uint8_t type)
{
  p->SetPortType(p->ctx, type);
}

static inline uint8_t IGpsPort_GetBaudRateIndex(IGpsPort_t *p)
{
  return p->GetBaudRateIndex(p->ctx);
}

static inline void IGpsPort_SetBaudRateIndex(IGpsPort_t *p, uint8_t index)
{
  p->SetBaudRateIndex(p->ctx, index);
}

static inline uint32_t IGpsPort_IndexToBaudRate(IGpsPort_t *p, uint8_t index)
{
  return p->IndexToBaudRate(p->ctx, index);
}

static inline uint8_t IGpsPort_SaveConfig(IGpsPort_t *p)
{
  return p->SaveConfig(p->ctx);
}

static inline uint8_t IGpsPort_IsValidPortType(IGpsPort_t *p, uint8_t type)
{
  return p->IsValidPortType(p->ctx, type);
}

static inline uint8_t IGpsPort_IsValidBaudRateIndex(IGpsPort_t *p,
                                                    uint8_t index)
{
  return p->IsValidBaudRateIndex(p->ctx, index);
}

#endif /* IGPS_PORT_H */
