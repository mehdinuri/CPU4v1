#ifndef _LCD
#define _LCD

/* /////////////////////////// */
/*  includes */
#include "Ports/UserInputTypes.h"

typedef struct _tSMCSLCDStream tSMCSLCDStream;
typedef tSMCSLCDStream *tpSMCSLCDStream;

/* /////////////////////////////////////////// */
/*  Public Methods */
extern void LCDSoftwareOpen(void);
extern void LCDSoftwareClose(void);
extern void InitLCDTask(void);

/* //////// Keypad  ////////////////////////////////////////////// */
#define NUMBER_OF_KEYS KEYPAD_KEY_COUNT

#define LCD_KEY_NO_SAMPLE 0
#define LCD_KEY_NEEDED_SAMPLE_COUNT 5
#define LCD_KEY_SCAN_TIME 40 /* 40 ms, assume code that scans keypad takes 0 ms */

#define LCD_COUNTER_OPENING_VALUE 0
#define LCD_COUNTER_CLOSING_VALUE                                              \
        (200000 / LCD_KEY_SCAN_TIME) /* 5 minutes = 5x60x1000 ms = 300000 ms */

/* cursor blinks */
#define LCD_CURSOR_BLINK_TOGGLE_TIME                                           \
        (600 / LCD_KEY_SCAN_TIME) /* cursor is visible for 600 ms */
#define LCD_CURSOR_VISIBLE 0x7F /* vertical bar */
#define LCD_CURSOR_INVISIBLE 0x00 /* blank */

/* update maestro menu */
#define LCD_MAESTRO_STATE_UPDATE_TIME                                          \
        LCD_KEY_SCAN_TIME /* (800/LCD_KEY_SCAN_TIME) */

/* menu size */
#define LCD_SIZE_OF_PAGE 4 /* in terms of lines */
#define LCD_SIZE_OF_MENU 6 /* in terms of lines */
#define LCD_SIZE_OF_ADMIN_MENU 6 /* in terms of lines */
#define LCD_SIZE_OF_GUEST_MENU 3 /* in terms of lines */
#define LCD_MENU_PAGE_ADMIN_TOTAL 2 /* in terms of pages */
#define LCD_MENU_PAGE_GUEST_TOTAL 1 /* in terms of pages */

/* lcd pages */
#define LCD_MENU_PAGE1 0
#define LCD_MENU_PAGE2 1
#define LCD_MENU_PAGE3 2
#define LCD_MENU_PAGE4 3
#define LCD_MENU_PAGE5 4
#define LCD_MENU_PAGE6 5
#define LCD_MENU_PAGE7 6
#define LCD_MENU_PAGE8 7
#define LCD_MENU_PAGE16 15

/* limited menu for guest users */
#define LCD_ADMIN_MENU_UPPER_INDEX 12
#define LCD_GUEST_MENU_UPPER_INDEX 3
#define LCD_NO_MENU_UPPER_INDEX 0

/* menu states */
#define LCD_STATE_NOT_IN_A_SUBMENU 0x00

/* logging in to LCD */
#define LCD_STATE_OPENING_SCREEN 0x01
#define LCD_STATE_USERNAME_SCREEN 0x02
#define LCD_STATE_PASSWORD_SCREEN 0x03

#define LCD_HOME_SCREEN_DATETIME 0x01
#define LCD_HOME_SCREEN_DEMANDS 0x02
#define LCD_HOME_SCREEN_CONNECTIONS 0x03

/* menu */
#define LCD_STATE_MENU 0x70

/* menu new */
#define LCD_STATE_MENU_START 0x80
#define LCD_STATE_HELP_MENU 0x80
#define LCD_STATE_LOG_MENU 0x81
#define LCD_STATE_GPRS_MODEM_LOG_MENU 0x82
#define LCD_STATE_TEST 0x83
#define LCD_STATE_REMOTE_CONNECTION 0x84
#define LCD_STATE_DEVICE_SETTINGS_MENU 0x85

#define LCD_STATE_GSM_GPRS_MENU 0x86
#define LCD_STATE_SEQ_STEP_DURS 0x87
#define LCD_STATE_SSM_TEST 0x88
#define LCD_STATE_TIME_MENU 0x89
#define LCD_STATE_POWER_MENU 0x8A
#define LCD_STATE_SP_TRACE_MENU 0x8B
#define LCD_STATE_POWER_RELAY_MENU 0x8C
#define LCD_STATE_LANGUAGE_MENU 0x8D
#define LCD_STATE_MAESTRO_MENU 0x8E
#define LCD_STATE_PROGRAM_MENU 0x8F
#define LCD_STATE_PSM_TEST 0x90

/* unlicensed */
#define LCD_STATE_UNLICENSED_USAGE 0xFF

/* HELP MENU */

/* LOG MENU */

/* TEST MENU */
#define LCD_TEST_MAIN 0x00
#define LCD_TEST_RELAY 0x01
#define LCD_TEST_SO_POWERS 0x02
#define LCD_POWER_CONSTANTS 0x03
#define LCD_POWER_RUNTIME_VALUES 0x04
#define LCD_POWER_MENU_SIZE 2
#define LCD_TEST_MENU_SIZE 1

#define LCD_MAX_SG_CHAR_NO 2

/* REMOTE CONNECTION MENU */
#define LCD_REMOTE_CONNECTION_MAIN 0x00
#define LCD_REMOTE_CONNECTION_IP 0x01
#define LCD_REMOTE_CONNECTION_PORT 0x02

/* DEVICE SETTINGS MENU */
#define LCD_MAX_BAUD_RATE_CHAR_NO 6

/* ////////////////////////////////////////// */
/* PROGRAM MENU */
/* program menu states */
#define LCD_PROGRAM_MENU_SIZE 7
#define LCD_PROGRAM_MENU_MAIN1 0x30
#define LCD_PROGRAM_MENU_FLASH 0x31
#define LCD_PROGRAM_MENU_SG 0x32
#define LCD_PROGRAM_MENU_PHASE 0x33
#define LCD_PROGRAM_MENU_SP 0x34
#define LCD_PROGRAM_MENU_MAIN2 0x40
#define LCD_PROGRAM_MENU_WP 0x45
#define LCD_PROGRAM_MENU_WS 0x46
#define LCD_PROGRAM_MENU_PAGE1_SIZE 4
#define LCD_PROGRAM_MENU_PAGE2_SIZE 2

/* new approaches to menu design, consider each of them separately as different */
/* states instead of menu */
#define LCD_STATE_OPERATIONS 0x50
#define LCD_STATE_TRANSITIONS 0x51

/* /////////////////////////////////////////// */

/* SYSTEM SETTINGS MENU */
#define LCD_DEVICE_SETTINGS_MENU_START 0x60
#define LCD_DEVICE_SETTINGS_MAIN 0x60
#define LCD_DEVICE_SETTINGS_TIME_DATE 0x61
#define LCD_DEVICE_SETTINGS_LANGUAGE 0x62
#define LCD_DEVICE_SETTINGS_GPS_SETTINGS 0x63
/* H&D Commented */

/*
 #define  LCD_DEVICE_SETTINGS_HEATER              0x64
 #define  LCD_DEVICE_SETTINGS_LAMP_DIMMING        0x65
 */
#define LCD_DEVICE_SETTINGS_CONNECTION_SETTINGS 0x64
#define LCD_DEVICE_SETTINGS_SERVER_SETTINGS     0x65
#define LCD_DEVICE_SETTINGS_USER_ACCOUNT        0x66
#define LCD_DEVICE_SETTINGS_USER_SETTINGS       0x67
#define LCD_DEVICE_SETTINGS_BROKEN_INPUT_SETTINGS 0x68
#define LCD_DEVICE_SETTINGS_LRLF_DETECT_TIME    0x69
#define LCD_DEVICE_SETTINGS_FACTORY_SETTINGS    0x6A
#define LCD_DEVICE_SETTINGS_MENU_SIZE           10

/* GPS SETTINGS MENU */
#define LCD_GPS_SETTINGS_MAIN 0x00
#define LCD_GPS_SETTINGS_PORT_CONFIG 0x01
#define LCD_GPS_SETTINGS_BAUD_RATE_CONFIG 0x02
#define LCD_GPS_SETTINGS_MENU_SIZE 1
#define LCD_GPS_MIN_BAUD_RATE_INDEX 1
#define LCD_GPS_MAX_BAUD_RATE_INDEX 11

/* IAP MODE */
#define LCD_MAESTRO_CPU_MODE_IAP 0
#define LCD_MAESTRO_CPU_MODE_NORMAL 1
#define LCD_MAESTRO_CPU_MODE_TOTAL 2

/* USER ACCOUNTS */
#define LCD_USER_ACCOUNTS_MAIN 0x00
#define LCD_USER_ACCOUNTS_CHANGE_PASSWORD 0x01
#define LCD_USER_ACCOUNTS_MENU_SIZE 1

/* ////////////////////////////////////////// */
/* user accounts - change passwords */
#define LCD_CHANGE_PASSWORD_FOURTH_STEP 0x00
#define LCD_CHANGE_PASSWORD_THIRD_STEP 0x01
#define LCD_CHANGE_PASSWORD_SECOND_STEP 0x02
#define LCD_CHANGE_PASSWORD_FIRST_STEP 0x03
#define LCD_CHANGE_PASSWORD_NO_STEP 0x04

/* REMOTE CONNECTION SELECTION MENU */
#define LCD_REMOTE_CONN_SEL_MAIN 0x70
#define LCD_REMOTE_CONN_SEL_TM_ETH 0x71
#define LCD_REMOTE_CONN_SEL_TM_GPRS 0x72
#define LCD_REMOTE_CONN_SEL_MLC_GPRS 0x73
#define LCD_REMOTE_CONN_SEL_MENU_SIZE 3

/* PHASE MENU */
#define LCD_PHASE_INFORMATION_MAIN 0x40
#define LCD_PHASE_INFORMATION_DISPLAY 0x41
#define LCD_PHASE_INFORMATION_UPDATE 0x42
#define LCD_PHASE_INFORMATION_MENU_SIZE 2

/* PHASE SUBMENU */
#define LCD_PHASE_INFORMATION_DISPLAY_HELP 0x40

/* show signal group signals */
#define LCD_STATE_GROUP_SIGNALS_PAGE1 0
#define LCD_STATE_GROUP_SIGNALS_PAGE2 1
#define LCD_STATE_GROUP_SIGNALS_PAGE3 2
#define LCD_STATE_GROUPS_PER_LINE 3
#define LCD_STATE_GROUPS_PER_LINE2 10
#define LCD_STATE_GROUPS_PER_LINE3 6
#define LCD_STATE_PHASES_PER_LINE 6
#define LCD_STATE_WEEKDAYS_PER_LINE 3
#define LCD_STATE_GROUPS_PER_PAGE 12

/* language */
#define LCD_LANGUAGE_MENU_ON 0x01
#define LCD_LANGUAGE_MENU_OFF 0x02

/* ////////////////////////////////////////// */
/* usernames */
#define LCD_USERNAME_FOURTH_STEP 0x00
#define LCD_USERNAME_THIRD_STEP 0x01
#define LCD_USERNAME_SECOND_STEP 0x02
#define LCD_USERNAME_FIRST_STEP 0x03
#define LCD_USERNAME_NO_STEP 0x04

/* ////////////////////////////////////////// */
/* passwords */
#define LCD_PASSWORD_FOURTH_STEP 0x00
#define LCD_PASSWORD_THIRD_STEP 0x01
#define LCD_PASSWORD_SECOND_STEP 0x02
#define LCD_PASSWORD_FIRST_STEP 0x03
#define LCD_PASSWORD_NO_STEP 0x04

#define LCD_PASSWORD_LENGTH 0x04
#define LCD_USERNAME_LENGTH 0x04

/* ////////////////////////////////////////// */
/* time menu indexes */
#define LCD_TIME_FIRST_STEP 0
#define LCD_TIME_MONTH_DAY_SECOND_STEP 0
#define LCD_TIME_MONTH_DAY_FIRST_STEP 1
#define LCD_TIME_MONTH_SECOND_STEP 2
#define LCD_TIME_MONTH_FIRST_STEP 3
#define LCD_TIME_YEAR_SECOND_STEP 4
#define LCD_TIME_YEAR_FIRST_STEP 5
#define LCD_TIME_HOUR_SECOND_STEP 6
#define LCD_TIME_HOUR_FIRST_STEP 7
#define LCD_TIME_MINUTE_SECOND_STEP 8
#define LCD_TIME_MINUTE_FIRST_STEP 9
#define LCD_TIME_LAST_STEP 9

#define LCD_TIME_STATE_TIME_DATE 0
#define LCD_TIME_STATE_DST 1

/* time line is like: DDMMYY HHMM */
/* the followings are lcd column indexes */
#define LCD_TIME_LINE_START 0
#define LCD_TIME_LINE_END                                                      \
        10 /* timeline end is the start of the minute second step */

/* relay menu */
#define LCD_BINARY_STATES 2
#define LCD_BINARY_ON 1
#define LCD_BINARY_OFF 0

/* lamp dimming and heater menu */
/* H&D Commented */

/*
 #define LCD_LOGIC_LEVELS             2
 #define LCD_LOGIC_LEVEL_HIGH         1
 #define LCD_LOGIC_LEVEL_LOW            0
 */

/* yes no menu */
#define LCD_NO 0
#define LCD_YES 1
#define LCD_YES_NO_MENU_SIZE 2

/* device on - off menu */
#define LCD_PROGRAM_START 0
#define LCD_PROGRAM_STOP 1
#define LCD_PROGRAM_START_STOP_MENU_SIZE 2

/* display update menu */
#define LCD_DISPLAY_SELECT 0
#define LCD_UPDATE_SELECT 1
#define LCD_DISPLAY_UPDATE_MENU_SIZE 2

/* maestro state menu indexes */
/* #define LCD_PHASE_ELAPSED_DURATION_START       10 */
/* #define LCD_PHASE_ELAPSED_DURATION_COLUMN_START      LCD_CHAR_WIDTH * */
/* LCD_PHASE_ELAPSED_DURATION_START */

/* lines */
#define LCD_FIRST_LINE 0
#define LCD_SECOND_LINE 1
#define LCD_THIRD_LINE 2
#define LCD_FOURTH_LINE 3
#define LCD_FIRST_COLUMN 0
#define LCD_LAST_COLUMN 18
#define LCD_LAST_COLUMN_NO 121
#define LCD_NUMBER_OF_LINES 4
#define LCD_LINE_SIZE 20
#define LCD_CHAR_WIDTH 6

/* language-specific glyphs from the LCD font table */
#define _CH 0xc7
#define _GH 0xd0
#define _IE 0xdd
#define _OE 0xd6
#define _SH 0xde
#define _UE 0xdc

/* constants */
#define ONE 1
#define TEN 10
#define HUNDRED 100
#define THOUSAND 1000
#define TEN_THOUSAND 10000
#define HUNDRED_THOUSAND 100000
#define MILLION 1000000

/* log menu indexes */
#define LCD_LOG_INDEX_FOURTH_STEP 0
#define LCD_LOG_INDEX_THIRD_STEP 1
#define LCD_LOG_INDEX_SECOND_STEP 2
#define LCD_LOG_INDEX_FIRST_STEP 3

#define LCD_LOG_INDEX_COLUMN_START (LCD_FIRST_COLUMN + 6)

/* string types */
#define LCD_STRING_TYPE_SHORT 0
#define LCD_STRING_TYPE_LONG 1

#define LCD_STANDARD_SIGNALS_SIZE 11

/* Other Chars and symbols */
#define LCD_CHR_CURSOR 95 /* index of cursor in char table */
#define LCD_CHR_LEFT_TRIANGLE 0x80
#define LCD_CHR_RIGHT_TRIANGLE 0x81
#define LCD_CHR_UP_DOWN 0x82
#define LCD_CHR_CHECK 0x83
#define LCD_CHR_QUESTION_MARK 0x3F
#define LCD_CHAR_SPACE 0x20
#define LCD_CHAR_0 0x30
#define LCD_CHAR_x 0x78
#define LCD_CHAR_s 0x84

#define MAX_DECIMAL_CHAR_NUMBER 10
#define MAX_HEX_CHAR_NUMBER 16
#define MAX_SG_CODE_SHOW_CHAR 10

/* member signal groups code show characters */
#define LCD_CHAR_1 9
#define LCD_CHAR_2 8
#define LCD_CHAR_3 7
#define LCD_CHAR_4 6
#define LCD_CHAR_5 5
#define LCD_CHAR_6 4
#define LCD_CHAR_7 3
#define LCD_CHAR_8 2
#define LCD_CHAR_9 1
#define LCD_CHAR_10 0

/*
 *  name: user requests from lcd
 *  expl: user can have requests to control device state.
 */
#define LCD_USER_REQUEST_NONE     0
#define LCD_USER_REQUEST_ALL_RED    1 /* request to assign red to all signal groups */
#define LCD_USER_REQUEST_DARK     2 /* request to assign dark to all signal groups */
#define LCD_USER_REQUEST_FLASH      3 /* request to assign flash signals to all signal groups */
#define LCD_USER_REQUEST_PLAN_RETURN  4 /* request to return from user determined states to work schedule examination */
#define LCD_USER_REQUEST_LANGUAGE   5 /* user wants to change lcd interface language */
#define LCD_USER_REQUEST_LEARNING   6 /* user triggers signal output power learning */
#define LCD_USER_REQUEST_TEST     7 /* user starts to test device: SSM modules are tested */
#define LCD_USER_REQUEST_MCS_LOGS 8
#define LCD_USER_REQUEST_REMOTE_CONNECTION 9

/* temp string buffer index */
#define STR_BUFFER_INDEX_0 0
#define STR_BUFFER_INDEX_1 1
#define STR_BUFFER_INDEX_2 2

/* digit extremums */
#define MIN_TWO_DIGIT_NUMBER    10
#define MIN_THREE_DIGIT_NUMBER  100

/* signal mode sources */
#define SIGNAL_MODE_SOURCE_MODULE_MISSING 0
#define SIGNAL_MODE_SOURCE_SO_WORKING_LAMP_TOTAL_CHANGE 1
#define SIGNAL_MODE_SOURCE_SG_LAST_RED_LAMP_FAILURE 2
#define SIGNAL_MODE_SOURCE_SG_NUMBER_OF_RED_LAMPS_FAILURE 3
#define SIGNAL_MODE_SOURCE_YELLOW_YELLOW_CONFLICT 4
#define SIGNAL_MODE_SOURCE_YELLOW_GREEN_CONFLICT 5
#define SIGNAL_MODE_SOURCE_GREEN_GREEN_CONFLICT 6
#define SIGNAL_MODE_SOURCE_INVALID_SIGNAL 7
#define SIGNAL_MODE_SOURCE_INVALID_SIGNAL_SEQUENCE 8
#define VALS_SIGNAL_MODE_SOURCE_MAX 9

#define TWO_DIGIT   2
#define THREE_DIGIT 3

/* Heater & Lamp Dimming */
/* states */
/* H&D Commented */

/*
 #define  LCD_H_A_LD_STATE_CONFIG_LEVEL                     1
 #define  LCD_H_A_LD_STATE_CONFIG_STATE                     2
 #define  LCD_H_A_LD_STATE_CONFIG_SAVED                     3
 */
/* Flag Settings */
/* states */
#define LCD_USER_SETTINGS_STATE_CONFIG_FLAG 1
#define LCD_USER_SETTINGS_STATE_STOP_MODE_INFO_FLAG 2
#define LCD_USER_SETTINGS_STATE_SAVED 3

/* Broken Input */
/* states */
#define LCD_BROKEN_INPUT_SETTINGS_STATE_LOOP_INPUT 1
#define LCD_BROKEN_INPUT_SETTINGS_STATE_DIGITAL_INPUT 2
#define LCD_BROKEN_INPUT_SETTINGS_STATE_SAVED 3

/* Server Settings */
/* states */
#define LCD_SERVER_SETTINGS_STATE_MCS_AVAILABLE 1
#define LCD_SERVER_SETTINGS_STATE_NTCIP_AVAILABLE 2
#define LCD_SERVER_SETTINGS_STATE_SAVED 3

/*  PSM Test */
#define PSM_TEST_LCD_STATE_NO 0
#define PSM_TEST_LCD_STATE_OP 1
#define PSM_TEST_LCD_STATE_VAL 2
#define PSM_TEST_LCD_STATE_SAVE 3

#define PSM_TEST_MAX_VALUE 110

#define PSM_TEST_VAL_START 2

#define PSM_TEST_OP_SUM 0
#define PSM_TEST_OP_SUBSTRACT 1

#define LCD_ADMIN_DEFAULT_USERNAME 1111
#define LCD_ADMIN_DEFAULT_PASSWORD 1111
/*  Public Methods */

extern void LCDLanguageSet(uint8_t bLang);
extern uint8_t LCDLanguageGet(void);
extern uint8_t LCDLanguageWrite(void);
extern uint8_t LCDLanguageRead(void);

extern void SetLCDPowerRelayRequest(uint8_t bState);
extern uint8_t GetLCDPowerRelayRequest(void);
extern uint8_t GetLCDPowerRelay(void);
extern void SetLCDPowerRelay(uint8_t bState);
extern void SetLCDState(uint8_t bState);
extern uint8_t GetLCDState(void);
extern void ShowMenuPage(uint8_t bPage);
extern void KeyOpeningScreen(void);
extern void ConnectionsScreen(void);
extern void StatusScreen(void);
extern void EmergencyScreen(void);
extern void DemandsScreen(void);
extern void LCD_RemoteConfigMenu(void);
extern void LCD_RemoteConfigMenuKeyScan(void);
extern void LCD_RemoteConfigMenuIPandPortConfig(void);
extern void LCD_RemoteConfigMenuIPandPortConfigKeyScan(void);
extern void LCD_RemoteConfigMenuSaveScreenKeyScan(void);
extern uint8_t PSMTestModuleNoGet(void);
extern uint8_t PSMTestOperationGet(uint8_t bPSMNo);
extern uint8_t PSMTestValueGet(uint8_t bPSMNo);
extern void PSMTestMenuInit(void);
extern void PSMTestMenuShow(uint8_t bPSMNo);
extern void KeyPSMTestMenu(void);

/* GPS Settings */
extern void PageGPSSettingsInit(void);
extern void PageGPSSettingsPortConfigInit(void);
extern void PageGPSSettingsBaudRateConfigInit(void);

/* User Accounts */
extern void PageUserAccountsInit(void);
extern void PageUserAccountsChangePasswordInit(void);

/* Remote Connection */
extern void PageRemoteConnectionInit(void);
extern void RemoteConnectionKeyScan(void);

/* Settings Menu */
extern void ShowSettingsMenuPage(uint8_t bPage);

/* Heater & Lamp Dimming */
/* H&D Commented */

/*
 *  extern  void  HeaterAndLampDimmingScreen(void);
 */

/* Security Settings */
extern void UserSettingsScreen(void);

/* Broken Input Settings */
extern void BrokenInputSettingsScreen(void);

/* Server Settings */
extern void ServerSettingsScreen(void);

/* MCS Connection Settings */
extern void MCSConnectionSettingsMenuPage(uint8_t bKey);
extern void MCSConnectionSettingsMenuKeyScan(void);
extern void MCSConnectionSettingShowAdjacentPage(uint8_t fNext);
extern void MCSConnectionSettingShowNextPage(uint8_t fNext);
extern void MCSConnectionSettingSetModemType(uint8_t bKey);
extern void LRLFDetectTimeMenuStateInit(void);

/* Opening Screen */
extern void OpeningScreenFirstLine(char *pStrPtr);
extern void OpeningScreenSecondLine(char *pStrPtr);
extern void OpeningScreenThirdLine(char *pStrPtr);
extern void OpeningScreenFourthLine(char *pStrPtr);

/* SO Measurements */
extern void LCDSOMeasurements(uint8_t bSSMNo, tpSMCSLCDStream pSScreen);

#endif /* ifndef _LCD */
