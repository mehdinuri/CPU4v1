/* App/Domain/Services/UiPowerService.h */
#ifndef UI_POWER_SERVICE_H
#define UI_POWER_SERVICE_H

#include <stdint.h>

typedef struct
{
  uint16_t netVoltageRaw;
  uint16_t voltage24v1Raw;
  uint16_t voltage24v2Raw;
  uint16_t voltage5v1Raw;
  uint16_t voltage5v2Raw;
  uint8_t isolatedVoltagePresent;
  uint8_t netFrequencyRaw;
  uint8_t valid;
} UiPowerMeasurement_t;

typedef struct
{
  UiPowerMeasurement_t measurements[2];
  uint32_t updateSequence;
} UiPowerService_t;

void UiPowerServiceInit(UiPowerService_t *service);
void UiPowerServiceUpdateMeasurement(UiPowerService_t *service,
                                     uint8_t psmNumber,
                                     const UiPowerMeasurement_t *measurement);
uint8_t UiPowerServiceGetMeasurement(const UiPowerService_t *service,
                                     uint8_t psmNumber,
                                     UiPowerMeasurement_t *measurement);
uint8_t UiPowerServiceGetLineVoltageTenthsVrms(
  const UiPowerService_t *service,
  uint8_t psmNumber,
  uint16_t *lineVoltageTenthsVrms);
uint32_t UiPowerServiceGetUpdateSequence(const UiPowerService_t *service);

#endif /* UI_POWER_SERVICE_H */
