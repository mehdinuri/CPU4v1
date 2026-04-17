#include "signalOutputCatch.h"
#include <string.h>
#include "CANRxTx.h"
#include "signalCheck.h"
#include "fdcan.h"
#include "maintenance.h"
/* ///////////////////////////////////////////////////////// */
/* Definitions */
#define SSM_MODULE_RESPONSE_DURATION 15 /* 150ms */
#define PSM_MODULE_RESPONSE_DURATION 100  /* 1s */

typedef struct _tSModuleActiveness
{
  uint32_t lActiveDevices;
  uint32_t lCurrentActiveDevices;
  uint32_t lPrevActiveDevices;

  uint8_t bSSMEventCntr[SIG_DEV_SSM_MAX];
  uint8_t bPSMEventCntr[SIG_DEV_PSM_MAX];
} tSModuleActiveness, *tpSModuleActiveness;

/* ///////////////////////////////////////////////////////// */
/*                    Private Data */
static tSModuleActiveness SModuleActiveness;
static uint32_t lfActiveDevices;

/* ///////////////////////////////////////////////////////// */
/*                    Private Methods */
static void SignalOutputCatchInit(void)
{
  memset(&SModuleActiveness, 0, sizeof(SModuleActiveness));
  lfActiveDevices = 0;
}

static uint8_t IsSSMValid(uint8_t bSSMNo)
{
  uint8_t bIndex;

  /* only valid (defined by signal program) SSMs will be evaluated */
  for (bIndex = 0; (bIndex < SIGNAL_OUTPUTS_MAX) ; bIndex++)
  {
    if (GetSOOwner(bIndex) && ((bIndex / SIGNAL_OUTPUTS_PER_SSM) == bSSMNo))
    {
      return TRUE;
    }
  }

  return FALSE;
}

static void ModuleIsActive(uint8_t bModuleNumber, uint8_t fState)
{
  if (fState)
  {
    lfActiveDevices |= laValue2Bit[bModuleNumber];
  }
  else
  {
    lfActiveDevices &= ~laValue2Bit[bModuleNumber];
  }
}

static uint8_t ModuleActiveNowGet(uint8_t bModuleNumber)
{
  return (SModuleActiveness.lCurrentActiveDevices
          & laValue2Bit[bModuleNumber]) >> bModuleNumber;
}

static void ModuleActivePrevSet(uint8_t bModuleNumber, uint8_t fState)
{
  if (fState)
  {
    SModuleActiveness.lPrevActiveDevices |= laValue2Bit[bModuleNumber];
  }
  else
  {
    SModuleActiveness.lPrevActiveDevices &= ~(laValue2Bit[bModuleNumber]);
  }
}

static uint8_t ModuleActivePrevGet(uint8_t bModuleNumber)
{
  return (SModuleActiveness.lPrevActiveDevices & laValue2Bit[bModuleNumber]) >>
         bModuleNumber;
}

static void ModuleControl(void)
{
  uint8_t bModuleNo = 0;

  for (bModuleNo = 0; bModuleNo <= SIG_DEV_PSM_LAST; bModuleNo++)
  {
    if (bModuleNo <= SIG_DEV_SSM_LAST)
    {
      if (IsSSMValid(bModuleNo) == TRUE)
      {
        if (ModuleActiveNowGet(bModuleNo))
        {
          SModuleActiveness.bSSMEventCntr[bModuleNo] = 0;

          ModuleIsActive(bModuleNo, TRUE);

          if (!(ModuleActivePrevGet(bModuleNo)))
          {
            ModuleActivePrevSet(bModuleNo, TRUE);

            LogRequest(EVENT_MODULE_RESPONDS, (bModuleNo + SIG_DEV_SSM_1), 0,
                       0);
          }
        }
        else
        {
          SModuleActiveness.bSSMEventCntr[bModuleNo]++;

          if (SModuleActiveness.bSSMEventCntr[bModuleNo]
              >= SSM_MODULE_RESPONSE_DURATION)
          {
            SModuleActiveness.bSSMEventCntr[bModuleNo] = 0;

            ModuleIsActive(bModuleNo, FALSE);

            LogRequest(EVENT_MODULE_MISSING,
                       (bModuleNo + SIG_DEV_SSM_1),
                       0,
                       SSM_MODULE_RESPONSE_DURATION * 10);

            ModuleActivePrevSet(bModuleNo, FALSE);
          }
        }

        SOCatchModuleActiveNowSet(bModuleNo, FALSE);
      }
    }
    else
    {
      if (ModuleActiveNowGet(bModuleNo))
      {
        SModuleActiveness.bPSMEventCntr[bModuleNo - SIG_DEV_PSM_FIRST] = 0;

        ModuleIsActive(bModuleNo, TRUE);

        if (!(ModuleActivePrevGet(bModuleNo)))
        {
          ModuleActivePrevSet(bModuleNo, TRUE);

          LogRequest(EVENT_MODULE_RESPONDS, bModuleNo, 0, 0);
        }
      }
      else
      {
        if ((ModuleActivePrevGet(bModuleNo)))
        {
          SModuleActiveness.bPSMEventCntr[bModuleNo - SIG_DEV_PSM_FIRST]++;

          if (SModuleActiveness.bPSMEventCntr[bModuleNo - SIG_DEV_PSM_FIRST]
              >= SSM_MODULE_RESPONSE_DURATION)
          {
            ModuleIsActive(bModuleNo, FALSE);

            LogRequest(EVENT_MODULE_MISSING,
                       bModuleNo,
                       0,
                       SSM_MODULE_RESPONSE_DURATION * 10);

            ModuleActivePrevSet(bModuleNo, FALSE);
          }
        }
      }

      SOCatchModuleActiveNowSet(bModuleNo, FALSE);
    }
  }
} /* ModuleControl */

uint8_t SOCatchIsCardActive(uint8_t bModuleNumber)
{
  return (lfActiveDevices & laValue2Bit[bModuleNumber]) >> bModuleNumber;
}

void SOCatchModuleActiveNowSet(uint8_t bModuleNumber, uint8_t fState)
{
  if (fState)
  {
    SModuleActiveness.lCurrentActiveDevices |= laValue2Bit[bModuleNumber];
  }
  else
  {
    SModuleActiveness.lCurrentActiveDevices &= ~(laValue2Bit[bModuleNumber]);
  }
}

void SOCatchTaskFunc(void const *argument)
{
  UNUSED(argument);

  tpSEvent pSNewEvent = NULL;

  SignalOutputCatchInit();

  while (FOREVER)
  {
    osThreadSuspend(osThreadGetId());

    if (CPCommStateGet())
    {
      ModuleControl();

      SSMVoltCurrentMeasurementsEvaluate();
      PSMVoltMeasurementsEvaluate();

      if (osThreadGetState(SignalCheckTaskHandle) == osThreadBlocked)
      {
        tpSNewMeasurements pSMeasurements =
          (tpSNewMeasurements) osMemoryPoolAlloc(NewMeasurementsMemPoolHandle,
                                                 osWaitForever);

        if (pSMeasurements != NULL)
        {
          memset(pSMeasurements, 0, sizeof(tSNewMeasurements));
          SORuntimeFlagsGet(pSMeasurements->SaSORuntimeFlags);
          SGCurrentsGet(pSMeasurements->SaSGCurrents);

          if (osMessageQueuePut(NewMeasurementsQueueHandle,
                                &pSMeasurements,
                                0,
                                0) != osOK)
          {
            osMemoryPoolFree(NewMeasurementsMemPoolHandle, pSMeasurements);
          }
        }
      }
      else
      {
        Error_Handler();
      }

      if (osMessageQueueGet(LogReqsQueueHandle, &pSNewEvent, NULL, 0) == osOK)
      {
        CanTxRequest(&hfdcan1,
                     FDCAN_EXTENDED_ID,
                     CAN_TX_MP_EXT_ID_EVENT,
                     FDCAN_DATA_FRAME,
                     FDCAN_BRS_OFF,
                     FDCAN_CLASSIC_CAN,
                     (uint8_t *) pSNewEvent,
                     sizeof(tSEvent));
        osMemoryPoolFree(LogReqsMemPoolHandle, pSNewEvent);
      }
    }

    MaintenanceSignalTask(EVENT_FLAGS_SIGNAL_OUTPUT_CATCH_TASK_ACTIVE);
  }
} /* SOCatchTaskFunc */
