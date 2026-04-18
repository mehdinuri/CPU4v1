/**
 ******************************************************************************
 * @file    Tasks/Src/measurement.c
 * @brief   Measurement task — uses globally wired ports from HardwarePorts.h.
 *          All business logic lives in App/Domain/MeasurementService.c.
 *          Adapter contexts and ISR setters live in
 *          App/Platform/STM32/Bootstrap/main_stm32.c.
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "measurement.h"
#include "HardwarePorts.h"
#include "Domain/MeasurementService.h"
#include "utilities.h"
#include "cmsis_os.h"
#include "adc.h"
#include "tim.h"

/* Private variables ---------------------------------------------------------*/
static MeasurementServiceCtx_t s_svc;

/* Private function prototypes -----------------------------------------------*/
static void GridVoltMeasurementStart(void);
static void GridFreqMeasurementStart(void);
static void RegulatorMeasurementsStart(void);

/* Private application code --------------------------------------------------*/
static void GridVoltMeasurementStart(void)
{
  ADC1StartDMA();
  Tim3Start();
}

static void GridFreqMeasurementStart(void)
{
  Tim2ICStartIT();
  Tim2OCStartIT();
}

static void RegulatorMeasurementsStart(void)
{
  ADC2StartDMA();
  ADC3StartDMA();
}

/* Public application code --------------------------------------------------*/

/* Service forwarding functions — external callers (can_msg_parser, etc.)
 * use the same public API as before. */
void MeasurementFlashStateSet(uint8_t fState)
{
  MeasurementService_FlashStateSet(&s_svc, fState);
}

void MeasurementPeriodSet(uint8_t bPeriod)
{
  MeasurementService_PeriodSet(&s_svc, bPeriod);
}

void MeasurementOffsetSet(uint8_t bOp, uint8_t bVal)
{
  MeasurementService_OffsetSet(&s_svc, bOp, bVal);
}

void MeasurementCommCheck(void)
{
  MeasurementService_CommCheck(&s_svc);
}

void MeasurementCommCntrReset(void)
{
  MeasurementService_CommCntrReset(&s_svc);
}

void MeasurementThreadFlagSet(void)
{
  osThreadFlagsSet(MeasurementTaskHandle, THREAD_FLAGS_MEASUREMENT_DONE);
}

/* USER CODE BEGIN Header_vMeasurementTask */
/**
 * @brief Function implementing the MeasurementTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_vMeasurementTask */
void MeasurementTaskFunc(void *argument)
{
  /* USER CODE BEGIN vMeasurementTask */
  UNUSED(argument);

  /* Initialise service using globally wired ports from MainApplication_Init().
   * Adapter contexts and port instances are owned by main_stm32.c. */
  MeasurementService_Init(&s_svc,
                           &g_indicatorLEDPort,
                           &g_signalInputPort,
                           &g_canTxPort,
                           &g_voltageSensorPort,
                           &g_frequencySensorPort,
                           &g_eepromPort);

  /* Start hardware measurement pipelines */
  GridFreqMeasurementStart();
  GridVoltMeasurementStart();
  RegulatorMeasurementsStart();

  /* Infinite loop */
  while (pdTRUE)
  {
    osThreadFlagsWait(THREAD_FLAGS_MEASUREMENT_DONE,
                      osFlagsWaitAll,
                      osWaitForever);

    MeasurementService_UpdateSensorData(&s_svc);
    MeasurementService_UpdateLEDs(&s_svc);

    if (MeasurementService_IsFlash(&s_svc) != 0U)
    {
      MeasurementService_FlashSync(&s_svc);
    }
    else
    {
      MeasurementService_SendMeasurements(&s_svc);
    }

    MaintenanceTaskSignal(EVENT_FLAGS_MAINTENANCE_MEASUREMENT_TASK_ACTIVE);
  }

  /* USER CODE END vMeasurementTask */
} /* MeasurementTaskFunc */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
