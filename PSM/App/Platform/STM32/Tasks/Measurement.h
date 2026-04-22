/**
 ******************************************************************************
 * @file    App/Platform/STM32/Tasks/Measurement.h
 * @brief   Measurement task public API.
 *          Domain types live in Domain/MeasurementService.h.
 *          Adapter wiring lives in App/Platform/STM32/Bootstrap/main_stm32.c.
 ******************************************************************************
 */

#ifndef APP_PLATFORM_STM32_TASKS_MEASUREMENT_H
#define APP_PLATFORM_STM32_TASKS_MEASUREMENT_H

#include <stdint.h>

/* Public function prototypes -----------------------------------------------*/

/* ISR/DMA-callback setters — write voltage/frequency into adapter contexts.
 * Defined in App/Platform/STM32/Bootstrap/main_stm32.c which owns the
 * adapter contexts; declared here so HAL callback forwarders in Core/Src/
 * can call them by including this header. */
extern void MeasurementNetVoltageSet(float volt);
extern void MeasurementRegVInSet(float vIn);
extern void MeasurementRegVOutSet(float vOut);
extern void MeasurementNetFrequencySet(uint8_t freq);

/* Command forwarders — called from CAN parser */
extern void MeasurementFlashStateSet(uint8_t state);
extern void MeasurementPeriodSet(uint8_t period);
extern void MeasurementOffsetSet(uint8_t op, uint8_t val);
extern void MeasurementCommCheck(void);
extern void MeasurementCommCntrReset(void);

/* Called from ADC DMA-complete callback to unblock the task */
extern void MeasurementThreadFlagSet(void);

#endif /* APP_PLATFORM_STM32_TASKS_MEASUREMENT_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
