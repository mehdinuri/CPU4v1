/**
 ******************************************************************************
 * @file    Domain/Measurement.c
 * @brief   Pure-computation measurement functions — no HAL, no FreeRTOS.
 ******************************************************************************
 */

#include "Domain/Measurement.h"

#include <math.h>

/* ---------------------------------------------------------------------------
 * Private helpers
 * ---------------------------------------------------------------------------*/

/* C11 §6.3.1.4: converting a float to an integer type when the value does not
 * fit is undefined behaviour — not guaranteed to wrap or clamp.  All paths that
 * produce a uint16_t must pass through this function.
 *
 * NaN compares false against every ordered comparison, so the < / > checks
 * would fall through to the cast and produce an implementation-defined value.
 * The explicit isnan() early-return makes a NaN reading deterministically
 * zero — safer than forwarding garbage to the CAN frame. */
static uint16_t SafeFloatToU16(float f)
{
  if (isnan(f))
  {
    return 0U;
  }

  if (f < 0.0f)
  {
    return 0U;
  }

  if (f > 65535.0f)
  {
    return 65535U;
  }

  return (uint16_t) f;
}

/* ---------------------------------------------------------------------------
 * Public application code
 * ---------------------------------------------------------------------------*/

uint16_t Measurement_ScaleNetVoltage(float netVoltage,
                                     const MeasurementOffset_t *offset)
{
  float offsetVoltage;

  switch ((OffsetOperation_e) offset->operation)
  {
      case OFFSET_OPERATION_SUM:
      {
        offsetVoltage = netVoltage + (float) offset->value;
        break;
      }

      case OFFSET_OPERATION_SUBTRACT:
      {
        offsetVoltage = netVoltage - (float) offset->value;
        break;
      }

      default:
      {
        offsetVoltage = netVoltage;
        break;
      }
  }

  return SafeFloatToU16(offsetVoltage
                        / MEASUREMENT_CP_NET_VOLTAGE_COEFFICIENT);
}

uint16_t Measurement_ScaleRegVoltage(float voltage, float coefficient)
{
  return SafeFloatToU16(voltage * coefficient);
}

uint8_t Measurement_DCIsOK(uint16_t regVIn, uint16_t regVOut)
{
  return (uint8_t) (
    (regVIn  >= MEASUREMENT_REG_VIN_OK_MIN)
    && (regVIn  <= MEASUREMENT_REG_VIN_OK_MAX)
    && (regVOut >= MEASUREMENT_REG_VOUT_OK_MIN)
    && (regVOut <= MEASUREMENT_REG_VOUT_OK_MAX));
}

uint8_t Measurement_ACIsOK(uint16_t netVoltage)
{
  return (uint8_t) (
    (netVoltage >= MEASUREMENT_NET_LEVEL_OK_MIN)
    && (netVoltage <= MEASUREMENT_NET_LEVEL_OK_MAX));
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
