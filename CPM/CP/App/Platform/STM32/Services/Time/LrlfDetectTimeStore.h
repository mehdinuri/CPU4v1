/* App/Platform/STM32/Services/Time/LrlfDetectTimeStore.h */
#ifndef LRLF_DETECT_TIME_STORE_H
#define LRLF_DETECT_TIME_STORE_H

#include <stdint.h>

typedef enum
{
  LRLF_DETECT_TIME_NONE = 0,
  LRLF_DETECT_TIME_300_MS,
  LRLF_DETECT_TIME_800_MS,
  LRLF_DETECT_TIME_2_S,
  LRLF_DETECT_TIME_MAX = LRLF_DETECT_TIME_2_S
} tELRLFDetectTime;

#define CAN_LRLF_DETECT_TIME 0x0DAU

uint8_t LRLFDetectTimeWrite(void);
uint8_t LRLFDetectTimeRead(void);
void LRLFDetectTimeSet(uint8_t bTime);
uint8_t LRLFDetectTimeGet(void);
void LRLFDetectTimeCheck(void);

#endif /* LRLF_DETECT_TIME_STORE_H */
