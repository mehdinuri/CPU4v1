/*
 * Tests/Unit/Test_CanBusPort.c
 *
 * Exercises MockCanBusAdapter + ICanBusPort wiring to document the contract.
 */
#include <string.h>
#include "unity.h"
#include "Ports/ICanBusPort.h"
#include "Adapters/Mock/MockCanBusAdapter.h"

static tSMockCanBusAdapterCtx mockCtx;
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
  tSCanFrame frame;

  frame.sStdId = 0x050U;
  frame.bLen = 4U;
  frame.abData[0] = 0xDEU;
  frame.abData[1] = 0xADU;
  frame.abData[2] = 0xBEU;
  frame.abData[3] = 0xEFU;

  TEST_ASSERT_EQUAL_UINT8(1U, CanBus_SendStd(&port, CAN_BUS_FDCAN1, &frame));
  TEST_ASSERT_EQUAL_UINT8(1U, mockCtx.bSentCount);
  TEST_ASSERT_EQUAL(CAN_BUS_FDCAN1, mockCtx.aSent[0].eBus);
  TEST_ASSERT_EQUAL_UINT16(0x050U, mockCtx.aSent[0].SFrame.sStdId);
  TEST_ASSERT_EQUAL_UINT8(4U, mockCtx.aSent[0].SFrame.bLen);
  TEST_ASSERT_EQUAL_UINT8(0xDEU, mockCtx.aSent[0].SFrame.abData[0]);
  TEST_ASSERT_EQUAL_UINT8(0xEFU, mockCtx.aSent[0].SFrame.abData[3]);
}

void test_send_records_multiple_frames_in_order(void)
{
  tSCanFrame f1 = { .sStdId = 0x100U, .bLen = 0U };
  tSCanFrame f2 = { .sStdId = 0x200U, .bLen = 0U };
  tSCanFrame f3 = { .sStdId = 0x300U, .bLen = 0U };

  CanBus_SendStd(&port, CAN_BUS_FDCAN1, &f1);
  CanBus_SendStd(&port, CAN_BUS_FDCAN2, &f2);
  CanBus_SendStd(&port, CAN_BUS_FDCAN1, &f3);

  TEST_ASSERT_EQUAL_UINT8(3U, mockCtx.bSentCount);
  TEST_ASSERT_EQUAL_UINT16(0x100U, mockCtx.aSent[0].SFrame.sStdId);
  TEST_ASSERT_EQUAL(CAN_BUS_FDCAN2, mockCtx.aSent[1].eBus);
  TEST_ASSERT_EQUAL_UINT16(0x300U, mockCtx.aSent[2].SFrame.sStdId);
}

void test_send_honours_force_fail(void)
{
  tSCanFrame frame = { .sStdId = 0x050U, .bLen = 0U };

  mockCtx.bForceSendFail = 1U;

  TEST_ASSERT_EQUAL_UINT8(0U, CanBus_SendStd(&port, CAN_BUS_FDCAN1, &frame));
  TEST_ASSERT_EQUAL_UINT8(0U, mockCtx.bSentCount);
}

void test_mock_saturates_at_capacity(void)
{
  tSCanFrame frame = { .sStdId = 0x050U, .bLen = 0U };
  uint8_t i;

  for (i = 0U; i < (MOCK_CAN_MAX_RECORDED + 5U); i++)
  {
    CanBus_SendStd(&port, CAN_BUS_FDCAN1, &frame);
  }

  TEST_ASSERT_EQUAL_UINT8(MOCK_CAN_MAX_RECORDED, mockCtx.bSentCount);
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
