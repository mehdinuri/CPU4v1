/* App/Adapters/STM32/SystemMonitorAdapter.h
 *
 * ISystemMonitorPort reading VBAT (PB0 / ADC1_IN15), the NTC
 * thermistor (PA2 / ADC1_IN3), and the CHRGING feedback input on PA7.
 * CHRGR_CEN (PA6) is driven by SetChargerEnable().
 *
 * Battery scaling assumes a 1:10 divider with 3.3 V ADC reference and
 * 12-bit conversion (matches the schematic). Thermistor is a 10 k NTC
 * at 25 degC; the adapter exposes raw counts for downstream
 * conversion, since the monitor layer only needs relative trends.
 */
#ifndef SYSTEM_MONITOR_ADAPTER_H
#define SYSTEM_MONITOR_ADAPTER_H

#include "Ports/ISystemMonitorPort.h"

#define SYSTEM_MONITOR_ADC_REF_MV 3300U
#define SYSTEM_MONITOR_ADC_MAX 4095U
#define SYSTEM_MONITOR_VBAT_DIVIDER_NUM 10U
#define SYSTEM_MONITOR_VBAT_DIVIDER_DEN 1U

typedef struct
{
  uint16_t lastVbatMilliVolts;
  int16_t lastThermistorDegC;
  uint8_t chargerEnabled;
} SystemMonitorAdapterCtx_t;

void SystemMonitorAdapterInit(SystemMonitorAdapterCtx_t *ctx);
ISystemMonitorPort_t SystemMonitorAdapterCreatePort(
  SystemMonitorAdapterCtx_t *ctx);

#endif /* SYSTEM_MONITOR_ADAPTER_H */
