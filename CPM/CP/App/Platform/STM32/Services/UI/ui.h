#ifndef _UI
#define _UI

#include <stdint.h>

#include "Ports/ISerialPort.h"

#define UI_FRAME_MAX_SIZE 255U

#define UI_REQ_TYPE_SERIAL 0x01U
#define UI_REQ_TYPE_USB    0x02U

typedef struct _tSUIRequest
{
  uint8_t bReqId;
  uint16_t sDataSize;
  uint8_t baData[UI_FRAME_MAX_SIZE];
} tSUIRequest, *tpSUIRequest;

void UIInit(ISerialPort_t *port);
void UIRxRequest(uint8_t bReqId, const uint8_t *pData, uint16_t sDataSize);
void UITxRequest(uint8_t bReqId, const uint8_t *pData, uint16_t sDataSize);

#endif /* _UI */
