/**
 ******************************************************************************
 * @file    Ports/IFrequencyCapturePort.h
 * @brief   Port interface for line-frequency edge capture and validation.
 *          Called from the TIM2 IC/OC ISRs.  STM32 adapter owns the FSM
 *          state and publishes each new measurement; mock lets tests drive
 *          arbitrary verdicts and frequencies.
 ******************************************************************************
 */

#ifndef PORTS_IFREQUENCYCAPTUREPORT_H
#define PORTS_IFREQUENCYCAPTUREPORT_H

#include <stdint.h>

/* ---------------------------------------------------------------------------
 * Evaluate verdict — matches tEFrequencyCaptureFSMVerdict in the domain
 * FSM.  Duplicated here so Core/Src consumers do not have to include the
 * Domain FSM header.
 * ---------------------------------------------------------------------------*/
typedef enum
{
  FREQ_CAPTURE_VERDICT_OK           = 0,  /* in tolerance band          */
  FREQ_CAPTURE_VERDICT_BAD          = 1,  /* out of band / no recent edge */
  FREQ_CAPTURE_VERDICT_ENTER_SLEEP  = 2   /* strike threshold reached   */
} tEFrequencyCaptureVerdict;

/* ---------------------------------------------------------------------------
 * Port interface
 * ---------------------------------------------------------------------------*/
typedef struct
{
  void *ctx;
  void   (*Restart)(void *ctx, uint32_t lNowMs);
  void   (*OnEdge) (void *ctx, uint32_t lCaptureValue, uint32_t lNowMs);
  uint8_t (*Evaluate)(void *ctx, uint32_t lNowMs); /* returns tEFrequencyCaptureVerdict */
} IFrequencyCapturePort_t;

/* ---------------------------------------------------------------------------
 * Zero-cost inline dispatch helpers
 * ---------------------------------------------------------------------------*/
static inline void FrequencyCapture_Restart(IFrequencyCapturePort_t *p,
                                             uint32_t lNowMs)
{
  p->Restart(p->ctx, lNowMs);
}

static inline void FrequencyCapture_OnEdge(IFrequencyCapturePort_t *p,
                                            uint32_t lCaptureValue,
                                            uint32_t lNowMs)
{
  p->OnEdge(p->ctx, lCaptureValue, lNowMs);
}

static inline tEFrequencyCaptureVerdict FrequencyCapture_Evaluate(
    IFrequencyCapturePort_t *p,
    uint32_t lNowMs)
{
  return (tEFrequencyCaptureVerdict) p->Evaluate(p->ctx, lNowMs);
}

#endif /* PORTS_IFREQUENCYCAPTUREPORT_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
