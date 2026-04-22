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
  uint8_t acLow : 8;
  uint8_t dc24V1Low : 8;
  uint8_t dc24V2Low : 8;
  uint8_t dc5V1Low : 8;
  uint8_t dc5V2Low : 8;
  uint8_t acHigh : 2;
  uint8_t dc24V1High : 2;
  uint8_t dc24V2High : 2;
  uint8_t dc5V1High : 2;
  uint8_t dc5V2High : 2;
  uint8_t isolatedVoltage : 1;
  uint8_t flashActive : 1;
  uint8_t gridFault : 1;
  uint8_t canOverflow : 1;
  uint8_t reserved : 2;
  uint8_t frequency : 8;
} MeasurementFrame_t;

typedef struct __attribute__((packed))
{
  uint8_t flashSync : 1;
  uint8_t reserved : 7;
  uint8_t padding[7];
} FlashSyncFrame_t;

/* Compile-time guards: classic CAN frames carry at most 8 bytes of payload.
 * If struct layout changes and grows beyond that, the FDCAN TX FIFO silently
 * rejects the frame.  These assertions catch the problem at build time. */
_Static_assert(sizeof(MeasurementFrame_t) <= 8U,
               "MeasurementFrame_t exceeds CAN frame payload limit (8 bytes)");
_Static_assert(sizeof(FlashSyncFrame_t) <= 8U,
               "FlashSyncFrame_t exceeds CAN frame payload limit (8 bytes)");

/* ---------------------------------------------------------------------------
 * Runtime state (was static SPSMRuntime in App/Platform/STM32/Tasks/Measurement.c)
 * ---------------------------------------------------------------------------*/
typedef struct
{
  MeasurementOffset_t offset;

  uint8_t flashState;
  uint32_t flashPeriod;
  uint32_t lastFlashTick;
  uint8_t midCycleSent;    /* 1 after mid-cycle sync frame has been sent
                            * in the current flash cycle; cleared at the
                            * start of the next half-period */
  uint16_t commErrorCntr;

  uint8_t commLedCntr;

  uint8_t netFrequency;
  uint16_t netVoltage;
  uint16_t regVIn;
  uint16_t regVOut;
} PsmRuntime_t;

/* ---------------------------------------------------------------------------
 * Service context — one instance per task
 * ---------------------------------------------------------------------------*/
typedef struct
{
  IIndicatorLEDPort_t    *ledPort;
  ISignalInputPort_t     *inputPort;
  ICANTxPort_t           *canPort;
  IVoltageSensorPort_t   *voltagePort;
  IFrequencySensorPort_t *freqPort;
  IEepromPort_t          *eepromPort;

  PsmRuntime_t runtime;
} MeasurementServiceCtx_t;

/* ---------------------------------------------------------------------------
 * Initialisation
 *   Stores port pointers, zeroes runtime state, reads and validates flash
 *   period and calibration offset from EEPROM, writes defaults if invalid.
 * ---------------------------------------------------------------------------*/
void MeasurementService_Init(MeasurementServiceCtx_t  *ctx,
                             IIndicatorLEDPort_t      *ledPort,
                             ISignalInputPort_t       *inputPort,
                             ICANTxPort_t             *canPort,
                             IVoltageSensorPort_t     *voltagePort,
                             IFrequencySensorPort_t   *freqPort,
                             IEepromPort_t            *eepromPort);

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
void MeasurementService_FlashSync(MeasurementServiceCtx_t *ctx,
                                  uint32_t nowMs);

/* Returns 1 if flash state is active (suppress normal measurement send). */
uint8_t MeasurementService_IsFlash(const MeasurementServiceCtx_t *ctx);

/* ---------------------------------------------------------------------------
 * Validation and runtime-apply helpers
 * ---------------------------------------------------------------------------*/

uint8_t MeasurementService_PeriodIsValid(uint8_t period);
uint8_t MeasurementService_OffsetOpIsValid(uint8_t op);

/* Apply already-validated settings to runtime state only.
 * Persistence is the caller's responsibility. */
void MeasurementService_PeriodApply(MeasurementServiceCtx_t *ctx,
                                    uint8_t period);

void MeasurementService_OffsetApply(MeasurementServiceCtx_t *ctx,
                                    uint8_t op,
                                    uint8_t val);

/* ---------------------------------------------------------------------------
 * Persistence helpers — validate + write EEPROM, no runtime touch.
 * Shared between the task-layer wrappers (which queue the apply) and the
 * domain-level Set entry points (which apply immediately).  Return non-zero
 * on success, zero on validation failure or EEPROM write failure.
 * ---------------------------------------------------------------------------*/
uint8_t MeasurementService_PeriodPersist(IEepromPort_t *eepromPort,
                                         uint8_t period);

uint8_t MeasurementService_OffsetPersist(IEepromPort_t *eepromPort,
                                         uint8_t op,
                                         uint8_t val);

/* ---------------------------------------------------------------------------
 * Command handlers — validate, persist, then update runtime state
 * ---------------------------------------------------------------------------*/
void MeasurementService_FlashStateSet(MeasurementServiceCtx_t *ctx,
                                      uint8_t state);

void MeasurementService_CommCheck(MeasurementServiceCtx_t *ctx);

void MeasurementService_CommCntrReset(MeasurementServiceCtx_t *ctx);

void MeasurementService_PeriodSet(MeasurementServiceCtx_t *ctx,
                                  uint8_t period);

void MeasurementService_OffsetSet(MeasurementServiceCtx_t *ctx,
                                  uint8_t op,
                                  uint8_t val);

#endif /* DOMAIN_MEASUREMENTSERVICE_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
