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

static tSMockPersistenceAdapterCtx mockCtx;
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
  tSSignalFlashConfig out;

  memset(&out, 0xAB, sizeof(out));

  TEST_ASSERT_EQUAL_UINT8(0U, SignalFlashConfig_Load(&port, &out));
}

void test_save_then_load_roundtrip(void)
{
  tSSignalFlashConfig in;
  tSSignalFlashConfig out;
  uint8_t i;

  memset(&in, 0, sizeof(in));
  in.aIsFlashing[0] = 1U;       /* R1 */
  in.aIsFlashing[4] = 1U;       /* Y2 */
  in.aIsFlashing[11] = 1U;      /* G4 */

  TEST_ASSERT_EQUAL_UINT8(1U, SignalFlashConfig_Save(&port, &in));
  TEST_ASSERT_EQUAL_UINT8(1U, SignalFlashConfig_Load(&port, &out));

  for (i = 0U; i < SIGNAL_FLASH_CONFIG_CHANNEL_COUNT; i++)
  {
    TEST_ASSERT_EQUAL_UINT8(in.aIsFlashing[i],
                            out.aIsFlashing[i]);
  }
}

void test_load_rejects_missing_magic(void)
{
  tSSignalFlashConfig out;

  /* Plant a blob without the 0x464D5353 magic — includes legacy 0xAAAA
   * marker layouts, which must not be accepted by the new format.
   */
  uint8_t bad[12] = {
    0xAAU, 0xAAU, 0xFFU, 0xFFU,       /* offset 0..3: legacy marker + flashbits */
    0x00U, 0x00U, 0x00U, 0x00U,       /* offset 4..7: zeros */
    0x00U, 0x00U, 0x00U, 0x00U        /* offset 8..11: zeros (invalid CRC) */
  };

  memcpy(mockCtx.SaSlots[PERSIST_KEY_SIGNAL_OUTPUTS_FLASH].abBlob, bad,
         sizeof(bad));
  mockCtx.SaSlots[PERSIST_KEY_SIGNAL_OUTPUTS_FLASH].sSize =
    (uint16_t) sizeof(bad);

  TEST_ASSERT_EQUAL_UINT8(0U, SignalFlashConfig_Load(&port, &out));
}

void test_load_rejects_bad_crc(void)
{
  tSSignalFlashConfig out;
  /* Magic is correct but CRC is deliberately wrong. */
  uint8_t bad[12] = {
    0x53U, 0x53U, 0x4DU, 0x46U,       /* magic 0x464D5353 (little-endian) */
    0xFFU, 0xFFU, 0x00U, 0x00U,       /* sFlashBits, sReserved */
    0xDEU, 0xADU, 0xBEU, 0xEFU        /* wrong CRC */
  };

  memcpy(mockCtx.SaSlots[PERSIST_KEY_SIGNAL_OUTPUTS_FLASH].abBlob, bad,
         sizeof(bad));
  mockCtx.SaSlots[PERSIST_KEY_SIGNAL_OUTPUTS_FLASH].sSize =
    (uint16_t) sizeof(bad);

  TEST_ASSERT_EQUAL_UINT8(0U, SignalFlashConfig_Load(&port, &out));
}

void test_save_uses_persistence_write(void)
{
  tSSignalFlashConfig in;

  memset(&in, 0, sizeof(in));

  TEST_ASSERT_EQUAL_UINT8(1U, SignalFlashConfig_Save(&port, &in));
  TEST_ASSERT_EQUAL_UINT32(1U, mockCtx.lWriteCount);
  TEST_ASSERT_EQUAL_UINT32(0U, mockCtx.lReadCount);
}

void test_save_propagates_write_failure(void)
{
  tSSignalFlashConfig in;

  memset(&in, 0, sizeof(in));
  mockCtx.SaSlots[PERSIST_KEY_SIGNAL_OUTPUTS_FLASH].bForceWriteFail = 1U;

  TEST_ASSERT_EQUAL_UINT8(0U, SignalFlashConfig_Save(&port, &in));
}

void test_load_propagates_read_failure(void)
{
  tSSignalFlashConfig in;
  tSSignalFlashConfig out;

  memset(&in, 0, sizeof(in));

  /* Save valid data then force Read to fail. */
  SignalFlashConfig_Save(&port, &in);
  mockCtx.SaSlots[PERSIST_KEY_SIGNAL_OUTPUTS_FLASH].bForceReadFail = 1U;

  TEST_ASSERT_EQUAL_UINT8(0U, SignalFlashConfig_Load(&port,
                                                     &out));
}

void test_wire_bit_encoding_bit_zero_means_flashing(void)
{
  /* Plant the wire format directly: magic + sFlashBits where bit N == 0
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
  /* sFlashBits: bit 2 cleared = flashing, all others set */
  blob[4] = (uint8_t) (~(1U << 2));
  blob[5] = 0xFFU;
  /* sReserved = 0 */
  blob[6] = 0x00U;
  blob[7] = 0x00U;

  uint32_t lCrc = Crc32_Compute(blob, 8U);

  blob[8] = (uint8_t) (lCrc & 0xFFU);
  blob[9] = (uint8_t) ((lCrc >> 8) & 0xFFU);
  blob[10] = (uint8_t) ((lCrc >> 16) & 0xFFU);
  blob[11] = (uint8_t) ((lCrc >> 24) & 0xFFU);

  memcpy(mockCtx.SaSlots[PERSIST_KEY_SIGNAL_OUTPUTS_FLASH].abBlob, blob,
         sizeof(blob));
  mockCtx.SaSlots[PERSIST_KEY_SIGNAL_OUTPUTS_FLASH].sSize =
    (uint16_t) sizeof(blob);

  tSSignalFlashConfig out;

  TEST_ASSERT_EQUAL_UINT8(1U, SignalFlashConfig_Load(&port, &out));
  TEST_ASSERT_EQUAL_UINT8(0U, out.aIsFlashing[0]);
  TEST_ASSERT_EQUAL_UINT8(0U, out.aIsFlashing[1]);
  TEST_ASSERT_EQUAL_UINT8(1U, out.aIsFlashing[2]);      /* only one flashing */
  TEST_ASSERT_EQUAL_UINT8(0U, out.aIsFlashing[11]);
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
