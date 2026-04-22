/**
 ******************************************************************************
 * @file    Domain/MeasurementService.c
 * @brief   Measurement service — pure C11, no HAL, no FreeRTOS.
 ******************************************************************************
 */

#include "Domain/MeasurementService.h"
#include "Domain/Measurement.h"
#include "Config/EepromMap.h"
#include <string.h>

/* ---------------------------------------------------------------------------
 * Private constants
 * ---------------------------------------------------------------------------*/
#define COMM_LED_PERIOD   5U
#define MAX_COMM_ERROR    999U
#define MAX_PERIOD        40U
#define DEFAULT_PERIOD    5U

/* Flash-period unit conversion — matches CP's internal convention:
 *   CP stores the period internally as "task-tick count" (default 50),
 *   then transmits it over CAN divided by CP_PERIOD_SCALE (default 5).
 *   PSM persists the raw wire value (5..40) and multiplies it by
 *   CP_PERIOD_SCALE (→ 50..400 ticks) × MEASUREMENT_TASK_PERIOD_MS (20 ms
 *   per MeasurementTask wake-up, driven by the ADC1 DMA half-buffer) to
 *   obtain the flash cycle length in milliseconds.
 *
 *   5 (wire) × 10 (CP_SCALE) × 20 ms = 1000 ms → 500 ms on / 500 ms off. */
#define CP_PERIOD_SCALE              10U
#define MEASUREMENT_TASK_PERIOD_MS   20U

/* Persistence sentinel — matches CP's `alreadySet == 0xF0` convention
 * (see CPM/CP/App/Platform/STM32/Services/Persistence/SettingsStorage.c).
 * A record whose first byte is not 0xF0 is treated as invalid and the
 * defaults are rewritten over it. */
#define EEPROM_SENTINEL_VALID        0xF0U

#define GET_MSB(x)  ((uint8_t) (((x) & 0xFF00U) >> 8U))
#define GET_LSB(x)  ((uint8_t) ((x) & 0x00FFU))

/* ---------------------------------------------------------------------------
 * Persistence record layout
 *   Each persisted record begins with an 8-bit sentinel byte that CP uses as
 *   `alreadySet`.  Addresses in EepromMap.h are the offset of the sentinel
 *   byte, and the record struct is read/written as a packed whole.
 * ---------------------------------------------------------------------------*/
typedef struct
{
  uint8_t alreadySet;  /* 0xF0 when valid */
  uint8_t period;       /* raw CP wire value, 5..40 */
} __attribute__((packed)) PeriodRecord_t;

typedef struct
{
  uint8_t alreadySet;
  MeasurementOffset_t offset;
} __attribute__((packed)) OffsetRecord_t;

/* ---------------------------------------------------------------------------
 * Private helpers
 * ---------------------------------------------------------------------------*/
static uint8_t PeriodRecordRead(MeasurementServiceCtx_t *ctx,
                                PeriodRecord_t *rec)
{
  return Eeprom_Read(ctx->eepromPort,
                     EEPROM_ADDR_PERIOD,
                     rec,
                     sizeof(*rec));
}

static uint8_t PeriodRecordWrite(MeasurementServiceCtx_t *ctx, uint8_t period)
{
  PeriodRecord_t rec;

  rec.alreadySet = EEPROM_SENTINEL_VALID;
  rec.period = period;

  return Eeprom_Write(ctx->eepromPort,
                      EEPROM_ADDR_PERIOD,
                      &rec,
                      sizeof(rec));
}

static uint8_t OffsetRecordRead(MeasurementServiceCtx_t *ctx,
                                OffsetRecord_t *rec)
{
  return Eeprom_Read(ctx->eepromPort,
                     EEPROM_ADDR_OFFSET,
                     rec,
                     sizeof(*rec));
}

static uint8_t OffsetRecordWrite(MeasurementServiceCtx_t *ctx,
                                 uint8_t op,
                                 uint8_t val)
{
  OffsetRecord_t rec;

  rec.alreadySet = EEPROM_SENTINEL_VALID;
  rec.offset.operation = op;
  rec.offset.value = val;

  return Eeprom_Write(ctx->eepromPort,
                      EEPROM_ADDR_OFFSET,
                      &rec,
                      sizeof(rec));
}

uint8_t MeasurementService_PeriodIsValid(uint8_t period)
{
  return (uint8_t) ((period >= (uint8_t) DEFAULT_PERIOD)
                    && (period <= (uint8_t) MAX_PERIOD));
}

uint8_t MeasurementService_OffsetOpIsValid(uint8_t op)
{
  return (uint8_t) (op <= (uint8_t) OFFSET_OPERATION_SUBTRACT);
}

void MeasurementService_PeriodApply(MeasurementServiceCtx_t *ctx,
                                    uint8_t period)
{
  if (MeasurementService_PeriodIsValid(period) == 0U)
  {
    return;
  }

  /* period is the raw CP wire value (5..40).  Multiply by CP's scale
   * factor to recover the task-tick count, then by the task period in ms
   * to get the total flash cycle length.  Default 5 → 1000 ms. */
  ctx->runtime.flashPeriod =
    (uint32_t) period * CP_PERIOD_SCALE * MEASUREMENT_TASK_PERIOD_MS;
}

void MeasurementService_OffsetApply(MeasurementServiceCtx_t *ctx,
                                    uint8_t op,
                                    uint8_t val)
{
  if (MeasurementService_OffsetOpIsValid(op) == 0U)
  {
    return;
  }

  ctx->runtime.offset.operation = op;
  ctx->runtime.offset.value = val;
}

static void FlashPeriodCheck(MeasurementServiceCtx_t *ctx)
{
  PeriodRecord_t rec;

  if ((PeriodRecordRead(ctx, &rec) != 0U)
      && (rec.alreadySet == EEPROM_SENTINEL_VALID)
      && (MeasurementService_PeriodIsValid(rec.period) != 0U))
  {
    MeasurementService_PeriodApply(ctx, rec.period);

    return;
  }

  /* Sentinel missing or payload out of range → write defaults and apply. */
  (void) PeriodRecordWrite(ctx, (uint8_t) DEFAULT_PERIOD);
  MeasurementService_PeriodApply(ctx, (uint8_t) DEFAULT_PERIOD);
}

static void OffsetCheck(MeasurementServiceCtx_t *ctx)
{
  OffsetRecord_t rec;

  if ((OffsetRecordRead(ctx, &rec) != 0U)
      && (rec.alreadySet == EEPROM_SENTINEL_VALID)
      && (MeasurementService_OffsetOpIsValid(rec.offset.operation) != 0U))
  {
    ctx->runtime.offset = rec.offset;

    return;
  }

  ctx->runtime.offset.operation = OFFSET_OPERATION_NONE;
  ctx->runtime.offset.value = 0U;
  (void) OffsetRecordWrite(ctx,
                           (uint8_t) OFFSET_OPERATION_NONE,
                           0U);
}

/* ---------------------------------------------------------------------------
 * Initialisation
 * ---------------------------------------------------------------------------*/
void MeasurementService_Init(MeasurementServiceCtx_t  *ctx,
                             IIndicatorLEDPort_t      *ledPort,
                             ISignalInputPort_t       *inputPort,
                             ICANTxPort_t             *canPort,
                             IVoltageSensorPort_t     *voltagePort,
                             IFrequencySensorPort_t   *freqPort,
                             IEepromPort_t            *eepromPort)
{
  memset(ctx, 0, sizeof(*ctx));
  ctx->ledPort = ledPort;
  ctx->inputPort = inputPort;
  ctx->canPort = canPort;
  ctx->voltagePort = voltagePort;
  ctx->freqPort = freqPort;
  ctx->eepromPort = eepromPort;

  FlashPeriodCheck(ctx);
  OffsetCheck(ctx);
}

/* ---------------------------------------------------------------------------
 * Per-tick operations
 * ---------------------------------------------------------------------------*/
void MeasurementService_UpdateSensorData(MeasurementServiceCtx_t *ctx)
{
  float net = VoltageSensor_GetNetVoltage(ctx->voltagePort);
  float vIn = VoltageSensor_GetRegVIn(ctx->voltagePort);
  float vOut = VoltageSensor_GetRegVOut(ctx->voltagePort);

  ctx->runtime.netVoltage = Measurement_ScaleNetVoltage(net,
                                                        &ctx->runtime.offset);
  ctx->runtime.regVIn = Measurement_ScaleRegVoltage(vIn,
                                                    MEASUREMENT_CP_REG_VIN_COEFFICIENT);
  ctx->runtime.regVOut = Measurement_ScaleRegVoltage(vOut,
                                                     MEASUREMENT_CP_REG_VOUT_COEFFICIENT);
  ctx->runtime.netFrequency = FrequencySensor_GetNetFrequency(ctx->freqPort);
}

void MeasurementService_UpdateLEDs(MeasurementServiceCtx_t *ctx)
{
  /* DC OK LED */
  if (Measurement_DCIsOK(ctx->runtime.regVIn, ctx->runtime.regVOut) != 0U)
  {
    IndicatorLED_SetState(ctx->ledPort, LED_ID_DCOK, 1U);
  }
  else
  {
    IndicatorLED_SetState(ctx->ledPort, LED_ID_DCOK, 0U);
  }

  /* AC OK LED */
  if (Measurement_ACIsOK(ctx->runtime.netVoltage) != 0U)
  {
    IndicatorLED_SetState(ctx->ledPort, LED_ID_ACOK, 1U);
  }
  else
  {
    IndicatorLED_SetState(ctx->ledPort, LED_ID_ACOK, 0U);
  }

  /* Error LED — active when either OK hardware pin reads high (fault) */
  if ((SignalInput_GetState(ctx->inputPort, SIGNAL_IN_ACOK) != 0U)
      || (SignalInput_GetState(ctx->inputPort, SIGNAL_IN_DCOK) != 0U))
  {
    IndicatorLED_SetState(ctx->ledPort, LED_ID_ERROR, 1U);
  }
  else
  {
    IndicatorLED_SetState(ctx->ledPort, LED_ID_ERROR, 0U);
  }
}

void MeasurementService_SendMeasurements(MeasurementServiceCtx_t *ctx)
{
  MeasurementFrame_t frame;

  memset(&frame, 0, sizeof(frame));

  frame.acLow = GET_LSB(ctx->runtime.netVoltage);
  frame.acHigh = GET_MSB(ctx->runtime.netVoltage);

  /* PSM hardware has a single 24 V supply (Vin) and a single 5 V supply
   * (Vout).  The CAN frame format carries two channels of each to support
   * hardware revisions with redundant supplies.  On the current PCB both
   * channels report the same measurement. */
  frame.dc24V1Low = GET_LSB(ctx->runtime.regVIn);
  frame.dc24V1High = GET_MSB(ctx->runtime.regVIn);
  frame.dc24V2Low = GET_LSB(ctx->runtime.regVIn);
  frame.dc24V2High = GET_MSB(ctx->runtime.regVIn);
  frame.dc5V1Low = GET_LSB(ctx->runtime.regVOut);
  frame.dc5V1High = GET_MSB(ctx->runtime.regVOut);
  frame.dc5V2Low = GET_LSB(ctx->runtime.regVOut);
  frame.dc5V2High = GET_MSB(ctx->runtime.regVOut);
  frame.isolatedVoltage = SignalInput_GetState(ctx->inputPort,
                                               SIGNAL_IN_COMMOK);
  frame.flashActive = ctx->runtime.flashState;
  frame.gridFault = (Measurement_ACIsOK(ctx->runtime.netVoltage)
                     == 0U) ? 1U : 0U;
  frame.canOverflow = (CANTx_GetOverflowCount(ctx->canPort) > 0U) ? 1U : 0U;
  frame.frequency = ctx->runtime.netFrequency;

  CANTx_Send(ctx->canPort,
             PSM_CAN_ID_MEASUREMENT,
             (const uint8_t *) &frame,
             (uint8_t) sizeof(frame));

  /* Toggle COM LED at a slower rate than the measurement cycle */
  if (ctx->runtime.commLedCntr >= COMM_LED_PERIOD)
  {
    ctx->runtime.commLedCntr = 0U;
    IndicatorLED_Toggle(ctx->ledPort, LED_ID_COM);
  }
  else
  {
    ctx->runtime.commLedCntr++;
  }
} /* MeasurementService_SendMeasurements */

void MeasurementService_FlashSync(MeasurementServiceCtx_t *ctx, uint32_t nowMs)
{
  uint32_t elapsed = nowMs - ctx->runtime.lastFlashTick;

  /* Total period in ms */
  uint32_t periodMs = ctx->runtime.flashPeriod;

  if (elapsed >= periodMs)
  {
    ctx->runtime.lastFlashTick = nowMs;
    elapsed = 0U;
  }

  if (elapsed == 0U)
  {
    FlashSyncFrame_t syncFrameOn = { 0 };   /* flashSync = 0: "start of cycle" */

    CANTx_Send(ctx->canPort,
               PSM_CAN_ID_FLASH_SYNC_1,
               (const uint8_t *) &syncFrameOn,
               (uint8_t) sizeof(syncFrameOn));
    IndicatorLED_SetState(ctx->ledPort, LED_ID_COM, 1U);
  }
  else if ((elapsed >= (periodMs / 2U))
           && (ctx->runtime.midCycleSent == 0U))
  {
    /* Latch so the mid-cycle sync frame is sent at most once per cycle. */
    ctx->runtime.midCycleSent = 1U;
    FlashSyncFrame_t syncFrameOff = { 0 };

    syncFrameOff.flashSync = 1U;         /* flashSync = 1: "mid-cycle" */
    CANTx_Send(ctx->canPort,
               PSM_CAN_ID_FLASH_SYNC_1,
               (const uint8_t *) &syncFrameOff,
               (uint8_t) sizeof(syncFrameOff));
    IndicatorLED_SetState(ctx->ledPort, LED_ID_COM, 0U);
  }
  else if (elapsed < (periodMs / 2U))
  {
    ctx->runtime.midCycleSent = 0U;
  }
  else
  {
    /* Mid-cycle already handled — no action */
  }
} /* MeasurementService_FlashSync */

uint8_t MeasurementService_IsFlash(const MeasurementServiceCtx_t *ctx)
{
  return ctx->runtime.flashState;
}

/* ---------------------------------------------------------------------------
 * Command handlers
 * ---------------------------------------------------------------------------*/
void MeasurementService_FlashStateSet(MeasurementServiceCtx_t *ctx,
                                      uint8_t state)
{
  ctx->runtime.flashState = state;
}

void MeasurementService_CommCheck(MeasurementServiceCtx_t *ctx)
{
  ctx->runtime.commErrorCntr++;
  if (ctx->runtime.commErrorCntr > (uint16_t) MAX_COMM_ERROR)
  {
    ctx->runtime.flashState = 1U;
    ctx->runtime.commErrorCntr = 0U;
  }
}

void MeasurementService_CommCntrReset(MeasurementServiceCtx_t *ctx)
{
  ctx->runtime.commErrorCntr = 0U;
}

uint8_t MeasurementService_PeriodPersist(IEepromPort_t *eepromPort,
                                         uint8_t period)
{
  if (MeasurementService_PeriodIsValid(period) == 0U)
  {
    return 0U;
  }

  PeriodRecord_t rec;

  rec.alreadySet = EEPROM_SENTINEL_VALID;
  rec.period = period;

  return Eeprom_Write(eepromPort,
                      EEPROM_ADDR_PERIOD,
                      &rec,
                      sizeof(rec));
}

uint8_t MeasurementService_OffsetPersist(IEepromPort_t *eepromPort,
                                         uint8_t op,
                                         uint8_t val)
{
  if (MeasurementService_OffsetOpIsValid(op) == 0U)
  {
    return 0U;
  }

  OffsetRecord_t rec;

  rec.alreadySet = EEPROM_SENTINEL_VALID;
  rec.offset.operation = op;
  rec.offset.value = val;

  return Eeprom_Write(eepromPort,
                      EEPROM_ADDR_OFFSET,
                      &rec,
                      sizeof(rec));
}

void MeasurementService_PeriodSet(MeasurementServiceCtx_t *ctx,
                                  uint8_t period)
{
  /* period = 0 would produce flashPeriod = 0, causing division-by-zero
   * in FlashSync; values > MAX_PERIOD would not survive FlashPeriodCheck
   * on next boot and would be overwritten with the default.  Persist first,
   * then apply only on successful write so flashPeriod stays in tick
   * units (a single atomic 32-bit write, safe against concurrent readers). */
  uint32_t newMs =
    (uint32_t) period * CP_PERIOD_SCALE * MEASUREMENT_TASK_PERIOD_MS;

  if ((MeasurementService_PeriodIsValid(period) == 0U)
      || (ctx->runtime.flashPeriod == newMs))
  {
    return;
  }

  if (MeasurementService_PeriodPersist(ctx->eepromPort, period) != 0U)
  {
    MeasurementService_PeriodApply(ctx, period);
  }
}

void MeasurementService_OffsetSet(MeasurementServiceCtx_t *ctx,
                                  uint8_t op,
                                  uint8_t val)
{
  if (MeasurementService_OffsetOpIsValid(op) == 0U)
  {
    return;
  }

  if ((ctx->runtime.offset.operation == op)
      && (ctx->runtime.offset.value     == val))
  {
    return;
  }

  if (MeasurementService_OffsetPersist(ctx->eepromPort, op, val) != 0U)
  {
    MeasurementService_OffsetApply(ctx, op, val);
  }
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
