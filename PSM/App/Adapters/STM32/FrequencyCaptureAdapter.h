/**
 ******************************************************************************
 * @file    Adapters/STM32/FrequencyCaptureAdapter.h
 * @brief   STM32 adapter for IFrequencyCapturePort — owns the domain
 *          FrequencyCaptureFSM state/config and publishes each new
 *          measurement via a caller-supplied callback.
 ******************************************************************************
 */

#ifndef ADAPTERS_STM32_FREQUENCYCAPTUREADAPTER_H
#define ADAPTERS_STM32_FREQUENCYCAPTUREADAPTER_H

#include "Ports/IFrequencyCapturePort.h"
#include "Domain/FrequencyCaptureFSM.h"

typedef void (*FrequencyPublishCb_t)(uint8_t freqHz);

typedef struct
{
  FrequencyCaptureFSMConfig_t config;
  FrequencyCaptureFSMState_t  state;
  FrequencyPublishCb_t        publish;  /* invoked on each new measurement */
} FrequencyCaptureAdapterCtx_t;

void FrequencyCaptureAdapterInit(FrequencyCaptureAdapterCtx_t *ctx,
                                  const FrequencyCaptureFSMConfig_t *cfg,
                                  FrequencyPublishCb_t publish,
                                  uint32_t nowMs);

IFrequencyCapturePort_t FrequencyCaptureAdapterCreatePort(
    FrequencyCaptureAdapterCtx_t *ctx);

#endif /* ADAPTERS_STM32_FREQUENCYCAPTUREADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
