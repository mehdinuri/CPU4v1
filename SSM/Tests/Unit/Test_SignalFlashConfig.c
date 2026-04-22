/*
 * Tests/Unit/Test_SignalFlashConfig.c
 *
 * Tests the App/Domain/SignalFlashConfig load/save round-trip, magic-number
 * + CRC guards, and the wire-level bit encoding. Uses MockPersistenceAdapter.
 */
#include <string.h>
#include "unity.h"
#include "Domain/SignalFlashConfig.h"
#include "Domain/Crc32.h"
#include "Adapters/Mock/MockPersistenceAdapter.h"

static MockPersistenceAdapterCtx_t mockCtx;
static IPersistencePort_t port;

void setUp(void)
{
  MockPersistenceAdapter_Init(&mockCtx);
  port = MockPersistenceAdapter_CreatePort(&mockCtx);
}

void tearDown(void)
{
}

void test_load_returns_zero_when_slot_empty(void)
{
  SignalFlashConfig_t out;

  memset(&out, 0xAB, sizeof(out));

  TEST_ASSERT_EQUAL_UINT8(0U, SignalFlashConfig_Load(&port, &out));
}

void test_save_then_load_roundtrip(void)
{
  SignalFlashConfig_t in;
  SignalFlashConfig_t out;
  uint8_t i;

  memset(&in, 0, sizeof(in));
  in.isFlashing[0] = 1U;       /* R1 */
  in.isFlashing[4] = 1U;       /* Y2 */
  in.isFlashing[11] = 1U;      /* G4 */

  TEST_ASSERT_EQUAL_UINT8(1U, SignalFlashConfig_Save(&port, &in));
  TEST_ASSERT_EQUAL_UINT8(1U, SignalFlashConfig_Load(&port, &out));

  for (i = 0U; i < SIGNAL_FLASH_CONFIG_CHANNEL_COUNT; i++)
  {
    TEST_ASSERT_EQUAL_UINT8(in.isFlashing[i],
                            out.isFlashing[i]);
  }
}

void test_load_rejects_missing_magic(void)
{
  SignalFlashConfig_t out;

  /* Plant a blob without the 0x464D5353 magic — includes legacy 0xAAAA
   * marker layouts, which must not be accepted by the new format.
   */
  uint8_t bad[12] = {
    0xAAU, 0xAAU, 0xFFU, 0xFFU,       /* offset 0..3: legacy marker + flashbits */
    0x00U, 0x00U, 0x00U, 0x00U,       /* offset 4..7: zeros */
    0x00U, 0x00U, 0x00U, 0x00U        /* offset 8..11: zeros (invalid CRC) */
  };

  memcpy(mockCtx.slots[PERSIST_KEY_SIGNAL_OUTPUTS_FLASH].abBlob, bad,
         sizeof(bad));
  mockCtx.slots[PERSIST_KEY_SIGNAL_OUTPUTS_FLASH].size =
    (uint16_t) sizeof(bad);

  TEST_ASSERT_EQUAL_UINT8(0U, SignalFlashConfig_Load(&port, &out));
}

void test_load_rejects_bad_crc(void)
{
  SignalFlashConfig_t out;
  /* Magic is correct but CRC is deliberately wrong. */
  uint8_t bad[12] = {
    0x53U, 0x53U, 0x4DU, 0x46U,       /* magic 0x464D5353 (little-endian) */
    0xFFU, 0xFFU, 0x00U, 0x00U,       /* flashBits, reserved */
    0xDEU, 0xADU, 0xBEU, 0xEFU        /* wrong CRC */
  };

  memcpy(mockCtx.slots[PERSIST_KEY_SIGNAL_OUTPUTS_FLASH].abBlob, bad,
         sizeof(bad));
  mockCtx.slots[PERSIST_KEY_SIGNAL_OUTPUTS_FLASH].size =
    (uint16_t) sizeof(bad);

  TEST_ASSERT_EQUAL_UINT8(0U, SignalFlashConfig_Load(&port, &out));
}

void test_save_uses_persistence_write(void)
{
  SignalFlashConfig_t in;

  memset(&in, 0, sizeof(in));

  TEST_ASSERT_EQUAL_UINT8(1U, SignalFlashConfig_Save(&port, &in));
  TEST_ASSERT_EQUAL_UINT32(1U, mockCtx.writeCount);
  TEST_ASSERT_EQUAL_UINT32(0U, mockCtx.readCount);
}

void test_save_propagates_write_failure(void)
{
  SignalFlashConfig_t in;

  memset(&in, 0, sizeof(in));
  mockCtx.slots[PERSIST_KEY_SIGNAL_OUTPUTS_FLASH].forceWriteFail = 1U;

  TEST_ASSERT_EQUAL_UINT8(0U, SignalFlashConfig_Save(&port, &in));
}

void test_load_propagates_read_failure(void)
{
  SignalFlashConfig_t in;
  SignalFlashConfig_t out;

  memset(&in, 0, sizeof(in));

  /* Save valid data then force Read to fail. */
  SignalFlashConfig_Save(&port, &in);
  mockCtx.slots[PERSIST_KEY_SIGNAL_OUTPUTS_FLASH].forceReadFail = 1U;

  TEST_ASSERT_EQUAL_UINT8(0U, SignalFlashConfig_Load(&port,
                                                     &out));
}

void test_wire_bit_encoding_bit_zero_means_flashing(void)
{
  /* Plant the wire format directly: magic + flashBits where bit N == 0
   * means channel N is flashing. Channel 2 (G1) should decode as
   * flashing; everything else steady. CRC is computed over the first 8
   * bytes to match the implementation.
   */
  uint8_t blob[12];

  /* Magic 0x464D5353 little-endian */
  blob[0] = 0x53U;
  blob[1] = 0x53U;
  blob[2] = 0x4DU;
  blob[3] = 0x46U;
  /* flashBits: bit 2 cleared = flashing, all others set */
  blob[4] = (uint8_t) (~(1U << 2));
  blob[5] = 0xFFU;
  /* reserved = 0 */
  blob[6] = 0x00U;
  blob[7] = 0x00U;

  uint32_t crc = Crc32_Compute(blob, 8U);

  blob[8] = (uint8_t) (crc & 0xFFU);
  blob[9] = (uint8_t) ((crc >> 8) & 0xFFU);
  blob[10] = (uint8_t) ((crc >> 16) & 0xFFU);
  blob[11] = (uint8_t) ((crc >> 24) & 0xFFU);

  memcpy(mockCtx.slots[PERSIST_KEY_SIGNAL_OUTPUTS_FLASH].abBlob, blob,
         sizeof(blob));
  mockCtx.slots[PERSIST_KEY_SIGNAL_OUTPUTS_FLASH].size =
    (uint16_t) sizeof(blob);

  SignalFlashConfig_t out;

  TEST_ASSERT_EQUAL_UINT8(1U, SignalFlashConfig_Load(&port, &out));
  TEST_ASSERT_EQUAL_UINT8(0U, out.isFlashing[0]);
  TEST_ASSERT_EQUAL_UINT8(0U, out.isFlashing[1]);
  TEST_ASSERT_EQUAL_UINT8(1U, out.isFlashing[2]);      /* only one flashing */
  TEST_ASSERT_EQUAL_UINT8(0U, out.isFlashing[11]);
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_load_returns_zero_when_slot_empty);
  RUN_TEST(test_save_then_load_roundtrip);
  RUN_TEST(test_load_rejects_missing_magic);
  RUN_TEST(test_load_rejects_bad_crc);
  RUN_TEST(test_save_uses_persistence_write);
  RUN_TEST(test_save_propagates_write_failure);
  RUN_TEST(test_load_propagates_read_failure);
  RUN_TEST(test_wire_bit_encoding_bit_zero_means_flashing);

  return UNITY_END();
}
