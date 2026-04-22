/* App/Platform/STM32/Tasks/FieldCanTxTask.c */
#include "FieldCanTxTask.h"

#include "cmsis_os2.h"
#include "defs.h"
#include "fdcan.h"

void FieldCanTxTaskFunc(void *argument)
{
  tpSFDCANTxMsg message = NULL;

  (void) argument;
  CANStart(&hfdcan1);

  for (;;)
  {
    if (osMessageQueueGet(FDCANTxReqsQueHandle,
                          &message,
                          NULL,
                          osWaitForever) != osOK)
    {
      continue;
    }

    if (message == NULL)
    {
      continue;
    }

    CANSendMessage(message);
    CANWaitTransmissionComplete(message->hfdcan);
    (void) osMemoryPoolFree(FDCANTxReqsMemPoolHandle, message);
  }
}
