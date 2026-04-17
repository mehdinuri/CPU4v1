/* App/Domain/Lcd/LcdPageRegistry.h
 *
 * Central registry of all available LCD pages.
 * Enables decoupled navigation between pages.
 */
#ifndef LCD_PAGE_REGISTRY_H
#define LCD_PAGE_REGISTRY_H

#include "LcdPage.h"

typedef struct
{
  LcdPage_t *home;
  LcdPage_t *login;
  LcdPage_t *menu;
  LcdPage_t *help;
  LcdPage_t *logs;
  LcdPage_t *connectionLogs;
  LcdPage_t *network;
  LcdPage_t *settings;
  LcdPage_t *settingsDateTime;
  LcdPage_t *settingsGps;
  LcdPage_t *settingsLanguage;
} LcdPageRegistry_t;

#endif /* LCD_PAGE_REGISTRY_H */
