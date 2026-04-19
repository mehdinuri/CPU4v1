/* App/Adapters/STM32/FieldCanQueueTx.c */
#include "FieldCanQueueTx.h"

#include <string.h>

#include "cmsis_os2.h"
#include "defs.h"
#include "fdcan.h"

static uint32_t EncodeTxDataLength(uint8_t length)
{
  if (length <= 8U)
  {
    return FDCAN_DLC_BYTES_0 | length;
  }

  if (length <= 12U)
  {
    return FDCAN_DLC_BYTES_12;
  }

  if (length <= 16U)
  {
    return FDCAN_DLC_BYTES_16;
  }

  if (length <= 20U)
  {
    return FDCAN_DLC_BYTES_20;
  }

  if (length <= 24U)
  {
    return FDCAN_DLC_BYTES_24;
  }

  if (length <= 32U)
  {
    return FDCAN_DLC_BYTES_32;
  }

  if (length <= 48U)
  {
    return FDCAN_DLC_BYTES_48;
  }

  if (length <= 64U)
  {
    return FDCAN_DLC_BYTES_64;
  }

  return FDCAN_DLC_BYTES_0;
}

uint8_t FieldCanQueueTxSendStandard(uint16_t identifier,
                                    const uint8_t *data,
                                    uint8_t length)
{
  tpSFDCANTxMsg message;

  if ((length > FDCAN_DATA_MAX_LEN) || ((data == NULL) && (length != 0U)))
  {
    return 0U;
  }

  message = (tpSFDCANTxMsg) osMemoryPoolAlloc(FDCANTxReqsMemPoolHandle, 0U);
  if (message == NULL)
  {
    return 0U;
  }

  (void) memset(message, 0, sizeof(*message));
  message->hfdcan = &hfdcan1;
  message->TxHeader.IdType = FDCAN_STANDARD_ID;
  message->TxHeader.Identifier = (uint32_t) identifier;
  message->TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
  message->TxHeader.DataLength = EncodeTxDataLength(length);
  message->TxHeader.TxFrameType = FDCAN_DATA_FRAME;
  message->TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
  message->TxHeader.MessageMarker = 0U;
  message->TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
  message->TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;

  if ((data != NULL) && (length != 0U))
  {
    (void) memcpy(message->Data, data, length);
  }

  if (osMessageQueuePut(FDCANTxReqsQueHandle, &message, 0U, 0U) != osOK)
  {
    (void) osMemoryPoolFree(FDCANTxReqsMemPoolHandle, message);
    return 0U;
  }

  return 1U;
}
