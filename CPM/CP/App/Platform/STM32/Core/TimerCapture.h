/*
 * App/Platform/STM32/Core/TimerCapture.h
 *
 * TIM2 input-capture / output-compare driver for the external 100 Hz
 * NEMA clock signal.
 *
 *   CH1 (PA0) — rising-edge input capture; measures signal period.
 *   CH2       — periodic OC interrupt (every 10 ms); validates frequency
 *               and de-initialises TIM2 when the signal is absent for
 *               too long.
 *
 * HAL callbacks HAL_TIM_IC_CaptureCallback and
 * HAL_TIM_OC_DelayElapsedCallback are defined in TimerCapture.c,
 * overriding the HAL weak stubs.
 */
#pragma once

#include <stdint.h>

/* TIM2 input-capture start / stop */
void Tim2StartICIT(void);
void Tim2StopICIT(void);

/* TIM2 output-compare start / stop */
void Tim2StartOCIT(void);
void Tim2StopOCIT(void);

/* Combined TIM2 de-initialisation */
void Tim2DeInit(void);

/* TIM1 base-timer control */
void Tim1StartIT(void);
void Tim1StopIT(void);
void Tim1DeInit(void);

/* Returns the most recently measured input frequency in Hz (0 = absent) */
uint32_t TimerCapture_GetFreqHz(void);
