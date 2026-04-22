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
   *   even → snap is stable, readers proceed
   *   odd  → writer is mid-update, readers retry
   *
   * Single-writer semantics (only one caller publishes at a time). With
   * multiple writers this would need additional synchronisation.
   */
  volatile uint32_t seq;

  /* Protected by the seqlock counter above. Marked volatile so the compiler
   * may not hoist reads of individual fields out of the reader retry loop.
   */
  volatile CurrentMeasurementSnapshot_t snap;
} AdcCurrentAdapterCtx_t;

void AdcCurrentAdapter_Init(AdcCurrentAdapterCtx_t *ctx);
ICurrentMeasurementPort_t AdcCurrentAdapter_CreatePort(
  AdcCurrentAdapterCtx_t *ctx);

/**
 * @brief Publish a new sample. Bumps the seqlock, writes the snapshot,
 *        bumps the seqlock again. Source is an array of 4 currents in mA
 *        (float — converted + clamped internally). Status bits carry
 *        measurement qualifiers such as saturation.
 *
 *        Safe from ISR or task context. Single-writer: do not call from
 *        two contexts concurrently.
 */
void AdcCurrentAdapter_Publish(AdcCurrentAdapterCtx_t *ctx,
                               const float *currents,
                               uint8_t status);

#endif /* ADAPTERS_STM32_ADC_CURRENT_ADAPTER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
