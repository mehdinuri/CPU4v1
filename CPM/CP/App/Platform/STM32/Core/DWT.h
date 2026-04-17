/*
 * App/Platform/STM32/Core/DWT.h
 *
 * ARM Data Watchpoint and Trace (DWT) cycle-counter utilities.
 * Provides sub-microsecond busy-wait delay and DWT initialisation.
 * Also exposes D2SRAMClocksEnable(), called once at startup before the
 * FreeRTOS scheduler launches.
 */
#pragma once

#include "stm32h7xx_hal.h"

void DWTInit(void);
void DWTDelayuSeconds(volatile uint32_t uSeconds);
void D2SRAMClocksEnable(void);
