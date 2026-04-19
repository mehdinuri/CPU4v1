/* Minimal local-admin transport skeleton.
 *
 * The legacy ASCII protocol and parser are intentionally removed.
 * RX/TX queues, memory pools, and task shells remain so the next
 * transport-neutral protocol can reuse the existing RTOS wiring.
 */
#include "ui.h"

#include <string.h>

#include "cmsis_os.h"
#include "DomainServices.h"
#include "HardwarePorts.h"
#include "defs.h"
#include "usb.h"

#define UI_DEFAULT_BAUDRATE 38400U
#define UI_GPS_PORT_TYPE_INTERNAL 1U
#define UI_DMA_TX_TIMEOUT 1000U

static ISerialPort_t *s_port;

static uint8_t UIAuxSerialAvailable(void)
{
  if ((g_mmiLocalSettingsService.gpsPort == NULL)
      || (IGpsPort_GetPortType(g_mmiLocalSettingsService.gpsPort)
          == UI_GPS_PORT_TYPE_INTERNAL))
  {
    return 1U;
  }

  return 0U;
}

static void UIOnRx(void *arg, const uint8_t *data, uint16_t len)
{
  (void) arg;
  UIRxRequest(UI_REQ_TYPE_SERIAL, data, len);
}

static void UIMsgDispatch(tpSUIRequest pSReq)
{
  (void) pSReq;
  /* Protocol implementation intentionally deferred. */
}

static void UIMsgSend(tpSUIRequest pSReq)
{
  if ((pSReq == NULL) || (pSReq->sDataSize == 0U))
  {
    return;
  }

  switch (pSReq->bReqId)
  {
      case UI_REQ_TYPE_SERIAL:
      {
        if ((s_port != NULL) && (UIAuxSerialAvailable() != 0U))
        {
          (void) SerialSend(s_port,
                            &pSReq->baData[0],
                            pSReq->sDataSize,
                            UI_DMA_TX_TIMEOUT);
        }
        break;
      }

      case UI_REQ_TYPE_USB:
      {
        USBStartTx(&pSReq->baData[0], pSReq->sDataSize);
        break;
      }

      default:
      {
        break;
      }
  }
}

void UIInit(ISerialPort_t *port)
{
  s_port = port;

  if (s_port == NULL)
  {
    return;
  }

  if (UIAuxSerialAvailable() != 0U)
  {
    (void) SerialSetBaudRate(s_port, UI_DEFAULT_BAUDRATE);
    SerialSetRxCallback(s_port, UIOnRx, NULL);
  }
  else
  {
    SerialSetRxCallback(s_port, NULL, NULL);
  }
}

void UIRxRequest(uint8_t bReqId, const uint8_t *pData, uint16_t sDataSize)
{
  tpSUIRequest pSReq;

  if (((bReqId != UI_REQ_TYPE_SERIAL) && (bReqId != UI_REQ_TYPE_USB))
      || (pData == NULL)
      || (sDataSize == 0U)
      || (sDataSize > UI_FRAME_MAX_SIZE))
  {
    return;
  }

  pSReq = (tpSUIRequest) osMemoryPoolAlloc(UIRxReqsMemPoolHandle, 0U);
  if (pSReq == NULL)
  {
    return;
  }

  (void) memset(pSReq, 0, sizeof(*pSReq));
  pSReq->bReqId = bReqId;
  pSReq->sDataSize = sDataSize;
  (void) memcpy(&pSReq->baData[0], pData, sDataSize);

  if (osMessageQueuePut(UIRxReqsQueHandle, &pSReq, 0U, 0U) != osOK)
  {
    (void) osMemoryPoolFree(UIRxReqsMemPoolHandle, pSReq);
  }
}

void UITxRequest(uint8_t bReqId, const uint8_t *pData, uint16_t sDataSize)
{
  tpSUIRequest pSReq;

  if (((bReqId != UI_REQ_TYPE_SERIAL) && (bReqId != UI_REQ_TYPE_USB))
      || (pData == NULL)
      || (sDataSize == 0U)
      || (sDataSize > UI_FRAME_MAX_SIZE))
  {
    return;
  }

  pSReq = (tpSUIRequest) osMemoryPoolAlloc(UITxReqsMemPoolHandle, 0U);
  if (pSReq == NULL)
  {
    return;
  }

  (void) memset(pSReq, 0, sizeof(*pSReq));
  pSReq->bReqId = bReqId;
  pSReq->sDataSize = sDataSize;
  (void) memcpy(&pSReq->baData[0], pData, sDataSize);

  if (osMessageQueuePut(UITxReqsQueHandle, &pSReq, 0U, 0U) != osOK)
  {
    (void) osMemoryPoolFree(UITxReqsMemPoolHandle, pSReq);
  }
}

void UIMsgParserTaskFunc(void *argument)
{
  tpSUIRequest pSReq = NULL;

  (void) argument;

  osDelay(1000U);

  UIInit(&g_auxSerialPort);
  USBStartRx();

  for (;;)
  {
    if (osMessageQueueGet(UIRxReqsQueHandle, &pSReq, NULL,
                          osWaitForever) == osOK)
    {
      UIMsgDispatch(pSReq);
      (void) osMemoryPoolFree(UIRxReqsMemPoolHandle, pSReq);
    }
  }
}

void UIMsgSenderTaskFunc(void *argument)
{
  tpSUIRequest pSReq = NULL;

  (void) argument;

  osDelay(1000U);

  for (;;)
  {
    if (osMessageQueueGet(UITxReqsQueHandle, &pSReq, NULL,
                          osWaitForever) == osOK)
    {
      UIMsgSend(pSReq);
      (void) osMemoryPoolFree(UITxReqsMemPoolHandle, pSReq);
    }
  }
}
