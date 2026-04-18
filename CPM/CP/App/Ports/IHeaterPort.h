/* App/Ports/IHeaterPort.h
 *
 * Port interface for the enclosure heater (active-high GPIO, PE10).
 * The heater is enabled/disabled by the domain based on temperature
 * thresholds or environmental monitoring logic.
 */
#ifndef IHEATER_PORT_H
#define IHEATER_PORT_H

#include <stddef.h>
#include <stdint.h>

typedef struct
{
  void *ctx;

  void (*Enable)(void *ctx);
  void (*Disable)(void *ctx);
  uint8_t (*GetState)(void *ctx);
} IHeaterPort_t;

static inline void HeaterEnable(IHeaterPort_t *p)
{
  p->Enable(p->ctx);
}

static inline void HeaterDisable(IHeaterPort_t *p)
{
  p->Disable(p->ctx);
}

static inline uint8_t HeaterGetState(IHeaterPort_t *p)
{
  if ((p == NULL) || (p->GetState == NULL))
  {
    return 0U;
  }

  return p->GetState(p->ctx);
}

#endif /* IHEATER_PORT_H */
