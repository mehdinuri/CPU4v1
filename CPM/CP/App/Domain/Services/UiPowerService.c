/* App/Domain/Services/UiPowerService.c */
#include "UiPowerService.h"

#include <string.h>

static uint16_t ConvertNetVoltageTenthsVrms(uint16_t raw)
{
  float converted = ((float) raw * 0.73029f) + 0.5f;

  if (converted < 0.0f)
  {
    return 0U;
  }

  if (converted > 65535.0f)
  {
    return 65535U;
  }

  return (uint16_t) converted;
}

void UiPowerServiceInit(UiPowerService_t *service)
{
  if (service != NULL)
  {
    (void) memset(service, 0, sizeof(*service));
  }
}

void UiPowerServiceUpdateMeasurement(UiPowerService_t *service,
                                     uint8_t psmNumber,
                                     const UiPowerMeasurement_t *measurement)
{
  uint8_t index;

  if ((service == NULL) || (measurement == NULL)
      || (psmNumber == 0U) || (psmNumber > 2U))
  {
    return;
  }

  index = (uint8_t) (psmNumber - 1U);
  service->measurements[index] = *measurement;
  service->measurements[index].valid = 1U;
  service->updateSequence++;
}

uint8_t UiPowerServiceGetMeasurement(const UiPowerService_t *service,
                                     uint8_t psmNumber,
                                     UiPowerMeasurement_t *measurement)
{
  uint8_t index;

  if ((service == NULL) || (measurement == NULL)
      || (psmNumber == 0U) || (psmNumber > 2U))
  {
    return 0U;
  }

  index = (uint8_t) (psmNumber - 1U);
  if (service->measurements[index].valid == 0U)
  {
    return 0U;
  }

  *measurement = service->measurements[index];
  return 1U;
}

uint8_t UiPowerServiceGetLineVoltageTenthsVrms(
  const UiPowerService_t *service,
  uint8_t psmNumber,
  uint16_t *lineVoltageTenthsVrms)
{
  UiPowerMeasurement_t measurement;

  if ((lineVoltageTenthsVrms == NULL)
      || (UiPowerServiceGetMeasurement(service, psmNumber, &measurement) == 0U))
  {
    return 0U;
  }

  *lineVoltageTenthsVrms =
    ConvertNetVoltageTenthsVrms(measurement.netVoltageRaw);
  return 1U;
}

uint32_t UiPowerServiceGetUpdateSequence(const UiPowerService_t *service)
{
  return (service == NULL) ? 0U : service->updateSequence;
}
