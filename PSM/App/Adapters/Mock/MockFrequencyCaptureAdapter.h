/**
 ******************************************************************************
 * @file    Adapters/Mock/MockFrequencyCaptureAdapter.h
 * @brief   Mock adapter for IFrequencyCapturePort.
 *
 *          Tests can pre-set fields on the context to drive specific port
 *          behaviour: publishOnEdge controls whether a fed edge triggers
 *          the publish callback, publishFreq is the value published, and
 *          eEvaluateVerdict is returned verbatim by Evaluate().
 ******************************************************************************
 */

#ifndef ADAPTERS_MOCK_MOCKFREQUENCYCAPTUREADAPTER_H
#define ADAPTERS_MOCK_MOCKFREQUENCYCAPTUREADAPTER_H

#include "Ports/IFrequencyCapturePort.h"

typedef void (*MockFrequencyPublishCb_t)(uint8_t freqHz);

typedef struct
{
  /* Fed by the mock on each call */
  uint32_t                 restartCount;
  uint32_t                 lastRestartNowMs;
  uint32_t                 onEdgeCount;
  uint32_t                 lastCaptureValue;
  uint32_t                 lastOnEdgeNowMs;
  uint32_t                 evaluateCount;
  uint32_t                 lastEvaluateNowMs;

  /* Controls what the mock does (pre-set by tests) */
  MockFrequencyPublishCb_t publish;          /* callback invoked on OnEdge if publishOnEdge */
  uint8_t                  publishOnEdge;
  uint8_t                  publishFreq;
  uint8_t                  eEvaluateVerdict; /* FrequencyCaptureVerdict_e */
} MockFrequencyCaptureAdapterCtx_t;

void                    MockFrequencyCaptureAdapterInit(
    MockFrequencyCaptureAdapterCtx_t *ctx);
IFrequencyCapturePort_t MockFrequencyCaptureAdapterCreatePort(
    MockFrequencyCaptureAdapterCtx_t *ctx);

#endif /* ADAPTERS_MOCK_MOCKFREQUENCYCAPTUREADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
