/* App/Platform/STM32/Services/Time/LrlfDetectTimeStore.c */
#include "LrlfDetectTimeStore.h"

#include "FieldCanQueueTx.h"
#include "PersistencePorts.h"

static uint8_t s_lrlfDetectTime = LRLF_DETECT_TIME_800_MS;
static uint8_t s_loaded;

static uint8_t ValueIsValid(uint8_t value)
{
  return (uint8_t) ((value >= LRLF_DETECT_TIME_300_MS)
                    && (value <= LRLF_DETECT_TIME_MAX));
}

static void EnsureLoaded(void)
{
  if (s_loaded != 0U)
  {
    return;
  }

  if ((PersistenceRead(&g_persistencePort,
                       PERSIST_OBJECT_LRLF_DETECT_TIME,
                       0U,
                       &s_lrlfDetectTime,
                       sizeof(s_lrlfDetectTime)) == 0U)
      || (ValueIsValid(s_lrlfDetectTime) == 0U))
  {
    s_lrlfDetectTime = LRLF_DETECT_TIME_800_MS;
    (void) PersistenceWrite(&g_persistencePort,
                            PERSIST_OBJECT_LRLF_DETECT_TIME,
                            0U,
                            &s_lrlfDetectTime,
                            sizeof(s_lrlfDetectTime));
  }

  s_loaded = 1U;
}

uint8_t LRLFDetectTimeWrite(void)
{
  EnsureLoaded();

  return PersistenceWrite(&g_persistencePort,
                          PERSIST_OBJECT_LRLF_DETECT_TIME,
                          0U,
                          &s_lrlfDetectTime,
                          sizeof(s_lrlfDetectTime));
}

uint8_t LRLFDetectTimeRead(void)
{
  EnsureLoaded();
  return 1U;
}

void LRLFDetectTimeSet(uint8_t bTime)
{
  s_lrlfDetectTime = bTime;
  s_loaded = 1U;
}

uint8_t LRLFDetectTimeGet(void)
{
  EnsureLoaded();
  return s_lrlfDetectTime;
}

void LRLFDetectTimeCheck(void)
{
  uint8_t current;

  EnsureLoaded();
  current = s_lrlfDetectTime;
  if (ValueIsValid(current) == 0U)
  {
    current = LRLF_DETECT_TIME_800_MS;
    LRLFDetectTimeSet(current);
    (void) LRLFDetectTimeWrite();
  }

  (void) FieldCanQueueTxSendStandard(CAN_LRLF_DETECT_TIME, &current, 1U);
}
