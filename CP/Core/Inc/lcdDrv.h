#ifndef _LCD_DRV
#define _LCD_DRV

/* ///////////////////////////////////////////////////////////////////////////////////////////////// */
/*  include files */
#include "defs.h"
#include "gpio.h"

/* Ports */
#define LCDResetPinInit() GPIOLCDResetPinInit()
#define LCDResetLow() GPIOLCDResetPinStateSet(FALSE)
#define LCDResetHigh() GPIOLCDResetPinStateSet(TRUE)

#define LCDA0PinInit() GPIOLCDCS1BPinInit()

#define WriteLCDDataBus(bData) GPIOLCDDataPinsSet(bData)
#define SetLCDBusPinModes() GPIOLCDDataPinsInit()

#define LCDE1PinInit() GPIOLCDE1PinInit()
#define ClearLCDE1() GPIOLCDE1PinStateSet(FALSE)
#define SetLCDE1() GPIOLCDE1PinStateSet(TRUE)

#define LCDE2PinInit() GPIOLCDE2PinInit()
#define ClearLCDE2() GPIOLCDE2PinStateSet(FALSE)
#define SetLCDE2() GPIOLCDE2PinStateSet(TRUE)

#define LCDWriteMode() GPIOLCDWritePinStateSet(FALSE)
#define LCDReadMode() GPIOLCDWritePinStateSet(TRUE)

#define AddressLCDControl() GPIOLCDCS1BPinStateSet(FALSE)
#define AddressLCDData() GPIOLCDCS1BPinStateSet(TRUE)

#define LCDWrPinInit() GPIOLCDWritePinInit()

#define LCDPowerPinInit() GPIOLCDPowerPinInit()
#define TurnOffLCD() GPIOLCDPowerPinStateSet(FALSE)
#define TurnOnLCD() GPIOLCDPowerPinStateSet(TRUE)
#define LCDIsOn() (GPIOLCDPowerPinStateGet() == GPIO_PIN_SET)
#define LCDIsOff() (GPIOLCDPowerPinStateGet() == GPIO_PIN_RESET)

/* lcd hardware */
#define LCD_NOT_RESPONDING 0xff

#define LCD_DISPLAY_HEIGHT 32
#define LCD_DISPLAY_WIDTH 122

#define LCD_CONTROLLER0 0
#define LCD_CONTROLLER1 1
#define LCD_CONTROLLER0_LAST_COLUMN 60
#define LCD_CONTROLLER1_START_PAGE 2
#define LCD_CONTROLLER1_START_COLUMN 61
#define LCD_CONTROLLER1_LAST_COLUMN 121
#define LCD_LAST_PAGE_NO 3
#define LCD_LAST_COLUMN_NO 121

#define LCD_WRITE 0
#define LCD_READ 1

#define LCD_ADDR_CONTROL 0
#define LCD_ADDR_DATA 1

#define LCD_DISPLAY_OFF 0xAE
#define LCD_DISPLAY_ON 0xAF
#define LCD_SET_START_LINE 0xC0
#define LCD_SET_PAGE_ADDRESS 0xB8
#define LCD_SET_COLUMN_ADDRESS 0x00
#define LCD_SELECT_ADC_CW 0xA0
#define LCD_SELECT_ADC_CCW 0xA1
#define LCD_STATIC_DRIVE_OFF 0xA4 /* display is off */
#define LCD_STATIC_DRIVE_ON 0xA5 /* display is on and select all common outputs */
#define LCD_DUTY_16 0xA8 /* duty is 1/16 for the controller */
#define LCD_DUTY_32 0xA9 /* duty is 1/32 for the controller */
#define LCD_READ_MODIFY_WRITE_START 0xE0
#define LCD_READ_MODIFY_WRITE_END 0xEE
#define LCD_SOFTWARE_RESET 0xE2

/* UI */
#define LCD_CHAR_WIDTH 6
#define LCD_CHAR_HEIGHT 8
#define LCD_LINE_SIZE 20 /* in terms of characters */

/* Turkish Chars */
#define _CH 0xc7
#define _GH 0xd0
#define _IE 0xdd
#define _OE 0xd6
#define _SH 0xde
#define _UE 0xdc
#define _ch 0xe7
#define _gh 0xd1
#define _ia 0xfd
#define _oe 0xd7
#define _sh 0xdf
#define _ue 0xdb

/* public methods */
void LCDWait(void);
void ResetLCD(void);

void WriteLCDCommand(uint8_t, /* controller no */
                     uint8_t, /* command */
                     uint8_t); /* parameter */
void WriteLCDData(uint8_t, /* page */
                  uint8_t, /* column */
                  uint8_t, /* length */
                  uint8_t *); /* write buffer */
void ClearLine(uint8_t); /* line no */
void LCDWriteChar(uint8_t, /* line no */
                  uint8_t *, /* column no */
                  char); /* character */

void InitLCD(void);
void OpenLCD(void);
void LCDClose(void);
void HardwareSetupLCD(void);

#endif /* ifndef _LCD_DRV */
