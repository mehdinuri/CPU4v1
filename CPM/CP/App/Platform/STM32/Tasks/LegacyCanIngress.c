/* App/Platform/STM32/Tasks/LegacyCanIngress.c */
#include "LegacyCanIngress.h"

#include <string.h>

#include "cmsis_os2.h"
#include "defs.h"

uint8_t LegacyCanIngressDlcToLength(uint32_t dlc)
{
  if (dlc <= FDCAN_DLC_BYTES_8)
  {
    return (uint8_t) (FDCAN_DLC_BYTES_0 | dlc);
  }

  if (dlc <= FDCAN_DLC_BYTES_12)
  {
    return 12U;
  }

  if (dlc <= FDCAN_DLC_BYTES_16)
  {
    return 16U;
  }

  if (dlc <= FDCAN_DLC_BYTES_20)
  {
    return 20U;
  }

  if (dlc <= FDCAN_DLC_BYTES_24)
  {
    return 24U;
  }

  if (dlc <= FDCAN_DLC_BYTES_32)
  {
    return 32U;
  }

  if (dlc <= FDCAN_DLC_BYTES_48)
  {
    return 48U;
  }

  return 64U;
}

void LegacyCanIngressOnRxFrame(tpSFDCANRxMsg rxMsg)
{
  tpSFDCANRxMsg queuedFrame;

  if (rxMsg == NULL)
  {
    return;
  }

  queuedFrame = (tpSFDCANRxMsg) osMemoryPoolAlloc(FDCANRxReqsMemPoolHandle, 0U);
  if (queuedFrame == NULL)
  {
    return;
  }

  (void) memcpy(queuedFrame, rxMsg, sizeof(*queuedFrame));
  if (osMessageQueuePut(FDCANRxReqsQueHandle, &queuedFrame, 0U, 0U) != osOK)
  {
    (void) osMemoryPoolFree(FDCANRxReqsMemPoolHandle, queuedFrame);
  }
}
