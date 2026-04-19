/* App/Adapters/STM32/CommsStatusAdapter.c */
#include "CommsStatusAdapter.h"

#include <string.h>

#include "MCS.h"

static void CopyAscii(char *target, uint16_t targetLength, const char *source)
{
  uint16_t index;

  if ((target == NULL) || (targetLength == 0U))
  {
    return;
  }

  (void) memset(target, 0, targetLength);
  if (source == NULL)
  {
    return;
  }

  for (index = 0U; index < (uint16_t) (targetLength - 1U); index++)
  {
    if (source[index] == '\0')
    {
      break;
    }

    target[index] = source[index];
  }
}

static uint8_t ReadSnapshot(void *ctx, CommsStatusSnapshot_t *snapshot)
{
  char jobBuffer[UI_COMMS_JOB_TEXT_MAX_LEN + 1U];
  uint8_t jobIndex;

  (void) ctx;

  if (snapshot == NULL)
  {
    return 0U;
  }

  (void) memset(snapshot, 0, sizeof(*snapshot));
  snapshot->modemType = MCSGetModemType();
  snapshot->gprsState = MCSGetGPRSState();
  snapshot->signalQuality = MCSGetGprsSignalQuality();
  snapshot->connected = MCSGetConnected();
  snapshot->modemAlive = MCSGetModemAlive();
  snapshot->simReady = MCSSimStatusGet();
  CopyAscii(&snapshot->imei[0],
            (uint16_t) sizeof(snapshot->imei),
            MCSGetGprsModemIMEI());
  CopyAscii(&snapshot->usrMac[0],
            (uint16_t) sizeof(snapshot->usrMac),
            MCSGetUSRModuleMAC());
  CopyAscii(&snapshot->ethernetMac[0],
            (uint16_t) sizeof(snapshot->ethernetMac),
            MCSGetRuntimeEthernetMAC());
  CopyAscii(&snapshot->operatorName[0],
            (uint16_t) sizeof(snapshot->operatorName),
            MCSGetGprsGsmOperator());
  CopyAscii(&snapshot->localIp[0],
            (uint16_t) sizeof(snapshot->localIp),
            MCSGetRuntimeLocalIPv4());
  CopyAscii(&snapshot->remoteIp[0],
            (uint16_t) sizeof(snapshot->remoteIp),
            MCSGetRuntimeRemoteIPv4());

  for (jobIndex = 0U; jobIndex < UI_COMMS_JOB_COUNT; jobIndex++)
  {
    (void) memset(&jobBuffer[0], 0, sizeof(jobBuffer));
    if (MCSJobCurrentGet(&jobBuffer[0], jobIndex) != 0U)
    {
      CopyAscii(&snapshot->jobCurrent[jobIndex][0],
                (uint16_t) sizeof(snapshot->jobCurrent[jobIndex]),
                &jobBuffer[0]);
    }
  }

  return 1U;
}

void CommsStatusAdapterInit(CommsStatusAdapterCtx_t *ctx)
{
  if (ctx != NULL)
  {
    (void) memset(ctx, 0, sizeof(*ctx));
  }
}

ICommsStatusPort_t CommsStatusAdapterCreatePort(CommsStatusAdapterCtx_t *ctx)
{
  ICommsStatusPort_t port;

  port.ctx = ctx;
  port.ReadSnapshot = ReadSnapshot;
  return port;
}
