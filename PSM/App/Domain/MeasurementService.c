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

#define GET_MSB(x)  ((uint8_t)(((x) & 0xFF00U) >> 8U))
#define GET_LSB(x)  ((uint8_t)((x) & 0x00FFU))

/* EEPROM addresses come from the shared Config/EepromMap.h — do not redefine
 * here.  The addresses must match Core/Inc/i2c.h I2C_E2PROM_ADD_PERIOD/_OFFSET. */

/* ---------------------------------------------------------------------------
 * Private helpers
 * ---------------------------------------------------------------------------*/
static uint8_t FlashPeriodRead(MeasurementServiceCtx_t *ctx)
{
  return Eeprom_Read(ctx->pEepromPort,
                     EEPROM_ADDR_PERIOD,
                     &ctx->SRuntime.lFlashPeriod,
                     sizeof(ctx->SRuntime.lFlashPeriod));
}

static uint8_t FlashPeriodWrite(MeasurementServiceCtx_t *ctx)
{
  return Eeprom_Write(ctx->pEepromPort,
                      EEPROM_ADDR_PERIOD,
                      &ctx->SRuntime.lFlashPeriod,
                      sizeof(ctx->SRuntime.lFlashPeriod));
}

static uint8_t OffsetRead(MeasurementServiceCtx_t *ctx)
{
  return Eeprom_Read(ctx->pEepromPort,
                     EEPROM_ADDR_OFFSET,
                     &ctx->SRuntime.SOffset,
                     sizeof(ctx->SRuntime.SOffset));
}

static uint8_t OffsetWrite(MeasurementServiceCtx_t *ctx)
{
  return Eeprom_Write(ctx->pEepromPort,
                      EEPROM_ADDR_OFFSET,
                      &ctx->SRuntime.SOffset,
                      sizeof(ctx->SRuntime.SOffset));
}

uint8_t MeasurementService_PeriodIsValid(uint8_t bPeriod)
{
  return (uint8_t) ((bPeriod >= (uint8_t) DEFAULT_PERIOD) &&
                    (bPeriod <= (uint8_t) MAX_PERIOD));
}

uint8_t MeasurementService_OffsetOpIsValid(uint8_t bOp)
{
  return (uint8_t) (bOp <= (uint8_t) OFFSET_OPERATION_SUBTRACT);
}

void MeasurementService_PeriodApply(MeasurementServiceCtx_t *ctx,
                                     uint8_t bPeriod)
{
  if (MeasurementService_PeriodIsValid(bPeriod) == 0U)
  {
    return;
  }

  ctx->SRuntime.lFlashPeriod = (uint32_t) bPeriod * 10U;
}

void MeasurementService_OffsetApply(MeasurementServiceCtx_t *ctx,
                                     uint8_t bOp,
                                     uint8_t bVal)
{
  if (MeasurementService_OffsetOpIsValid(bOp) == 0U)
  {
    return;
  }

  ctx->SRuntime.SOffset.eOperation = bOp;
  ctx->SRuntime.SOffset.bValue     = bVal;
}

static void FlashPeriodCheck(MeasurementServiceCtx_t *ctx)
{
  if (FlashPeriodRead(ctx) != 0U)
  {
    if ((ctx->SRuntime.lFlashPeriod <= 0xFFU) &&
        (MeasurementService_PeriodIsValid((uint8_t) ctx->SRuntime.lFlashPeriod) != 0U))
    {
      MeasurementService_PeriodApply(ctx, (uint8_t) ctx->SRuntime.lFlashPeriod);
      return;
    }
  }

  ctx->SRuntime.lFlashPeriod = (uint32_t) DEFAULT_PERIOD;
  (void) FlashPeriodWrite(ctx);
  MeasurementService_PeriodApply(ctx, (uint8_t) DEFAULT_PERIOD);
}

static void OffsetCheck(MeasurementServiceCtx_t *ctx)
{
  if (OffsetRead(ctx) != 0U)
  {
    if ((MeasurementService_OffsetOpIsValid(ctx->SRuntime.SOffset.eOperation) != 0U) &&
        (ctx->SRuntime.SOffset.bValue < 0xFFU))
    {
      return;
    }
  }

  ctx->SRuntime.SOffset.eOperation = OFFSET_OPERATION_NONE;
  ctx->SRuntime.SOffset.bValue     = 0U;
  (void) OffsetWrite(ctx);
}

/* ---------------------------------------------------------------------------
 * Initialisation
 * ---------------------------------------------------------------------------*/
void MeasurementService_Init(MeasurementServiceCtx_t  *ctx,
                              IIndicatorLEDPort_t      *pLEDPort,
                              ISignalInputPort_t       *pInputPort,
                              ICANTxPort_t             *pCANPort,
                              IVoltageSensorPort_t     *pVoltagePort,
                              IFrequencySensorPort_t   *pFreqPort,
                              IEepromPort_t            *pEepromPort)
{
  memset(ctx, 0, sizeof(*ctx));
  ctx->pLEDPort     = pLEDPort;
  ctx->pInputPort   = pInputPort;
  ctx->pCANPort     = pCANPort;
  ctx->pVoltagePort = pVoltagePort;
  ctx->pFreqPort    = pFreqPort;
  ctx->pEepromPort  = pEepromPort;

  FlashPeriodCheck(ctx);
  OffsetCheck(ctx);
}

/* ---------------------------------------------------------------------------
 * Per-tick operations
 * ---------------------------------------------------------------------------*/
void MeasurementService_UpdateSensorData(MeasurementServiceCtx_t *ctx)
{
  float fNet  = VoltageSensor_GetNetVoltage(ctx->pVoltagePort);
  float fVIn  = VoltageSensor_GetRegVIn(ctx->pVoltagePort);
  float fVOut = VoltageSensor_GetRegVOut(ctx->pVoltagePort);

  ctx->SRuntime.sNetVoltage   = Measurement_ScaleNetVoltage(fNet, &ctx->SRuntime.SOffset);
  ctx->SRuntime.sRegVIn       = Measurement_ScaleRegVoltage(fVIn,
                                   MEASUREMENT_CP_REG_VIN_COEFFICIENT);
  ctx->SRuntime.sRegVOut      = Measurement_ScaleRegVoltage(fVOut,
                                   MEASUREMENT_CP_REG_VOUT_COEFFICIENT);
  ctx->SRuntime.bNetFrequency = FrequencySensor_GetNetFrequency(ctx->pFreqPort);
}

void MeasurementService_UpdateLEDs(MeasurementServiceCtx_t *ctx)
{
  /* DC OK LED */
  if (Measurement_DCIsOK(ctx->SRuntime.sRegVIn, ctx->SRuntime.sRegVOut) != 0U)
  {
    IndicatorLED_SetState(ctx->pLEDPort, LED_ID_DCOK, 1U);
  }
  else
  {
    IndicatorLED_SetState(ctx->pLEDPort, LED_ID_DCOK, 0U);
  }

  /* AC OK LED */
  if (Measurement_ACIsOK(ctx->SRuntime.sNetVoltage) != 0U)
  {
    IndicatorLED_SetState(ctx->pLEDPort, LED_ID_ACOK, 1U);
  }
  else
  {
    IndicatorLED_SetState(ctx->pLEDPort, LED_ID_ACOK, 0U);
  }

  /* Error LED — active when either OK hardware pin reads high (fault) */
  if ((SignalInput_GetState(ctx->pInputPort, SIGNAL_IN_ACOK) != 0U) ||
      (SignalInput_GetState(ctx->pInputPort, SIGNAL_IN_DCOK) != 0U))
  {
    IndicatorLED_SetState(ctx->pLEDPort, LED_ID_ERROR, 1U);
  }
  else
  {
    IndicatorLED_SetState(ctx->pLEDPort, LED_ID_ERROR, 0U);
  }
}

void MeasurementService_SendMeasurements(MeasurementServiceCtx_t *ctx)
{
  tSMeasurementFrame SFrame;
  memset(&SFrame, 0, sizeof(SFrame));

  SFrame.bACLow          = GET_LSB(ctx->SRuntime.sNetVoltage);
  SFrame.bACHigh         = GET_MSB(ctx->SRuntime.sNetVoltage);

  /* PSM hardware has a single 24 V supply (Vin) and a single 5 V supply
   * (Vout).  The CAN frame format carries two channels of each to support
   * hardware revisions with redundant supplies.  On the current PCB both
   * channels report the same measurement. */
  SFrame.b24V1Low        = GET_LSB(ctx->SRuntime.sRegVIn);
  SFrame.b24V1High       = GET_MSB(ctx->SRuntime.sRegVIn);
  SFrame.b24V2Low        = GET_LSB(ctx->SRuntime.sRegVIn);
  SFrame.b24V2High       = GET_MSB(ctx->SRuntime.sRegVIn);
  SFrame.b5V1Low         = GET_LSB(ctx->SRuntime.sRegVOut);
  SFrame.b5V1High        = GET_MSB(ctx->SRuntime.sRegVOut);
  SFrame.b5V2Low         = GET_LSB(ctx->SRuntime.sRegVOut);
  SFrame.b5V2High        = GET_MSB(ctx->SRuntime.sRegVOut);
  SFrame.bIsolatedVoltage = SignalInput_GetState(ctx->pInputPort, SIGNAL_IN_COMMOK);
  SFrame.bFrequency      = ctx->SRuntime.bNetFrequency;

  CANTx_Send(ctx->pCANPort,
             PSM_CAN_ID_MEASUREMENT,
             (const uint8_t *) &SFrame,
             (uint8_t) sizeof(SFrame));

  /* Toggle COM LED at a slower rate than the measurement cycle */
  if (ctx->SRuntime.bCommLedCntr >= COMM_LED_PERIOD)
  {
    ctx->SRuntime.bCommLedCntr = 0U;
    IndicatorLED_Toggle(ctx->pLEDPort, LED_ID_COM);
  }
  else
  {
    ctx->SRuntime.bCommLedCntr++;
  }
}

void MeasurementService_FlashSync(MeasurementServiceCtx_t *ctx)
{
  ctx->SRuntime.sFlashStateCntr++;
  if (ctx->SRuntime.sFlashStateCntr >= ctx->SRuntime.lFlashPeriod)
  {
    ctx->SRuntime.sFlashStateCntr = 0U;
  }

  if (ctx->SRuntime.sFlashStateCntr == 0U)
  {
    tSFlashSyncFrame SSyncFrameOn = {0};   /* bFlashSync = 0: "start of cycle" */
    CANTx_Send(ctx->pCANPort,
               PSM_CAN_ID_FLASH_SYNC_1,
               (const uint8_t *) &SSyncFrameOn,
               (uint8_t) sizeof(SSyncFrameOn));
    IndicatorLED_SetState(ctx->pLEDPort, LED_ID_COM, 1U);
  }
  else if (ctx->SRuntime.sFlashStateCntr ==
           (uint16_t)(ctx->SRuntime.lFlashPeriod / 2U))
  {
    tSFlashSyncFrame SSyncFrameOff = {0};
    SSyncFrameOff.bFlashSync = 1U;         /* bFlashSync = 1: "mid-cycle" */
    CANTx_Send(ctx->pCANPort,
               PSM_CAN_ID_FLASH_SYNC_1,
               (const uint8_t *) &SSyncFrameOff,
               (uint8_t) sizeof(SSyncFrameOff));
    IndicatorLED_SetState(ctx->pLEDPort, LED_ID_COM, 0U);
  }
  else
  {
    /* Mid-cycle — no action */
  }
}

uint8_t MeasurementService_IsFlash(const MeasurementServiceCtx_t *ctx)
{
  return ctx->SRuntime.bFlashState;
}

/* ---------------------------------------------------------------------------
 * Command handlers
 * ---------------------------------------------------------------------------*/
void MeasurementService_FlashStateSet(MeasurementServiceCtx_t *ctx,
                                       uint8_t fState)
{
  ctx->SRuntime.bFlashState = fState;
}

void MeasurementService_CommCheck(MeasurementServiceCtx_t *ctx)
{
  ctx->SRuntime.sCommErrorCntr++;
  if (ctx->SRuntime.sCommErrorCntr > (uint16_t) MAX_COMM_ERROR)
  {
    ctx->SRuntime.bFlashState    = 1U;
    ctx->SRuntime.sCommErrorCntr = 0U;
  }
}

void MeasurementService_CommCntrReset(MeasurementServiceCtx_t *ctx)
{
  ctx->SRuntime.sCommErrorCntr = 0U;
}

void MeasurementService_PeriodSet(MeasurementServiceCtx_t *ctx,
                                   uint8_t bPeriod)
{
  /* Reject out-of-range values before touching EEPROM or runtime state.
   * bPeriod = 0 would produce lFlashPeriod = 0, causing division-by-zero
   * in FlashSync; values > MAX_PERIOD would not survive FlashPeriodCheck
   * on next boot and would be overwritten with the default. */
  if (MeasurementService_PeriodIsValid(bPeriod) == 0U)
  {
    return;
  }

  uint32_t lNewTicks = (uint32_t) bPeriod * 10U;

  if (ctx->SRuntime.lFlashPeriod != lNewTicks)
  {
    /* Write the raw (unmultiplied) value to EEPROM using a local buffer so
     * we never store the raw value in lFlashPeriod.  This keeps
     * lFlashPeriod always in tick units — a single atomic 32-bit write
     * from old-ticks to new-ticks, safe against concurrent readers. */
    uint32_t lRaw = (uint32_t) bPeriod;
    if (Eeprom_Write(ctx->pEepromPort,
                     EEPROM_ADDR_PERIOD,
                     &lRaw,
                     sizeof(lRaw)) != 0U)
    {
      MeasurementService_PeriodApply(ctx, bPeriod);
    }
  }
}

void MeasurementService_OffsetSet(MeasurementServiceCtx_t *ctx,
                                   uint8_t bOp,
                                   uint8_t bVal)
{
  /* Reject unknown operation codes — only NONE, SUM, and SUBTRACT are valid. */
  if (MeasurementService_OffsetOpIsValid(bOp) == 0U)
  {
    return;
  }

  if ((ctx->SRuntime.SOffset.eOperation != bOp) ||
      (ctx->SRuntime.SOffset.bValue     != bVal))
  {
    tSMeasurementOffset SOld = ctx->SRuntime.SOffset;

    MeasurementService_OffsetApply(ctx, bOp, bVal);
    if (OffsetWrite(ctx) == 0U)
    {
      ctx->SRuntime.SOffset = SOld;
    }
  }
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
