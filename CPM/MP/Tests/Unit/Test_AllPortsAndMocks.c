/* Tests/Unit/Test_AllPortsAndMocks.c
 *
 * Compile-and-link sanity test: instantiates every mock adapter and
 * invokes every port through its inline dispatch helper at least once,
 * so we catch signature drift or missing symbols the moment a port or
 * mock changes. Semantic tests for individual adapters live in their
 * own files.
 */

#include <stdint.h>
#include <string.h>

#include "unity.h"

#include "Adapters/Mock/MockControlBusAdapter.h"
#include "Adapters/Mock/MockEventLogAdapter.h"
#include "Adapters/Mock/MockFieldBusAdapter.h"
#include "Adapters/Mock/MockPersistenceAdapter.h"
#include "Adapters/Mock/MockRealtimeClockAdapter.h"
#include "Adapters/Mock/MockSafetyRelayAdapter.h"
#include "Adapters/Mock/MockStatusLEDAdapter.h"
#include "Adapters/Mock/MockSystemMonitorAdapter.h"
#include "Adapters/Mock/MockUnitAlarmAdapter.h"
#include "Adapters/Mock/MockWatchdogAdapter.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_control_bus_port_round_trip(void)
{
  MockControlBusAdapterCtx_t ctx;
  ControlBusFrame_t frame;

  MockControlBusAdapterInit(&ctx);
  IControlBusPort_t port = MockControlBusAdapterCreatePort(&ctx);

  frame.extendedId = 0x4300U;
  frame.length = 3U;
  frame.data[0] = 0x01U;
  frame.data[1] = 0x02U;
  frame.data[2] = 0x03U;

  TEST_ASSERT_EQUAL_UINT8(1U, ControlBusSendFrame(&port, &frame));
  TEST_ASSERT_EQUAL_UINT32(1U, ctx.txCount);
  TEST_ASSERT_EQUAL_UINT32(0x4300U, ctx.txBuffer[0].extendedId);
}

void test_field_bus_port_returns_snapshot(void)
{
  MockFieldBusAdapterCtx_t ctx;
  FieldBusSnapshot_t snapshot;
  FieldBusSnapshot_t readBack;

  MockFieldBusAdapterInit(&ctx);
  IFieldBusPort_t port = MockFieldBusAdapterCreatePort(&ctx);

  (void) memset(&snapshot, 0, sizeof(snapshot));
  snapshot.cpu.cpuImageSequence = 0xBEEFU;
  MockFieldBusAdapterSetSnapshot(&ctx, &snapshot);

  TEST_ASSERT_EQUAL_UINT8(1U, FieldBusReadSnapshot(&port, &readBack));
  TEST_ASSERT_EQUAL_UINT16(0xBEEFU, readBack.cpu.cpuImageSequence);
  TEST_ASSERT_EQUAL_UINT32(1U, ctx.readCount);
}

void test_status_led_port_set_state(void)
{
  MockStatusLEDAdapterCtx_t ctx;

  MockStatusLEDAdapterInit(&ctx);
  IStatusLEDPort_t port = MockStatusLEDAdapterCreatePort(&ctx);

  TEST_ASSERT_EQUAL_UINT8(1U,
                          StatusLEDSetState(&port,
                                            STATUS_LED_STATE_BLINK_FAST));
  TEST_ASSERT_EQUAL(STATUS_LED_STATE_BLINK_FAST, ctx.lastState);
}

void test_event_log_port_fifo(void)
{
  MockEventLogAdapterCtx_t ctx;
  EventLogRecord_t r1 = { .timestampSeconds = 10U, .eventCode = 1U };
  EventLogRecord_t r2 = { .timestampSeconds = 20U, .eventCode = 2U };
  EventLogRecord_t out;
  uint32_t count;

  MockEventLogAdapterInit(&ctx);
  IEventLogPort_t port = MockEventLogAdapterCreatePort(&ctx);

  TEST_ASSERT_EQUAL_UINT8(1U, EventLogAppend(&port, &r1));
  TEST_ASSERT_EQUAL_UINT8(1U, EventLogAppend(&port, &r2));
  TEST_ASSERT_EQUAL_UINT8(1U, EventLogCount(&port, &count));
  TEST_ASSERT_EQUAL_UINT32(2U, count);

  TEST_ASSERT_EQUAL_UINT8(1U, EventLogReadNext(&port, &out));
  TEST_ASSERT_EQUAL_UINT16(1U, out.eventCode);
  TEST_ASSERT_EQUAL_UINT8(1U, EventLogReadNext(&port, &out));
  TEST_ASSERT_EQUAL_UINT16(2U, out.eventCode);
  TEST_ASSERT_EQUAL_UINT8(0U, EventLogReadNext(&port, &out));
}

void test_persistence_port_read_write_round_trip(void)
{
  MockPersistenceAdapterCtx_t ctx;
  const uint8_t written[] = { 0xDEU, 0xADU, 0xBEU, 0xEFU };
  uint8_t readBack[4];
  uint32_t size;

  MockPersistenceAdapterInit(&ctx);
  IPersistencePort_t port = MockPersistenceAdapterCreatePort(&ctx);

  TEST_ASSERT_EQUAL_UINT8(1U, PersistenceWrite(&port, 16U, written, 4U));
  TEST_ASSERT_EQUAL_UINT8(1U, PersistenceRead(&port, 16U, readBack, 4U));
  TEST_ASSERT_EQUAL_MEMORY(written, readBack, 4U);

  TEST_ASSERT_EQUAL_UINT8(1U, PersistenceSize(&port, &size));
  TEST_ASSERT_EQUAL_UINT32(MOCK_PERSISTENCE_SIZE, size);
}

void test_realtime_clock_port_get_set(void)
{
  MockRealtimeClockAdapterCtx_t ctx;
  RealtimeClockTime_t t = { .year = 2026U, .month = 4U, .day = 17U };
  RealtimeClockTime_t readBack;

  MockRealtimeClockAdapterInit(&ctx);
  IRealtimeClockPort_t port = MockRealtimeClockAdapterCreatePort(&ctx);

  TEST_ASSERT_EQUAL_UINT8(1U, RealtimeClockSetTime(&port, &t));
  TEST_ASSERT_EQUAL_UINT8(1U, RealtimeClockGetTime(&port, &readBack));
  TEST_ASSERT_EQUAL_UINT16(2026U, readBack.year);
  TEST_ASSERT_EQUAL_UINT8(4U, readBack.month);
  TEST_ASSERT_EQUAL_UINT8(17U, readBack.day);
}

void test_system_monitor_port_reads(void)
{
  MockSystemMonitorAdapterCtx_t ctx;
  uint16_t mv;
  int16_t tempC;

  MockSystemMonitorAdapterInit(&ctx);
  ISystemMonitorPort_t port = MockSystemMonitorAdapterCreatePort(&ctx);

  TEST_ASSERT_EQUAL_UINT8(1U,
                          SystemMonitorGetBatteryVoltageMilliVolts(&port, &mv));
  TEST_ASSERT_EQUAL_UINT16(12000U, mv);
  TEST_ASSERT_EQUAL_UINT8(1U, SystemMonitorGetThermistorDegC(&port, &tempC));
  TEST_ASSERT_EQUAL_INT16(25, tempC);
}

void test_watchdog_port_feed(void)
{
  MockWatchdogAdapterCtx_t ctx;

  MockWatchdogAdapterInit(&ctx);
  IWatchdogPort_t port = MockWatchdogAdapterCreatePort(&ctx);

  TEST_ASSERT_EQUAL_UINT8(1U, WatchdogFeed(&port));
  TEST_ASSERT_EQUAL_UINT8(1U, WatchdogFeed(&port));
  TEST_ASSERT_EQUAL_UINT32(2U, ctx.feedCount);
}

void test_unit_alarm_port_round_trip(void)
{
  MockUnitAlarmAdapterCtx_t ctx;
  uint8_t out;

  MockUnitAlarmAdapterInit(&ctx);
  IUnitAlarmPort_t port = MockUnitAlarmAdapterCreatePort(&ctx);

  TEST_ASSERT_EQUAL_UINT8(1U,
                          UnitAlarmPortSetUnitAlarmStatus1(&port, 0x12U));
  TEST_ASSERT_EQUAL_UINT8(1U,
                          UnitAlarmPortGetUnitAlarmStatus1(&port, &out));
  TEST_ASSERT_EQUAL_UINT8(0x12U, out);
}

void test_safety_relay_port_set_get(void)
{
  MockSafetyRelayAdapterCtx_t ctx;
  SafetyRelayState_t s;

  MockSafetyRelayAdapterInit(&ctx);
  ISafetyRelayPort_t port = MockSafetyRelayAdapterCreatePort(&ctx);

  TEST_ASSERT_EQUAL_UINT8(1U,
                          SafetyRelaySetState(&port,
                                              SAFETY_RELAY_STATE_CLOSED));
  TEST_ASSERT_EQUAL_UINT8(1U,
                          SafetyRelayGetActualState(&port, &s));
  TEST_ASSERT_EQUAL(SAFETY_RELAY_STATE_CLOSED, s);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_control_bus_port_round_trip);
  RUN_TEST(test_field_bus_port_returns_snapshot);
  RUN_TEST(test_status_led_port_set_state);
  RUN_TEST(test_event_log_port_fifo);
  RUN_TEST(test_persistence_port_read_write_round_trip);
  RUN_TEST(test_realtime_clock_port_get_set);
  RUN_TEST(test_system_monitor_port_reads);
  RUN_TEST(test_watchdog_port_feed);
  RUN_TEST(test_unit_alarm_port_round_trip);
  RUN_TEST(test_safety_relay_port_set_get);

  return UNITY_END();
}
