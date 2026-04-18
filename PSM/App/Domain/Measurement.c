/**
 ******************************************************************************
 * @file    Domain/Measurement.c
 * @brief   Pure-computation measurement functions — no HAL, no FreeRTOS.
 ******************************************************************************
 */

#include "Domain/Measurement.h"

/* ---------------------------------------------------------------------------
 * Public application code
 * ---------------------------------------------------------------------------*/

uint16_t Measurement_ScaleNetVoltage(float fNetVoltage,
                                     const tSMeasurementOffset *pOffset)
{
    float fOffsetVoltage;

    switch ((tEOffsetOperation)pOffset->bOperation)
    {
        case OFFSET_OPERATION_SUM:
        {
            fOffsetVoltage = fNetVoltage + (float)pOffset->bValue;
        }
        break;

        case OFFSET_OPERATION_SUBTRACT:
        {
            fOffsetVoltage = fNetVoltage - (float)pOffset->bValue;
        }
        break;

        default:
        {
            fOffsetVoltage = fNetVoltage;
        }
        break;
    }

    return (uint16_t)(fOffsetVoltage / MEASUREMENT_CP_NET_VOLTAGE_COEFFICIENT);
}

uint16_t Measurement_ScaleRegVoltage(float fVoltage, float fCoefficient)
{
    return (uint16_t)(fVoltage * fCoefficient);
}

uint8_t Measurement_DCIsOK(uint16_t sRegVIn, uint16_t sRegVOut)
{
    return (uint8_t)(
        (sRegVIn  >= MEASUREMENT_REG_VIN_OK_MIN)  &&
        (sRegVIn  <= MEASUREMENT_REG_VIN_OK_MAX)  &&
        (sRegVOut >= MEASUREMENT_REG_VOUT_OK_MIN) &&
        (sRegVOut <= MEASUREMENT_REG_VOUT_OK_MAX));
}

uint8_t Measurement_ACIsOK(uint16_t sNetVoltage)
{
    return (uint8_t)(
        (sNetVoltage >= MEASUREMENT_NET_LEVEL_OK_MIN) &&
        (sNetVoltage <= MEASUREMENT_NET_LEVEL_OK_MAX));
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
