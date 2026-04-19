/* App/Domain/Services/UiLanguageService.h */
#ifndef UI_LANGUAGE_SERVICE_H
#define UI_LANGUAGE_SERVICE_H

#include <stdint.h>

#include "Domain/Lcd/LcdLanguage.h"
#include "Ports/IPersistencePort.h"

typedef struct
{
  IPersistencePort_t *persistencePort;
  uint8_t language;
} UiLanguageService_t;

void UiLanguageServiceInit(UiLanguageService_t *service);
void UiLanguageServiceBind(UiLanguageService_t *service,
                           IPersistencePort_t *persistencePort);
uint8_t UiLanguageServiceLoad(UiLanguageService_t *service);
uint8_t UiLanguageServiceSave(const UiLanguageService_t *service);
uint8_t UiLanguageServiceGet(const UiLanguageService_t *service);
uint8_t UiLanguageServiceSet(UiLanguageService_t *service, uint8_t language);

#endif /* UI_LANGUAGE_SERVICE_H */
