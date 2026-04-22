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
} OffsetOperation_e;

typedef struct
{
  uint8_t operation;   /* OffsetOperation_e value */
  uint8_t value;       /* raw offset magnitude    */
} MeasurementOffset_t;

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
 *   regVIn  : value = Voltage_V * CP_REG_VIN_COEFFICIENT
 *   regVOut : value = Voltage_V * CP_REG_VOUT_COEFFICIENT
 *   netVolt : value = Voltage_V / CP_NET_VOLTAGE_COEFFICIENT
 * ---------------------------------------------------------------------------*/
#define MEASUREMENT_REG_VIN_OK_MIN   (220U) /* = 22.0 V × 10   */
#define MEASUREMENT_REG_VIN_OK_MAX   (260U) /* = 26.0 V × 10   */
#define MEASUREMENT_REG_VOUT_OK_MIN  (47U)  /* =  4.7 V × 10   */
#define MEASUREMENT_REG_VOUT_OK_MAX  (67U)  /* =  6.7 V × 10   */
#define MEASUREMENT_NET_LEVEL_OK_MIN (226U) /* = 165 V / 0.73029 */
#define MEASUREMENT_NET_LEVEL_OK_MAX (363U) /* = 265 V / 0.73029 */

/* ---------------------------------------------------------------------------
 * Pure-computation API
 * ---------------------------------------------------------------------------*/

/**
 * @brief Apply calibration offset to a raw net voltage then scale to the
 *        control-panel integer representation.
 *
 * @param netVoltage  Raw measured net voltage (float, engineering units)
 * @param offset      Offset descriptor (operation + magnitude); must not be NULL
 * @return             Scaled integer: (netVoltage ± offset) / CP_NET_VOLTAGE_COEFFICIENT
 */
uint16_t Measurement_ScaleNetVoltage(float netVoltage,
                                     const MeasurementOffset_t *offset);

/**
 * @brief Scale a DC voltage reading to the control-panel integer representation.
 *
 * @param voltage      Raw measured voltage (float, engineering units)
 * @param coefficient  Multiplicative scaling factor (e.g. 10.0 for ×10)
 * @return              (uint16_t)(voltage * coefficient)
 */
uint16_t Measurement_ScaleRegVoltage(float voltage, float coefficient);

/**
 * @brief Check whether both DC regulator voltages are within acceptable range.
 *
 * @param regVIn   Scaled Vin  (from Measurement_ScaleRegVoltage)
 * @param regVOut  Scaled Vout (from Measurement_ScaleRegVoltage)
 * @return 1 if both in range, 0 otherwise
 */
uint8_t Measurement_DCIsOK(uint16_t regVIn, uint16_t regVOut);

/**
 * @brief Check whether the AC grid voltage is within acceptable range.
 *
 * @param netVoltage  Scaled net voltage (from Measurement_ScaleNetVoltage)
 * @return 1 if in range, 0 otherwise
 */
uint8_t Measurement_ACIsOK(uint16_t netVoltage);

#endif /* DOMAIN_MEASUREMENT_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
