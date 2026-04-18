/**
 ******************************************************************************
 * @file    Ports/ICurrentMeasurementPort.h
 * @brief   Port: sample the most recently measured per-group RMS currents.
 *          Adapters serve from a lock-free double-buffer populated by an
 *          ADC/DMA ISR. GetLatest returns the consistent snapshot at call time.
 ******************************************************************************
 */

#ifndef PORTS_ICURRENT_MEASUREMENT_PORT_H
#define PORTS_ICURRENT_MEASUREMENT_PORT_H

#include <stdint.h>

#define CURRENT_CHANNEL_COUNT 4U    /* Four signal groups */
#define CURRENT_MEASUREMENT_STATUS_SATURATED 0x01U

typedef struct
{
  uint16_t aCurrents_mA[CURRENT_CHANNEL_COUNT];      /* RMS current per group, in mA */
  uint8_t bStatus;                                   /* CURRENT_MEASUREMENT_STATUS_* */
  uint32_t lSeqNo;                                   /* monotonic sample counter (ok to wrap) */
} tSCurrentMeasurementSnapshot;

typedef struct ICurrentMeasurementPort
{
  void *pCtx;

  void (*GetLatest)(void *pCtx, tSCurrentMeasurementSnapshot *pOut);
} ICurrentMeasurementPort_t;

static inline void CurrentMeasurement_GetLatest(
  ICurrentMeasurementPort_t *pPort,
  tSCurrentMeasurementSnapshot *
  pOut)
{
  pPort->GetLatest(pPort->pCtx, pOut);
}

#endif /* PORTS_ICURRENT_MEASUREMENT_PORT_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
