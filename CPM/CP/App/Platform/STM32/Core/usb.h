/**
 ******************************************************************************
 * @file           : usb.h
 * @brief          : Header for usb.c file.
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_H__
#define __USB_H__

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

#include "usb_device.h"
#include "ui.h"
#include "usbd_custom_hid_if.h"
/* Private define ------------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/

/* Private types -------------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Public define ------------------------------------------------------------*/
#define USB_DATA_PACKET_MAX_SIZE 64

/* Public macros ------------------------------------------------------------*/

/* Public types -------------------------------------------------------------*/
typedef struct _tSUSBData
{
  uint8_t baData[UI_FRAME_MAX_SIZE];
  uint16_t sIndex;
  uint16_t sTotalBytes;
} tSUSBData, *tpSUSBData;

/* Public function prototypes -----------------------------------------------*/
extern void USBReceiveReport(const uint8_t *pbData, uint16_t sDataLen);
extern void USBStartRx(void);
extern void USBSend(void);
extern void USBStartTx(const uint8_t *pbData, uint16_t sDataLen);
extern void USBStop(void);
extern void USBDeInit(void);

#endif /* __USB_H__ */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
