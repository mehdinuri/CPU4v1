/**
 ******************************************************************************
 * @file    Ports/IVoltageSensorPort.h
 * @brief   Port interface for voltage sensor readings (net AC voltage and DC
 *          regulator voltages).  Values are float, engineering units (Volts).
 *          STM32: context fields written by ADC ISR callbacks.
 *          Host: configurable float fields set by test.
 ******************************************************************************
 */

#ifndef PORTS_IVOLTAGESENSORPORT_H
#define PORTS_IVOLTAGESENSORPORT_H

/* ---------------------------------------------------------------------------
 * Port interface
 * ---------------------------------------------------------------------------*/
typedef struct
{
  void *ctx;
  float (*GetNetVoltage)(void *ctx); /* AC grid voltage (V) */
  float (*GetRegVIn)(void *ctx);     /* DC regulator Vin (V) */
  float (*GetRegVOut)(void *ctx);    /* DC regulator Vout (V) */
} IVoltageSensorPort_t;

/* ---------------------------------------------------------------------------
 * Zero-cost inline dispatch helpers
 * ---------------------------------------------------------------------------*/
static inline float VoltageSensor_GetNetVoltage(IVoltageSensorPort_t *p)
{
  return p->GetNetVoltage(p->ctx);
}

static inline float VoltageSensor_GetRegVIn(IVoltageSensorPort_t *p)
{
  return p->GetRegVIn(p->ctx);
}

static inline float VoltageSensor_GetRegVOut(IVoltageSensorPort_t *p)
{
  return p->GetRegVOut(p->ctx);
}

#endif /* PORTS_IVOLTAGESENSORPORT_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
