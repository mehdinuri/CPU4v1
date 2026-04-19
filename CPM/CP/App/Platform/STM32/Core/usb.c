/**
 ******************************************************************************
 * File Name          : usb.c
 * Description        : Code for usb read and write
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "usb.h"

#include <string.h>

#include "usbd_custom_hid_if.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static tSUSBData SUSBTransmitter;
static uint8_t baTxBuf[USB_DATA_PACKET_MAX_SIZE];

/* Private function prototypes -----------------------------------------------*/

/* Public typedef -----------------------------------------------------------*/

/* Public define ------------------------------------------------------------*/

/* Public variables ---------------------------------------------------------*/
extern USBD_HandleTypeDef hUsbDeviceFS;

/* Public function prototypes -----------------------------------------------*/

/* Private application code --------------------------------------------------*/

/* Public application code --------------------------------------------------*/
void USBReceiveReport(const uint8_t *pbData, uint16_t sDataLen)
{
  if ((pbData != NULL) && (sDataLen > 0U))
  {
    uint16_t sSafeLength = sDataLen;

    if (sSafeLength > USBD_CUSTOM_HID_REPORT_COUNT)
    {
      sSafeLength = USBD_CUSTOM_HID_REPORT_COUNT;
    }

    UIRxRequest(UI_REQ_TYPE_USB, pbData, sSafeLength);
  }
}

void USBStartRx(void)
{
  /* No state to reset in the raw-report transport skeleton. */
}

void USBSend(void)
{
  if (SUSBTransmitter.sIndex < SUSBTransmitter.sTotalBytes)
  {
    uint16_t sRemainingBytes = SUSBTransmitter.sTotalBytes
                               - SUSBTransmitter.sIndex;

    memset(&baTxBuf, 0, sizeof(baTxBuf));
    baTxBuf[0] = USBD_CUSTOM_HID_CONFIG_TOOL_IN_REPORT_ID;

    if (sRemainingBytes > USB_DATA_PACKET_MAX_SIZE - 1)
    {
      sRemainingBytes = USB_DATA_PACKET_MAX_SIZE - 1;
    }

    memcpy(&baTxBuf[1],
           &SUSBTransmitter.baData[SUSBTransmitter.sIndex],
           sRemainingBytes);
    SUSBTransmitter.sIndex += sRemainingBytes;
    USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, baTxBuf, sizeof(baTxBuf));
  }
}

void USBStartTx(const uint8_t *pbData, uint16_t sDataLen)
{
  if ((pbData == NULL) || (sDataLen == 0U))
  {
    return;
  }

  if (sDataLen > sizeof(SUSBTransmitter.baData))
  {
    sDataLen = sizeof(SUSBTransmitter.baData);
  }

  memset(&SUSBTransmitter.baData[0], 0, sizeof(SUSBTransmitter.baData));
  memcpy(&SUSBTransmitter.baData[0], pbData, sDataLen);

  SUSBTransmitter.sTotalBytes = sDataLen;
  SUSBTransmitter.sIndex = 0;

  USBSend();
}

void USBStop(void)
{
  USBD_Stop(&hUsbDeviceFS);
}

void USBDeInit(void)
{
  USBStop();

  USBD_DeInit(&hUsbDeviceFS);
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
