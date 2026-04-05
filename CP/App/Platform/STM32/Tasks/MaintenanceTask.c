/*
 * Platform/STM32/Tasks/MaintenanceTask.c
 *
 * FreeRTOS task that performs periodic housekeeping:
 *   1. Feeds the IWDG (independent watchdog) — must happen within the
 *      configured IWDG timeout (default 4 s for 32 kHz LSI / prescaler 64).
 *   2. Logs the stack high-water mark of each task to a diagnostic buffer
 *      (accessible via SNMP or JTAG debugger).
 *   3. Checks for any pending fault conditions (lamp faults, Detector faults)
 *      and sets a status LED if available.
 *
 * Priority : osPriorityIdle  (lowest — watchdog feed still works because
 *                              the idle hook is not used for blocking ops)
 * Period   : 1 000 ms
 * Argument : unused
 *
 * IMPORTANT: If this task is starved for > IWDG_TIMEOUT_S seconds, the MCU
 * will reset.  Ensure no higher-priority task busy-loops indefinitely.
 */
#include "Tasks.h"

#ifdef STM32H743xx
#include "stm32h7xx_hal.h"
extern IWDG_HandleTypeDef hiwdg1;
#endif

/* Watchdog feed interval — must be less than the configured IWDG timeout. */
#define MAINTENANCE_PERIOD_MS   1000U

/* Number of Tasks whose stack HWM is tracked (update if task count changes). */
#define TASK_COUNT_MAX  16U

/* Stack HWM diagnostic log (readable via debugger). */
static volatile uint32_t s_stackHwm[TASK_COUNT_MAX];

void MaintenanceTask(void *argument)
{
  (void) argument;
  uint8_t hwmIdx = 0U;

  for (;;)
  {
    osDelay(MAINTENANCE_PERIOD_MS);

    /* --- 1. Feed IWDG --- */
    #ifdef STM32H743xx
    HAL_IWDG_Refresh(&hiwdg1);
    #endif

    /* --- 2. Log stack high-water marks --- */

    /* TODO: Iterate osThreadEnumerate() result and call
     * osThreadGetStackSpace() for each task thread ID, storing the
     * result in s_stackHwm[].
     *
     * osThreadId_t threadIds[TASK_COUNT_MAX];
     * uint32_t count = osThreadEnumerate(threadIds, TASK_COUNT_MAX);
     * for (uint32_t i = 0; i < count && i < TASK_COUNT_MAX; i++) {
     *     s_stackHwm[i] = osThreadGetStackSpace(threadIds[i]);
     * }
     */
    (void) hwmIdx;
    (void) s_stackHwm;

    /* --- 3. Status LED heartbeat --- */
    #ifdef STM32H743xx

    /* TODO: Toggle a heartbeat LED to indicate the system is alive.
     *
     * HAL_GPIO_TogglePin(HEARTBEAT_LED_GPIO_Port, HEARTBEAT_LED_Pin);
     */
    #endif
  }
}
