/**
 ******************************************************************************
 * @file    Ports/IFrequencySensorPort.h
 * @brief   Port interface for AC grid frequency measurement.
 *          STM32: context field written by TIM2 input-capture ISR callback.
 *          Host: configurable uint8_t field set by test.
 ******************************************************************************
 */

#ifndef PORTS_IFREQUENCYSENSORPORT_H
#define PORTS_IFREQUENCYSENSORPORT_H

#include <stdint.h>

/* ---------------------------------------------------------------------------
 * Port interface
 * ---------------------------------------------------------------------------*/
typedef struct
{
  void *ctx;
  uint8_t (*GetNetFrequency)(void *ctx); /* AC grid frequency in Hz */
} IFrequencySensorPort_t;

/* ---------------------------------------------------------------------------
 * Zero-cost inline dispatch helper
 * ---------------------------------------------------------------------------*/
static inline uint8_t FrequencySensor_GetNetFrequency(IFrequencySensorPort_t *p)
{
  return p->GetNetFrequency(p->ctx);
}

#endif /* PORTS_IFREQUENCYSENSORPORT_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
