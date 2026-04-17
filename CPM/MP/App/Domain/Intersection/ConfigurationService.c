/* App/Domain/Intersection/ConfigurationService.c */

#include "Intersection/ConfigurationService.h"

#include <stddef.h>
#include <string.h>

static uint8_t PhaseListContains(const IntersectionPhaseReferenceList_t *list,
                                 uint8_t phaseNumber)
{
  uint32_t i;

  if (list == NULL)
  {
    return 0U;
  }

  for (i = 0U; i < list->length; i++)
  {
    if (list->values[i] == phaseNumber)
    {
      return 1U;
    }
  }

  return 0U;
}

static uint8_t PhasesAreConcurrent(const IntersectionConfig_t *config,
                                   uint8_t phaseA,
                                   uint8_t phaseB)
{
  if ((phaseA == 0U) || (phaseB == 0U))
  {
    return 0U;
  }

  if (phaseA == phaseB)
  {
    return 1U;
  }

  if ((phaseA > config->phaseCount) || (phaseB > config->phaseCount))
  {
    return 0U;
  }

  const IntersectionPhaseConfig_t *a = &config->phases[phaseA - 1U];
  const IntersectionPhaseConfig_t *b = &config->phases[phaseB - 1U];

  return (uint8_t) (PhaseListContains(&a->concurrency, phaseB)
                    || PhaseListContains(&b->concurrency, phaseA));
}

static uint8_t ChannelControlPhase(const IntersectionConfig_t *config,
                                   uint8_t channelIndex)
{
  if (channelIndex >= MP_CHANNEL_COUNT_MAX)
  {
    return 0U;
  }

  const IntersectionChannelConfig_t *channel = &config->channels[channelIndex];

  if (channel->controlType == INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_VEHICLE)
  {
    return channel->controlSource;
  }

  if (channel->controlType
      == INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_PEDESTRIAN)
  {
    return channel->controlSource;
  }

  /* Overlap / other channel types — treated as non-conflicting at this
   * layer. A future enhancement resolves overlap includedPhases into
   * effective phase sets. */
  return 0U;
}

static void BuildConflictMatrix(ConfigurationService_t *service)
{
  uint32_t a;
  uint32_t b;

  (void) memset(&service->conflictMatrix, 0, sizeof(service->conflictMatrix));

  for (a = 0U; a < MP_CHANNEL_COUNT_MAX; a++)
  {
    uint8_t phaseA = ChannelControlPhase(&service->config, (uint8_t) a);

    if (phaseA == 0U)
    {
      continue;
    }

    for (b = a + 1U; b < MP_CHANNEL_COUNT_MAX; b++)
    {
      uint8_t phaseB = ChannelControlPhase(&service->config, (uint8_t) b);

      if (phaseB == 0U)
      {
        continue;
      }

      if (PhasesAreConcurrent(&service->config, phaseA, phaseB) == 0U)
      {
        service->conflictMatrix.matrix[a][b] = 1U;
        service->conflictMatrix.matrix[b][a] = 1U;
      }
    }
  }
}

void ConfigurationServiceInit(ConfigurationService_t *service,
                              IPersistencePort_t *persistencePort)
{
  if (service == NULL)
  {
    return;
  }

  (void) memset(service, 0, sizeof(*service));
  ChannelStateResolverInit(&service->outputMapping);
  service->state = CONFIG_STATE_UNINITIALIZED;
  service->persistencePort = persistencePort;
}

uint8_t ConfigurationServiceSetConfig(ConfigurationService_t *service,
                                      const IntersectionConfig_t *config)
{
  if ((service == NULL) || (config == NULL))
  {
    return 0U;
  }

  service->state = CONFIG_STATE_LOADING;
  service->config = *config;
  BuildConflictMatrix(service);
  service->configEpoch++;

  return 1U;
}

uint8_t ConfigurationServiceSetOutputMapping(ConfigurationService_t *service,
                                             const ChannelOutputMapping_t *
                                             mapping)
{
  if ((service == NULL) || (mapping == NULL))
  {
    return 0U;
  }

  service->outputMapping = *mapping;

  return 1U;
}

uint8_t ConfigurationServiceValidate(ConfigurationService_t *service)
{
  if (service == NULL)
  {
    return 0U;
  }

  if (service->state == CONFIG_STATE_UNINITIALIZED)
  {
    return 0U;
  }

  if (service->config.phaseCount == 0U)
  {
    service->state = CONFIG_STATE_INVALID;

    return 0U;
  }

  if (service->config.phaseCount > INTERSECTION_PHASE_COUNT_MAX)
  {
    service->state = CONFIG_STATE_INVALID;

    return 0U;
  }

  if (service->config.ringCount == 0U)
  {
    service->state = CONFIG_STATE_INVALID;

    return 0U;
  }

  service->state = CONFIG_STATE_VALID;

  return 1U;
}

ConfigurationState_t ConfigurationServiceGetState(
  const ConfigurationService_t *service)
{
  if (service == NULL)
  {
    return CONFIG_STATE_UNINITIALIZED;
  }

  return service->state;
}

const IntersectionConfig_t *ConfigurationServiceGetConfig(
  const ConfigurationService_t *service)
{
  if (service == NULL)
  {
    return NULL;
  }

  return &service->config;
}

const ChannelOutputMapping_t *ConfigurationServiceGetMapping(
  const ConfigurationService_t *service)
{
  if (service == NULL)
  {
    return NULL;
  }

  return &service->outputMapping;
}

const ChannelConflictMatrix_t *ConfigurationServiceGetConflictMatrix(
  const ConfigurationService_t *service)
{
  if (service == NULL)
  {
    return NULL;
  }

  return &service->conflictMatrix;
}

uint8_t ConfigurationServiceChannelsConflict(
  const ConfigurationService_t *service,
  uint8_t channelA,
  uint8_t channelB)
{
  if (service == NULL)
  {
    return 0U;
  }

  if ((channelA >= MP_CHANNEL_COUNT_MAX) || (channelB >= MP_CHANNEL_COUNT_MAX))
  {
    return 0U;
  }

  return service->conflictMatrix.matrix[channelA][channelB];
}
