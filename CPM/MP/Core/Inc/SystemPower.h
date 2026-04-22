#ifndef SYSTEM_POWER_H
#define SYSTEM_POWER_H

#include <stdint.h>

void ClearResetFlags(void);
void ClearStandbyFlag(void);
void ClearWakeupFlag(void);
void ClearAllFlags(void);
void PrepareForStandbyMode(void);
void EnterStandbyModeWithPreparation(uint8_t fPrep);
void EnterStandbyMode(void);
void CheckWakeupOnReset(void);

#endif /* SYSTEM_POWER_H */
