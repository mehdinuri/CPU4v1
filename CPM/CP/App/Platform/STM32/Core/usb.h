/**
 ******************************************************************************
 * @file           : usb.h
 * @brief          : Header for usb.c file.
 *                   This file contains the common defines for usb packet
 * transformation.
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_H__
#define __USB_H__

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

#include "ui.h"
#include "usb_device.h"
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
  uint8_t baData[UI_COMM_MAX_PACKET_LENGTH + 1];
  uint16_t sIndex;
  uint16_t sTotalBytes;

  struct
  {
    uint8_t fBusy : 1;
    uint8_t fCheckStarter : 1;
    uint8_t fCheckTerminator : 1;
    uint8_t fReserved : 5;
  } SFlags;
} tSUSBData, *tpSUSBData;

/* Public function prototypes -----------------------------------------------*/
extern void USBReceiveByte(uint8_t bRxByte);
extern void USBStartRx(void);
extern void USBSend(void);
extern void USBStartTx(uint8_t *pbData, uint16_t sDataLen);
extern void USBStop(void);
extern void USBDeInit(void);

#endif /* __USB_H__ */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
