/*
 * Tests/Unit/Test_CanBusPort.c
 *
 * Exercises MockCanBusAdapter + ICanBusPort wiring to document the contract.
 */
#include <string.h>
#include "unity.h"
#include "Ports/ICanBusPort.h"
#include "Adapters/Mock/MockCanBusAdapter.h"

static MockCanBusAdapterCtx_t mockCtx;
static ICanBusPort_t port;

void setUp(void)
{
  MockCanBusAdapter_Init(&mockCtx);
  port = MockCanBusAdapter_CreatePort(&mockCtx);
}

void tearDown(void)
{
}

void test_send_records_frame(void)
{
  CanFrame_t frame;

  frame.stdId = 0x050U;
  frame.len = 4U;
  frame.abData[0] = 0xDEU;
  frame.abData[1] = 0xADU;
  frame.abData[2] = 0xBEU;
  frame.abData[3] = 0xEFU;

  TEST_ASSERT_EQUAL_UINT8(1U, CanBus_SendStd(&port, CAN_BUS_FDCAN1, &frame));
  TEST_ASSERT_EQUAL_UINT8(1U, mockCtx.sentCount);
  TEST_ASSERT_EQUAL(CAN_BUS_FDCAN1, mockCtx.sent[0].eBus);
  TEST_ASSERT_EQUAL_UINT16(0x050U, mockCtx.sent[0].frame.stdId);
  TEST_ASSERT_EQUAL_UINT8(4U, mockCtx.sent[0].frame.len);
  TEST_ASSERT_EQUAL_UINT8(0xDEU, mockCtx.sent[0].frame.abData[0]);
  TEST_ASSERT_EQUAL_UINT8(0xEFU, mockCtx.sent[0].frame.abData[3]);
}

void test_send_records_multiple_frames_in_order(void)
{
  CanFrame_t f1 = { .stdId = 0x100U, .len = 0U };
  CanFrame_t f2 = { .stdId = 0x200U, .len = 0U };
  CanFrame_t f3 = { .stdId = 0x300U, .len = 0U };

  CanBus_SendStd(&port, CAN_BUS_FDCAN1, &f1);
  CanBus_SendStd(&port, CAN_BUS_FDCAN2, &f2);
  CanBus_SendStd(&port, CAN_BUS_FDCAN1, &f3);

  TEST_ASSERT_EQUAL_UINT8(3U, mockCtx.sentCount);
  TEST_ASSERT_EQUAL_UINT16(0x100U, mockCtx.sent[0].frame.stdId);
  TEST_ASSERT_EQUAL(CAN_BUS_FDCAN2, mockCtx.sent[1].eBus);
  TEST_ASSERT_EQUAL_UINT16(0x300U, mockCtx.sent[2].frame.stdId);
}

void test_send_honours_force_fail(void)
{
  CanFrame_t frame = { .stdId = 0x050U, .len = 0U };

  mockCtx.forceSendFail = 1U;

  TEST_ASSERT_EQUAL_UINT8(0U, CanBus_SendStd(&port, CAN_BUS_FDCAN1, &frame));
  TEST_ASSERT_EQUAL_UINT8(0U, mockCtx.sentCount);
}

void test_mock_saturates_at_capacity(void)
{
  CanFrame_t frame = { .stdId = 0x050U, .len = 0U };
  uint8_t i;

  for (i = 0U; i < (MOCK_CAN_MAX_RECORDED + 5U); i++)
  {
    CanBus_SendStd(&port, CAN_BUS_FDCAN1, &frame);
  }

  TEST_ASSERT_EQUAL_UINT8(MOCK_CAN_MAX_RECORDED, mockCtx.sentCount);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_send_records_frame);
  RUN_TEST(test_send_records_multiple_frames_in_order);
  RUN_TEST(test_send_honours_force_fail);
  RUN_TEST(test_mock_saturates_at_capacity);

  return UNITY_END();
}
