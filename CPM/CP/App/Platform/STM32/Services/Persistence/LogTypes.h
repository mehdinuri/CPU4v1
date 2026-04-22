/* App/Platform/STM32/Services/Persistence/LogTypes.h */
#ifndef LOG_TYPES_H
#define LOG_TYPES_H

#include <stdint.h>

#define EVENT_LOG_VALUE_MAX_LENGTH 46U
#define EVENT_LOG_STORAGE_RECORD_SIZE 64U

typedef struct _SEvent
{
  uint8_t bEvent;
  uint8_t bParam;
  uint16_t sParam;
  uint32_t lParam;
} __attribute__((packed)) tSEvent, *tpSEvent;

typedef struct _SLogRecord
{
  uint8_t bSeconds;
  uint8_t bMinutes;
  uint8_t bHours;
  uint8_t bMonthDay;
  uint8_t bMonth;
  uint16_t sYear;
  tSEvent SEvent;
} __attribute__((packed)) tSLogRecord, *tpSLogRecord;

typedef struct
{
  uint32_t sequence;
  uint16_t eventLogID;
  uint32_t eventLogTime;
  uint16_t eventLogTimeMilliseconds;
  uint8_t eventLogClass;
  uint8_t valueLength;
  uint8_t eventLogValue[EVENT_LOG_VALUE_MAX_LENGTH];
  uint32_t crc32;
} __attribute__((packed)) EventLogStorageRecord_t;

#define EVENT_NONE 0U
#define EVENT_POWER_ON 1U
#define EVENT_POWER_NORMAL_TO_STAND_BY 28U
#define EVENT_RESET_WINDOW_WATCHDOG 39U
#define EVENT_RESET_INDEPENDENT_WATCHDOG 40U
#define EVENT_RESET_LOW_POWER 41U
#define EVENT_RESET_POWER_ON_CLEAR_CIRCUIT 61U
#define EVENT_DOOR_OPEN 64U
#define EVENT_DOOR_CLOSED 65U
#define EVENT_RESET_SOFTWARE 100U
#define EVENT_RESET_PIN 101U
#define EVENT_RESET_PORRST 102U
#define EVENT_TASK_STACK_OVERFLOW 127U

#endif /* LOG_TYPES_H */
