/* App/Platform/STM32/Core/SystemRuntime.h */
#ifndef SYSTEM_RUNTIME_H
#define SYSTEM_RUNTIME_H

#include <stdint.h>

typedef enum
{
  RESET_SOURCE_NONE = 0,
  RESET_SOURCE_IWDG,
  RESET_SOURCE_WWDG,
  RESET_SOURCE_LOW_POWER,
  RESET_SOURCE_SOFTWARE,
  RESET_SOURCE_PIN,
  RESET_SOURCE_POR
} tEResetSource;

void SetDeviceResetEvent(void);
uint8_t GetDeviceResetEvent(void);
void SetDeviceResetSource(void);
uint8_t GetDeviceResetSource(void);
void ClearAllFlags(void);
uint8_t GetStandbyState(void);
void SetStandbyState(uint8_t fState);
void NotifyStandbyState(void);
void CheckWakeupState(void);
void ExecStandbyInfoOps(void);
void SecureSystemReset(void);

#endif /* SYSTEM_RUNTIME_H */
