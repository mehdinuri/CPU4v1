/**
 ******************************************************************************
 * @file    Domain/Measurement.h
 * @brief   Pure-computation measurement functions — no HAL, no FreeRTOS.
 *          All state is passed explicitly; no global mutable state.
 ******************************************************************************
 */

#ifndef DOMAIN_MEASUREMENT_H
#define DOMAIN_MEASUREMENT_H

#include <stdint.h>

/* ---------------------------------------------------------------------------
 * Offset calibration
 * ---------------------------------------------------------------------------*/
typedef enum
{
  OFFSET_OPERATION_NONE     = 0,
  OFFSET_OPERATION_SUM      = 1,
  OFFSET_OPERATION_SUBTRACT = 2
} tEOffsetOperation;

typedef struct
{
  uint8_t eOperation;   /* tEOffsetOperation value */
  uint8_t bValue;       /* raw offset magnitude    */
} tSMeasurementOffset;

/* ---------------------------------------------------------------------------
 * Scaling coefficients (also used in Tasks layer)
 *
 * NET_VOLTAGE_COEFFICIENT (0.73029):
 *   AC grid voltage is divided by this factor to produce the integer value
 *   transmitted on the CAN bus.  The value comes from the resistor divider
 *   ratio on the PSM v1.0 schematic (R_top / (R_top + R_bot) for the AC
 *   measurement path).  Update this constant when the PCB hardware revision
 *   changes.  Reference: PSM Hardware Design Guide §3.2.
 *
 * REG_VIN / REG_VOUT_COEFFICIENT (10.0):
 *   DC regulator voltages are multiplied by 10 so that one integer unit
 *   corresponds to 0.1 V (e.g. 24.0 V → 240, 5.7 V → 57).
 * ---------------------------------------------------------------------------*/
#define MEASUREMENT_CP_NET_VOLTAGE_COEFFICIENT  (0.73029f)
#define MEASUREMENT_CP_REG_VIN_COEFFICIENT      (10.0f)
#define MEASUREMENT_CP_REG_VOUT_COEFFICIENT     (10.0f)

/* ---------------------------------------------------------------------------
 * OK-range thresholds (scaled integer units)
 *   sRegVIn  : value = Voltage_V * CP_REG_VIN_COEFFICIENT
 *   sRegVOut : value = Voltage_V * CP_REG_VOUT_COEFFICIENT
 *   sNetVolt : value = Voltage_V / CP_NET_VOLTAGE_COEFFICIENT
 * ---------------------------------------------------------------------------*/
#define MEASUREMENT_REG_VIN_OK_MIN   (220U) /* 22.0 V */
#define MEASUREMENT_REG_VIN_OK_MAX   (260U) /* 26.0 V */
#define MEASUREMENT_REG_VOUT_OK_MIN  (47U)  /*  4.7 V */
#define MEASUREMENT_REG_VOUT_OK_MAX  (67U)  /*  6.7 V */
#define MEASUREMENT_NET_LEVEL_OK_MIN (226U) /* 165 V  */
#define MEASUREMENT_NET_LEVEL_OK_MAX (363U) /* 265 V  */

/* ---------------------------------------------------------------------------
 * Pure-computation API
 * ---------------------------------------------------------------------------*/

/**
 * @brief Apply calibration offset to a raw net voltage then scale to the
 *        control-panel integer representation.
 *
 * @param fNetVoltage  Raw measured net voltage (float, engineering units)
 * @param pOffset      Offset descriptor (operation + magnitude); must not be NULL
 * @return             Scaled integer: (fNetVoltage ± offset) / CP_NET_VOLTAGE_COEFFICIENT
 */
uint16_t Measurement_ScaleNetVoltage(float fNetVoltage,
                                     const tSMeasurementOffset *pOffset);

/**
 * @brief Scale a DC voltage reading to the control-panel integer representation.
 *
 * @param fVoltage      Raw measured voltage (float, engineering units)
 * @param fCoefficient  Multiplicative scaling factor (e.g. 10.0 for ×10)
 * @return              (uint16_t)(fVoltage * fCoefficient)
 */
uint16_t Measurement_ScaleRegVoltage(float fVoltage, float fCoefficient);

/**
 * @brief Check whether both DC regulator voltages are within acceptable range.
 *
 * @param sRegVIn   Scaled Vin  (from Measurement_ScaleRegVoltage)
 * @param sRegVOut  Scaled Vout (from Measurement_ScaleRegVoltage)
 * @return 1 if both in range, 0 otherwise
 */
uint8_t Measurement_DCIsOK(uint16_t sRegVIn, uint16_t sRegVOut);

/**
 * @brief Check whether the AC grid voltage is within acceptable range.
 *
 * @param sNetVoltage  Scaled net voltage (from Measurement_ScaleNetVoltage)
 * @return 1 if in range, 0 otherwise
 */
uint8_t Measurement_ACIsOK(uint16_t sNetVoltage);

#endif /* DOMAIN_MEASUREMENT_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
