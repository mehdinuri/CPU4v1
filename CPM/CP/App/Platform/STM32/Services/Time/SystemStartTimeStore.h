/* App/Platform/STM32/Services/Time/SystemStartTimeStore.h */
#ifndef SYSTEM_START_TIME_STORE_H
#define SYSTEM_START_TIME_STORE_H

#include <stdint.h>

#define SYSTEM_START_TIME_ALREADY_WRITTEN 0x55U
#define SYSTEM_START_TIME_DEF_MIN_SYSTEM_UP_HOURS 24U

typedef struct _SSystemStartTime
{
  uint8_t bAlreadyWritten;
  uint8_t bMonthDay;
  uint8_t bMonth;
  uint16_t sYear;
} __attribute__((packed)) tSSystemStartTime, *tpSSystemStartTime;

void SystemStartTimeInit(void);
void SystemStartTimeSet(tpSSystemStartTime pSSystemStartTime);
uint8_t SystemStartTimeSave(void);
uint8_t SystemStartTimeRead(void);
void SystemStartTimeGet(tpSSystemStartTime pSSystemStartTime);
void SystemStartTimeStart(void);
uint8_t IsSystemStartTimeWritten(void);
void SystemStartTimeSetMinUpHours(uint8_t bMinUpHours);
uint8_t SystemStartTimeGetMinUpHours(void);
void SystemStartTimeSetUpHours(uint8_t bUpHours);
void SystemStartTimeIncUpHours(void);
uint8_t SystemStartTimeGetUpHours(void);

#endif /* SYSTEM_START_TIME_STORE_H */
