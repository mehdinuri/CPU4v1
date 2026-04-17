/* App/Domain/Intersection/ConfigurationService.h
 *
 * Owns MP's copy of the active intersection configuration plus MP-only
 * derived tables: the output-to-channel mapping and the per-pair
 * channel conflict matrix (computed from phase concurrency lists).
 *
 * Receives updates from the control-bus adapter as CP streams packets;
 * downstream monitors consume the service via read-only accessors.
 */
#ifndef CONFIGURATION_SERVICE_H
#define CONFIGURATION_SERVICE_H

#include <stdint.h>

#include "Intersection/ChannelStateResolver.h"
#include "Intersection/IntersectionConfig.h"
#include "Ports/IPersistencePort.h"

typedef enum
{
  CONFIG_STATE_UNINITIALIZED = 0U,
  CONFIG_STATE_LOADING = 1U,
  CONFIG_STATE_VALID = 2U,
  CONFIG_STATE_INVALID = 3U
} ConfigurationState_t;

typedef struct
{
  uint8_t matrix[MP_CHANNEL_COUNT_MAX][MP_CHANNEL_COUNT_MAX];
} ChannelConflictMatrix_t;

typedef struct
{
  IntersectionConfig_t config;
  ChannelOutputMapping_t outputMapping;
  ChannelConflictMatrix_t conflictMatrix;
  ConfigurationState_t state;
  uint16_t configEpoch;
  IPersistencePort_t *persistencePort;
} ConfigurationService_t;

void ConfigurationServiceInit(ConfigurationService_t *service,
                              IPersistencePort_t *persistencePort);
uint8_t ConfigurationServiceSetConfig(ConfigurationService_t *service,
                                      const IntersectionConfig_t *config);
uint8_t ConfigurationServiceSetOutputMapping(ConfigurationService_t *service,
                                             const ChannelOutputMapping_t *
                                             mapping);
uint8_t ConfigurationServiceValidate(ConfigurationService_t *service);
ConfigurationState_t ConfigurationServiceGetState(
  const ConfigurationService_t *service);
const IntersectionConfig_t *ConfigurationServiceGetConfig(
  const ConfigurationService_t *service);
const ChannelOutputMapping_t *ConfigurationServiceGetMapping(
  const ConfigurationService_t *service);
const ChannelConflictMatrix_t *ConfigurationServiceGetConflictMatrix(
  const ConfigurationService_t *service);
uint8_t ConfigurationServiceChannelsConflict(
  const ConfigurationService_t *service,
  uint8_t channelA,
  uint8_t channelB);

#endif /* CONFIGURATION_SERVICE_H */
