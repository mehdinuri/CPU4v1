/* App/Domain/Services/UiLanguageService.c */
#include "UiLanguageService.h"

#include <string.h>

static uint8_t NormalizeLanguage(uint8_t language)
{
  return (uint8_t) ((language < LANGUAGES_MAX) ? language : LANGUAGE_TURKISH);
}

void UiLanguageServiceInit(UiLanguageService_t *service)
{
  if (service != NULL)
  {
    (void) memset(service, 0, sizeof(*service));
    service->language = LANGUAGE_TURKISH;
  }
}

void UiLanguageServiceBind(UiLanguageService_t *service,
                           IPersistencePort_t *persistencePort)
{
  if (service != NULL)
  {
    service->persistencePort = persistencePort;
  }
}

uint8_t UiLanguageServiceLoad(UiLanguageService_t *service)
{
  uint8_t language = LANGUAGE_TURKISH;

  if ((service == NULL) || (service->persistencePort == NULL))
  {
    return 0U;
  }

  if (PersistenceRead(service->persistencePort,
                      PERSIST_OBJECT_LANGUAGE,
                      0U,
                      &language,
                      sizeof(language)) == 0U)
  {
    return 0U;
  }

  service->language = NormalizeLanguage(language);
  return 1U;
}

uint8_t UiLanguageServiceSave(const UiLanguageService_t *service)
{
  uint8_t language;

  if ((service == NULL) || (service->persistencePort == NULL))
  {
    return 0U;
  }

  language = NormalizeLanguage(service->language);
  return PersistenceWrite(service->persistencePort,
                          PERSIST_OBJECT_LANGUAGE,
                          0U,
                          &language,
                          sizeof(language));
}

uint8_t UiLanguageServiceGet(const UiLanguageService_t *service)
{
  return (service == NULL) ? LANGUAGE_TURKISH
         : NormalizeLanguage(service->language);
}

uint8_t UiLanguageServiceSet(UiLanguageService_t *service, uint8_t language)
{
  if (service == NULL)
  {
    return 0U;
  }

  service->language = NormalizeLanguage(language);
  return 1U;
}
