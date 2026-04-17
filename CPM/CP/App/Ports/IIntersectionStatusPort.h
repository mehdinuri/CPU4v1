/* App/Ports/IIntersectionStatusPort.h
 */
#ifndef IINTERSECTION_STATUS_PORT_H
#define IINTERSECTION_STATUS_PORT_H

#include <stdint.h>

typedef struct
{
  uint8_t bSigModeSource;
  uint8_t bParam1;
} LcdSetRuntime_t;

typedef struct
{
  void *ctx;

  uint8_t (*GetSetTotal)(void *ctx);
  uint8_t (*IsSetEmergent)(void *ctx, uint8_t setNo);
  uint8_t (*GetSetRuntime)(void *ctx, uint8_t setNo, LcdSetRuntime_t *runtime);
} IIntersectionStatusPort_t;

static inline uint8_t IntersectionStatusGetSetTotal(
  IIntersectionStatusPort_t *p)
{
  return p->GetSetTotal(p->ctx);
}

static inline uint8_t IntersectionStatusIsSetEmergent(
  IIntersectionStatusPort_t *p,
  uint8_t setNo)
{
  return p->IsSetEmergent(p->ctx, setNo);
}

static inline uint8_t IntersectionStatusGetSetRuntime(
  IIntersectionStatusPort_t *p,
  uint8_t setNo,
  LcdSetRuntime_t *runtime)
{
  return p->GetSetRuntime(p->ctx, setNo, runtime);
}

#endif /* IINTERSECTION_STATUS_PORT_H */
