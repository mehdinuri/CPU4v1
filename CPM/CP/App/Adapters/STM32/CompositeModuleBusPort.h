/* App/Adapters/STM32/CompositeModuleBusPort.h */
#ifndef COMPOSITE_MODULE_BUS_PORT_H
#define COMPOSITE_MODULE_BUS_PORT_H

#include "Ports/IModuleBusPort.h"

typedef struct
{
  IModuleBusPort_t *fieldInputPort;
  IModuleBusPort_t *moduleBusPort;
} CompositeModuleBusPortCtx_t;

void CompositeModuleBusPortInit(CompositeModuleBusPortCtx_t *ctx,
                                IModuleBusPort_t *fieldInputPort,
                                IModuleBusPort_t *moduleBusPort);
IModuleBusPort_t CompositeModuleBusPortCreatePort(
  CompositeModuleBusPortCtx_t *ctx);

#endif /* COMPOSITE_MODULE_BUS_PORT_H */
