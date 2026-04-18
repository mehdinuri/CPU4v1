/**
 ******************************************************************************
 * @file    Tasks/Inc/measurement.h
 * @brief   Measurement task public API.
 *          Domain types live in Domain/MeasurementService.h.
 *          Adapter wiring lives in App/Platform/STM32/Bootstrap/main_stm32.c.
 ******************************************************************************
 */

#ifndef TASKS_MEASUREMENT_H
#define TASKS_MEASUREMENT_H

#include <stdint.h>

/* Public function prototypes -----------------------------------------------*/

/* ISR/DMA-callback setters — write voltage/frequency into adapter contexts.
 * Defined in App/Platform/STM32/Bootstrap/main_stm32.c which owns the
 * adapter contexts; declared here so HAL callback forwarders in Core/Src/
 * can call them by including this header. */
extern void MeasurementNetVoltageSet(float fVolt);
extern void MeasurementRegVInSet(float fVIn);
extern void MeasurementRegVOutSet(float fVOut);
extern void MeasurementNetFrequencySet(uint8_t bFreq);

/* Command forwarders — called from CAN parser */
extern void MeasurementFlashStateSet(uint8_t fState);
extern void MeasurementPeriodSet(uint8_t bPeriod);
extern void MeasurementOffsetSet(uint8_t bOp, uint8_t bVal);
extern void MeasurementCommCheck(void);
extern void MeasurementCommCntrReset(void);

/* Called from ADC DMA-complete callback to unblock the task */
extern void MeasurementThreadFlagSet(void);

#endif /* TASKS_MEASUREMENT_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
