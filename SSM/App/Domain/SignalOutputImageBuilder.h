/**
 ******************************************************************************
 * @file    Domain/SignalOutputImageBuilder.h
 * @brief   Pure-computation builder for tSSignalOutputImage.
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
  uint8_t bFlashActive;
  /* 1 = flash half-cycle ON (drive aFlashChannels); 0 = half-cycle OFF (all dark) */
  uint8_t bFlashSyncActive;
  /* Per-channel flash pattern (consulted when bFlashActive && bFlashSyncActive)  */
  uint8_t aFlashChannels[SIGNAL_OUTPUT_CHANNEL_COUNT];
  /* Per-channel run state (consulted when !bFlashActive)                         */
  uint8_t aRunChannels[SIGNAL_OUTPUT_CHANNEL_COUNT];
} tSSignalOutputBuildInputs;

/**
 * @brief Fold run/flash/sync state into the image that will be applied to GPIO.
 *
 * Rules:
 *   - bFlashActive=0                        → image := aRunChannels
 *   - bFlashActive=1, bFlashSyncActive=0    → image := all zeros (dark half of flash cycle)
 *   - bFlashActive=1, bFlashSyncActive=1    → image := aFlashChannels
 *
 * @param pInputs  State snapshot (non-NULL)
 * @param pImage   Output image (non-NULL)
 */
void SignalOutputImageBuilder_Build(const tSSignalOutputBuildInputs *pInputs,
                                    tSSignalOutputImage *pImage);

/**
 * @brief Same as Build(), but returns 1 if any signal group carried an
 *        illegal lamp combination and had to be collapsed to all-off.
 *
 * Allowed per-group combinations are:
 *   - dark
 *   - red
 *   - yellow
 *   - green
 *   - red+yellow
 *
 * Any combination containing green together with another lamp, or all three
 * lamps at once, is treated as a conflicting indication and forced dark for
 * that group.
 */
uint8_t SignalOutputImageBuilder_BuildSafe(
  const tSSignalOutputBuildInputs *pInputs,
  tSSignalOutputImage *pImage);

#endif /* DOMAIN_SIGNAL_OUTPUT_IMAGE_BUILDER_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
