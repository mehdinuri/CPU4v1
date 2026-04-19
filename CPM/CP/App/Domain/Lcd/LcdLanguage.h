/* App/Domain/Lcd/LcdLanguage.h
 */
#ifndef LCD_LANGUAGE_H
#define LCD_LANGUAGE_H

#include <stdint.h>

#ifndef LANGUAGE_TURKISH
#define LANGUAGE_TURKISH 0U
#endif

#ifndef LANGUAGE_ENGLISH
#define LANGUAGE_ENGLISH 1U
#endif

#ifndef LANGUAGES_MAX
#define LANGUAGES_MAX    2U
#endif

const char *Lcd_GetStateStr(uint8_t state, uint8_t lang);
const char *Lcd_GetDayStr(uint8_t day, uint8_t lang);
const char *Lcd_GetAdvanceModeStr(uint8_t mode, uint8_t lang);
const char *Lcd_GetSignalSourceParamStr(uint8_t source, uint8_t lang);
const char *Lcd_GetNoEmergencyStr(uint8_t lang);
const char *Lcd_GetHelpStr(uint8_t lang);
const char *Lcd_GetLoginUserStr(uint8_t lang);
const char *Lcd_GetLoginPassStr(uint8_t lang);
const char *Lcd_GetNoLogStr(uint8_t lang);
const char *Lcd_GetErrorStr(uint8_t lang);
const char *Lcd_GetLocalIpStr(uint8_t lang);
const char *Lcd_GetServerIpStr(uint8_t lang);

const char *Lcd_GetSettingsMenuEntryStr(uint8_t index, uint8_t lang);

#endif /* LCD_LANGUAGE_H */
