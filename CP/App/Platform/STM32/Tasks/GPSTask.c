/*
 * Platform/STM32/Tasks/GPSTask.c
 *
 * FreeRTOS task that reads NMEA sentences from UART5 (GPS receiver) and
 * extracts UTC time.  On a valid $GPRMC or $GPZDA sentence the parsed epoch
 * is forwarded to the system clock port via SystemClock_SetEpoch().
 *
 * Priority : osPriorityBelowNormal
 * Trigger  : event-driven (UART Rx DMA half/full callback)
 * Argument : pointer to ISystemClockPort_t
 *
 * NOTE: The NMEA parser is a TODO stub.  Wire HAL_UART_Receive_DMA on UART5
 * and implement NMEA sentence accumulation + field parsing.
 */
#include "Tasks.h"
#include "Ports/ISystemClockPort.h"

#ifdef STM32H743xx
#include "stm32h7xx_hal.h"
extern UART_HandleTypeDef huart5;
#endif

/* Size of the raw UART receive ring buffer. */
#define GPS_UART_BUF_SIZE  256U

/* Minimum $GPRMC sentence length (enough to hold time + validity field). */
#define NMEA_MIN_LEN  30U

void GPSTask(void *argument)
{
  ISystemClockPort_t *clk = (ISystemClockPort_t *) argument;

  #ifdef STM32H743xx
  static uint8_t rxBuf[GPS_UART_BUF_SIZE];

  /* TODO: HAL impl — start DMA reception on UART5.
   *
   * HAL_UART_Receive_DMA(&huart5, rxBuf, GPS_UART_BUF_SIZE);
   */
  (void) rxBuf;
  #endif

  for (;;)
  {
    /* Block until UART Rx DMA callback notifies this task. */
    osThreadFlagsWait(0x0001U, osFlagsWaitAny, osWaitForever);

    #ifdef STM32H743xx

    /* TODO: HAL impl — scan rxBuf for complete NMEA sentences.
     *
     * Parse $GPRMC or $GPZDA to extract UTC:
     *   uint32_t epoch = NMEA_ParseEpoch(rxBuf, ...);
     *   if (epoch > 0 && clk != NULL) {
     *       SystemClock_SetEpoch(clk, epoch);
     *   }
     */
    #endif
    (void) clk;
  }
}
