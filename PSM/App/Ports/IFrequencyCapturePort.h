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
 * Evaluate verdict — matches FrequencyCaptureFSMVerdict_e in the domain
 * FSM.  Duplicated here so Core/Src consumers do not have to include the
 * Domain FSM header.
 * ---------------------------------------------------------------------------*/
typedef enum
{
  FREQ_CAPTURE_VERDICT_OK           = 0,  /* in tolerance band          */
  FREQ_CAPTURE_VERDICT_BAD          = 1,  /* out of band / no recent edge */
  FREQ_CAPTURE_VERDICT_ENTER_FLASH  = 2   /* strike threshold reached — */
                                          /* driver should enter flash  */
                                          /* mode (not MCU standby)     */
} FrequencyCaptureVerdict_e;

/* ---------------------------------------------------------------------------
 * Port interface
 * ---------------------------------------------------------------------------*/
typedef struct
{
  void *ctx;
  void   (*Restart)(void *ctx, uint32_t nowMs);
  void   (*OnEdge) (void *ctx, uint32_t captureValue, uint32_t nowMs);
  uint8_t (*Evaluate)(void *ctx, uint32_t nowMs); /* returns FrequencyCaptureVerdict_e */
} IFrequencyCapturePort_t;

/* ---------------------------------------------------------------------------
 * Zero-cost inline dispatch helpers
 * ---------------------------------------------------------------------------*/
static inline void FrequencyCapture_Restart(IFrequencyCapturePort_t *p,
                                             uint32_t nowMs)
{
  p->Restart(p->ctx, nowMs);
}

static inline void FrequencyCapture_OnEdge(IFrequencyCapturePort_t *p,
                                            uint32_t captureValue,
                                            uint32_t nowMs)
{
  p->OnEdge(p->ctx, captureValue, nowMs);
}

static inline FrequencyCaptureVerdict_e FrequencyCapture_Evaluate(
    IFrequencyCapturePort_t *p,
    uint32_t nowMs)
{
  return (FrequencyCaptureVerdict_e) p->Evaluate(p->ctx, nowMs);
}

#endif /* PORTS_IFREQUENCYCAPTUREPORT_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
