/* App/Adapters/STM32/FactoryResetAdapter.c */
#include "FactoryResetAdapter.h"

#include <string.h>

#include "DomainServices.h"
#include "Lcd/LcdLanguage.h"
#include "LrlfDetectTimeStore.h"
#include "MCS.h"
#include "PersistencePorts.h"

static uint8_t EraseConfigObject(ConfigRepositoryObjectId_t objectId)
{
  uint32_t capacity;

  if ((g_configRepositoryPort.GetCapacity == NULL)
      || (g_configRepositoryPort.Erase == NULL))
  {
    return 0U;
  }

  capacity = ConfigRepositoryGetCapacity(&g_configRepositoryPort, objectId);

  if (capacity == 0U)
  {
    return 0U;
  }

  return ConfigRepositoryErase(&g_configRepositoryPort, objectId, 0U, capacity);
}

static uint8_t RequestFactoryReset(void *ctx)
{
  const IntersectionConfig_t *config;
  uint16_t setId;
  uint8_t lrlfDetectTime;

  (void) ctx;

  if ((EraseConfigObject(CONFIG_REPOSITORY_OBJECT_SLOT_A) == 0U)
      || (EraseConfigObject(CONFIG_REPOSITORY_OBJECT_SLOT_B) == 0U)
      || (EraseConfigObject(CONFIG_REPOSITORY_OBJECT_MIGRATION_JOURNAL) == 0U))
  {
    return 0U;
  }

  (void) ConfigurationServiceLoad(&g_configurationService);
  config = ConfigurationServiceGetActiveConfig(&g_configurationService);
  setId = ConfigurationServiceGetActiveSetId(&g_configurationService);
  (void) IntersectionActivationServiceLoadCommittedLivePlan(
    &g_intersectionActivationService,
    config,
    setId);
  (void) IntersectionEngineLoadConfig(&g_intersectionEngine, config);

  (void) UiLanguageServiceSet(&g_uiLanguageService, LANGUAGE_TURKISH);
  (void) UiLanguageServiceSave(&g_uiLanguageService);

  lrlfDetectTime = LRLF_DETECT_TIME_800_MS;
  (void) PersistenceWrite(&g_persistencePort,
                          PERSIST_OBJECT_LRLF_DETECT_TIME,
                          0U,
                          &lrlfDetectTime,
                          sizeof(lrlfDetectTime));
  (void) MCSRestoreSnmpBootstrap();

  SystemResetPortRequest(&g_systemResetPort);

  return 1U;
}

void FactoryResetAdapterInit(FactoryResetAdapterCtx_t *ctx)
{
  if (ctx != NULL)
  {
    (void) memset(ctx, 0, sizeof(*ctx));
  }
}

IFactoryResetPort_t FactoryResetAdapterCreatePort(FactoryResetAdapterCtx_t *ctx)
{
  IFactoryResetPort_t port;

  port.ctx = ctx;
  port.RequestFactoryReset = RequestFactoryReset;
  return port;
}
