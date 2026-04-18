/**
 ******************************************************************************
 * @file    Domain/MeasurementService.h
 * @brief   Measurement service — pure C11, no HAL, no FreeRTOS.
 *          Orchestrates measurement computation, LED state, CAN output, and
 *          EEPROM persistence via injected port interfaces.
 ******************************************************************************
 */

#ifndef DOMAIN_MEASUREMENTSERVICE_H
#define DOMAIN_MEASUREMENTSERVICE_H

#include <stdint.h>
#include "Domain/Measurement.h"
#include "Ports/IIndicatorLEDPort.h"
#include "Ports/ISignalInputPort.h"
#include "Ports/IEepromPort.h"
#include "Ports/ICANTxPort.h"
#include "Ports/IVoltageSensorPort.h"
#include "Ports/IFrequencySensorPort.h"

/* ---------------------------------------------------------------------------
 * CAN message IDs (PSM outgoing frames)
 * ---------------------------------------------------------------------------*/
#define PSM_CAN_ID_MEASUREMENT  0x05AU
#define PSM_CAN_ID_FLASH_SYNC_1 0x069U

/* ---------------------------------------------------------------------------
 * CAN wire-format structs (HAL-free).
 * These must match the frame layout expected by the control panel (CP).
 * ---------------------------------------------------------------------------*/
typedef struct __attribute__((packed))
{
  uint8_t bACLow           : 8;
  uint8_t b24V1Low         : 8;
  uint8_t b24V2Low         : 8;
  uint8_t b5V1Low          : 8;
  uint8_t b5V2Low          : 8;
  uint8_t bACHigh          : 2;
  uint8_t b24V1High        : 2;
  uint8_t b24V2High        : 2;
  uint8_t b5V1High         : 2;
  uint8_t b5V2High         : 2;
  uint8_t bIsolatedVoltage : 1;
  uint8_t bReserved        : 5;
  uint8_t bFrequency       : 8;
} tSMeasurementFrame;

typedef struct __attribute__((packed))
{
  uint8_t bFlashSync : 1;
  uint8_t bReserved  : 7;
  uint8_t baReserved[7];
} tSFlashSyncFrame;

/* Compile-time guards: classic CAN frames carry at most 8 bytes of payload.
 * If struct layout changes and grows beyond that, the FDCAN TX FIFO silently
 * rejects the frame.  These assertions catch the problem at build time. */
_Static_assert(sizeof(tSMeasurementFrame) <= 8U,
               "tSMeasurementFrame exceeds CAN frame payload limit (8 bytes)");
_Static_assert(sizeof(tSFlashSyncFrame) <= 8U,
               "tSFlashSyncFrame exceeds CAN frame payload limit (8 bytes)");

/* ---------------------------------------------------------------------------
 * Runtime state (was static SPSMRuntime in Tasks/Src/measurement.c)
 * ---------------------------------------------------------------------------*/
typedef struct
{
  tSMeasurementOffset SOffset;

  uint8_t  bFlashState;
  uint32_t lFlashPeriod;
  uint16_t sFlashStateCntr;
  uint16_t sCommErrorCntr;

  uint8_t  bCommLedCntr;

  uint8_t  bNetFrequency;
  uint16_t sNetVoltage;
  uint16_t sRegVIn;
  uint16_t sRegVOut;
} tSPSMRuntime;

/* ---------------------------------------------------------------------------
 * Service context — one instance per task
 * ---------------------------------------------------------------------------*/
typedef struct
{
  IIndicatorLEDPort_t    *pLEDPort;
  ISignalInputPort_t     *pInputPort;
  ICANTxPort_t           *pCANPort;
  IVoltageSensorPort_t   *pVoltagePort;
  IFrequencySensorPort_t *pFreqPort;
  IEepromPort_t          *pEepromPort;

  tSPSMRuntime SRuntime;
} MeasurementServiceCtx_t;

/* ---------------------------------------------------------------------------
 * Initialisation
 *   Stores port pointers, zeroes runtime state, reads and validates flash
 *   period and calibration offset from EEPROM, writes defaults if invalid.
 * ---------------------------------------------------------------------------*/
void MeasurementService_Init(MeasurementServiceCtx_t  *ctx,
                              IIndicatorLEDPort_t      *pLEDPort,
                              ISignalInputPort_t       *pInputPort,
                              ICANTxPort_t             *pCANPort,
                              IVoltageSensorPort_t     *pVoltagePort,
                              IFrequencySensorPort_t   *pFreqPort,
                              IEepromPort_t            *pEepromPort);

/* ---------------------------------------------------------------------------
 * Per-tick operations (called from MeasurementTaskFunc after flag is set)
 * ---------------------------------------------------------------------------*/

/* Read voltages and frequency from sensor ports; scale to integer units. */
void MeasurementService_UpdateSensorData(MeasurementServiceCtx_t *ctx);

/* Set DCOK / ACOK / Error LEDs based on scaled measurements and input pins. */
void MeasurementService_UpdateLEDs(MeasurementServiceCtx_t *ctx);

/* Build and transmit the measurement CAN frame.
 * Toggles the COM LED every COMM_LED_PERIOD calls. */
void MeasurementService_SendMeasurements(MeasurementServiceCtx_t *ctx);

/* Advance the flash-sync counter and send sync CAN frames at period edges. */
void MeasurementService_FlashSync(MeasurementServiceCtx_t *ctx);

/* Returns 1 if flash state is active (suppress normal measurement send). */
uint8_t MeasurementService_IsFlash(const MeasurementServiceCtx_t *ctx);

/* ---------------------------------------------------------------------------
 * Validation and runtime-apply helpers
 * ---------------------------------------------------------------------------*/

uint8_t MeasurementService_PeriodIsValid(uint8_t bPeriod);
uint8_t MeasurementService_OffsetOpIsValid(uint8_t bOp);

/* Apply already-validated settings to runtime state only.
 * Persistence is the caller's responsibility. */
void MeasurementService_PeriodApply(MeasurementServiceCtx_t *ctx,
                                     uint8_t bPeriod);

void MeasurementService_OffsetApply(MeasurementServiceCtx_t *ctx,
                                     uint8_t bOp,
                                     uint8_t bVal);

/* ---------------------------------------------------------------------------
 * Command handlers — validate, persist, then update runtime state
 * ---------------------------------------------------------------------------*/
void MeasurementService_FlashStateSet(MeasurementServiceCtx_t *ctx,
                                       uint8_t fState);

void MeasurementService_CommCheck(MeasurementServiceCtx_t *ctx);

void MeasurementService_CommCntrReset(MeasurementServiceCtx_t *ctx);

void MeasurementService_PeriodSet(MeasurementServiceCtx_t *ctx,
                                   uint8_t bPeriod);

void MeasurementService_OffsetSet(MeasurementServiceCtx_t *ctx,
                                   uint8_t bOp,
                                   uint8_t bVal);

#endif /* DOMAIN_MEASUREMENTSERVICE_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
