/**
 ******************************************************************************
 * @file    Domain/SignalOutputImageBuilder.h
 * @brief   Pure-computation builder for SignalOutputImage_t.
 *          Collapses the run/flash/sync decision tree into one testable function.
 ******************************************************************************
 */

#ifndef DOMAIN_SIGNAL_OUTPUT_IMAGE_BUILDER_H
#define DOMAIN_SIGNAL_OUTPUT_IMAGE_BUILDER_H

#include <stdint.h>
#include "Ports/ISignalOutputPort.h"

/* Inputs to the image builder — all state reduced to plain data.
 * The Tasks layer fills this from CAN state accessors; the builder never
 * touches HAL or CAN state directly.
 */
typedef struct
{
  /* 1 = flash mode active (flash config pattern overrides run state)  */
  uint8_t flashActive;
  /* 1 = flash half-cycle ON (drive flashChannels); 0 = half-cycle OFF (all dark) */
  uint8_t flashSyncActive;
  /* Per-channel flash pattern (consulted when flashActive && flashSyncActive)  */
  uint8_t flashChannels[SIGNAL_OUTPUT_CHANNEL_COUNT];
  /* Per-channel run state (consulted when !flashActive)                         */
  uint8_t runChannels[SIGNAL_OUTPUT_CHANNEL_COUNT];
} SignalOutputBuildInputs_t;

/**
 * @brief Fold run/flash/sync state into the image that will be applied to GPIO.
 *
 * Rules:
 *   - flashActive=0                        → image := runChannels
 *   - flashActive=1, flashSyncActive=0    → image := all zeros (dark half of flash cycle)
 *   - flashActive=1, flashSyncActive=1    → image := flashChannels
 *
 * @param inputs  State snapshot (non-NULL)
 * @param image   Output image (non-NULL)
 */
void SignalOutputImageBuilder_Build(const SignalOutputBuildInputs_t *inputs,
                                    SignalOutputImage_t *image);

#endif /* DOMAIN_SIGNAL_OUTPUT_IMAGE_BUILDER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
