/**
 ******************************************************************************
 * @file    Adapters/STM32/AdcCurrentAdapter.h
 * @brief   STM32 adapter for ICurrentMeasurementPort.
 *          Seqlock-based single-buffer publish/read: the writer bumps a
 *          sequence counter on each side of the data write (even = stable,
 *          odd = write in progress); the reader retries if the counter
 *          changed across its read. Safe from either ISR or task context.
 ******************************************************************************
 */

#ifndef ADAPTERS_STM32_ADC_CURRENT_ADAPTER_H
#define ADAPTERS_STM32_ADC_CURRENT_ADAPTER_H

#include <stdint.h>
#include "Ports/ICurrentMeasurementPort.h"

typedef struct
{
  /* Seqlock counter.
   *   even → SSnap is stable, readers proceed
   *   odd  → writer is mid-update, readers retry
   *
   * Single-writer semantics (only one caller publishes at a time). With
   * multiple writers this would need additional synchronisation.
   */
  volatile uint32_t lSeq;
  tSCurrentMeasurementSnapshot SSnap;
} tSAdcCurrentAdapterCtx;

void AdcCurrentAdapter_Init(tSAdcCurrentAdapterCtx *pCtx);
ICurrentMeasurementPort_t AdcCurrentAdapter_CreatePort(
  tSAdcCurrentAdapterCtx *pCtx);

/**
 * @brief Publish a new sample. Bumps the seqlock, writes the snapshot,
 *        bumps the seqlock again. Source is an array of 4 currents in mA
 *        (float — converted + clamped internally).
 *
 *        Safe from ISR or task context. Single-writer: do not call from
 *        two contexts concurrently.
 */
void AdcCurrentAdapter_Publish(tSAdcCurrentAdapterCtx *pCtx,
                               const float *pfCurrents_mA);

#endif /* ADAPTERS_STM32_ADC_CURRENT_ADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
