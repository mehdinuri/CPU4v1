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
  if (config == NULL)
  {
    return 0U;
  }

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
  if ((config == NULL) || (channelIndex >= MP_CHANNEL_COUNT_MAX))
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

  /* Overlaps and other compound channel types are resolved separately into
   * explicit phase masks by ChannelPhaseMask(). */
  return 0U;
}

static uint16_t PhaseMaskFromList(const IntersectionPhaseReferenceList_t *list,
                                  uint8_t phaseCount)
{
  uint16_t mask = 0U;
  uint8_t index;

  if (list == NULL)
  {
    return 0U;
  }

  for (index = 0U; index < list->length; index++)
  {
    uint8_t phaseNumber = list->values[index];

    if ((phaseNumber == 0U) || (phaseNumber > phaseCount)
        || (phaseNumber > INTERSECTION_PHASE_COUNT_MAX))
    {
      continue;
    }

    mask |= (uint16_t) (1U << (phaseNumber - 1U));
  }

  return mask;
}

static uint16_t ChannelPhaseMask(const IntersectionConfig_t *config,
                                 uint8_t channelIndex)
{
  const IntersectionChannelConfig_t *channel;

  if ((config == NULL) || (channelIndex >= MP_CHANNEL_COUNT_MAX))
  {
    return 0U;
  }

  channel = &config->channels[channelIndex];

  switch ((IntersectionChannelControlType_t) channel->controlType)
  {
      case INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_VEHICLE:
      case INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_PEDESTRIAN:
      {
        if ((channel->controlSource == 0U)
            || (channel->controlSource > INTERSECTION_PHASE_COUNT_MAX)
            || (channel->controlSource > config->phaseCount))
        {
          return 0U;
        }

        return (uint16_t) (1U << (channel->controlSource - 1U));
      }

      case INTERSECTION_CHANNEL_CONTROL_TYPE_OVERLAP:
      case INTERSECTION_CHANNEL_CONTROL_TYPE_PED_OVERLAP:
      case INTERSECTION_CHANNEL_CONTROL_TYPE_QUEUE_JUMP:
      {
        if ((channel->controlSource == 0U)
            || (channel->controlSource > INTERSECTION_OVERLAP_COUNT_MAX))
        {
          return 0U;
        }

        return PhaseMaskFromList(
          &config->overlaps[channel->controlSource - 1U].includedPhases,
          config->phaseCount);
      }

      default:
      {
        return 0U;
      }
  }
}

static uint8_t PhaseMasksCanRunTogether(const IntersectionConfig_t *config,
                                        uint16_t phaseMaskA,
                                        uint16_t phaseMaskB)
{
  uint8_t phaseA;
  uint8_t phaseB;

  if ((config == NULL) || (phaseMaskA == 0U) || (phaseMaskB == 0U))
  {
    return 0U;
  }

  for (phaseA = 0U; phaseA < INTERSECTION_PHASE_COUNT_MAX; phaseA++)
  {
    if ((phaseMaskA & (uint16_t) (1U << phaseA)) == 0U)
    {
      continue;
    }

    for (phaseB = 0U; phaseB < INTERSECTION_PHASE_COUNT_MAX; phaseB++)
    {
      if ((phaseMaskB & (uint16_t) (1U << phaseB)) == 0U)
      {
        continue;
      }

      if (PhasesAreConcurrent(config, (uint8_t) (phaseA + 1U),
                              (uint8_t) (phaseB + 1U)) != 0U)
      {
        return 1U;
      }
    }
  }

  return 0U;
}

static uint8_t ClampDsToByte(uint16_t value)
{
  return (value > 0xFFU) ? 0xFFU : (uint8_t) value;
}

static uint8_t ChannelMinYellowDs(const IntersectionConfig_t *config,
                                  uint8_t channelIndex)
{
  const IntersectionChannelConfig_t *channel;

  if ((config == NULL) || (channelIndex >= MP_CHANNEL_COUNT_MAX))
  {
    return 0U;
  }

  channel = &config->channels[channelIndex];

  switch ((IntersectionChannelControlType_t) channel->controlType)
  {
      case INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_VEHICLE:
      case INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_PEDESTRIAN:
      {
        uint8_t phaseNumber = ChannelControlPhase(config, channelIndex);

        if (phaseNumber == 0U)
        {
          return 0U;
        }

        return ClampDsToByte(config->phases[phaseNumber - 1U].yellowChangeDs);
      }

      case INTERSECTION_CHANNEL_CONTROL_TYPE_OVERLAP:
      case INTERSECTION_CHANNEL_CONTROL_TYPE_PED_OVERLAP:
      case INTERSECTION_CHANNEL_CONTROL_TYPE_QUEUE_JUMP:
      {
        if ((channel->controlSource == 0U)
            || (channel->controlSource > INTERSECTION_OVERLAP_COUNT_MAX))
        {
          return 0U;
        }

        return ClampDsToByte(
          config->overlaps[channel->controlSource - 1U].trailYellowDs);
      }

      default:
      {
        return 0U;
      }
  }
}

static uint8_t ChannelRedClearDs(const IntersectionConfig_t *config,
                                 uint8_t channelIndex)
{
  const IntersectionChannelConfig_t *channel;

  if ((config == NULL) || (channelIndex >= MP_CHANNEL_COUNT_MAX))
  {
    return 0U;
  }

  channel = &config->channels[channelIndex];

  switch ((IntersectionChannelControlType_t) channel->controlType)
  {
      case INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_VEHICLE:
      case INTERSECTION_CHANNEL_CONTROL_TYPE_PHASE_PEDESTRIAN:
      {
        uint8_t phaseNumber = ChannelControlPhase(config, channelIndex);

        if (phaseNumber == 0U)
        {
          return 0U;
        }

        return ClampDsToByte(config->phases[phaseNumber - 1U].redClearDs);
      }

      case INTERSECTION_CHANNEL_CONTROL_TYPE_OVERLAP:
      case INTERSECTION_CHANNEL_CONTROL_TYPE_PED_OVERLAP:
      case INTERSECTION_CHANNEL_CONTROL_TYPE_QUEUE_JUMP:
      {
        if ((channel->controlSource == 0U)
            || (channel->controlSource > INTERSECTION_OVERLAP_COUNT_MAX))
        {
          return 0U;
        }

        return ClampDsToByte(
          config->overlaps[channel->controlSource - 1U].trailRedDs);
      }

      default:
      {
        return 0U;
      }
  }
}

static void LoadConflictMatrixFromMasks(ConfigurationService_t *service)
{
  uint8_t a;
  uint8_t b;

  if (service == NULL)
  {
    return;
  }

  (void) memset(&service->conflictMatrix, 0, sizeof(service->conflictMatrix));

  for (a = 0U; a < MP_CHANNEL_COUNT_MAX; a++)
  {
    uint32_t normalizedMask = 0U;

    for (b = 0U; b < MP_CHANNEL_COUNT_MAX; b++)
    {
      if (a == b)
      {
        continue;
      }

      if (((service->channelConflictMask[a] & (uint32_t) (1UL << b)) != 0U)
          || ((service->channelConflictMask[b] & (uint32_t) (1UL << a)) != 0U))
      {
        service->conflictMatrix.matrix[a][b] = 1U;
        normalizedMask |= (uint32_t) (1UL << b);
      }
    }

    service->channelConflictMask[a] = normalizedMask;
  }
}

static void BuildConflictMatrix(ConfigurationService_t *service)
{
  uint8_t channelIndex;
  uint16_t channelPhaseMasks[MP_CHANNEL_COUNT_MAX];

  if (service == NULL)
  {
    return;
  }

  for (channelIndex = 0U; channelIndex < MP_CHANNEL_COUNT_MAX; channelIndex++)
  {
    channelPhaseMasks[channelIndex] =
      ChannelPhaseMask(&service->config, channelIndex);
    service->channelMinYellowDs[channelIndex] =
      ChannelMinYellowDs(&service->config, channelIndex);
    service->channelRedClearDs[channelIndex] =
      ChannelRedClearDs(&service->config, channelIndex);
  }

  for (channelIndex = 0U; channelIndex < MP_CHANNEL_COUNT_MAX; channelIndex++)
  {
    uint8_t otherIndex;
    uint32_t conflictMask = 0U;

    if (channelPhaseMasks[channelIndex] == 0U)
    {
      service->channelConflictMask[channelIndex] = 0U;
      continue;
    }

    for (otherIndex = 0U; otherIndex < MP_CHANNEL_COUNT_MAX; otherIndex++)
    {
      if ((otherIndex == channelIndex)
          || (channelPhaseMasks[otherIndex] == 0U))
      {
        continue;
      }

      if (PhaseMasksCanRunTogether(&service->config,
                                   channelPhaseMasks[channelIndex],
                                   channelPhaseMasks[otherIndex]) == 0U)
      {
        conflictMask |= (uint32_t) (1UL << otherIndex);
      }
    }

    service->channelConflictMask[channelIndex] = conflictMask;
  }

  LoadConflictMatrixFromMasks(service);
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

uint8_t ConfigurationServiceSetMonitoringProfile(
  ConfigurationService_t *service,
  const uint32_t *channelConflictMask,
  const uint8_t *channelMinYellowDs,
  const uint8_t *channelRedClearDs)
{
  if ((service == NULL) || (channelConflictMask == NULL)
      || (channelMinYellowDs == NULL) || (channelRedClearDs == NULL))
  {
    return 0U;
  }

  (void) memcpy(&service->channelConflictMask[0],
                channelConflictMask,
                sizeof(service->channelConflictMask));
  (void) memcpy(&service->channelMinYellowDs[0],
                channelMinYellowDs,
                sizeof(service->channelMinYellowDs));
  (void) memcpy(&service->channelRedClearDs[0],
                channelRedClearDs,
                sizeof(service->channelRedClearDs));
  LoadConflictMatrixFromMasks(service);

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

uint8_t ConfigurationServiceGetChannelMinYellowDs(
  const ConfigurationService_t *service,
  uint8_t channelIndex)
{
  if ((service == NULL) || (channelIndex >= MP_CHANNEL_COUNT_MAX))
  {
    return 0U;
  }

  return service->channelMinYellowDs[channelIndex];
}

uint8_t ConfigurationServiceGetChannelRedClearDs(
  const ConfigurationService_t *service,
  uint8_t channelIndex)
{
  if ((service == NULL) || (channelIndex >= MP_CHANNEL_COUNT_MAX))
  {
    return 0U;
  }

  return service->channelRedClearDs[channelIndex];
}
