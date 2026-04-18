/**
 ******************************************************************************
 * @file    Domain/VoltageRMS.h
 * @brief   Pure ADC-sample-to-voltage conversion — no HAL, no FreeRTOS.
 *          Used by adc.c ISR callbacks to produce engineering-unit floats
 *          from raw 12-bit DMA buffers.
 ******************************************************************************
 */

#ifndef DOMAIN_VOLTAGE_RMS_H
#define DOMAIN_VOLTAGE_RMS_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Compute RMS AC voltage from a window of ADC samples.
 *
 * Removes DC mean first (AC-coupled RMS), then scales by the hardware
 * front-end chain: (V_REF / adc_max) × scaling.  For PSM the grid chain
 * is V_REF = 3.38, adc_max = 4095, scaling = 276.89 (ZMPT107 + 200k/750R
 * divider).
 *
 * @param pSamples   Window of raw 12-bit ADC samples (must not be NULL)
 * @param sampleCount  Number of samples in the window (must be > 0)
 * @param fVRef      Reference voltage (e.g. 3.38f)
 * @param fAdcMax    Full-scale ADC count (e.g. 4095.0f for 12-bit)
 * @param fScaling   Front-end scaling factor (e.g. 276.89f for grid chain)
 * @return           Scaled RMS voltage in engineering units
 */
float VoltageRMS_ComputeAC(const uint16_t *pSamples,
                           size_t sampleCount,
                           float fVRef,
                           float fAdcMax,
                           float fScaling);

/**
 * @brief Convert a single DC ADC sample to engineering units.
 *
 * Linear: sample × (V_REF / adc_max) × scaling.
 *
 * @param sSample    Raw 12-bit ADC sample
 * @param fVRef      Reference voltage
 * @param fAdcMax    Full-scale ADC count
 * @param fScaling   Divider scaling factor (e.g. 2.05 for 5V divider)
 * @return           Scaled voltage in engineering units
 */
float VoltageRMS_ConvertDC(uint16_t sSample,
                           float fVRef,
                           float fAdcMax,
                           float fScaling);

#endif /* DOMAIN_VOLTAGE_RMS_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
