/*
 * Platform/STM32/Tasks/TimeTask.c
 *
 * FreeRTOS task that synchronises the RTC to GPS-derived UTC every second.
 * If GPS is unavailable, the RTC free-runs.  The task also applies any
 * pending DST offset change received via SNMP SET.
 *
 * Priority : osPriorityLow
 * Period   : 1 000 ms
 * Argument : pointer to ISystemClockPort_t
 */
#include "Tasks.h"
#include "Ports/ISystemClockPort.h"

/* Synchronisation interval in milliseconds. */
#define TIME_SYNC_INTERVAL_MS  1000U

/* Flag set by GPSTask when a fresh epoch is ready (shared variable —
 * protected by the task period; single-writer, single-reader). */
static volatile uint32_t s_gpsEpoch = 0U;
static volatile bool s_gpsEpochNew = false;

/**
 * Called by GPSTask after parsing a valid NMEA sentence.
 * Sets the pending GPS epoch to be applied on the next TimeTask tick.
 */
void TimeTask_SetGPSEpoch(uint32_t epoch)
{
  s_gpsEpoch = epoch;
  s_gpsEpochNew = true;
}

void TimeTask(void *argument)
{
  ISystemClockPort_t *clk = (ISystemClockPort_t *) argument;

  for (;;)
  {
    osDelay(TIME_SYNC_INTERVAL_MS);

    if (clk == NULL)
    {
      continue;
    }

    /* Apply a fresh GPS epoch if one has arrived since the last tick. */
    if (s_gpsEpochNew)
    {
      s_gpsEpochNew = false;
      SystemClock_SetEpoch(clk, s_gpsEpoch);
    }

    /* TODO: Apply any pending DST offset change from SNMP SET handler.
     * The NTCIP 1201 daylightSavingAdjust OID may trigger an offset update;
     * store the new offset in the RTCAdapterCtx_t.dstOffsetSeconds field
     * and read it back through getDstOffsetSeconds(). */
  }
}
