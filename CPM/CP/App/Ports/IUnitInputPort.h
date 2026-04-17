/* App/Ports/IUnitInputPort.h
 *
 * Port interface for local controller unit inputs that influence runtime
 * behavior without going through SNMP or the field bus.
 */
#ifndef IUNIT_INPUT_PORT_H
#define IUNIT_INPUT_PORT_H

#include <stddef.h>
#include <stdint.h>

typedef struct
{
  void *ctx;

  uint8_t (*GetDimmingInputActive)(void *ctx);
  uint8_t (*GetInterconnectCommand)(void *ctx);
  uint8_t (*GetInterconnectInputsValid)(void *ctx);
} IUnitInputPort_t;

static inline uint8_t UnitInputGetDimmingInputActive(IUnitInputPort_t *p)
{
  if ((p == NULL) || (p->GetDimmingInputActive == NULL))
  {
    return 0U;
  }

  return p->GetDimmingInputActive(p->ctx);
}

static inline uint8_t UnitInputGetInterconnectCommand(IUnitInputPort_t *p)
{
  if ((p == NULL) || (p->GetInterconnectCommand == NULL))
  {
    return 0U;
  }

  return p->GetInterconnectCommand(p->ctx);
}

static inline uint8_t UnitInputGetInterconnectInputsValid(IUnitInputPort_t *p)
{
  if ((p == NULL) || (p->GetInterconnectInputsValid == NULL))
  {
    return 1U;
  }

  return p->GetInterconnectInputsValid(p->ctx);
}

#endif /* IUNIT_INPUT_PORT_H */
