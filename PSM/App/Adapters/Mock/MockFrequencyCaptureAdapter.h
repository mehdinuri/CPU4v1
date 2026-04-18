/**
 ******************************************************************************
 * @file    Adapters/Mock/MockFrequencyCaptureAdapter.h
 * @brief   Mock adapter for IFrequencyCapturePort.
 *
 *          Tests can pre-set fields on the context to drive specific port
 *          behaviour: bPublishOnEdge controls whether a fed edge triggers
 *          the publish callback, bPublishFreq is the value published, and
 *          eEvaluateVerdict is returned verbatim by Evaluate().
 ******************************************************************************
 */

#ifndef ADAPTERS_MOCK_MOCKFREQUENCYCAPTUREADAPTER_H
#define ADAPTERS_MOCK_MOCKFREQUENCYCAPTUREADAPTER_H

#include "Ports/IFrequencyCapturePort.h"

typedef void (*MockFrequencyPublishCb_t)(uint8_t bFreqHz);

typedef struct
{
  /* Fed by the mock on each call */
  uint32_t                 lRestartCount;
  uint32_t                 lLastRestartNowMs;
  uint32_t                 lOnEdgeCount;
  uint32_t                 lLastCaptureValue;
  uint32_t                 lLastOnEdgeNowMs;
  uint32_t                 lEvaluateCount;
  uint32_t                 lLastEvaluateNowMs;

  /* Controls what the mock does (pre-set by tests) */
  MockFrequencyPublishCb_t publish;          /* callback invoked on OnEdge if fPublishOnEdge */
  uint8_t                  fPublishOnEdge;
  uint8_t                  bPublishFreq;
  uint8_t                  eEvaluateVerdict; /* tEFrequencyCaptureVerdict */
} MockFrequencyCaptureAdapterCtx_t;

void                    MockFrequencyCaptureAdapterInit(
    MockFrequencyCaptureAdapterCtx_t *ctx);
IFrequencyCapturePort_t MockFrequencyCaptureAdapterCreatePort(
    MockFrequencyCaptureAdapterCtx_t *ctx);

#endif /* ADAPTERS_MOCK_MOCKFREQUENCYCAPTUREADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
