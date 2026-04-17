/*
 * App/Platform/STM32/Core/TimerCapture.c
 *
 * TIM2 input-capture (CH1) and output-compare (CH2) driver.
 *
 * Timer clock: 240 MHz / prescaler 2400 = 100 kHz → 10 µs per count.
 * CH1 measures the rising-edge period of an external 100 Hz signal.
 * CH2 fires every EVALUATION_INTERVAL_MS to validate the captured
 * frequency; after BAD_READINGS_BEFORE_SLEEP consecutive failures the
 * timer is de-initialised (signal absent — no point staying active).
 *
 * Moved from Core/Src/tim.c USER CODE blocks.
 */
#include "TimerCapture.h"
#include "tim.h"
#include "defs.h"
#include "gpio.h"

/* --------------------------------------------------------------------------
 * Private constants
 * --------------------------------------------------------------------------*/
#define TARGET_FREQ_HZ           100U
#define FREQ_TOLERANCE_HZ          5U
#define TIMER_CLOCK_FREQ       100000UL  /* Hz — must match TIM2 prescaler  */
#define MAX_TIMER_COUNT    0xFFFFFFFFUL
#define CAPTURE_TIMEOUT_MS       100U

#define EVALUATION_INTERVAL_MS    10U
#define EVALUATION_INTERVAL_COUNTS \
        (EVALUATION_INTERVAL_MS * (TIMER_CLOCK_FREQ / 1000UL))

#define BAD_READINGS_BEFORE_SLEEP 100U

/* --------------------------------------------------------------------------
 * Private state — volatile because modified from ISR context
 * --------------------------------------------------------------------------*/
static volatile uint32_t s_ICValue1 = 0U;
static volatile uint32_t s_ICValue2 = 0U;
static volatile uint32_t s_ICCaptureDiff = 0U;
static volatile uint8_t s_fFirstValueCaptured = FALSE;
static volatile uint32_t s_MeasuredFreqHz = 0U;
static volatile uint32_t s_LastCaptureTime = 0U;
static volatile uint16_t s_BadReadingsCntr = 0U;
static volatile uint32_t s_NextCompareValue = 0U;

/* --------------------------------------------------------------------------
 * Public API
 * --------------------------------------------------------------------------*/

uint32_t TimerCapture_GetFreqHz(void)
{
  return s_MeasuredFreqHz;
}

void Tim2StartICIT(void)
{
  s_fFirstValueCaptured = 0U;
  s_LastCaptureTime = HAL_GetTick();

  if (HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
}

void Tim2StopICIT(void)
{
  HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_1);
}

void Tim2StartOCIT(void)
{
  if (HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
}

void Tim2StopOCIT(void)
{
  HAL_TIM_OC_Stop_IT(&htim2, TIM_CHANNEL_2);
}

void Tim2DeInit(void)
{
  Tim2StopOCIT();
  Tim2StopICIT();
  HAL_TIM_OC_DeInit(&htim2);
  HAL_TIM_IC_DeInit(&htim2);
}

void Tim1StartIT(void)
{
  if (HAL_TIM_Base_Start_IT(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
}

void Tim1StopIT(void)
{
  HAL_TIM_Base_Stop_IT(&htim1);
}

void Tim1DeInit(void)
{
  Tim1StopIT();
  HAL_TIM_Base_DeInit(&htim1);
}

/* --------------------------------------------------------------------------
 * HAL callbacks — override weak definitions from stm32h7xx_hal_tim.c
 * --------------------------------------------------------------------------*/

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  if ((htim->Instance == TIM2) && (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1))
  {
    s_LastCaptureTime = HAL_GetTick();

    if (s_fFirstValueCaptured == FALSE)
    {
      s_ICValue1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
      s_fFirstValueCaptured = TRUE;
    }
    else
    {
      s_ICValue2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);

      if (s_ICValue2 > s_ICValue1)
      {
        s_ICCaptureDiff = s_ICValue2 - s_ICValue1;
      }
      else if (s_ICValue2 < s_ICValue1)
      {
        s_ICCaptureDiff = ((MAX_TIMER_COUNT - s_ICValue1) + s_ICValue2) + 1UL;
      }
      else
      {
        s_ICCaptureDiff = 0U;
        s_fFirstValueCaptured = FALSE;
      }

      if (s_ICCaptureDiff != 0U)
      {
        s_MeasuredFreqHz = TIMER_CLOCK_FREQ / s_ICCaptureDiff;
      }
      else
      {
        s_MeasuredFreqHz = 0U;
      }

      s_ICValue1 = s_ICValue2;
    }
  }
} /* HAL_TIM_IC_CaptureCallback */

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
  if ((htim->Instance == TIM2) && (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2))
  {
    uint32_t lCurTime = HAL_GetTick();
    uint32_t lFreq = s_MeasuredFreqHz;
    uint8_t fSignalTimedout = FALSE;

    if (!s_fFirstValueCaptured
        || ((lCurTime - s_LastCaptureTime) > CAPTURE_TIMEOUT_MS))
    {
      fSignalTimedout = TRUE;

      if ((s_MeasuredFreqHz != 0U) || s_fFirstValueCaptured)
      {
        s_MeasuredFreqHz = 0U;
        s_fFirstValueCaptured = 0U;
      }

      lFreq = 0U;
    }

    if (!fSignalTimedout
        && (lFreq >= (TARGET_FREQ_HZ - FREQ_TOLERANCE_HZ))
        && (lFreq <= (TARGET_FREQ_HZ + FREQ_TOLERANCE_HZ)))
    {
      s_BadReadingsCntr = 0U;
    }
    else
    {
      s_BadReadingsCntr++;
    }

    if (s_BadReadingsCntr >= BAD_READINGS_BEFORE_SLEEP)
    {
      Tim2DeInit();
    }
    else
    {
      uint32_t lCurrentCounterValue = __HAL_TIM_GET_COUNTER(htim);

      s_NextCompareValue = lCurrentCounterValue + EVALUATION_INTERVAL_COUNTS;

      if (s_NextCompareValue > MAX_TIMER_COUNT)
      {
        s_NextCompareValue -= (MAX_TIMER_COUNT + 1UL);
      }

      __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_2, s_NextCompareValue);
    }
  }
} /* HAL_TIM_OC_DelayElapsedCallback */
