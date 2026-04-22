/* App/Platform/STM32/Services/Persistence/SettingsStorage.h */
#ifndef SETTINGS_STORAGE_H
#define SETTINGS_STORAGE_H

#include <stdint.h>

#define USER_SETTINGS_CHANGE_CONTROL_VLAUE 240U
#define BROKEN_INPUT_SETTINGS_SET_CONTROL_VLAUE 240U
#define SERVER_SETTINGS_SET_CONTROL_VLAUE 240U

typedef struct _tSUserSettings
{
  uint8_t fSettingsChanged;
  uint8_t fConfigFlag;
  uint8_t fLogFlag;
  uint8_t fTrafficCountsFlag;
  uint8_t bTrafficCountsPeriod;
  uint8_t fStandbyInfoFlag;
} __attribute__((packed)) tSUserSettings, *tpSUserSettings;

typedef struct _tSBrokenInputSettings
{
  uint8_t fAlreadySet;

  struct
  {
    uint8_t fLoopBusy : 1;
    uint8_t fDigitalBusy : 1;
    uint8_t fReserved : 6;
  } __attribute__((packed)) SFlags;
} __attribute__((packed)) tSBrokenInputSettings, *tpSBrokenInputSettings;

typedef struct _tSServerSettings
{
  uint8_t fAlreadySet;

  struct
  {
    uint8_t fMCSAvailable : 1;
    uint8_t fNTCIPAvailable : 1;
    uint8_t fReserved : 6;
  } __attribute__((packed)) SFlags;
} __attribute__((packed)) tSServerSettings, *tpSServerSettings;

uint8_t UserSettingsSave(void);
uint8_t UserSettingsRead(void);
void UserSettingsInit(void);
void UserSettingsSet(tpSUserSettings pSUserSettings);
void UserSettingsGet(tpSUserSettings pSUserSettings);
uint8_t IsUserSettingsChanged(void);
uint8_t UserSettingsConfigFlagGet(void);
uint8_t UserSettingsLogFlagGet(void);
uint8_t UserSettingsTrafficCountsFlagGet(void);
uint8_t UserSettingsTrafficCountsPeriodGet(void);
uint8_t UserSettingsStandbyFlagGet(void);

uint8_t BrokenInputSettingsSave(void);
uint8_t BrokenInputSettingsRead(void);
void BrokenInputSettingsInit(void);
void BrokenInputSettingsSet(tpSBrokenInputSettings pSBrokenInputSettings);
void BrokenInputSettingsGet(tpSBrokenInputSettings pSBrokenInputSettings);
uint8_t IsBrokenInputSettingsSet(void);
uint8_t BrokenInputSettingsLoopFlagGet(void);
uint8_t BrokenInputSettingsDigitalFlagGet(void);

uint8_t ServerSettingsSave(void);
uint8_t ServerSettingsRead(void);
void ServerSettingsSet(tpSServerSettings pSServerSettings);
void ServerSettingsGet(tpSServerSettings pSServerSettings);
void ServerSettingsInit(void);
uint8_t IsServerSettingsSet(void);
uint8_t ServerSettingsMCSAvailableGet(void);
uint8_t ServerSettingsNTCIPAvailableGet(void);

#endif /* SETTINGS_STORAGE_H */
