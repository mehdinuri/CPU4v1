/*
 * Tests/Unit/Test_MeasurementService.c
 *
 * Unit tests for App/Domain/MeasurementService.c
 * Uses all six mock adapters — no HAL, no FreeRTOS.
 */
#include <string.h>

#include "unity.h"
#include "Domain/MeasurementService.h"
#include "MockIndicatorLEDAdapter.h"
#include "MockSignalInputAdapter.h"
#include "MockEepromAdapter.h"
#include "MockCANTxAdapter.h"
#include "MockVoltageSensorAdapter.h"
#include "MockFrequencySensorAdapter.h"

/* ---------------------------------------------------------------------------
 * Test fixtures — one instance of each adapter context and port
 * ---------------------------------------------------------------------------*/
static MeasurementServiceCtx_t s_svc;

static MockIndicatorLEDAdapterCtx_t s_led;
static MockSignalInputAdapterCtx_t s_input;
static MockEepromAdapterCtx_t s_eeprom;
static MockCANTxAdapterCtx_t s_can;
static MockVoltageSensorAdapterCtx_t s_voltage;
static MockFrequencySensorAdapterCtx_t s_freq;

static IIndicatorLEDPort_t s_ledPort;
static ISignalInputPort_t s_inputPort;
static IEepromPort_t s_eepromPort;
static ICANTxPort_t s_canPort;
static IVoltageSensorPort_t s_voltPort;
static IFrequencySensorPort_t s_freqPort;

/* ---------------------------------------------------------------------------
 * Unity boilerplate
 * ---------------------------------------------------------------------------*/
void setUp(void)
{
  MockIndicatorLEDAdapterInit(&s_led);
  MockSignalInputAdapterInit(&s_input);
  MockEepromAdapterInit(&s_eeprom);
  MockCANTxAdapterInit(&s_can);
  MockVoltageSensorAdapterInit(&s_voltage);
  MockFrequencySensorAdapterInit(&s_freq);

  s_ledPort = MockIndicatorLEDAdapterCreatePort(&s_led);
  s_inputPort = MockSignalInputAdapterCreatePort(&s_input);
  s_eepromPort = MockEepromAdapterCreatePort(&s_eeprom);
  s_canPort = MockCANTxAdapterCreatePort(&s_can);
  s_voltPort = MockVoltageSensorAdapterCreatePort(&s_voltage);
  s_freqPort = MockFrequencySensorAdapterCreatePort(&s_freq);

  MeasurementService_Init(&s_svc,
                          &s_ledPort,
                          &s_inputPort,
                          &s_canPort,
                          &s_voltPort,
                          &s_freqPort,
                          &s_eepromPort);
}

void tearDown(void)
{
}

/* ---------------------------------------------------------------------------
 * Init / EEPROM defaults
 * ---------------------------------------------------------------------------*/

void test_init_writes_default_period_when_eeprom_fails(void)
{
  /* Force a fresh init with EEPROM returning failure */
  MockEepromAdapterInit(&s_eeprom);
  s_eeprom.readResult = 0U;
  s_eepromPort = MockEepromAdapterCreatePort(&s_eeprom);

  MeasurementService_Init(&s_svc,
                          &s_ledPort,
                          &s_inputPort,
                          &s_canPort,
                          &s_voltPort,
                          &s_freqPort,
                          &s_eepromPort);

  /* Both period and offset reads fail → two default writes */
  TEST_ASSERT_EQUAL_UINT32(2U, s_eeprom.writeCount);
}

void test_init_accepts_valid_period_from_eeprom(void)
{
  /* Sentinel byte + raw wire value at 0x0004..0x0005 */
  uint8_t stored[2] = { 0xF0U, 10U }; /* valid: 5 <= 10 <= 40 */

  MockEepromAdapterInit(&s_eeprom);
  __builtin_memcpy(&s_eeprom.buf[0x0004U], stored, sizeof(stored));
  s_eepromPort = MockEepromAdapterCreatePort(&s_eeprom);

  MeasurementService_Init(&s_svc,
                          &s_ledPort,
                          &s_inputPort,
                          &s_canPort,
                          &s_voltPort,
                          &s_freqPort,
                          &s_eepromPort);

  /* Valid record: period=10 × CP_SCALE(10) × TASK_PERIOD(20 ms) = 2000 ms */
  TEST_ASSERT_EQUAL_UINT32(2000U, s_svc.runtime.flashPeriod);
}

void test_init_default_period_is_multiplied_when_eeprom_fails(void)
{
  /* Regression: FlashPeriodCheck default path must multiply the default
   * raw value (5) by CP_SCALE × TASK_PERIOD so the runtime period lands
   * at 1000 ms (1 Hz), not the raw units (5). */
  MockEepromAdapterInit(&s_eeprom);
  s_eeprom.readResult = 0U;   /* EEPROM read failure */
  s_eepromPort = MockEepromAdapterCreatePort(&s_eeprom);

  MeasurementService_Init(&s_svc,
                          &s_ledPort,
                          &s_inputPort,
                          &s_canPort,
                          &s_voltPort,
                          &s_freqPort,
                          &s_eepromPort);

  TEST_ASSERT_EQUAL_UINT32(1000U, s_svc.runtime.flashPeriod);
}

void test_init_writes_default_period_when_value_out_of_range(void)
{
  uint8_t stored[2] = { 0xF0U, 3U }; /* sentinel valid, period<5 invalid */

  MockEepromAdapterInit(&s_eeprom);
  __builtin_memcpy(&s_eeprom.buf[0x0004U], stored, sizeof(stored));
  s_eepromPort = MockEepromAdapterCreatePort(&s_eeprom);

  MeasurementService_Init(&s_svc,
                          &s_ledPort,
                          &s_inputPort,
                          &s_canPort,
                          &s_voltPort,
                          &s_freqPort,
                          &s_eepromPort);

  /* Period out of range → write default 5 to EEPROM, then multiply → 1000 ms */
  TEST_ASSERT_EQUAL_UINT32(1000U, s_svc.runtime.flashPeriod);
  TEST_ASSERT_TRUE(s_eeprom.writeCount >= 1U);
}

/* ---------------------------------------------------------------------------
 * UpdateSensorData
 * ---------------------------------------------------------------------------*/

void test_update_sensor_data_scales_voltages(void)
{
  s_voltage.netVoltage = 220.0f;
  s_voltage.regVIn = 24.0f;
  s_voltage.regVOut = 5.0f;

  MeasurementService_UpdateSensorData(&s_svc);

  /* 220.0 / 0.73029 ≈ 301 */
  TEST_ASSERT_EQUAL_UINT16(301U, s_svc.runtime.netVoltage);
  /* 24.0 * 10.0 = 240 */
  TEST_ASSERT_EQUAL_UINT16(240U, s_svc.runtime.regVIn);
  /* 5.0 * 10.0 = 50 */
  TEST_ASSERT_EQUAL_UINT16(50U, s_svc.runtime.regVOut);
}

void test_update_sensor_data_copies_frequency(void)
{
  s_freq.frequency = 50U;

  MeasurementService_UpdateSensorData(&s_svc);

  TEST_ASSERT_EQUAL_UINT8(50U, s_svc.runtime.netFrequency);
}

/* ---------------------------------------------------------------------------
 * UpdateLEDs — DCOK
 * ---------------------------------------------------------------------------*/

void test_led_dcok_on_when_voltages_in_range(void)
{
  s_voltage.regVIn = 24.0f;  /* 240 — in [220, 260] */
  s_voltage.regVOut = 5.7f;  /* 57  — in [47, 67]   */

  MeasurementService_UpdateSensorData(&s_svc);
  MeasurementService_UpdateLEDs(&s_svc);

  TEST_ASSERT_EQUAL_UINT8(1U, s_led.ledStates[LED_ID_DCOK]);
}

void test_led_dcok_off_when_vin_low(void)
{
  s_voltage.regVIn = 21.9f;  /* 219 < 220 */
  s_voltage.regVOut = 5.7f;

  MeasurementService_UpdateSensorData(&s_svc);
  MeasurementService_UpdateLEDs(&s_svc);

  TEST_ASSERT_EQUAL_UINT8(0U, s_led.ledStates[LED_ID_DCOK]);
}

void test_led_dcok_off_when_vin_high(void)
{
  s_voltage.regVIn = 26.1f;  /* 261 > 260 */
  s_voltage.regVOut = 5.7f;

  MeasurementService_UpdateSensorData(&s_svc);
  MeasurementService_UpdateLEDs(&s_svc);

  TEST_ASSERT_EQUAL_UINT8(0U, s_led.ledStates[LED_ID_DCOK]);
}

/* ---------------------------------------------------------------------------
 * UpdateLEDs — ACOK
 * ---------------------------------------------------------------------------*/

void test_led_acok_on_when_voltage_in_range(void)
{
  s_voltage.netVoltage = 220.0f; /* 301 — in [226, 363] */

  MeasurementService_UpdateSensorData(&s_svc);
  MeasurementService_UpdateLEDs(&s_svc);

  TEST_ASSERT_EQUAL_UINT8(1U, s_led.ledStates[LED_ID_ACOK]);
}

void test_led_acok_off_when_voltage_low(void)
{
  s_voltage.netVoltage = 100.0f; /* ~136 < 226 */

  MeasurementService_UpdateSensorData(&s_svc);
  MeasurementService_UpdateLEDs(&s_svc);

  TEST_ASSERT_EQUAL_UINT8(0U, s_led.ledStates[LED_ID_ACOK]);
}

/* ---------------------------------------------------------------------------
 * UpdateLEDs — Error
 * ---------------------------------------------------------------------------*/

void test_led_error_on_when_acok_pin_high(void)
{
  s_input.inputStates[SIGNAL_IN_ACOK] = 1U;
  s_input.inputStates[SIGNAL_IN_DCOK] = 0U;

  MeasurementService_UpdateLEDs(&s_svc);

  TEST_ASSERT_EQUAL_UINT8(1U, s_led.ledStates[LED_ID_ERROR]);
}

void test_led_error_off_when_both_pins_low(void)
{
  s_input.inputStates[SIGNAL_IN_ACOK] = 0U;
  s_input.inputStates[SIGNAL_IN_DCOK] = 0U;

  MeasurementService_UpdateLEDs(&s_svc);

  TEST_ASSERT_EQUAL_UINT8(0U, s_led.ledStates[LED_ID_ERROR]);
}

/* ---------------------------------------------------------------------------
 * SendMeasurements
 * ---------------------------------------------------------------------------*/

void test_send_measurements_uses_correct_can_id(void)
{
  MeasurementService_UpdateSensorData(&s_svc);
  MeasurementService_SendMeasurements(&s_svc);

  TEST_ASSERT_EQUAL_UINT32(1U,                      s_can.sendCount);
  TEST_ASSERT_EQUAL_UINT32(PSM_CAN_ID_MEASUREMENT,  s_can.lastID);
}

void test_send_measurements_encodes_frequency(void)
{
  s_freq.frequency = 60U;

  MeasurementService_UpdateSensorData(&s_svc);
  MeasurementService_SendMeasurements(&s_svc);

  /* frequency is the last byte of MeasurementFrame_t */
  TEST_ASSERT_EQUAL_UINT8(60U, s_can.lastData[s_can.lastLen - 1U]);
}

void test_send_measurements_toggles_com_led_after_period(void)
{
  uint32_t i;

  /* COM LED toggles after COMM_LED_PERIOD (5) calls to SendMeasurements.
  * First 5 calls: counter goes 0→1→2→3→4→5, toggle fires on 6th call. */
  for (i = 0U; i < 5U; i++)
  {
    MeasurementService_SendMeasurements(&s_svc);
  }

  TEST_ASSERT_EQUAL_UINT32(0U, s_led.toggleCallCount);

  /* 6th call → commLedCntr reaches COMM_LED_PERIOD → toggle fires */
  MeasurementService_SendMeasurements(&s_svc);

  TEST_ASSERT_EQUAL_UINT32(1U, s_led.toggleCallCount);
}

/* ---------------------------------------------------------------------------
 * FlashSync
 * ---------------------------------------------------------------------------*/

void test_flash_sync_sends_sync0_at_period_boundary(void)
{
  /* period 5 = 1000ms total cycle. */
  MeasurementService_PeriodApply(&s_svc, 5U);
  s_svc.runtime.lastFlashTick = 0U;

  /* At t=1000, it should trigger "start of cycle" (Sync ON) */
  MeasurementService_FlashSync(&s_svc, 1000U);

  TEST_ASSERT_EQUAL_UINT32(PSM_CAN_ID_FLASH_SYNC_1, s_can.lastID);
  TEST_ASSERT_EQUAL_UINT8(0U, s_can.lastData[0] & 0x01U); /* flashSync bit 0 = ON */
  TEST_ASSERT_EQUAL_UINT8(1U, s_led.ledStates[LED_ID_COM]);
}

void test_flash_sync_sends_sync1_at_half_period(void)
{
  MeasurementService_PeriodApply(&s_svc, 5U);
  s_svc.runtime.lastFlashTick = 1000U;
  s_svc.runtime.midCycleSent = 0U;

  /* At t=1500 (500ms elapsed), it should trigger "mid-cycle" (Sync OFF) */
  MeasurementService_FlashSync(&s_svc, 1500U);

  TEST_ASSERT_EQUAL_UINT32(PSM_CAN_ID_FLASH_SYNC_1, s_can.lastID);
  TEST_ASSERT_EQUAL_UINT8(1U, s_can.lastData[0] & 0x01U); /* flashSync bit 1 = OFF */
  TEST_ASSERT_EQUAL_UINT8(0U, s_led.ledStates[LED_ID_COM]);
  TEST_ASSERT_EQUAL_UINT8(1U, s_svc.runtime.midCycleSent); /* latch set */
}

void test_flash_sync_no_redundant_send_mid_cycle(void)
{
  MeasurementService_PeriodApply(&s_svc, 5U);
  s_svc.runtime.lastFlashTick = 1000U;
  s_svc.runtime.midCycleSent = 1U; /* already sent mid-cycle */

  s_can.sendCount = 0U;
  MeasurementService_FlashSync(&s_svc, 1510U);

  TEST_ASSERT_EQUAL_UINT32(0U, s_can.sendCount);
}

void test_send_measurements_reports_status_bits(void)
{
  s_svc.runtime.netVoltage = 0U; /* Force AC Fault */
  s_svc.runtime.flashState = 1U; /* Force Flash Active */
  s_can.mockOverflow = 1U;        /* Force CAN Overflow */

  MeasurementService_SendMeasurements(&s_svc);

  TEST_ASSERT_EQUAL_UINT32(PSM_CAN_ID_MEASUREMENT, s_can.lastID);
  /* bit 1: FlashActive, bit 2: GridFault, bit 3: CanOverflow */
  /* Expected: 1 << 3 | 1 << 4 | 1 << 5 = 8 | 16 | 32 = 56 (0x38) */
  /* isolatedVoltage is bit 2, let's assume it is 0. */
  TEST_ASSERT_EQUAL_UINT8(0x38U, s_can.lastData[6] & 0x38U);
}

/* ---------------------------------------------------------------------------
 * CommCheck / CommCntrReset / FlashState / PeriodSet / OffsetSet
 * ---------------------------------------------------------------------------*/

void test_comm_check_triggers_flash_after_max_errors(void)
{
  uint32_t i;

  for (i = 0U; i <= 999U; i++)
  {
    MeasurementService_CommCheck(&s_svc);
  }

  TEST_ASSERT_EQUAL_UINT8(1U, s_svc.runtime.flashState);
  TEST_ASSERT_EQUAL_UINT16(0U, s_svc.runtime.commErrorCntr);
}

void test_comm_cntr_reset_clears_counter(void)
{
  s_svc.runtime.commErrorCntr = 500U;

  MeasurementService_CommCntrReset(&s_svc);

  TEST_ASSERT_EQUAL_UINT16(0U, s_svc.runtime.commErrorCntr);
}

void test_flash_state_set(void)
{
  MeasurementService_FlashStateSet(&s_svc, 1U);

  TEST_ASSERT_EQUAL_UINT8(1U, MeasurementService_IsFlash(&s_svc));
}

void test_flash_state_clear(void)
{
  s_svc.runtime.flashState = 1U;

  MeasurementService_FlashStateSet(&s_svc, 0U);

  TEST_ASSERT_EQUAL_UINT8(0U, MeasurementService_IsFlash(&s_svc));
}

/* ---------------------------------------------------------------------------
 * PeriodSet
 * ---------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------------
 * OffsetCheck SUBTRACT fix (fix 1.1): SUBTRACT offset must survive Init
 * ---------------------------------------------------------------------------*/

void test_init_accepts_subtract_offset_from_eeprom(void)
{
  /* Offset record layout at 0x0006..0x0008:
   *   [0] = sentinel 0xF0
   *   [1] = operation
   *   [2] = value                                                        */
  uint8_t stored[3] = { 0xF0U, (uint8_t) OFFSET_OPERATION_SUBTRACT, 5U };

  MockEepromAdapterInit(&s_eeprom);
  __builtin_memcpy(&s_eeprom.buf[0x0006U], stored, sizeof(stored));
  s_eepromPort = MockEepromAdapterCreatePort(&s_eeprom);

  MeasurementService_Init(&s_svc,
                          &s_ledPort,
                          &s_inputPort,
                          &s_canPort,
                          &s_voltPort,
                          &s_freqPort,
                          &s_eepromPort);

  /* Offset must be accepted, not overwritten with NONE */
  TEST_ASSERT_EQUAL_UINT8(OFFSET_OPERATION_SUBTRACT,
                          s_svc.runtime.offset.operation);
  TEST_ASSERT_EQUAL_UINT8(5U, s_svc.runtime.offset.value);
}

void test_init_rejects_record_with_invalid_sentinel(void)
{
  /* Fresh EEPROM (all 0xFF) must be treated as invalid even if the payload
   * bytes happen to satisfy the range check.  Defaults get rewritten. */
  MockEepromAdapterInit(&s_eeprom);
  memset(&s_eeprom.buf[0x0000U], 0xFFU, sizeof(s_eeprom.buf));
  s_eepromPort = MockEepromAdapterCreatePort(&s_eeprom);

  MeasurementService_Init(&s_svc,
                          &s_ledPort,
                          &s_inputPort,
                          &s_canPort,
                          &s_voltPort,
                          &s_freqPort,
                          &s_eepromPort);

  TEST_ASSERT_EQUAL_UINT32(1000U, s_svc.runtime.flashPeriod);
  TEST_ASSERT_EQUAL_UINT8(OFFSET_OPERATION_NONE,
                          s_svc.runtime.offset.operation);
  TEST_ASSERT_EQUAL_UINT8(0U, s_svc.runtime.offset.value);
  TEST_ASSERT_TRUE(s_eeprom.writeCount >= 2U);
}

/* ---------------------------------------------------------------------------
 * PeriodSet validation (fix 2.2): out-of-range values must be rejected
 * ---------------------------------------------------------------------------*/

void test_period_set_rejects_zero(void)
{
  uint32_t writes = s_eeprom.writeCount;

  MeasurementService_PeriodSet(&s_svc, 0U);

  TEST_ASSERT_EQUAL_UINT32(writes, s_eeprom.writeCount);
}

void test_period_set_rejects_above_max(void)
{
  uint32_t writes = s_eeprom.writeCount;

  MeasurementService_PeriodSet(&s_svc, 200U);

  TEST_ASSERT_EQUAL_UINT32(writes, s_eeprom.writeCount);
}

void test_offset_set_rejects_invalid_operation(void)
{
  uint32_t writes = s_eeprom.writeCount;

  MeasurementService_OffsetSet(&s_svc, 0xFFU, 10U);

  TEST_ASSERT_EQUAL_UINT32(writes, s_eeprom.writeCount);
}

/* ---------------------------------------------------------------------------
 * PeriodSet
 * ---------------------------------------------------------------------------*/

void test_period_set_writes_eeprom_and_multiplies(void)
{
  uint32_t initial_writes = s_eeprom.writeCount;

  MeasurementService_PeriodSet(&s_svc, 10U);

  TEST_ASSERT_EQUAL_UINT32(initial_writes + 1U, s_eeprom.writeCount);
  TEST_ASSERT_EQUAL_UINT32(2000U, s_svc.runtime.flashPeriod);
}

void test_period_set_no_write_when_unchanged(void)
{
  /* PeriodSet now compares flashPeriod (ms) against period * 200.
   * Setting the same raw value twice should skip the second EEPROM write. */
  MeasurementService_PeriodSet(&s_svc, 10U);
  uint32_t writes_after_first = s_eeprom.writeCount;

  MeasurementService_PeriodSet(&s_svc, 10U);

  TEST_ASSERT_EQUAL_UINT32(writes_after_first, s_eeprom.writeCount);
  TEST_ASSERT_EQUAL_UINT32(2000U, s_svc.runtime.flashPeriod);
}

void test_period_set_does_not_update_runtime_when_eeprom_write_fails(void)
{
  s_eeprom.writeResult = 0U;

  MeasurementService_PeriodSet(&s_svc, 10U);

  TEST_ASSERT_EQUAL_UINT32(1000U, s_svc.runtime.flashPeriod);
}

/* ---------------------------------------------------------------------------
 * OffsetSet
 * ---------------------------------------------------------------------------*/

void test_offset_set_writes_eeprom_on_first_call(void)
{
  uint32_t initial_writes = s_eeprom.writeCount;

  MeasurementService_OffsetSet(&s_svc, OFFSET_OPERATION_SUM, 10U);

  TEST_ASSERT_EQUAL_UINT32(initial_writes + 1U, s_eeprom.writeCount);
  TEST_ASSERT_EQUAL_UINT8(OFFSET_OPERATION_SUM,
                          s_svc.runtime.offset.operation);
  TEST_ASSERT_EQUAL_UINT8(10U, s_svc.runtime.offset.value);
}

void test_offset_set_no_write_when_unchanged(void)
{
  MeasurementService_OffsetSet(&s_svc, OFFSET_OPERATION_SUM, 10U);
  uint32_t writes = s_eeprom.writeCount;

  MeasurementService_OffsetSet(&s_svc, OFFSET_OPERATION_SUM, 10U);

  TEST_ASSERT_EQUAL_UINT32(writes, s_eeprom.writeCount);
}

void test_offset_set_writes_when_op_changes(void)
{
  MeasurementService_OffsetSet(&s_svc, OFFSET_OPERATION_SUM, 10U);
  uint32_t writes = s_eeprom.writeCount;

  MeasurementService_OffsetSet(&s_svc, OFFSET_OPERATION_SUBTRACT, 10U);

  TEST_ASSERT_EQUAL_UINT32(writes + 1U, s_eeprom.writeCount);
}

void test_offset_set_does_not_update_runtime_when_eeprom_write_fails(void)
{
  s_eeprom.writeResult = 0U;

  MeasurementService_OffsetSet(&s_svc, OFFSET_OPERATION_SUM, 10U);

  TEST_ASSERT_EQUAL_UINT8(OFFSET_OPERATION_NONE,
                          s_svc.runtime.offset.operation);
  TEST_ASSERT_EQUAL_UINT8(0U, s_svc.runtime.offset.value);
}

/* ---------------------------------------------------------------------------
 * Main
 * ---------------------------------------------------------------------------*/
int main(void)
{
  UNITY_BEGIN();

  RUN_TEST(test_init_writes_default_period_when_eeprom_fails);
  RUN_TEST(test_init_default_period_is_multiplied_when_eeprom_fails);
  RUN_TEST(test_init_accepts_valid_period_from_eeprom);
  RUN_TEST(test_init_writes_default_period_when_value_out_of_range);

  RUN_TEST(test_update_sensor_data_scales_voltages);
  RUN_TEST(test_update_sensor_data_copies_frequency);

  RUN_TEST(test_led_dcok_on_when_voltages_in_range);
  RUN_TEST(test_led_dcok_off_when_vin_low);
  RUN_TEST(test_led_dcok_off_when_vin_high);
  RUN_TEST(test_led_acok_on_when_voltage_in_range);
  RUN_TEST(test_led_acok_off_when_voltage_low);
  RUN_TEST(test_led_error_on_when_acok_pin_high);
  RUN_TEST(test_led_error_off_when_both_pins_low);

  RUN_TEST(test_send_measurements_uses_correct_can_id);
  RUN_TEST(test_send_measurements_encodes_frequency);
  RUN_TEST(test_send_measurements_toggles_com_led_after_period);
  RUN_TEST(test_send_measurements_reports_status_bits);

  RUN_TEST(test_flash_sync_sends_sync0_at_period_boundary);
  RUN_TEST(test_flash_sync_sends_sync1_at_half_period);
  RUN_TEST(test_flash_sync_no_redundant_send_mid_cycle);

  RUN_TEST(test_comm_check_triggers_flash_after_max_errors);
  RUN_TEST(test_comm_cntr_reset_clears_counter);
  RUN_TEST(test_flash_state_set);
  RUN_TEST(test_flash_state_clear);

  RUN_TEST(test_init_accepts_subtract_offset_from_eeprom);
  RUN_TEST(test_init_rejects_record_with_invalid_sentinel);
  RUN_TEST(test_period_set_rejects_zero);
  RUN_TEST(test_period_set_rejects_above_max);
  RUN_TEST(test_offset_set_rejects_invalid_operation);

  RUN_TEST(test_period_set_writes_eeprom_and_multiplies);
  RUN_TEST(test_period_set_no_write_when_unchanged);
  RUN_TEST(test_period_set_does_not_update_runtime_when_eeprom_write_fails);

  RUN_TEST(test_offset_set_writes_eeprom_on_first_call);
  RUN_TEST(test_offset_set_no_write_when_unchanged);
  RUN_TEST(test_offset_set_writes_when_op_changes);
  RUN_TEST(test_offset_set_does_not_update_runtime_when_eeprom_write_fails);

  return UNITY_END();
} /* main */
