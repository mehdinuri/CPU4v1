/* App/Platform/STM32/Bootstrap/main_stm32.c
 *
 * Hardware adapter wiring for PSM.
 *
 * Owns the static adapter contexts, creates every port instance, and
 * exposes them through HardwarePorts.h.  Called once from
 * MX_FREERTOS_Init() before the scheduler starts, after MX_GPIO_Init().
 *
 * Rule: one adapter instance per physical resource.
 *       No other translation unit may instantiate an adapter.
 */
#include "HardwarePorts.h"

#include "Adapters/STM32/IndicatorLEDAdapter.h"
#include "Adapters/STM32/SignalInputAdapter.h"
#include "Adapters/STM32/EepromAdapter.h"
#include "Adapters/STM32/CANTxAdapter.h"
#include "Adapters/STM32/CANRxAdapter.h"
#include "Adapters/STM32/VoltageSensorAdapter.h"
#include "Adapters/STM32/FrequencySensorAdapter.h"
#include "Adapters/STM32/FrequencyCaptureAdapter.h"
#include "Adapters/STM32/WatchdogAdapter.h"

#include "stm32g4xx_hal.h"  /* HAL_GetTick() */

/* ------------------------------------------------------------------
 * Adapter contexts — one per hardware resource.
 * These are the sole owners of adapter state.
 * ------------------------------------------------------------------ */
static IndicatorLEDAdapterCtx_t       s_led;
static SignalInputAdapterCtx_t        s_input;
static EepromAdapterCtx_t             s_eeprom;
static CANTxAdapterCtx_t              s_canTx;
static CANRxAdapterCtx_t              s_canRx;
static VoltageSensorAdapterCtx_t      s_voltage;
static FrequencySensorAdapterCtx_t    s_freq;
static FrequencyCaptureAdapterCtx_t   s_freqCapture;
static WatchdogAdapterCtx_t           s_watchdog;

/* ------------------------------------------------------------------
 * Port instances — declared extern in HardwarePorts.h.
 * ------------------------------------------------------------------ */
IIndicatorLEDPort_t       g_indicatorLEDPort;
ISignalInputPort_t        g_signalInputPort;
IEepromPort_t             g_eepromPort;
ICANTxPort_t              g_canTxPort;
ICANRxPort_t              g_canRxPort;
IVoltageSensorPort_t      g_voltageSensorPort;
IFrequencySensorPort_t    g_frequencySensorPort;
IFrequencyCapturePort_t   g_frequencyCapturePort;
IWatchdogPort_t           g_watchdogPort;

/* ------------------------------------------------------------------
 * Grid-frequency capture configuration — passed to the adapter at
 * init time; matches the TIM2 channel setup in Core/Src/tim.c.
 * ------------------------------------------------------------------ */
static const tSFrequencyCaptureFSMConfig s_freqCaptureCfg = {
  .lClockFreqHz            = 100000U,
  .lCounterMax             = 0xFFFFFFFFU,
  .lCaptureTimeoutMs       = 100U,
  .bTargetFreqHz           = 50U,
  .bFreqToleranceHz        = 5U,
  .sBadReadingsBeforeSleep = 500U
};

/* ------------------------------------------------------------------
 * ISR/DMA-callback wrappers.
 * These thin functions bridge the globally-named ISR entry points
 * (declared in App/Platform/STM32/Tasks/Measurement.h) to the adapter-owned inline
 * setters.  The adapters own the field write logic; this file only
 * routes the call to the correct adapter context.
 * ------------------------------------------------------------------ */
void MeasurementNetVoltageSet(float fVolt)
{
  VoltageSensorAdapter_SetNetVoltage(&s_voltage, fVolt);
}

void MeasurementRegVInSet(float fVIn)
{
  VoltageSensorAdapter_SetRegVIn(&s_voltage, fVIn);
}

void MeasurementRegVOutSet(float fVOut)
{
  VoltageSensorAdapter_SetRegVOut(&s_voltage, fVOut);
}

void MeasurementNetFrequencySet(uint8_t bFreq)
{
  FrequencySensorAdapter_SetNetFrequency(&s_freq, bFreq);
}

/* ------------------------------------------------------------------
 * MainApplication_Init — called once from MX_FREERTOS_Init() before
 * the scheduler starts.
 * ------------------------------------------------------------------ */
void MainApplication_Init(void)
{
  IndicatorLEDAdapterInit(&s_led);
  g_indicatorLEDPort = IndicatorLEDAdapterCreatePort(&s_led);

  SignalInputAdapterInit(&s_input);
  g_signalInputPort = SignalInputAdapterCreatePort(&s_input);

  EepromAdapterInit(&s_eeprom);
  g_eepromPort = EepromAdapterCreatePort(&s_eeprom);

  CANTxAdapterInit(&s_canTx);
  g_canTxPort = CANTxAdapterCreatePort(&s_canTx);

  CANRxAdapterInit(&s_canRx);
  g_canRxPort = CANRxAdapterCreatePort(&s_canRx);

  VoltageSensorAdapterInit(&s_voltage);
  g_voltageSensorPort = VoltageSensorAdapterCreatePort(&s_voltage);

  FrequencySensorAdapterInit(&s_freq);
  g_frequencySensorPort = FrequencySensorAdapterCreatePort(&s_freq);

  /* Frequency-capture adapter publishes via MeasurementNetFrequencySet, which
   * routes to the FrequencySensor adapter context above. */
  FrequencyCaptureAdapterInit(&s_freqCapture,
                               &s_freqCaptureCfg,
                               MeasurementNetFrequencySet,
                               HAL_GetTick());
  g_frequencyCapturePort = FrequencyCaptureAdapterCreatePort(&s_freqCapture);

  WatchdogAdapterInit(&s_watchdog);
  g_watchdogPort = WatchdogAdapterCreatePort(&s_watchdog);
} /* MainApplication_Init */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
