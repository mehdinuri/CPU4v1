/**
 ******************************************************************************
 * @file    Adapters/Mock/MockVoltageSensorAdapter.h
 * @brief   Mock adapter for IVoltageSensorPort — configurable float values.
 ******************************************************************************
 */

#ifndef ADAPTERS_MOCK_MOCKVOLTAGESENSORADAPTER_H
#define ADAPTERS_MOCK_MOCKVOLTAGESENSORADAPTER_H

#include "Ports/IVoltageSensorPort.h"

typedef struct
{
  float netVoltage; /* pre-set by test to simulate AC grid voltage  */
  float regVIn;     /* pre-set by test to simulate DC Vin           */
  float regVOut;    /* pre-set by test to simulate DC Vout          */
} MockVoltageSensorAdapterCtx_t;

void                   MockVoltageSensorAdapterInit(MockVoltageSensorAdapterCtx_t *ctx);
IVoltageSensorPort_t   MockVoltageSensorAdapterCreatePort(MockVoltageSensorAdapterCtx_t *ctx);

#endif /* ADAPTERS_MOCK_MOCKVOLTAGESENSORADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
