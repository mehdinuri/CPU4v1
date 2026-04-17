/* App/Platform/STM32/Core/Tim2CaptureView.h
 *
 * Read-only view of the live TIM2 line-sync capture owned by Core/Src/tim.c.
 */
#ifndef TIM2_CAPTURE_VIEW_H
#define TIM2_CAPTURE_VIEW_H

#include <stdint.h>

uint32_t Tim2CapturedFreqHzGet(void);

#endif /* TIM2_CAPTURE_VIEW_H */
