/**
 ******************************************************************************
 * @file    Ports/IIndicatorLEDPort.h
 * @brief   Port interface for indicator LED outputs (DCOK, ACOK, Error, COM).
 *          Implementations: IndicatorLEDAdapter (STM32), MockIndicatorLEDAdapter (Host).
 ******************************************************************************
 */

#ifndef PORTS_IINDICATORLEDPORT_H
#define PORTS_IINDICATORLEDPORT_H

#include <stdint.h>

/* ---------------------------------------------------------------------------
 * LED identifiers
 * ---------------------------------------------------------------------------*/
typedef enum
{
  LED_ID_DCOK  = 0,
  LED_ID_ACOK  = 1,
  LED_ID_ERROR = 2,
  LED_ID_COM   = 3
} IndicatorLEDId_t;

/* ---------------------------------------------------------------------------
 * Port interface
 * ---------------------------------------------------------------------------*/
typedef struct
{
  void *ctx;
  void (*SetState)(void *ctx, IndicatorLEDId_t id, uint8_t on);
  void (*Toggle)(void *ctx, IndicatorLEDId_t id);
} IIndicatorLEDPort_t;

/* ---------------------------------------------------------------------------
 * Zero-cost inline dispatch helpers
 * ---------------------------------------------------------------------------*/
static inline void IndicatorLED_SetState(IIndicatorLEDPort_t *p,
                                          IndicatorLEDId_t id,
                                          uint8_t on)
{
  p->SetState(p->ctx, id, on);
}

static inline void IndicatorLED_Toggle(IIndicatorLEDPort_t *p,
                                        IndicatorLEDId_t id)
{
  p->Toggle(p->ctx, id);
}

#endif /* PORTS_IINDICATORLEDPORT_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
